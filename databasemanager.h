#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QList>
#include "scenemodels.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager &instance();
    bool openDatabase();
    void closeDatabase();
    bool initTables(); // Create necessary tables if not exist

    // Scene Management
    QList<SceneInfo> getAllScenes();
    SceneInfo getSceneById(int id);
    bool addScene(SceneInfo &scene); // Updates ID
    bool updateScene(const SceneInfo &scene);
    bool deleteScene(int id);
    
    // Scene Binding Management
    QList<SceneDeviceBinding> getSceneBindings(int sceneId);
    bool saveSceneBindings(int sceneId, const QList<SceneDeviceBinding> &bindings);
    bool clearSceneBindings(int sceneId);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    QSqlDatabase m_db;
    void initBuiltinScenes(); // Check and insert default scenes
};

#endif // DATABASEMANAGER_H
