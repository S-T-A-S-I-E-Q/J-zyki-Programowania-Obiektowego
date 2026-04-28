#ifndef PYTHONRUNNER_H
#define PYTHONRUNNER_H

#include <QObject>
#include <QProcess>
#include <QStringList>

/**
 * @brief Klasa odpowiedzialna za uruchamianie skryptów w języku Python.
 * 
 * Pozwala na wywoływanie skryptów Pythona jako podprocesów w tle bez blokowania wątku głównego.
 */
class PythonRunner : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy PythonRunner.
     * @param parent Obiekt nadrzędny.
     */
    explicit PythonRunner(QObject *parent = nullptr);

    /**
     * @brief Uruchamia podany skrypt Pythona z przekazanymi argumentami.
     * @param scriptName Nazwa pliku skryptu (np. "gtfs_processor.py").
     * @param args Lista argumentów przekazywanych do skryptu.
     */
    void runScript(const QString& scriptName, const QStringList& args);

signals:
    /**
     * @brief Sygnał emitowany po pomyślnym wykonaniu skryptu.
     * @param output Zwrócony przez skrypt standardowy strumień wyjścia (stdout).
     */
    void scriptFinished(const QString& output);

    /**
     * @brief Sygnał emitowany, gdy skrypt zgłosi błąd w trakcie wykonywania.
     * @param errorMsg Treść błędu ze standardowego strumienia błędów (stderr).
     */
    void errorOccurred(const QString& errorMsg);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    QProcess* m_process;
};

#endif // PYTHONRUNNER_H
