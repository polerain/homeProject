#include "devicecontrolwidget.h"
#include "tcpmanager.h"
#include <QMessageBox>
#include <QStyle>

// DeviceItemWidget implementation has been moved to deviceitemwidget.cpp

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
    m_allDevices = DeviceManager::instance().getAllDevices();

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
