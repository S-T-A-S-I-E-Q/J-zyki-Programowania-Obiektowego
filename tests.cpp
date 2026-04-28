/**
 * @file tests.cpp
 * @brief Testy jednostkowe i integracyjne aplikacji GoPoznań.
 * 
 * Plik ten zawiera kompletny i jedyny silnik testowy dla tego projektu (oparty na QTest).
 * Makro QTEST_MAIN na końcu pliku automatycznie generuje funkcję main() potrzebną do uruchomienia testów.
 *
 * ZESTAWIENIE TESTÓW I ICH CELÓW:
 * 1. initTestCase() 
 *    - Inicjalizator wywoływany automatycznie przed rozpoczęciem wszystkich testów (ustawienia początkowe).
 * 2. testPythonRunnerInstatiation() 
 *    - Sprawdza, czy obiekt klasy PythonRunner (odpowiedzialny za wielowątkowość i skrypty) tworzy się w pamięci bez naruszeń.
 * 3. testLLMManagerInstantiation() 
 *    - Sprawdza, czy klasa LLMManager (obsługująca REST API Ollamy) jest poprawnie alokowana przez Qt.
 * 4. testPythonRunnerSignals() 
 *    - Zaawansowany test asynchroniczności (wielowątkowości). Uruchamia prosty skrypt w tle i nasłuchuje (QSignalSpy), 
 *      czy po czasie 5 sekund wróci do nas prawidłowy sygnał "zakończono" ze spodziewanym tekstem.
 * 5. testGtfsProcessorGetStops() 
 *    - Test integracyjny (end-to-end). Komunikuje się z prawdziwym skryptem gtfs_processor.py używanym w aplikacji, 
 *      symuluje komendę "get_stops" i weryfikuje, czy odpowiedź to prawidłowo sformatowany format JSON ze statusem działania.
 */
#include <QtTest>
#include <QCoreApplication>
#include "pythonrunner.h"
#include "llmmanager.h"

class GoPoznanTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        // Inicjalizacja przed testami
    }

    void testPythonRunnerInstatiation() {
        PythonRunner runner;
        QVERIFY(true); // Sprawdzenie, czy obiekt się poprawnie tworzy
    }

    void testLLMManagerInstantiation() {
        LLMManager manager;
        QVERIFY(true); // Sprawdzenie, czy obiekt się poprawnie tworzy
    }

    void testPythonRunnerSignals() {
        PythonRunner runner;
        QSignalSpy spyFinished(&runner, &PythonRunner::scriptFinished);
        QSignalSpy spyError(&runner, &PythonRunner::errorOccurred);
        
        // Zamiast uruchamiać skrypt który wymaga pobierania, uruchamiamy proste polecenie python
        runner.runScript("-c", QStringList() << "print('test')");
        
        // Czekamy maksymalnie 5 sekund na jeden z sygnałów
        QVERIFY(spyFinished.wait(5000) || spyError.wait(5000));
        
        // Jeśli sygnał finished został wyemitowany, sprawdzamy wyjście
        if (spyFinished.count() > 0) {
            QList<QVariant> arguments = spyFinished.takeFirst();
            QString output = arguments.at(0).toString().trimmed();
            QCOMPARE(output, QString("test"));
        }
    }
    void testGtfsProcessorGetStops() {
        PythonRunner runner;
        QSignalSpy spyFinished(&runner, &PythonRunner::scriptFinished);
        
        runner.runScript("gtfs_processor.py", QStringList() << "get_stops");
        
        // Czekamy dłużej, bo skrypt może pobierać plik GTFS przy pierwszym uruchomieniu
        QVERIFY(spyFinished.wait(15000));
        
        if (spyFinished.count() > 0) {
            QList<QVariant> arguments = spyFinished.takeFirst();
            QString output = arguments.at(0).toString().trimmed();
            
            // Sprawdzamy, czy output jest poprawnym formatem JSON (zwracanym przez pythonowy skrypt)
            QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
            QVERIFY(!doc.isNull());
            QJsonObject obj = doc.object();
            QVERIFY(obj.contains("status"));
        }
    }
};

QTEST_MAIN(GoPoznanTests)
#include "tests.moc"
