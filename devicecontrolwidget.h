#ifndef DEVICECONTROLWIDGET_H
#define DEVICECONTROLWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include "deviceitemwidget.h"
#include "devicemanager.h"

class DeviceControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceControlWidget(QWidget *parent = nullptr);

public slots:
    void updateDeviceStatus(const QString &data); // Parse data from TcpManager

private slots:
    void onRefreshClicked();
    void onConnectClicked();
    void onDeviceControl(const QString &cmd);
    void onRoomSelected(const QString &room); // New slot for room filtering

private:
    void setupUi();
    void loadDevices(); 
    void filterDevices(const QString &room); // Helper to filter list

    QLineEdit *m_ipEdit;
    QLineEdit *m_portEdit;
    QListWidget *m_deviceList;
    QPushButton *m_connectBtn;
    
    QList<DeviceInfo> m_allDevices;
};

#endif // DEVICECONTROLWIDGET_H
