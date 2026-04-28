/**
 * @file pythonrunner.cpp
 * @brief Implementacja klasy uruchamiającej skrypty Pythona jako podprocesy.
 * 
 * UWAGA: WIELOWĄTKOWOŚĆ / ASYNCHRONICZNOŚĆ
 * Wykorzystany obiekt QProcess wykonuje komendy systemowe w tle, dzięki czemu interfejs 
 * GUI (MainWindow) pozostaje responsywny przez cały czas pobierania i przetwarzania danych.
 */
#include "pythonrunner.h"
#include <QDebug>

/**
 * @brief Konstruktor. Powołuje do życia obiekt QProcess i konfiguruje powiązania jego sygnałów.
 */
PythonRunner::PythonRunner(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PythonRunner::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &PythonRunner::onProcessError);
}

/**
 * @brief Metoda startująca skrypt Pythona w nowym podprocesie.
 * @param scriptName Ścieżka do pliku np. "gtfs_processor.py".
 * @param args Lista parametrów (np. flaga "find_route").
 */
void PythonRunner::runScript(const QString& scriptName, const QStringList& args)
{
    QStringList processArgs;
    processArgs << scriptName << args;
    
    //!!!!!!!!!!!!!!!!!!! Zakładamy, że komenda "python" lub "python3" jest dostępna w zmiennej środowiskowej PATH.
#ifdef Q_OS_WIN
    QString program = "C:/Users/stasi/AppData/Local/Microsoft/WindowsApps/python.exe";
#else
    QString program = "python3";
#endif

    m_process->start(program, processArgs);
}

/**
 * @brief Slot wywoływany w momencie, gdy system operacyjny zakomunikuje zamknięcie procesu Pythona.
 * Analizuje standardowe wyjście (stdout) oraz błędy (stderr).
 * @param exitCode Kod zakończenia (0 = poprawny).
 * @param exitStatus Informacja, czy program zakończył się normalnie, czy z powodu błędu ("Crash").
 */
void PythonRunner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        emit errorOccurred("Skrypt Pythona zakończył działanie z błędem krytycznym (Crash).");
        return;
    }

    QString stdOut = m_process->readAllStandardOutput();
    QString stdErr = m_process->readAllStandardError();

    if (exitCode != 0) {
        emit errorOccurred(QString("Błąd wykonania skryptu (Kod %1):\n%2").arg(exitCode).arg(stdErr));
    } else {
        emit scriptFinished(stdOut);
    }
}

/**
 * @brief Obsługuje i formatuje błędy zgłaszane przez samą próbę uruchomienia podprocesu.
 * @param error Typ napotkanego błędu QProcess::ProcessError.
 */
void PythonRunner::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch(error) {
        case QProcess::FailedToStart:
            errorMsg = "Nie udało się uruchomić procesu Pythona. Upewnij się, że Python jest zainstalowany i dodany do zmiennej PATH.";
            break;
        case QProcess::Crashed:
            errorMsg = "Proces Pythona nieoczekiwanie zakończył działanie.";
            break;
        case QProcess::Timedout:
            errorMsg = "Przekroczono czas oczekiwania na proces Pythona.";
            break;
        default:
            errorMsg = "Wystąpił nieznany błąd procesu Pythona.";
            break;
    }
    emit errorOccurred(errorMsg);
}
