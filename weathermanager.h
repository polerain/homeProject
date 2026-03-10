#ifndef WEATHERMANAGER_H
#define WEATHERMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrl>

class WeatherManager : public QObject
{
    Q_OBJECT
public:
    static WeatherManager& instance();
    
    // Query weather for a specific city
    void queryWeather(const QString& city);
    
    struct WeatherData {
        QString condition;
        QString temperature;
        QString humidity;
        QString wind;
        QString clothing;
        QString advice;
        QString homeSettings;
        
        WeatherData() : condition("未知"), temperature("未知"), humidity("未知"), 
                       wind("未知"), clothing("未知"), advice("未知"), homeSettings("未知") {}
    };

signals:
    void weatherReceived(const WeatherData& weather);
    void errorOccurred(const QString& message);

private slots:
    void onWorkflowResponse(QNetworkReply* reply);

private:
    explicit WeatherManager(QObject* parent = nullptr);
    ~WeatherManager();
    WeatherManager(const WeatherManager&) = delete;
    WeatherManager& operator=(const WeatherManager&) = delete;
    
    QNetworkAccessManager* m_networkManager;
    QString m_apiKey;
};

#endif // WEATHERMANAGER_H
