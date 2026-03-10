#include "devicemanager.h"

DeviceManager::DeviceManager(QObject *parent) : QObject(parent)
{
    initDevices();
}

DeviceManager &DeviceManager::instance()
{
    static DeviceManager instance;
    return instance;
}

void DeviceManager::initDevices()
{
    m_devices = {
        {"客厅主灯", "LIGHT", "OFF", "light_living", "客厅"},
        {"客厅空调", "AC", "OFF", "ac_living", "客厅"},
        {"客厅窗帘", "CURTAIN", "CLOSED", "curtain_living", "客厅"},

        {"卧室顶灯", "LIGHT", "OFF", "light_bedroom", "卧室"},

        {"厨房顶灯", "LIGHT", "OFF", "light_kitchen", "厨房"},
        {"排气扇", "FAN", "OFF", "fan_kitchen", "厨房"},

        {"卫生间灯", "LIGHT", "OFF", "light_restroom", "卫生间"},

        {"餐厅吊灯", "LIGHT", "OFF", "light_dining", "餐厅"}};
}

QList<DeviceInfo> DeviceManager::getAllDevices() const
{
    return m_devices;
}

QList<DeviceInfo> DeviceManager::getDevicesByRoom(const QString &room) const
{
    if (room == "全部") return m_devices;

    QList<DeviceInfo> result;
    for (const auto &dev : m_devices)
    {
        if (dev.room == room)
        {
            result.append(dev);
        }
    }
    return result;
}

DeviceInfo DeviceManager::getDeviceById(const QString &id) const
{
    for (const auto &dev : m_devices)
    {
        if (dev.id == id)
        {
            return dev;
        }
    }
    return DeviceInfo();
}
