#include "databasemanager.h"
#include <QCoreApplication>
#include <QDir>

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
    // Ensure the database driver is available
    if (QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        // Set database name/path
        QString dbPath = QCoreApplication::applicationDirPath() + "/smart_home.db";
        m_db.setDatabaseName(dbPath);
        qDebug() << "Database path:" << dbPath;
    }
    else
    {
        qDebug() << "SQLite driver not available!";
    }
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::openDatabase()
{
    if (m_db.isOpen())
    {
        return true;
    }
    if (m_db.open())
    {
        qDebug() << "Database opened successfully.";
        return initTables();
    }
    else
    {
        qDebug() << "Error opening database:" << m_db.lastError().text();
        return false;
    }
}

void DatabaseManager::closeDatabase()
{
    if (m_db.isOpen())
    {
        m_db.close();
        qDebug() << "Database closed.";
    }
}

bool DatabaseManager::initTables()
{
    QSqlQuery query;
    // Example: Create User table
    bool success = query.exec("CREATE TABLE IF NOT EXISTS users ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "username TEXT UNIQUE NOT NULL, "
                              "password TEXT NOT NULL)");
    if (!success)
    {
        qDebug() << "Failed to create users table:" << query.lastError().text();
        return false;
    }

    // Create Device table
    success = query.exec("CREATE TABLE IF NOT EXISTS devices ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "name TEXT NOT NULL, "
                         "type TEXT NOT NULL, "
                         "status INTEGER DEFAULT 0, "
                         "params TEXT)"); // params can store JSON or formatted string
    if (!success)
    {
        qDebug() << "Failed to create devices table:" << query.lastError().text();
        return false;
    }

    // Create History/Log table
    success = query.exec("CREATE TABLE IF NOT EXISTS logs ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                         "user TEXT, "
                         "device_name TEXT, "
                         "action TEXT, "
                         "result TEXT)");
    if (!success)
    {
        qDebug() << "Failed to create logs table:" << query.lastError().text();
        return false;
    }

    // --- RESET SCENE TABLES (Requested by User) ---
    // Drop old tables to ensure schema update
    query.exec("DROP TABLE IF EXISTS scenes");
    query.exec("DROP TABLE IF EXISTS scene_devices"); // Old name
    query.exec("DROP TABLE IF EXISTS scene_device_bindings"); // New name

    // Create Scenes table
    success = query.exec("CREATE TABLE IF NOT EXISTS scenes ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "name TEXT NOT NULL UNIQUE, "
                         "description TEXT, "
                         "icon TEXT, "
                         "is_enabled INTEGER DEFAULT 1, "
                         "last_activated DATETIME)");
    if (!success) {
        qDebug() << "Failed to create scenes table:" << query.lastError().text();
        return false;
    }

    // Create Scene_Device_Bindings table
    success = query.exec("CREATE TABLE IF NOT EXISTS scene_device_bindings ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "scene_id INTEGER NOT NULL, "
                         "device_id INTEGER NOT NULL, "
                         "target_state TEXT, "
                         "target_value TEXT, "
                         "FOREIGN KEY(scene_id) REFERENCES scenes(id) ON DELETE CASCADE)");
    if (!success) {
        qDebug() << "Failed to create scene_device_bindings table:" << query.lastError().text();
        return false;
    }

    return true;
}

QList<DatabaseManager::SceneData> DatabaseManager::getAllScenes()
{
    QList<SceneData> list;
    QSqlQuery query("SELECT id, name, description, icon, is_enabled, last_activated FROM scenes ORDER BY id DESC");
    while (query.next())
    {
        SceneData d;
        d.id = query.value(0).toInt();
        d.name = query.value(1).toString();
        d.description = query.value(2).toString();
        d.icon = query.value(3).toString();
        d.isEnabled = query.value(4).toBool();
        d.lastActivated = query.value(5).toString();
        list.append(d);
    }
    return list;
}

DatabaseManager::SceneData DatabaseManager::getSceneById(int id)
{
    SceneData d;
    d.id = -1; // Invalid
    QSqlQuery query;
    query.prepare("SELECT id, name, description, icon, is_enabled, last_activated FROM scenes WHERE id = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next())
    {
        d.id = query.value(0).toInt();
        d.name = query.value(1).toString();
        d.description = query.value(2).toString();
        d.icon = query.value(3).toString();
        d.isEnabled = query.value(4).toBool();
        d.lastActivated = query.value(5).toString();
    }
    return d;
}

bool DatabaseManager::addScene(const SceneData &scene, const QList<SceneDeviceBinding> &bindings)
{
    m_db.transaction();
    QSqlQuery query;
    query.prepare("INSERT INTO scenes (name, description, icon, is_enabled) VALUES (:name, :desc, :icon, :enabled)");
    query.bindValue(":name", scene.name);
    query.bindValue(":desc", scene.description);
    query.bindValue(":icon", scene.icon);
    query.bindValue(":enabled", scene.isEnabled ? 1 : 0);

    if (!query.exec())
    {
        qDebug() << "Add scene failed:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

    int sceneId = query.lastInsertId().toInt();

    for (const auto &bind : bindings)
    {
        QSqlQuery bindQuery;
        bindQuery.prepare("INSERT INTO scene_device_bindings (scene_id, device_id, target_state, target_value) "
                          "VALUES (:sid, :did, :state, :val)");
        bindQuery.bindValue(":sid", sceneId);
        bindQuery.bindValue(":did", bind.deviceId);
        bindQuery.bindValue(":state", bind.targetState);
        bindQuery.bindValue(":val", bind.targetValue);
        if (!bindQuery.exec())
        {
            qDebug() << "Add scene binding failed:" << bindQuery.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    return m_db.commit();
}

bool DatabaseManager::updateScene(const SceneData &scene, const QList<SceneDeviceBinding> &bindings)
{
    m_db.transaction();
    QSqlQuery query;
    query.prepare("UPDATE scenes SET name = :name, description = :desc, is_enabled = :enabled WHERE id = :id");
    query.bindValue(":name", scene.name);
    query.bindValue(":desc", scene.description);
    query.bindValue(":enabled", scene.isEnabled ? 1 : 0);
    query.bindValue(":id", scene.id);

    if (!query.exec())
    {
        qDebug() << "Update scene failed:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

    // Clear old bindings
    QSqlQuery delQuery;
    delQuery.prepare("DELETE FROM scene_device_bindings WHERE scene_id = :id");
    delQuery.bindValue(":id", scene.id);
    if (!delQuery.exec())
    {
        qDebug() << "Delete old bindings failed:" << delQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    // Add new bindings
    for (const auto &bind : bindings)
    {
        QSqlQuery bindQuery;
        bindQuery.prepare("INSERT INTO scene_device_bindings (scene_id, device_id, target_state, target_value) "
                          "VALUES (:sid, :did, :state, :val)");
        bindQuery.bindValue(":sid", scene.id);
        bindQuery.bindValue(":did", bind.deviceId);
        bindQuery.bindValue(":state", bind.targetState);
        bindQuery.bindValue(":val", bind.targetValue);
        if (!bindQuery.exec())
        {
            qDebug() << "Add scene binding failed:" << bindQuery.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    return m_db.commit();
}

bool DatabaseManager::deleteScene(int sceneId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM scenes WHERE id = :id");
    query.bindValue(":id", sceneId);

    // Manually delete bindings first
    QSqlQuery bindQuery;
    bindQuery.prepare("DELETE FROM scene_device_bindings WHERE scene_id = :id");
    bindQuery.bindValue(":id", sceneId);
    bindQuery.exec();

    return query.exec();
}

QList<DatabaseManager::SceneDeviceBinding> DatabaseManager::getSceneBindings(int sceneId)
{
    QList<SceneDeviceBinding> list;
    QSqlQuery query;
    query.prepare("SELECT sd.id, sd.scene_id, sd.device_id, d.name, d.identifier, d.type, sd.target_state, sd.target_value "
                  "FROM scene_device_bindings sd "
                  "LEFT JOIN devices d ON sd.device_id = d.id "
                  "WHERE sd.scene_id = :sid");
    query.bindValue(":sid", sceneId);
    if (query.exec())
    {
        while (query.next())
        {
            SceneDeviceBinding d;
            d.id = query.value(0).toInt();
            d.sceneId = query.value(1).toInt();
            d.deviceId = query.value(2).toInt();
            d.deviceName = query.value(3).toString();
            d.deviceIdentifier = query.value(4).toString();
            d.deviceType = query.value(5).toString();
            d.targetState = query.value(6).toString();
            d.targetValue = query.value(7).toString();
            list.append(d);
        }
    }
    return list;
}

bool DatabaseManager::checkSceneNameExists(const QString &name, int excludeId)
{
    QSqlQuery query;
    if (excludeId == -1)
    {
        query.prepare("SELECT COUNT(*) FROM scenes WHERE name = :name");
        query.bindValue(":name", name);
    }
    else
    {
        query.prepare("SELECT COUNT(*) FROM scenes WHERE name = :name AND id != :id");
        query.bindValue(":name", name);
        query.bindValue(":id", excludeId);
    }
    if (query.exec() && query.next())
    {
        return query.value(0).toInt() > 0;
    }
    return false;
}

QList<DatabaseManager::DeviceData> DatabaseManager::getAllDevices()
{
    QList<DeviceData> list;
    QSqlQuery query("SELECT id, name, identifier, type, status FROM devices");
    while (query.next())
    {
        DeviceData d;
        d.id = query.value(0).toInt();
        d.name = query.value(1).toString();
        d.identifier = query.value(2).toString();
        d.type = query.value(3).toString();
        d.status = query.value(4).toString();
        list.append(d);
    }
    return list;
}
