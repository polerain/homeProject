#include "dashboardwidget.h"
#include "devicecontrolwidget.h"
#include "scenewidget.h"
#include "historywidget.h"
#include "aiassistantwidget.h"
#include "tcpmanager.h"

DashboardWidget::DashboardWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();

    // Listen for environmental data to update History Widget
    connect(&TcpManager::instance(), &TcpManager::dataReceived, this, [this](const QString &data)
            {
        // Format: ENV_DATA:TEMP=25.5,HUMID=60.0
        if (data.startsWith("ENV_DATA:")) {
            double temp = 0.0;
            double humid = 0.0;
            
            QString content = data.mid(9);
            QStringList parts = content.split(",");
            for(const QString &p : parts) {
                if(p.startsWith("TEMP=")) temp = p.mid(5).toDouble();
                if(p.startsWith("HUMID=")) humid = p.mid(6).toDouble();
            }
            
            // Find History Widget and update
            // Note: History is at index 3 (0-based)
            HistoryWidget *hist = qobject_cast<HistoryWidget*>(m_contentStack->widget(3));
            if(hist) {
                hist->addDataPoint(temp, humid);
            }
        } });
}

void DashboardWidget::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setupSidebar();
    setupContentArea();

    mainLayout->addWidget(m_menuList, 1);
    mainLayout->addWidget(m_contentStack, 4);
}

void DashboardWidget::setupSidebar()
{
    m_menuList = new QListWidget(this);
    m_menuList->addItem("🏠 首页总览"); // 0
    m_menuList->addItem("📱 设备控制"); // 1
    m_menuList->addItem("🎬 场景模式"); // 2
    m_menuList->addItem("📊 历史记录"); // 3
    m_menuList->addItem("🤖 AI助手"); // 4
    m_menuList->addItem("⚠️ 异常报警");  // 5
    m_menuList->addItem("⚙️ 系统设置");  // 6

    // Set Object Name for styling
    m_menuList->setObjectName("sidebar");

    // Set default selection
    m_menuList->setCurrentRow(0);
}

void DashboardWidget::setupContentArea()
{
    m_contentStack = new QStackedWidget(this);

    // 0. Home Overview (Placeholder)
    QLabel *homeLabel = new QLabel("欢迎使用智能家居监控平台\n请点击左侧菜单选择功能", this);
    homeLabel->setAlignment(Qt::AlignCenter);
    QFont f = homeLabel->font();
    f.setPointSize(16);
    homeLabel->setFont(f);
    m_contentStack->addWidget(homeLabel);

    // 1. Device Control
    m_contentStack->addWidget(new DeviceControlWidget(this));

    // 2. Scene Mode
    m_contentStack->addWidget(new SceneWidget(this));

    // 3. History Data
    m_contentStack->addWidget(new HistoryWidget(this));

    // 4. AI Assistant
    m_contentStack->addWidget(new AIAssistantWidget(this));

    // 5. Alarm (Placeholder)
    QLabel *alarmLabel = new QLabel("异常报警模块开发中...", this);
    alarmLabel->setAlignment(Qt::AlignCenter);
    m_contentStack->addWidget(alarmLabel);

    // 6. Settings (Placeholder)
    QLabel *settingsLabel = new QLabel("系统设置模块开发中...", this);
    settingsLabel->setAlignment(Qt::AlignCenter);
    m_contentStack->addWidget(settingsLabel);

    // Connect list selection to stack switch
    connect(m_menuList, &QListWidget::currentRowChanged, m_contentStack, &QStackedWidget::setCurrentIndex);
}
