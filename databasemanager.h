#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager &instance();

    struct SceneData
    {
        int id;
        QString name;
        QString description;
        bool enabled;
        // Could store icon path, trigger type, etc.
    };

    struct SceneDeviceBinding
    {
        int id;
        int sceneId;
        int deviceId;
        QString deviceName;  // Join from Device table
        QString deviceType;  // Join from Device table
        QString identifier;  // Join from Device table (e.g. LIGHT_LIVING)
        QString targetState; // ON/OFF/OPEN/CLOSE/SET_TEMP/SET_MODE
        QString targetValue; // For temp or mode (e.g. 25 or COOL)
        int delayMs;         // Execution delay
    };

    bool openDatabase();
    void closeDatabase();
    bool initTables(); // Create necessary tables if not exist

    // Scene Management
    QList<SceneData> getAllScenes();
    QList<SceneDeviceBinding> getSceneBindings(int sceneId);
    bool addScene(const SceneData &scene, const QList<SceneDeviceBinding> &bindings);
    bool updateScene(const SceneData &scene, const QList<SceneDeviceBinding> &bindings);
    bool deleteScene(int sceneId);
    SceneData getScene(int sceneId);

    // Device Management
    struct DeviceData
    {
        int id;
        QString name;
        QString type;
        int status;
        QString params;
    };
    QList<DeviceData> getAllDevices();

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
