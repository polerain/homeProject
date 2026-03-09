#ifndef AIASSISTANT_H
#define AIASSISTANT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrl>

class AIAssistant : public QObject
{
    Q_OBJECT
public:
    static AIAssistant& instance();
    
    // Get AI suggestions for weather, clothing, and home settings
    void getHomeSuggestions(const QString& location = "北京");
    
    // Parse weather data from API
    struct WeatherData {
        QString temperature;
        QString condition;
        QString humidity;
        QString wind;
        
        WeatherData() : temperature("未知"), condition("未知"), humidity("未知"), wind("未知") {}
    };
    
    struct Suggestions {
        QString clothing;
        QString homeSettings;
        QString advice;
        
        Suggestions() : clothing("请查看天气情况自行决定"), homeSettings("请根据实际情况调整"), advice("") {}
    };

signals:
    void suggestionsReceived(const WeatherData& weather, const Suggestions& suggestions);
    void errorOccurred(const QString& message);

private slots:
    void onSuggestionsFinished(QNetworkReply* reply);

private:
    explicit AIAssistant(QObject* parent = nullptr);
    ~AIAssistant();
    AIAssistant(const AIAssistant&) = delete;
    AIAssistant& operator=(const AIAssistant&) = delete;
    
    QNetworkAccessManager* m_networkManager;
    QString m_apiKey;
};

#endif // AIASSISTANT_H
