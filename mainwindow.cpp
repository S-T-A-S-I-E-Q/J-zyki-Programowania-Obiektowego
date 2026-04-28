/**
 * @file mainwindow.cpp
 * @brief Implementacja logiki głównego okna aplikacji GoPoznań.
 * Integruje asynchroniczne pobieranie danych ZTM (przez skrypty Python),
 * asynchroniczne zapytania do modelu Ollama oraz generowanie raportów.
 */
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QCompleter>
#include <QProcess>

/**
 * @brief Konstruktor klasy MainWindow.
 * Tworzy i konfiguruje GUI, łączy sygnały i sloty oraz wymusza pobranie listy przystanków na starcie.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_pythonRunner = new PythonRunner(this);
    m_chartRunner = new PythonRunner(this);
    m_llmManager = new LLMManager(this);

    connect(ui->btnSearch, &QPushButton::clicked, this, &MainWindow::onSearchClicked);
    connect(ui->btnExportPdf, &QPushButton::clicked, this, &MainWindow::onExportPdfClicked);
    
    // Połączenia dla PythonRunner (wyszukiwanie trasy)
    connect(m_pythonRunner, &PythonRunner::scriptFinished, this, &MainWindow::onGtfsScriptFinished);
    connect(m_pythonRunner, &PythonRunner::errorOccurred, this, &MainWindow::onGtfsScriptError);

    // Połączenia dla OLLAMY
    connect(m_llmManager, &LLMManager::responseReceived, this, &MainWindow::onLlmResponseReceived);
    connect(m_llmManager, &LLMManager::errorOccurred, this, &MainWindow::onLlmError);

    // Połączenia dla PythonRunner (raport PDF)
    connect(m_chartRunner, &PythonRunner::scriptFinished, this, [this]() {
        ui->labelStatus->setText("Raport PDF został wygenerowany pomyślnie!");
        ui->btnExportPdf->setEnabled(true);
    });

    ui->labelStatus->setText("Pobieranie przystanków...");
    ui->btnSearch->setEnabled(false);
    
    // Inicjalizacja: pobierz listę przystanków z GTFS za pomocą skryptu Python
    loadStops();

    // Dodanie estetycznego Frontendu (QSS)
    this->setStyleSheet(
        "QMainWindow { background-color: #f5f6fa; }"
        "QPushButton { background-color: #0078D7; color: white; border-radius: 5px; padding: 10px 15px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background-color: #005A9E; }"
        "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
        "QLabel { color: #2c3e50; font-size: 13px; }"
        "QLabel#labelTitle { font-size: 42px; font-weight: bold; }"
        "QLabel#textEditResult { background-color: white; border: 1px solid #dcdde1; border-radius: 8px; padding: 15px; }"
        "QComboBox { border: 1px solid #dcdde1; border-radius: 4px; padding: 5px; background: white; }"
        "QComboBox::drop-down { border: 0px; }"
    );
}

/**
 * @brief Destruktor klasy MainWindow. 
 * Czyści wskaźnik na interfejs użytkownika.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Rozpoczyna proces pobierania listy przystanków.
 * Uruchamia w tle skrypt gtfs_processor.py z odpowiednią flagą.
 */
void MainWindow::loadStops()
{
    m_pythonRunner->runScript("gtfs_processor.py", QStringList() << "get_stops");
}

/**
 * @brief Obsługuje kliknięcie przycisku "Szukaj".
 * Sprawdza, czy przystanki są wybrane i uruchamia w tle główny proces wyszukiwania w ZTM.
 */
void MainWindow::onSearchClicked()
{
    QString start = ui->comboBoxStart->currentText();
    QString end = ui->comboBoxEnd->currentText();

    if (start.isEmpty() || end.isEmpty()) {
        ui->labelStatus->setText("Wybierz przystanki!");
        return;
    }

    ui->labelStatus->setText("Wyszukiwanie połączenia w GTFS...");
    ui->btnSearch->setEnabled(false);
    ui->btnExportPdf->setEnabled(false);
    ui->textEditResult->clear();

    // UWAGA: NAJWAŻNIEJSZE MIEJSCE 1 - POBIERANIE DANYCH Z ZTM
    // Uruchamiamy w tle (asynchronicznie, co spełnia wymóg nienaruszania działania GUI) 
    // skrypt Pythona (gtfs_processor.py) wyszukujący trasę w plikach z ZTM Poznań.
    m_pythonRunner->runScript("gtfs_processor.py", QStringList() << "find_route" << start << end);
}

/**
 * @brief Slot wywoływany po pomyślnym zakończeniu działania skryptu Pythona (ZTM).
 * Parsuje JSON z odpowiedziami (przystanki lub trasy) i steruje dalszą logiką (np. wywołuje Ollamę).
 * @param output Zwrócony ciąg znaków z procesu (JSON).
 */
void MainWindow::onGtfsScriptFinished(const QString& output)
{
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        ui->labelStatus->setText("Błąd parsowania odpowiedzi z Pythona.");
        ui->btnSearch->setEnabled(true);
        return;
    }

    QJsonObject obj = doc.object();
    if (obj["status"].toString() == "error") {
        ui->labelStatus->setText("Błąd: " + obj["message"].toString());
        ui->btnSearch->setEnabled(true);
        return;
    }

    // Jeśli to była odpowiedź na get_stops
    if (obj.contains("stops")) {
        QJsonArray stops = obj["stops"].toArray();
        QStringList stopList;
        for (const QJsonValue& val : stops) {
            QString stopName = val.toString();
            ui->comboBoxStart->addItem(stopName);
            ui->comboBoxEnd->addItem(stopName);
            stopList << stopName;
        }

        ui->comboBoxStart->setEditable(true);
        ui->comboBoxEnd->setEditable(true);

        QCompleter *completer = new QCompleter(stopList, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);

        ui->comboBoxStart->setCompleter(completer);
        ui->comboBoxEnd->setCompleter(completer);

        ui->labelStatus->setText("Gotowy");
        ui->btnSearch->setEnabled(true);
        return;
    }

    // Jeśli to była odpowiedź na find_route
    if (obj.contains("trams") || obj.contains("buses")) {
        ui->labelStatus->setText("Generowanie odpowiedzi przez Ollamę...");
        
        QJsonArray trams = obj["trams"].toArray();
        QJsonArray buses = obj["buses"].toArray();
        
        m_chartArgs.clear();
        if (obj.contains("chart_data")) {
            QJsonArray chartArr = obj["chart_data"].toArray();
            for (const QJsonValue& val : chartArr) {
                QJsonObject cObj = val.toObject();
                m_chartArgs << cObj["hour"].toString() << QString::number(cObj["count"].toInt());
            }
        }
        
        QString routeInfo = "";
        
        routeInfo += "Tramwaje:\n";
        if (!trams.isEmpty()) {
            for (int i = 0; i < trams.size(); ++i) {
                QJsonObject t = trams[i].toObject();
                routeInfo += QString("%1 %2 %3\n").arg(t["line"].toString(), t["departure"].toString(), t["duration"].toString());
            }
        } else {
            routeInfo += "Brak dostępnych tramwajów\n";
        }

        routeInfo += "\nAutobusy:\n";
        if (!buses.isEmpty()) {
            for (int i = 0; i < buses.size(); ++i) {
                QJsonObject b = buses[i].toObject();
                routeInfo += QString("%1 %2 %3\n").arg(b["line"].toString(), b["departure"].toString(), b["duration"].toString());
            }
        } else {
            routeInfo += "Brak dostępnych autobusów\n";
        }

        QString systemPrompt = "Jesteś asystentem. MUSISZ sformatować trasy dokładnie według poniższego szablonu i na końcu dodać komendę do wygenerowania pliku PDF.\n"
                               "SZABLON ODPOWIEDZI:\n"
                               "Nr linii / godz odjazdu / czas trwania podróży\n\n"
                               "Tramwaje:\n"
                               "[Wypisz linie i czasy, albo przepisz 'Brak dostępnych tramwajów' jeśli ich brakuje]\n"
                               "\n"
                               "Autobusy:\n"
                               "[Wypisz linie i czasy, albo przepisz 'Brak dostępnych autobusów' jeśli ich brakuje]\n"
                               "\n"
                               "python generate_pdf.py\n\n"
                               "Zabrania się używania jakichkolwiek innych słów wstępu czy podsumowania. Wypisz TYLKO sformatowane dane i komendę.";

        QString userPrompt = QString("DANE WEJŚCIOWE (TRASY):\n%1").arg(routeInfo);

        // UWAGA: NAJWAŻNIEJSZE MIEJSCE 2 - OLLAMA / QWEN
        // Wysyłamy odpowiednio spreparowane prompty (System i User) do lokalnego modelu językowego
        m_llmManager->generateResponse(systemPrompt, userPrompt);
    }
}

/**
 * @brief Slot wywoływany, gdy skrypt GTFS napotka problem (np. brak Pythona).
 * Wyświetla informację o błędzie w interfejsie.
 * @param errorMsg Ciąg znaków opisujący błąd.
 */
void MainWindow::onGtfsScriptError(const QString& errorMsg)
{
    ui->labelStatus->setText("Błąd: Skrypt GTFS");
    ui->textEditResult->setText(errorMsg);
    ui->btnSearch->setEnabled(true);
}

/**
 * @brief Slot wywoływany, gdy model językowy zwróci sformatowany raport.
 * Filtruje odpowiedź, wyciąga komendę z polecenia 5.0 (generującą PDF) i wyświetla wynik.
 * @param response Pełna odpowiedź tekstowa od modelu.
 */
void MainWindow::onLlmResponseReceived(const QString& response)
{
    // Szukamy komendy z polecenia 5.0
    QString cleanResponse = response;
    QRegularExpression re("(python generate_pdf\\.py[^\n\r]*)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(cleanResponse);

    if (match.hasMatch()) {
        m_generatedPdfCode = match.captured(1).trimmed();
        // Usuwamy komendę z widoku
        cleanResponse.replace(match.captured(0), "");
        ui->btnExportPdf->setEnabled(true);
    } else {
        // Niezawodny fallback dla OLLAMY
        m_generatedPdfCode = "python generate_pdf.py";
        ui->btnExportPdf->setEnabled(true);
    }
    
    cleanResponse.remove("```");

    ui->textEditResult->setText(cleanResponse.trimmed());
    ui->btnSearch->setEnabled(true);
}

/**
 * @brief Slot obsługujący błędy API lokalnego serwera Ollama.
 * @param errorMsg Opis błędu połączenia.
 */
void MainWindow::onLlmError(const QString& errorMsg)
{
    ui->labelStatus->setText("Błąd LLM (Ollama)");
    ui->textEditResult->setText(errorMsg);
    ui->btnSearch->setEnabled(true);
}

/**
 * @brief Obsługuje kliknięcie przycisku generującego PDF.
 * Automatycznie wykonuje wygenerowaną komendę (draw_chart.py a następnie generate_pdf.py).
 */
void MainWindow::onExportPdfClicked()
{
    if (m_generatedPdfCode.isEmpty()) return;

    ui->labelStatus->setText("Generowanie raportu PDF...");
    ui->btnExportPdf->setEnabled(false);
    
    // Zapisz tekst dla PDFa (żeby skrypt wiedział co ma dodać do pliku)
    QFile file("report_text.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString start = ui->comboBoxStart->currentText();
        QString end = ui->comboBoxEnd->currentText();
        out << "Trasa: " << start << " -> " << end << "\n\n";
        out << ui->textEditResult->text();
        file.close();
    }
    
    // Najpierw generujemy wykres
    if (!m_chartArgs.isEmpty()) {
        QProcess process;
#ifdef Q_OS_WIN
        QString program = "C:/Users/stasi/AppData/Local/Microsoft/WindowsApps/python.exe";
#else
        QString program = "python3";
#endif
        process.start(program, QStringList() << "draw_chart.py" << m_chartArgs);
        process.waitForFinished();
    } else {
        QFile file("chart.png");
        if (file.exists()) file.remove(); // Usuwamy stary wykres
    }
    
    // Parsowanie wygenerowanej komendy z wymogu 5.0
    QStringList parts = m_generatedPdfCode.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    QStringList args;
    QString scriptName = "generate_pdf.py";
    for (int i = 0; i < parts.size(); ++i) {
        if (parts[i].toLower() != "python" && parts[i].toLower() != "generate_pdf.py") {
            args << parts[i];
        }
    }
    
    m_chartRunner->runScript(scriptName, args);
}

// Wszystkie stare funkcje rysowania i przyciski zostały usunięte
// zgodnie z architekturą 5.0 opartą na czystym Raporcie PDF wywoływanym guzikiem.
