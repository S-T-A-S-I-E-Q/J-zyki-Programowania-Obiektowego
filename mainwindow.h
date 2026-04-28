/**
 * @file mainwindow.h
 * @brief Definicja głównego okna aplikacji (interfejs użytkownika).
 * Łączy skrypty Pythona (PythonRunner) z modelem językowym (LLMManager).
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "pythonrunner.h"
#include "llmmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @brief Klasa głównego okna aplikacji GoPoznań.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSearchClicked();
    void onExportPdfClicked();
    
    // Sloty do obsługi skryptu wyszukującego przystanki i trasę
    void onGtfsScriptFinished(const QString& output);
    void onGtfsScriptError(const QString& errorMsg);

    // Sloty do obsługi Ollamy
    void onLlmResponseReceived(const QString& response);
    void onLlmError(const QString& errorMsg);

private:
    Ui::MainWindow *ui;
    PythonRunner *m_pythonRunner;
    PythonRunner *m_chartRunner;
    LLMManager *m_llmManager;
    QString m_generatedPdfCode;
    QStringList m_chartArgs;
    
    void loadStops();
};
#endif // MAINWINDOW_H
