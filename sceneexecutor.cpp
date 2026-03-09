#include "sceneexecutor.h"
#include "tcpmanager.h"
#include <QDebug>
#include <QDateTime>

SceneExecutor::SceneExecutor(QObject *parent) : QObject(parent)
{
}

SceneExecutor::ExecutionResult SceneExecutor::execute(int sceneId)
{
    ExecutionResult result = {true, 0, 0, 0, {}, {}};
    
    // 1. Check if scene exists and is enabled
    auto scene = DatabaseManager::instance().getSceneById(sceneId);
    if (scene.id == -1) {
        result.success = false;
        result.errorMessages.append("场景不存在");
        return result;
    }
    
    if (!scene.isEnabled) {
        result.success = false;
        result.errorMessages.append("场景未启用");
        return result;
    }
    
    // 2. Check bindings
    auto bindings = DatabaseManager::instance().getSceneBindings(sceneId);
    if (bindings.isEmpty()) {
        result.success = false;
        result.errorMessages.append("该场景未绑定任何设备");
        return result;
    }
    
    // 3. Check network
    if (!TcpManager::instance().isConnected()) {
        result.success = false;
        result.errorMessages.append("网络未连接，无法发送指令");
        return result;
    }
    
    result.totalCommands = bindings.size();
    
    // 4. Execute commands
    for (const auto &binding : bindings) {
        if (binding.deviceIdentifier.isEmpty()) {
            result.failCount++;
            result.failedDevices.append(binding.deviceName.isEmpty() ? "Unknown Device" : binding.deviceName);
            result.errorMessages.append(QString("设备 %1 标识符缺失").arg(binding.deviceName));
            continue;
        }
        
        QString cmd = generateCommand(binding);
        if (cmd.isEmpty()) {
            result.failCount++;
            result.failedDevices.append(binding.deviceName);
            result.errorMessages.append(QString("无法生成设备 %1 的控制指令").arg(binding.deviceName));
            continue;
        }
        
        // Send command
        // Note: TcpManager::sendCommand is void and async. 
        // We assume success if connected. Ideally we should wait for ACK, 
        // but for this simplified executor we fire and forget or assume optimistic success.
        // However, we should try to catch immediate errors if TcpManager exposes them.
        // Given current TcpManager implementation, we just call sendCommand.
        TcpManager::instance().sendCommand(cmd);
        result.successCount++;
    }
    
    // 5. Update last activated time
    if (result.successCount > 0) {
        // We can update the DB directly via QSqlQuery or add a method in DatabaseManager
        // For simplicity, let's just log it or ignore updating the struct in memory for now
        // Ideally: DatabaseManager::instance().updateSceneLastActivated(sceneId);
        // But prompt didn't strictly require a separate method for this, just "update scene".
        // Let's leave it for now or do a quick raw query update if strict.
        // Actually, let's do it properly via a raw query here to save time adding method to DM
        QSqlQuery query;
        query.prepare("UPDATE scenes SET last_activated = :time WHERE id = :id");
        query.bindValue(":time", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        query.bindValue(":id", sceneId);
        query.exec();
        
        // Trigger status refresh
        TcpManager::instance().sendCommand("GET_ALL_STATUS");
    }
    
    if (result.failCount > 0) {
        result.success = false; // Partial failure is considered not fully successful
    }
    
    return result;
}

QString SceneExecutor::generateCommand(const DatabaseManager::SceneDeviceBinding &binding)
{
    QString id = binding.deviceIdentifier.toUpper();
    QString state = binding.targetState;
    QString val = binding.targetValue;
    
    // LIGHT: LIGHT_ID_ON / OFF
    if (binding.deviceType == "LIGHT") {
        if (state == "ON" || state == "OFF") {
            return QString("%1_%2").arg(id).arg(state);
        }
    }
    // AC: AC_ID_ON / OFF / TEMP_26 / MODE_COOL / FAN_AUTO
    else if (binding.deviceType == "AC") {
        if (state == "ON" || state == "OFF") {
            return QString("%1_%2").arg(id).arg(state);
        } else if (state == "TEMP") {
            return QString("%1_TEMP_%2").arg(id).arg(val);
        } else if (state == "MODE") {
            // val is "COOL", "HEAT", etc.
            return QString("%1_MODE_%2").arg(id).arg(val);
        } else if (state == "FAN") {
            // val is "AUTO", "LOW", etc.
            return QString("%1_FAN_%2").arg(id).arg(val);
        }
    }
    // CURTAIN: CURTAIN_ID_OPEN / CLOSE / STOP / PERCENT_50 (If supported)
    else if (binding.deviceType == "CURTAIN") {
        if (state == "OPEN" || state == "CLOSE" || state == "STOP") {
            return QString("%1_%2").arg(id).arg(state);
        }
        // If percentage: CURTAIN_LIVING_50 (Hypothetical, simulator supports simple open/close for now)
    }
    
    return QString();
}
