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

// Custom Widget for List Item
class DeviceItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceItemWidget(const QString &name, const QString &type, const QString &status, const QString &id, QWidget *parent = nullptr);
    void setStatus(const QString &status);
    QString getName() const { return m_name; }
    QString getType() const { return m_type; }
    QString getId() const { return m_id; } // Add ID for better identification

signals:
    void controlClicked(const QString &cmd);

private:
    QLabel *m_iconLabel;
    QLabel *m_nameLabel;
    QLabel *m_statusLabel;
    QPushButton *m_controlBtn;
    
    // AC Specific Controls
    QWidget *m_acControlWidget;
    QSpinBox *m_tempSpinBox;
    QComboBox *m_modeCombo;
    QComboBox *m_fanCombo;
    
    QString m_name; // Display Name (e.g. 客厅灯)
    QString m_id;   // Internal ID (e.g. light_living)
    QString m_type; // Type (e.g. LIGHT)
};

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

private:
    void setupUi();
    void loadDevices(); // Ideally load from Database, for now hardcode or dynamic

    QLineEdit *m_ipEdit;
    QLineEdit *m_portEdit;
    QListWidget *m_deviceList;
    QPushButton *m_connectBtn;
};

#endif // DEVICECONTROLWIDGET_H
