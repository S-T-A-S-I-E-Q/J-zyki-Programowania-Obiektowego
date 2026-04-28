#ifndef LLMMANAGER_H
#define LLMMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

/**
 * @brief Klasa odpowiedzialna za komunikację z modelem językowym (LLM) Ollama za pomocą REST API.
 */
class LLMManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy LLMManager.
     * @param parent Obiekt nadrzędny (domyślnie nullptr).
     */
    explicit LLMManager(QObject *parent = nullptr);

    /**
     * @brief Wysyła zapytanie (prompt) do modelu językowego.
     * @param systemPrompt Instrukcja systemowa dla modelu (np. rola).
     * @param userPrompt Właściwe zapytanie użytkownika.
     */
    void generateResponse(const QString& systemPrompt, const QString& userPrompt);

signals:
    /**
     * @brief Sygnał emitowany po pomyślnym otrzymaniu odpowiedzi od modelu.
     * @param response Wygenerowany tekst odpowiedzi.
     */
    void responseReceived(const QString& response);

    /**
     * @brief Sygnał emitowany w przypadku wystąpienia błędu podczas komunikacji z modelem.
     * @param errorMsg Treść błędu.
     */
    void errorOccurred(const QString& errorMsg);

private slots:
    /**
     * @brief Wewnętrzny slot obsługujący odpowiedź z serwera po zakończeniu żądania.
     * @param reply Obiekt odpowiedzi sieciowej.
     */
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_networkManager;
};

#endif // LLMMANAGER_H
