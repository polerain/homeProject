#include "aiassistantwidget.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QFont>
#include <QDateTime>

AIAssistantWidget::AIAssistantWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();

    connect(&WeatherManager::instance(), &WeatherManager::weatherReceived, this, &AIAssistantWidget::onWeatherReceived);
    connect(&WeatherManager::instance(), &WeatherManager::errorOccurred, this, &AIAssistantWidget::onWeatherErrorOccurred);
}

void AIAssistantWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 标题
    QLabel *title = new QLabel("🌤️ 天气查询", this);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);

    // 位置输入
    QHBoxLayout *locationLayout = new QHBoxLayout();
    QLabel *locationLabel = new QLabel("城市：", this);
    m_locationEdit = new QLineEdit("兰州", this);
    m_locationEdit->setFixedWidth(150);
    m_weatherBtn = new QPushButton("查询天气", this);
    m_weatherBtn->setFixedWidth(100);

    locationLayout->addWidget(locationLabel);
    locationLayout->addWidget(m_locationEdit);
    locationLayout->addStretch();
    locationLayout->addWidget(m_weatherBtn);

    // 结果显示区域
    QGroupBox *resultGroup = new QGroupBox("天气信息", this);
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);

    // 天气信息卡片
    QFrame *weatherCard = new QFrame();
    weatherCard->setObjectName("weatherCard");
    QVBoxLayout *weatherCardLayout = new QVBoxLayout(weatherCard);
    weatherCardLayout->setContentsMargins(20, 20, 20, 20);
    weatherCardLayout->setSpacing(15);

    m_weatherConditionLabel = new QLabel("天气状况：--", this);
    m_weatherTemperatureLabel = new QLabel("温度：--", this);
    m_weatherHumidityLabel = new QLabel("湿度：--", this);
    m_weatherWindLabel = new QLabel("风力：--", this);

    m_weatherConditionLabel->setObjectName("weatherLabel");
    m_weatherTemperatureLabel->setObjectName("weatherLabel");
    m_weatherHumidityLabel->setObjectName("weatherLabel");
    m_weatherWindLabel->setObjectName("weatherLabel");

    weatherCardLayout->addWidget(m_weatherConditionLabel);
    weatherCardLayout->addWidget(m_weatherTemperatureLabel);
    weatherCardLayout->addWidget(m_weatherHumidityLabel);
    weatherCardLayout->addWidget(m_weatherWindLabel);

    QLabel *divider = new QLabel("─────────────────────");
    divider->setAlignment(Qt::AlignCenter);
    divider->setObjectName("dividerLabel");
    weatherCardLayout->addWidget(divider);

    QLabel *clothingTitle = new QLabel("穿衣建议：", this);
    clothingTitle->setObjectName("sectionTitle");
    m_weatherClothingLabel = new QLabel("请查询天气...", this);
    m_weatherClothingLabel->setObjectName("suggestionLabel");
    m_weatherClothingLabel->setWordWrap(true);
    m_weatherClothingLabel->setMinimumHeight(50);

    QLabel *homeSettingsTitle = new QLabel("家居设置：", this);
    homeSettingsTitle->setObjectName("sectionTitle");
    m_weatherHomeSettingsLabel = new QLabel("请查询天气...", this);
    m_weatherHomeSettingsLabel->setObjectName("suggestionLabel");
    m_weatherHomeSettingsLabel->setWordWrap(true);
    m_weatherHomeSettingsLabel->setMinimumHeight(50);

    QLabel *adviceTitle = new QLabel("综合建议：", this);
    adviceTitle->setObjectName("sectionTitle");
    m_weatherAdviceLabel = new QLabel("请查询天气...", this);
    m_weatherAdviceLabel->setObjectName("suggestionLabel");
    m_weatherAdviceLabel->setWordWrap(true);
    m_weatherAdviceLabel->setMinimumHeight(50);

    weatherCardLayout->addWidget(clothingTitle);
    weatherCardLayout->addWidget(m_weatherClothingLabel);
    weatherCardLayout->addWidget(homeSettingsTitle);
    weatherCardLayout->addWidget(m_weatherHomeSettingsLabel);
    weatherCardLayout->addWidget(adviceTitle);
    weatherCardLayout->addWidget(m_weatherAdviceLabel);

    resultLayout->addWidget(weatherCard);

    // 添加到主布局
    mainLayout->addWidget(title);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(locationLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(resultGroup);
    mainLayout->addStretch();

    // 样式
    setStyleSheet(R"(
        QWidget {
            background-color: #f5f5f5;
        }
        QGroupBox {
            border: 2px solid #4CAF50;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            font-weight: bold;
            color: #4CAF50;
            background-color: white;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        QLineEdit {
            padding: 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
            background-color: white;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            padding: 8px 16px;
            border: none;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
        QLabel#weatherLabel {
            font-size: 14px;
            color: #333;
            padding: 5px;
        }
        QLabel#dividerLabel {
            color: #ccc;
            font-size: 12px;
            padding: 5px 0;
        }
        QLabel#sectionTitle {
            font-size: 14px;
            font-weight: bold;
            color: #4CAF50;
            padding: 5px 0 0 0;
        }
        QLabel#suggestionLabel {
            font-size: 13px;
            color: #555;
            padding: 5px;
            background-color: #fafafa;
            border-radius: 4px;
        }
        QFrame#weatherCard {
            background-color: white;
            border-radius: 8px;
        }
    )");

    // 连接按钮点击事件
    connect(m_weatherBtn, &QPushButton::clicked, this, &AIAssistantWidget::onQueryWeather);
}

void AIAssistantWidget::onQueryWeather()
{
    QString city = m_locationEdit->text().trimmed();
    if (city.isEmpty())
    {
        QMessageBox::warning(this, "警告", "请输入城市名称");
        return;
    }

    m_weatherBtn->setEnabled(false);
    m_weatherBtn->setText("正在查询...");

    WeatherManager::instance().queryWeather(city);
}

void AIAssistantWidget::onWeatherReceived(const WeatherManager::WeatherData &weather)
{
    m_weatherConditionLabel->setText("天气状况：" + weather.condition);
    m_weatherTemperatureLabel->setText("温度：" + weather.temperature);
    m_weatherHumidityLabel->setText("湿度：" + weather.humidity);
    m_weatherWindLabel->setText("风力：" + weather.wind);
    
    m_weatherClothingLabel->setText(weather.clothing);
    m_weatherHomeSettingsLabel->setText(weather.homeSettings);
    m_weatherAdviceLabel->setText(weather.advice);

    m_weatherBtn->setEnabled(true);
    m_weatherBtn->setText("查询天气");

    DatabaseManager::LogData log;
    log.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    log.user = "用户";
    log.deviceName = "天气查询";
    log.action = "查询天气: " + m_locationEdit->text();
    log.result = "成功";
    DatabaseManager::instance().addLog(log);
}

void AIAssistantWidget::onWeatherErrorOccurred(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    m_weatherBtn->setEnabled(true);
    m_weatherBtn->setText("查询天气");

    DatabaseManager::LogData log;
    log.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    log.user = "用户";
    log.deviceName = "天气查询";
    log.action = "查询天气: " + m_locationEdit->text();
    log.result = "失败: " + message;
    DatabaseManager::instance().addLog(log);
}
