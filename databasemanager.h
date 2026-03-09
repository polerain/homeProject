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
    bool openDatabase();
    void closeDatabase();
    bool initTables(); // Create necessary tables if not exist

    // Scene related methods
    struct SceneData
    {
        int id;
        QString name;
        QString description;
        QString icon;
        bool isEnabled;
        QString lastActivated; // Timestamp
    };
    struct SceneDeviceBinding
    {
        int id;
        int sceneId;
        int deviceId;
        QString deviceName;
        QString deviceIdentifier; // e.g. "LIGHT_LIVING"
        QString deviceType;
        QString targetState; // "ON", "OFF", "OPEN", "CLOSE"
        QString targetValue; // "26", "COOL", "AUTO"
    };

    QList<SceneData> getAllScenes();
    SceneData getSceneById(int id);
    bool addScene(const SceneData &scene, const QList<SceneDeviceBinding> &bindings);
    bool updateScene(const SceneData &scene, const QList<SceneDeviceBinding> &bindings);
    bool deleteScene(int sceneId);
    QList<SceneDeviceBinding> getSceneBindings(int sceneId);
    bool checkSceneNameExists(const QString &name, int excludeId = -1);

    // Device related helper
    struct DeviceData
    {
        int id;
        QString name;
        QString identifier; // e.g. "LIGHT_LIVING"
        QString type;
        QString status;
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
