#include "aiassistantwidget.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QFont>
#include <QDateTime>

AIAssistantWidget::AIAssistantWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();

    // 连接AI助手信号
    connect(&AIAssistant::instance(), &AIAssistant::suggestionsReceived, this, &AIAssistantWidget::onSuggestionsReceived);
    connect(&AIAssistant::instance(), &AIAssistant::errorOccurred, this, &AIAssistantWidget::onErrorOccurred);
}

void AIAssistantWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 标题
    QLabel *title = new QLabel("🤖 AI 智能助手", this);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);

    // 位置输入
    QHBoxLayout *locationLayout = new QHBoxLayout();
    QLabel *locationLabel = new QLabel("城市：", this);
    m_locationEdit = new QLineEdit("北京", this);
    m_locationEdit->setFixedWidth(150);
    m_getBtn = new QPushButton("获取建议", this);
    m_getBtn->setFixedWidth(100);

    locationLayout->addWidget(locationLabel);
    locationLayout->addWidget(m_locationEdit);
    locationLayout->addStretch();
    locationLayout->addWidget(m_getBtn);

    // 结果显示区域
    QGroupBox *resultGroup = new QGroupBox("建议结果", this);
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);

    // 天气信息
    QGroupBox *weatherGroup = new QGroupBox("📊 天气信息", this);
    QVBoxLayout *weatherLayout = new QVBoxLayout(weatherGroup);

    m_temperatureLabel = new QLabel("温度：--", this);
    m_conditionLabel = new QLabel("天气：--", this);
    m_humidityLabel = new QLabel("湿度：--", this);
    m_windLabel = new QLabel("风力：--", this);

    weatherLayout->addWidget(m_temperatureLabel);
    weatherLayout->addWidget(m_conditionLabel);
    weatherLayout->addWidget(m_humidityLabel);
    weatherLayout->addWidget(m_windLabel);

    // 建议信息
    QGroupBox *suggestionGroup = new QGroupBox("💡 智能建议", this);
    QVBoxLayout *suggestionLayout = new QVBoxLayout(suggestionGroup);

    QLabel *clothingTitle = new QLabel("👕 穿衣建议：", this);
    QFont clothingFont = clothingTitle->font();
    clothingFont.setBold(true);
    clothingTitle->setFont(clothingFont);
    m_clothingLabel = new QLabel("请获取建议...", this);
    m_clothingLabel->setWordWrap(true);
    m_clothingLabel->setMinimumHeight(50);

    QLabel *homeSettingsTitle = new QLabel("🏠 家居设置：", this);
    QFont homeSettingsFont = homeSettingsTitle->font();
    homeSettingsFont.setBold(true);
    homeSettingsTitle->setFont(homeSettingsFont);
    m_homeSettingsLabel = new QLabel("请获取建议...", this);
    m_homeSettingsLabel->setWordWrap(true);
    m_homeSettingsLabel->setMinimumHeight(50);

    QLabel *adviceTitle = new QLabel("✨ 综合建议：", this);
    QFont adviceFont = adviceTitle->font();
    adviceFont.setBold(true);
    adviceTitle->setFont(adviceFont);
    m_adviceLabel = new QLabel("请获取建议...", this);
    m_adviceLabel->setWordWrap(true);
    m_adviceLabel->setMinimumHeight(50);

    suggestionLayout->addWidget(clothingTitle);
    suggestionLayout->addWidget(m_clothingLabel);
    suggestionLayout->addWidget(homeSettingsTitle);
    suggestionLayout->addWidget(m_homeSettingsLabel);
    suggestionLayout->addWidget(adviceTitle);
    suggestionLayout->addWidget(m_adviceLabel);

    resultLayout->addWidget(weatherGroup);
    resultLayout->addWidget(suggestionGroup);

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
        QLabel {
            font-size: 12px;
            color: #333;
        }
    )");

    // 刷新界面
    update();

    // 连接按钮点击事件
    connect(m_getBtn, &QPushButton::clicked, this, &AIAssistantWidget::onGetSuggestions);
}

void AIAssistantWidget::onGetSuggestions()
{
    QString location = m_locationEdit->text().trimmed();
    if (location.isEmpty())
    {
        QMessageBox::warning(this, "警告", "请输入城市名称");
        return;
    }

    m_getBtn->setEnabled(false);
    m_getBtn->setText("正在获取...");

    // 获取建议
    AIAssistant::instance().getHomeSuggestions(location);
}

void AIAssistantWidget::onSuggestionsReceived(const AIAssistant::WeatherData &weather, const AIAssistant::Suggestions &suggestions)
{
    m_temperatureLabel->setText("温度：" + weather.temperature);
    m_conditionLabel->setText("天气：" + weather.condition);
    m_humidityLabel->setText("湿度：" + weather.humidity);
    m_windLabel->setText("风力：" + weather.wind);

    m_clothingLabel->setText(suggestions.clothing);
    m_homeSettingsLabel->setText(suggestions.homeSettings);
    m_adviceLabel->setText(suggestions.advice);

    m_getBtn->setEnabled(true);
    m_getBtn->setText("获取建议");

    // Log success
    DatabaseManager::LogData log;
    log.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    log.user = "用户";
    log.deviceName = "AI助手";
    log.action = "获取建议: " + m_locationEdit->text();
    log.result = "成功";
    DatabaseManager::instance().addLog(log);
}

void AIAssistantWidget::onErrorOccurred(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    m_getBtn->setEnabled(true);
    m_getBtn->setText("获取建议");

    // Log error
    DatabaseManager::LogData log;
    log.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    log.user = "用户";
    log.deviceName = "AI助手";
    log.action = "获取建议: " + m_locationEdit->text();
    log.result = "失败: " + message;
    DatabaseManager::instance().addLog(log);
}
