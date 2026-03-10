#ifndef ALARMWIDGET_H
#define ALARMWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>

class AlarmWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AlarmWidget(QWidget *parent = nullptr);

public slots:
    void checkAlarmCondition(double temp, double humidity); // Check env data
    void checkDeviceStatus(const QString &deviceName, bool isOffline); // Check device status

private slots:
    void onClearAlarmsClicked();
    void onSaveSettingsClicked();
    void loadAlarms();
    void loadSettings();

private:
    void setupUi();
    void triggerAlarm(const QString &type, const QString &content);

    QTableWidget *m_alarmTable;
    QPushButton *m_btnClear;
    
    // Settings
    QDoubleSpinBox *m_tempThresholdSpin;
    QDoubleSpinBox *m_humidThresholdSpin;
    QCheckBox *m_soundCheck;
    QPushButton *m_btnSaveSettings;
};

#endif // ALARMWIDGET_H
