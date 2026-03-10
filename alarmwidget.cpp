#include "alarmwidget.h"
#include "databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QDateTime>
// #include <QSound> // Qt Multimedia module required, for now we use beep or placeholder

AlarmWidget::AlarmWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    loadSettings();
    loadAlarms();
}

void AlarmWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. Settings Area
    QGroupBox *settingsGroup = new QGroupBox("报警设置", this);
    QHBoxLayout *settingsLayout = new QHBoxLayout(settingsGroup);

    settingsLayout->addWidget(new QLabel("高温阈值(°C):"));
    m_tempThresholdSpin = new QDoubleSpinBox(this);
    m_tempThresholdSpin->setRange(0, 100);
    settingsLayout->addWidget(m_tempThresholdSpin);

    settingsLayout->addWidget(new QLabel("高湿阈值(%):"));
    m_humidThresholdSpin = new QDoubleSpinBox(this);
    m_humidThresholdSpin->setRange(0, 100);
    settingsLayout->addWidget(m_humidThresholdSpin);

    m_soundCheck = new QCheckBox("开启声音提示", this);
    settingsLayout->addWidget(m_soundCheck);

    m_btnSaveSettings = new QPushButton("保存设置", this);
    settingsLayout->addWidget(m_btnSaveSettings);
    settingsLayout->addStretch();

    mainLayout->addWidget(settingsGroup);

    // 2. Alarm List
    QGroupBox *listGroup = new QGroupBox("报警记录", this);
    QVBoxLayout *listLayout = new QVBoxLayout(listGroup);

    m_alarmTable = new QTableWidget(this);
    m_alarmTable->setColumnCount(3);
    m_alarmTable->setHorizontalHeaderLabels({"时间", "报警类型", "报警内容"});
    m_alarmTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_alarmTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listLayout->addWidget(m_alarmTable);

    m_btnClear = new QPushButton("清空记录", this);
    m_btnClear->setStyleSheet("background-color: #dc3545; color: white;");
    listLayout->addWidget(m_btnClear, 0, Qt::AlignRight);

    mainLayout->addWidget(listGroup);

    // Connects
    connect(m_btnSaveSettings, &QPushButton::clicked, this, &AlarmWidget::onSaveSettingsClicked);
    connect(m_btnClear, &QPushButton::clicked, this, &AlarmWidget::onClearAlarmsClicked);
}

void AlarmWidget::loadSettings()
{
    auto s = DatabaseManager::instance().getAlarmSettings();
    m_tempThresholdSpin->setValue(s.tempHighThreshold);
    m_humidThresholdSpin->setValue(s.humidHighThreshold);
    m_soundCheck->setChecked(s.soundEnabled);
}

void AlarmWidget::loadAlarms()
{
    m_alarmTable->setRowCount(0);
    auto alarms = DatabaseManager::instance().getAlarms();
    for (const auto &a : alarms)
    {
        int row = m_alarmTable->rowCount();
        m_alarmTable->insertRow(row);
        m_alarmTable->setItem(row, 0, new QTableWidgetItem(a.timestamp));
        m_alarmTable->setItem(row, 1, new QTableWidgetItem(a.type));
        m_alarmTable->setItem(row, 2, new QTableWidgetItem(a.content));
    }
}

void AlarmWidget::onSaveSettingsClicked()
{
    DatabaseManager::AlarmSettings s;
    s.tempHighThreshold = m_tempThresholdSpin->value();
    s.humidHighThreshold = m_humidThresholdSpin->value();
    s.soundEnabled = m_soundCheck->isChecked();

    DatabaseManager::instance().saveAlarmSettings(s);
    QMessageBox::information(this, "成功", "报警设置已保存");
}

void AlarmWidget::onClearAlarmsClicked()
{
    if (QMessageBox::question(this, "确认", "确定要清空所有报警记录吗？") == QMessageBox::Yes)
    {
        DatabaseManager::instance().clearAlarms();
        loadAlarms();
    }
}

void AlarmWidget::checkAlarmCondition(double temp, double humidity)
{
    auto s = DatabaseManager::instance().getAlarmSettings();
    qDebug() << "Checking alarm conditions - Temp:" << temp << "Threshold:" << s.tempHighThreshold 
             << "Humid:" << humidity << "Threshold:" << s.humidHighThreshold;

    if (temp > s.tempHighThreshold)
    {
        triggerAlarm("温度报警", QString("当前温度 %1°C 超过阈值 %2°C").arg(temp).arg(s.tempHighThreshold));
    }

    if (humidity > s.humidHighThreshold)
    {
        triggerAlarm("湿度报警", QString("当前湿度 %1% 超过阈值 %2%").arg(humidity).arg(s.humidHighThreshold));
    }
}

void AlarmWidget::checkDeviceStatus(const QString &deviceName, bool isOffline)
{
    if (isOffline)
    {
        triggerAlarm("设备离线", QString("设备 [%1] 连接断开").arg(deviceName));
    }
}

void AlarmWidget::triggerAlarm(const QString &type, const QString &content)
{
    // Avoid duplicate alarms in short time? (Simple logic: just add)
    // Add to DB
    DatabaseManager::instance().addAlarm(type, content);

    // Refresh list
    loadAlarms();

    // Log
    DatabaseManager::LogData log;
    log.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    log.user = "系统";
    log.deviceName = "报警模块";
    log.action = "触发报警: " + type;
    log.result = content;
    DatabaseManager::instance().addLog(log);

    // Sound & Popup
    auto s = DatabaseManager::instance().getAlarmSettings();
    if (s.soundEnabled)
    {
        QApplication::beep(); // Use beep for simplicity
    }

    // Optional: Show non-blocking notification or message box
    // QMessageBox::warning(this, "异常报警", type + "\n" + content);
}
