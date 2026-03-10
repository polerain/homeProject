#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>

struct DeviceInfo {
    QString name;
    QString type;
    QString status;
    QString id;
    QString room;
};

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    static DeviceManager &instance();

    QList<DeviceInfo> getAllDevices() const;
    QList<DeviceInfo> getDevicesByRoom(const QString &room) const;
    DeviceInfo getDeviceById(const QString &id) const;

private:
    explicit DeviceManager(QObject *parent = nullptr);
    QList<DeviceInfo> m_devices;
    void initDevices();
};

#endif // DEVICEMANAGER_H
