#ifndef SCENEMODELS_H
#define SCENEMODELS_H

#include <QString>
#include <QList>
#include <QVariant>

// 场景基础信息
struct SceneInfo
{
    int id = -1;
    QString name;
    QString description;
    bool isBuiltin = false;
    bool isEnabled = true;
    QString icon;
};

// 场景绑定的设备配置
struct SceneDeviceBinding
{
    int id = -1;
    int sceneId = -1;
    QString deviceId;
    QString deviceName;
    QString deviceType;
    bool isSelected = true;

    // 目标状态
    QString targetPower;       // ON / OFF
    int targetTemp = -1;       // AC Temp (16-30)
    QString targetMode;        // AC Mode (COOL, HEAT, FAN, DRY)
    QString targetFan;         // AC Fan (AUTO, LOW, MID, HIGH)
    int targetBrightness = -1; // Light Brightness (0-100)
    QString targetLightColor;  // Light Color Temp (WARM, NATURAL, COLD)
    QString targetPosition;    // Curtain Position (OPEN, CLOSE)

    int sortOrder = 0;
};

#endif // SCENEMODELS_H
