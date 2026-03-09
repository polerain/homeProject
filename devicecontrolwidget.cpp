#include "devicecontrolwidget.h"
#include "tcpmanager.h"
#include <QMessageBox>
#include <QStyle>

// --- DeviceItemWidget Implementation ---

DeviceItemWidget::DeviceItemWidget(const QString &name, const QString &type, const QString &status, const QString &id, QWidget *parent)
    : QWidget(parent), m_name(name), m_type(type), m_id(id)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(15);

    // Icon placeholder (Circle with color)
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(50, 50);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setObjectName("deviceIcon"); // For QSS
    QFont iconFont = m_iconLabel->font();
    iconFont.setPixelSize(24);
    m_iconLabel->setFont(iconFont);

    // Info
    QVBoxLayout *infoLayout = new QVBoxLayout();
    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setObjectName("deviceTitle"); // For QSS
    m_statusLabel = new QLabel(status, this);
    m_statusLabel->setObjectName("deviceStatus"); // For QSS
    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_statusLabel);

    // Control Button
    m_controlBtn = new QPushButton(this);
    m_controlBtn->setFixedSize(100, 36);
    m_controlBtn->setObjectName("actionButton"); // For QSS

    if (type == "LIGHT")
    {
        m_iconLabel->setText("💡");
        m_controlBtn->setText(status == "ON" ? "关闭" : "开启");

        // --- Light Extended Controls ---
        m_lightControlWidget = new QWidget(this);
        QHBoxLayout *lightLayout = new QHBoxLayout(m_lightControlWidget);
        lightLayout->setContentsMargins(0, 0, 0, 0);
        lightLayout->setSpacing(5);

        // Brightness
        QLabel *briLabel = new QLabel("亮度:", this);
        m_brightnessSpinBox = new QSpinBox(this);
        m_brightnessSpinBox->setRange(0, 100);
        m_brightnessSpinBox->setValue(80);
        m_brightnessSpinBox->setSuffix("%");
        m_brightnessSpinBox->setFixedWidth(60);
        connect(m_brightnessSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val)
                {
            // LIGHT_LIVING_BRI_80
            emit controlClicked(QString("%1_BRI_%2").arg(m_id.toUpper()).arg(val)); });

        // Color Temp
        m_colorTempCombo = new QComboBox(this);
        m_colorTempCombo->addItems({"暖光", "自然光", "冷白光"});
        m_colorTempCombo->setFixedWidth(80);
        connect(m_colorTempCombo, &QComboBox::currentTextChanged, [this](const QString &text)
                {
            // LIGHT_LIVING_COLOR_WARM
            QString color = "NATURAL";
            if(text == "暖光") color = "WARM";
            else if(text == "冷白光") color = "COLD";
            emit controlClicked(QString("%1_COLOR_%2").arg(m_id.toUpper()).arg(color)); });

        lightLayout->addWidget(briLabel);
        lightLayout->addWidget(m_brightnessSpinBox);
        lightLayout->addWidget(m_colorTempCombo);

        infoLayout->addWidget(m_lightControlWidget);

        connect(m_controlBtn, &QPushButton::clicked, [this]()
                {
            QString cmd = (m_statusLabel->text() == "ON") ? "OFF" : "ON";
            // Use ID for command construction: LIGHT_LIVING_ON
            // Expected ID format: light_living -> LIGHT_LIVING
            emit controlClicked(QString("%1_%2").arg(m_id.toUpper()).arg(cmd)); });
    }
    else if (type == "AC")
    {
        m_iconLabel->setText("❄️");
        m_controlBtn->setText(status == "ON" ? "关闭空调" : "开启空调");

        // --- AC Extended Controls ---
        m_acControlWidget = new QWidget(this);
        QHBoxLayout *acLayout = new QHBoxLayout(m_acControlWidget);
        acLayout->setContentsMargins(0, 0, 0, 0);
        acLayout->setSpacing(5);

        // Temperature
        m_tempSpinBox = new QSpinBox(this);
        m_tempSpinBox->setRange(16, 30);
        m_tempSpinBox->setValue(26);
        m_tempSpinBox->setSuffix("°C");
        m_tempSpinBox->setFixedWidth(70);
        connect(m_tempSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val)
                {
            // AC_LIVING_TEMP_26
            emit controlClicked(QString("%1_TEMP_%2").arg(m_id.toUpper()).arg(val)); });

        // Mode
        m_modeCombo = new QComboBox(this);
        m_modeCombo->addItems({"制冷", "制暖", "送风", "除湿"});
        m_modeCombo->setFixedWidth(70);
        connect(m_modeCombo, &QComboBox::currentTextChanged, [this](const QString &text)
                {
            // AC_LIVING_MODE_COOL
            QString mode = "COOL";
            if(text == "制暖") mode = "HEAT";
            else if(text == "送风") mode = "FAN";
            else if(text == "除湿") mode = "DRY";
            emit controlClicked(QString("%1_MODE_%2").arg(m_id.toUpper()).arg(mode)); });

        // Fan Speed
        m_fanCombo = new QComboBox(this);
        m_fanCombo->addItems({"自动", "低风", "中风", "高风"});
        m_fanCombo->setFixedWidth(70);
        connect(m_fanCombo, &QComboBox::currentTextChanged, [this](const QString &text)
                {
             // AC_LIVING_FAN_AUTO
            QString speed = "AUTO";
            if(text == "低风") speed = "LOW";
            else if(text == "中风") speed = "MID";
            else if(text == "高风") speed = "HIGH";
            emit controlClicked(QString("%1_FAN_%2").arg(m_id.toUpper()).arg(speed)); });

        acLayout->addWidget(m_tempSpinBox);
        acLayout->addWidget(m_modeCombo);
        acLayout->addWidget(m_fanCombo);

        // Add to main layout (insert before button)
        infoLayout->addWidget(m_acControlWidget);

        connect(m_controlBtn, &QPushButton::clicked, [this]()
                {
            QString cmd = (m_statusLabel->text() == "ON") ? "OFF" : "ON";
            emit controlClicked(QString("%1_%2").arg(m_id.toUpper()).arg(cmd)); });
    }
    else if (type == "CURTAIN")
    {
        m_iconLabel->setText("🪟");
        m_controlBtn->setText("开/关窗帘");
        connect(m_controlBtn, &QPushButton::clicked, [this]()
                { emit controlClicked(QString("%1_TOGGLE").arg(m_id.toUpper())); });
    }
    else if (type == "FAN")
    {
        m_iconLabel->setText("🌀");
        m_controlBtn->setText(status == "ON" ? "关闭" : "开启");
        connect(m_controlBtn, &QPushButton::clicked, [this]()
                {
            QString cmd = (m_statusLabel->text() == "ON") ? "OFF" : "ON";
            emit controlClicked(QString("%1_%2").arg(m_id.toUpper()).arg(cmd)); });
    }
    else
    {
        m_controlBtn->setText("操作");
    }

    layout->addWidget(m_iconLabel);
    layout->addLayout(infoLayout);
    layout->addStretch();
    layout->addWidget(m_controlBtn);

    setStatus(status);
}

void DeviceItemWidget::setStatus(const QString &status)
{
    m_statusLabel->setText(status);
    bool isActive = (status == "ON" || status == "OPEN" || status == "100");

    m_iconLabel->setProperty("active", isActive);
    style()->unpolish(m_iconLabel);
    style()->polish(m_iconLabel);

    if (isActive) // 100 means open for curtain
    {
        if (m_type == "LIGHT")
            m_controlBtn->setText("关闭");
        if (m_type == "AC")
            m_controlBtn->setText("关闭空调");
        if (m_type == "CURTAIN")
            m_controlBtn->setText("关闭窗帘");
    }
    else
    {
        if (m_type == "LIGHT")
            m_controlBtn->setText("开启");
        if (m_type == "AC")
            m_controlBtn->setText("开启空调");
        if (m_type == "CURTAIN")
            m_controlBtn->setText("开启窗帘");
    }
}

// --- DeviceControlWidget Implementation ---

DeviceControlWidget::DeviceControlWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    loadDevices();

    // Connect TcpManager signals
    connect(&TcpManager::instance(), &TcpManager::connected, this, [this]()
            {
        m_connectBtn->setText("断开连接");
        QMessageBox::information(this, "提示", "设备连接成功！"); });

    connect(&TcpManager::instance(), &TcpManager::disconnected, this, [this]()
            { m_connectBtn->setText("连接设备"); });

    connect(&TcpManager::instance(), &TcpManager::dataReceived, this, &DeviceControlWidget::updateDeviceStatus);
}

void DeviceControlWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Top Bar: Connection Config
    QHBoxLayout *topLayout = new QHBoxLayout();
    m_ipEdit = new QLineEdit("127.0.0.1", this);
    m_portEdit = new QLineEdit("12345", this);
    m_connectBtn = new QPushButton("连接设备", this);
    QPushButton *refreshBtn = new QPushButton("刷新状态", this);

    topLayout->addWidget(new QLabel("IP:"));
    topLayout->addWidget(m_ipEdit);
    topLayout->addWidget(new QLabel("Port:"));
    topLayout->addWidget(m_portEdit);
    topLayout->addWidget(m_connectBtn);
    topLayout->addWidget(refreshBtn);
    topLayout->addStretch();

    // Room Selection Bar
    QHBoxLayout *roomLayout = new QHBoxLayout();
    QLabel *roomLabel = new QLabel("选择房间:", this);
    roomLabel->setObjectName("sectionHeader"); // QSS
    roomLayout->addWidget(roomLabel);

    QStringList rooms = {"全部", "客厅", "卧室", "厨房", "卫生间", "餐厅"};
    for (const QString &room : rooms)
    {
        QPushButton *btn = new QPushButton(room, this);
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        btn->setObjectName("roomButton"); // QSS
        if (room == "全部")
            btn->setChecked(true);

        connect(btn, &QPushButton::clicked, [this, room]()
                { onRoomSelected(room); });
        roomLayout->addWidget(btn);
    }
    roomLayout->addStretch();

    // Device List
    m_deviceList = new QListWidget(this);
    m_deviceList->setObjectName("deviceList"); // QSS

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(roomLayout); // Add room bar
    mainLayout->addWidget(m_deviceList);

    connect(m_connectBtn, &QPushButton::clicked, this, &DeviceControlWidget::onConnectClicked);
    connect(refreshBtn, &QPushButton::clicked, this, &DeviceControlWidget::onRefreshClicked);
}

void DeviceControlWidget::loadDevices()
{
    m_allDevices = {
        {"客厅主灯", "LIGHT", "OFF", "light_living", "客厅"},
        {"客厅空调", "AC", "OFF", "ac_living", "客厅"},
        {"客厅窗帘", "CURTAIN", "CLOSED", "curtain_living", "客厅"},

        {"卧室顶灯", "LIGHT", "OFF", "light_bedroom", "卧室"},

        {"厨房顶灯", "LIGHT", "OFF", "light_kitchen", "厨房"},
        {"排气扇", "FAN", "OFF", "fan_kitchen", "厨房"}, // Use FAN type icon

        {"卫生间灯", "LIGHT", "OFF", "light_restroom", "卫生间"},

        {"餐厅吊灯", "LIGHT", "OFF", "light_dining", "餐厅"}};

    // Initial display: Show all
    filterDevices("全部");
}

void DeviceControlWidget::onRoomSelected(const QString &room)
{
    filterDevices(room);
}

void DeviceControlWidget::filterDevices(const QString &room)
{
    m_deviceList->clear();

    for (const auto &dev : m_allDevices)
    {
        if (room != "全部" && dev.room != room)
        {
            continue;
        }

        QListWidgetItem *item = new QListWidgetItem(m_deviceList);

        // Adjust height based on type
        if (dev.type == "AC" || dev.type == "LIGHT")
        {
            item->setSizeHint(QSize(0, 120)); // Taller for AC and LIGHT
        }
        else
        {
            item->setSizeHint(QSize(0, 90));
        }

        DeviceItemWidget *widget = new DeviceItemWidget(dev.name, dev.type, dev.status, dev.id);
        connect(widget, &DeviceItemWidget::controlClicked, this, &DeviceControlWidget::onDeviceControl);

        m_deviceList->setItemWidget(item, widget);
    }

    // Refresh status after filtering (if connected)
    if (TcpManager::instance().isConnected())
    {
        onRefreshClicked();
    }
}

void DeviceControlWidget::onConnectClicked()
{
    if (TcpManager::instance().isConnected())
    {
        TcpManager::instance().disconnectFromDevice();
    }
    else
    {
        QString ip = m_ipEdit->text();
        quint16 port = m_portEdit->text().toUShort();
        TcpManager::instance().connectToDevice(ip, port);
    }
}

void DeviceControlWidget::onRefreshClicked()
{
    TcpManager::instance().sendCommand("GET_ALL_STATUS");
}

void DeviceControlWidget::onDeviceControl(const QString &cmd)
{
    TcpManager::instance().sendCommand(cmd);
}

void DeviceControlWidget::updateDeviceStatus(const QString &data)
{
    // Simple parsing logic
    // Expected format: "LIGHT_LIVING_ON_OK" or "light_living:ON,light_bedroom:OFF..."

    // If bulk status update
    if (data.contains(":"))
    {
        QStringList parts = data.split(",");
        for (const QString &part : parts)
        {
            QStringList kv = part.split(":");
            if (kv.size() == 2)
            {
                QString key = kv[0].trimmed(); // e.g., light_living
                QString val = kv[1].trimmed(); // e.g., ON

                // Iterate list items to find match
                for (int i = 0; i < m_deviceList->count(); ++i)
                {
                    QListWidgetItem *item = m_deviceList->item(i);
                    DeviceItemWidget *widget = qobject_cast<DeviceItemWidget *>(m_deviceList->itemWidget(item));
                    if (widget)
                    {
                        // Direct match using ID
                        // key: light_living
                        // widget->getId(): light_living
                        if (key.compare(widget->getId(), Qt::CaseInsensitive) == 0)
                        {
                            widget->setStatus(val);
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Single command response, e.g. LIGHT_LIVING_ON_OK
        // Trigger a refresh to get sync
        onRefreshClicked();
    }
}
