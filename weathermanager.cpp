#include "weathermanager.h"
#include <QJsonParseError>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QByteArray>
#include <QString>

WeatherManager::WeatherManager(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_apiKey = "cztei_hBylFnLT8Q7zpEbheGZhaR3ZDYtKZtOxDS8X2vklkkSPt5966aH3koS7fULqN1zTl";
}

WeatherManager::~WeatherManager()
{
    if (m_networkManager)
    {
        delete m_networkManager;
    }
}

WeatherManager &WeatherManager::instance()
{
    static WeatherManager instance;
    return instance;
}

void WeatherManager::queryWeather(const QString &city)
{
    QNetworkRequest request;
    request.setUrl(QUrl("https://api.coze.cn/v1/workflow/run"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    QJsonObject parameters;
    parameters["city"] = city;

    QJsonObject requestBody;
    requestBody["workflow_id"] = "7615505990242549775";
    requestBody["parameters"] = parameters;

    QJsonDocument doc(requestBody);
    QByteArray postData = doc.toJson();

    QNetworkReply *reply = m_networkManager->post(request, postData);
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
            { onWorkflowResponse(reply); });
}

void WeatherManager::onWorkflowResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        emit errorOccurred("网络错误：" + reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        emit errorOccurred("JSON解析错误：" + parseError.errorString());
        return;
    }

    QJsonObject responseObject = jsonDoc.object();

    QString dataStr = responseObject["data"].toString();

    QJsonParseError dataParseError;
    QJsonDocument dataDoc = QJsonDocument::fromJson(dataStr.toUtf8(), &dataParseError);

    if (dataParseError.error != QJsonParseError::NoError)
    {
        emit errorOccurred("数据解析错误：" + dataParseError.errorString());
        return;
    }

    QJsonObject dataObj = dataDoc.object();

    QString outputStr = dataObj["output"].toString();

    QJsonParseError outputParseError;
    QJsonDocument outputDoc = QJsonDocument::fromJson(outputStr.toUtf8(), &outputParseError);

    if (outputParseError.error != QJsonParseError::NoError)
    {
        emit errorOccurred("输出解析错误：" + outputParseError.errorString());
        return;
    }

    QJsonObject outputObj = outputDoc.object();

    WeatherData weather;
    weather.condition = outputObj["condition"].toString("未知");
    weather.temperature = outputObj["temperature"].toString("未知");
    weather.humidity = outputObj["humidity"].toString("未知");
    weather.wind = outputObj["wind"].toString("未知");
    weather.clothing = outputObj["clothing"].toString("未知");
    weather.advice = outputObj["advice"].toString("未知");
    weather.homeSettings = outputObj["homeSettings"].toString("未知");

    emit weatherReceived(weather);
}
