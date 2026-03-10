#include "aiassistant.h"
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
#include <QVariant>

AIAssistant::AIAssistant(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    // 硅基流动API密钥
    m_apiKey = qgetenv("SILICONFLOW_API_KEY");
    if (m_apiKey.isEmpty())
    {
        // 默认API密钥
        m_apiKey = "sk-urcfwkvoytihfgrvsxwjyoiseokaqaoxxemqtwsrotdpojwh";
    }
}

AIAssistant::~AIAssistant()
{
    if (m_networkManager)
    {
        delete m_networkManager;
    }
}

AIAssistant &AIAssistant::instance()
{
    static AIAssistant instance;
    return instance;
}

void AIAssistant::getHomeSuggestions(const QString &location)
{
    // 构建提示词
    QString prompt = QString("你是一个智能家居助手，请根据当前天气情况为用户提供穿衣和家居设置建议。"
                             "当前城市：%1。"
                             "请严格以纯JSON格式返回结果，**禁止使用Markdown代码块**，**禁止包含任何额外文字**。"
                             "**请务必返回单行JSON字符串**，不要换行。"
                             "返回JSON格式示例："
                             "{\"temperature\":\"10°C~25°C\",\"condition\":\"晴\",\"humidity\":\"60%\",\"wind\":\"东风3级\",\"clothing\":\"建议穿着短袖\",\"homeSettings\":\"建议开启加湿器\",\"advice\":\"天气不错\"}"
                             "字段说明："
                             "1. temperature: 气温"
                             "2. condition: 天气状况"
                             "3. humidity: 湿度"
                             "4. wind: 风力"
                             "5. clothing: 穿衣建议"
                             "6. homeSettings: 家居设置建议"
                             "7. advice: 综合建议")
                         .arg(location);

    // 构建请求体
    QJsonObject requestBody;
    requestBody["model"] = "Pro/MiniMaxAI/MiniMax-M2.5";
    requestBody["messages"] = QJsonArray{
        QJsonObject{
            {"role", "user"},
            {"content", prompt}}};
    requestBody["temperature"] = 0.7;
    requestBody["max_tokens"] = 1000;

    QJsonDocument jsonDoc(requestBody);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Compact);

    // 构建请求
    QNetworkRequest request(QUrl("https://api.siliconflow.cn/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    // 发送请求
    QNetworkReply *reply = m_networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
            { onSuggestionsFinished(reply); });
}

void AIAssistant::onSuggestionsFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        emit errorOccurred("网络错误：" + reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();

    // 解析响应
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        emit errorOccurred("JSON解析错误：" + parseError.errorString());
        return;
    }

    QJsonObject responseObject = jsonDoc.object();

    // 检查是否有错误
    if (responseObject.contains("error"))
    {
        QJsonObject errorObj = responseObject["error"].toObject();
        emit errorOccurred("API错误：" + errorObj["message"].toString());
        return;
    }

    // 提取AI返回的内容
    QJsonArray choicesArray = responseObject["choices"].toArray();
    if (choicesArray.isEmpty())
    {
        emit errorOccurred("无效的响应格式");
        return;
    }

    QJsonObject choice = choicesArray[0].toObject();
    QJsonObject message = choice["message"].toObject();
    QString content = message["content"].toString();

    // 尝试提取 JSON 部分（处理 Markdown 格式）
    int jsonStart = content.indexOf('{');
    int jsonEnd = content.lastIndexOf('}');
    if (jsonStart != -1 && jsonEnd != -1 && jsonEnd > jsonStart)
    {
        content = content.mid(jsonStart, jsonEnd - jsonStart + 1);
    }

    // 解析AI返回的JSON内容
    QJsonParseError parseError2;
    QJsonDocument contentDoc = QJsonDocument::fromJson(content.toUtf8(), &parseError2);

    if (parseError2.error != QJsonParseError::NoError)
    {
        // 如果AI返回的不是标准JSON，直接返回原始内容
        emit errorOccurred("无法解析AI响应：" + parseError2.errorString() + "\n原始内容：" + content);
        return;
    }

    if (!contentDoc.isObject())
    {
        emit errorOccurred("AI响应格式错误，期望为JSON对象");
        return;
    }

    QJsonObject dataObj = contentDoc.object();

    WeatherData weather;
    weather.temperature = dataObj["temperature"].toString("未知");
    weather.condition = dataObj["condition"].toString("未知");
    weather.humidity = dataObj["humidity"].toString("未知");
    weather.wind = dataObj["wind"].toString("未知");

    Suggestions suggestions;
    suggestions.clothing = dataObj["clothing"].toString("请查看天气情况自行决定");
    suggestions.homeSettings = dataObj["homeSettings"].toString("请根据实际情况调整");
    suggestions.advice = dataObj["advice"].toString("");

    emit suggestionsReceived(weather, suggestions);
}
