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
    auto scene = DatabaseManager::instance().getScene(sceneId);
    if (scene.id == -1)
    {
        result.success = false;
        result.errorMessages.append("场景不存在");
        return result;
    }

    if (!scene.enabled)
    {
        result.success = false;
        result.errorMessages.append("场景未启用");
        return result;
    }

    // 2. Check bindings
    auto bindings = DatabaseManager::instance().getSceneBindings(sceneId);
    if (bindings.isEmpty())
    {
        result.success = false;
        result.errorMessages.append("该场景未绑定任何设备");
        return result;
    }

    // 3. Check network
    // Allow local simulation if not connected, but warn
    // For now, let's just proceed and let TcpManager handle it (it might be in simulation mode)
    // if (!TcpManager::instance().isConnected()) { ... }

    result.totalCommands = bindings.size();

    // 4. Execute commands
    for (const auto &binding : bindings)
    {
        // Use deviceName or identifier if available.
        // In our struct, we have 'identifier' but it might be empty if not properly populated.
        // Fallback to constructing one or using device ID if protocol supports it.
        // For this project, let's assume 'identifier' is key (e.g. LIGHT_LIVING)

        QString identifier = binding.identifier;
        if (identifier.isEmpty())
        {
            // Try to infer from name or just skip
            // For robustness, let's skip and log error
            result.failCount++;
            result.failedDevices.append(binding.deviceName);
            result.errorMessages.append("设备标识符缺失: " + binding.deviceName);
            continue;
        }

        QString cmd = generateCommand(binding);
        if (cmd.isEmpty())
        {
            result.failCount++;
            result.failedDevices.append(binding.deviceName);
            result.errorMessages.append("无法生成指令: " + binding.deviceName);
            continue;
        }

        TcpManager::instance().sendCommand(cmd);
        result.successCount++;
    }

    if (result.failCount > 0)
    {
        result.success = false;
    }

    // 记录场景执行日志
    DatabaseManager::LogData log;
    log.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    log.user = "用户"; // 假设是用户手动触发
    log.deviceName = "场景: " + scene.name;
    log.action = "执行场景";

    if (result.success)
    {
        log.result = QString("成功 (执行%1个指令)").arg(result.successCount);
    }
    else
    {
        log.result = QString("失败 (成功%1/总%2): %3").arg(result.successCount).arg(result.totalCommands).arg(result.errorMessages.join("; "));
    }
    DatabaseManager::instance().addLog(log);

    return result;
}

QString SceneExecutor::generateCommand(const DatabaseManager::SceneDeviceBinding &binding)
{
    // Format: DEV_CMD:ID=LIGHT_LIVING,CMD=ON
    // Format: DEV_CMD:ID=AIR_LIVING,CMD=SET_TEMP,VAL=26

    QString cmd = "DEV_CMD:ID=" + binding.identifier + ",CMD=" + binding.targetState;
    if (!binding.targetValue.isEmpty())
    {
        cmd += ",VAL=" + binding.targetValue;
    }
    return cmd;
}
