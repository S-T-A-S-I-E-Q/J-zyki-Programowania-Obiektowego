/**
 * @file llmmanager.cpp
 * @brief Implementacja komunikacji z modelem LLM (Ollama) przez REST API.
 * 
 * UWAGA: WIELOWĄTKOWOŚĆ / ASYNCHRONICZNOŚĆ
 * Wykorzystany obiekt QNetworkAccessManager operuje asynchronicznie (własne wątki sieciowe).
 * Zapytania HTTP nie blokują głównego okna aplikacji.
 */
#include "llmmanager.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>

/**
 * @brief Konstruktor. Inicjalizuje asynchronicznego menedżera zapytań sieciowych.
 */
LLMManager::LLMManager(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &LLMManager::onReplyFinished);
}

/**
 * @brief Formatuje zapytanie do JSON i wysyła je asynchronicznie do lokalnej Ollamy.
 * @param systemPrompt Wytyczne systemu dla modelu.
 * @param userPrompt Zapytanie użytkownika z listą połączeń.
 */
void LLMManager::generateResponse(const QString& systemPrompt, const QString& userPrompt)
{
    QUrl url("http://localhost:11434/api/generate");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["model"] = "qwen2.5:3b"; // Używamy modelu wskazanego przez użytkownika
    json["system"] = systemPrompt;
    json["prompt"] = userPrompt;
    json["stream"] = false;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    m_networkManager->post(request, data);
}

/**
 * @brief Slot sieciowy nasłuchujący odpowiedzi serwera Ollama.
 * Wyciąga sam tekst wygenerowany przez model i emituje sygnał.
 * @param reply Wskaźnik na sieć/obiekt z odpowiedzią (usuwany automatycznie na końcu).
 */
void LLMManager::onReplyFinished(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObj = jsonDoc.object();
        
        QString generatedText = jsonObj["response"].toString();
        emit responseReceived(generatedText);
    } else {
        emit errorOccurred("Błąd połączenia z Ollamą: " + reply->errorString() + "\nUpewnij się, że model działa lokalnie.");
    }
    reply->deleteLater();
}
