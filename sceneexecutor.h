#ifndef SCENEEXECUTOR_H
#define SCENEEXECUTOR_H

#include <QObject>
#include <QStringList>
#include "databasemanager.h"

class SceneExecutor : public QObject
{
    Q_OBJECT
public:
    struct ExecutionResult {
        bool success;
        int totalCommands;
        int successCount;
        int failCount;
        QStringList failedDevices;
        QStringList errorMessages;
    };

    explicit SceneExecutor(QObject *parent = nullptr);
    
    // Execute a scene by ID
    ExecutionResult execute(int sceneId);

private:
    QString generateCommand(const DatabaseManager::SceneDeviceBinding &binding);
};

#endif // SCENEEXECUTOR_H
