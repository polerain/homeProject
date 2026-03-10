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

    // Create Scene table
    success = query.exec("CREATE TABLE IF NOT EXISTS scenes ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "name TEXT NOT NULL, "
                         "description TEXT, "
                         "enabled INTEGER DEFAULT 1)");
    if (!success)
    {
        qDebug() << "Failed to create scenes table:" << query.lastError().text();
        return false;
    }

    // Create Scene-Device Binding table
    success = query.exec("CREATE TABLE IF NOT EXISTS scene_bindings ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "scene_id INTEGER NOT NULL, "
                         "device_id INTEGER NOT NULL, "
                         "target_state TEXT, "
                         "target_value TEXT, "
                         "delay_ms INTEGER DEFAULT 0, "
                         "FOREIGN KEY(scene_id) REFERENCES scenes(id) ON DELETE CASCADE, "
                         "FOREIGN KEY(device_id) REFERENCES devices(id) ON DELETE CASCADE)");
    if (!success)
    {
        qDebug() << "Failed to create scene_bindings table:" << query.lastError().text();
        return false;
    }

    return true;
}

QList<DatabaseManager::SceneData> DatabaseManager::getAllScenes()
{
    QList<SceneData> list;
    if (!m_db.isOpen())
        return list;

    QSqlQuery query("SELECT id, name, description, enabled FROM scenes ORDER BY id DESC");
    while (query.next())
    {
        SceneData d;
        d.id = query.value(0).toInt();
        d.name = query.value(1).toString();
        d.description = query.value(2).toString();
        d.enabled = query.value(3).toBool();
        list.append(d);
    }
    return list;
}

QList<DatabaseManager::SceneDeviceBinding> DatabaseManager::getSceneBindings(int sceneId)
{
    QList<SceneDeviceBinding> list;
    if (!m_db.isOpen())
        return list;

    QSqlQuery query;
    query.prepare("SELECT sb.id, sb.scene_id, sb.device_id, d.name, d.type, d.params, " // Using params as identifier placeholder for now or add identifier column
                  "sb.target_state, sb.target_value, sb.delay_ms "
                  "FROM scene_bindings sb "
                  "JOIN devices d ON sb.device_id = d.id "
                  "WHERE sb.scene_id = ?");
    query.addBindValue(sceneId);

    if (query.exec())
    {
        while (query.next())
        {
            SceneDeviceBinding b;
            b.id = query.value(0).toInt();
            b.sceneId = query.value(1).toInt();
            b.deviceId = query.value(2).toInt();
            b.deviceName = query.value(3).toString();
            b.deviceType = query.value(4).toString();
            // Assuming params stores identifier or just use name as identifier
            // For robust system, 'identifier' should be a column in devices table.
            // Here we just use a placeholder or parse params if needed.
            b.identifier = query.value(5).toString();

            b.targetState = query.value(6).toString();
            b.targetValue = query.value(7).toString();
            b.delayMs = query.value(8).toInt();
            list.append(b);
        }
    }
    else
    {
        qDebug() << "getSceneBindings error:" << query.lastError().text();
    }
    return list;
}

bool DatabaseManager::addScene(const SceneData &scene, const QList<SceneDeviceBinding> &bindings)
{
    if (!m_db.isOpen())
        return false;

    m_db.transaction();

    QSqlQuery query;
    query.prepare("INSERT INTO scenes (name, description, enabled) VALUES (?, ?, ?)");
    query.addBindValue(scene.name);
    query.addBindValue(scene.description);
    query.addBindValue(scene.enabled ? 1 : 0);

    if (!query.exec())
    {
        qDebug() << "Add scene failed:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

    int newSceneId = query.lastInsertId().toInt();

    for (const auto &b : bindings)
    {
        QSqlQuery bindQuery;
        bindQuery.prepare("INSERT INTO scene_bindings (scene_id, device_id, target_state, target_value, delay_ms) "
                          "VALUES (?, ?, ?, ?, ?)");
        bindQuery.addBindValue(newSceneId);
        bindQuery.addBindValue(b.deviceId);
        bindQuery.addBindValue(b.targetState);
        bindQuery.addBindValue(b.targetValue);
        bindQuery.addBindValue(b.delayMs);

        if (!bindQuery.exec())
        {
            qDebug() << "Add binding failed:" << bindQuery.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    return m_db.commit();
}

bool DatabaseManager::updateScene(const SceneData &scene, const QList<SceneDeviceBinding> &bindings)
{
    if (!m_db.isOpen())
        return false;

    m_db.transaction();

    // Update Scene Info
    QSqlQuery query;
    query.prepare("UPDATE scenes SET name=?, description=?, enabled=? WHERE id=?");
    query.addBindValue(scene.name);
    query.addBindValue(scene.description);
    query.addBindValue(scene.enabled ? 1 : 0);
    query.addBindValue(scene.id);

    if (!query.exec())
    {
        qDebug() << "Update scene failed:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

    // Delete old bindings
    QSqlQuery delQuery;
    delQuery.prepare("DELETE FROM scene_bindings WHERE scene_id=?");
    delQuery.addBindValue(scene.id);
    if (!delQuery.exec())
    {
        qDebug() << "Delete old bindings failed:" << delQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    // Insert new bindings
    for (const auto &b : bindings)
    {
        QSqlQuery bindQuery;
        bindQuery.prepare("INSERT INTO scene_bindings (scene_id, device_id, target_state, target_value, delay_ms) "
                          "VALUES (?, ?, ?, ?, ?)");
        bindQuery.addBindValue(scene.id);
        bindQuery.addBindValue(b.deviceId);
        bindQuery.addBindValue(b.targetState);
        bindQuery.addBindValue(b.targetValue);
        bindQuery.addBindValue(b.delayMs);

        if (!bindQuery.exec())
        {
            qDebug() << "Insert new binding failed:" << bindQuery.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    return m_db.commit();
}

bool DatabaseManager::deleteScene(int sceneId)
{
    if (!m_db.isOpen())
        return false;

    // Bindings are deleted via CASCADE or manually
    // Since we used ON DELETE CASCADE in CREATE TABLE, deleting scene is enough.
    // However, if CASCADE isn't supported/enabled, manual delete is safer.

    QSqlQuery query;
    query.prepare("DELETE FROM scenes WHERE id = ?");
    query.addBindValue(sceneId);

    return query.exec();
}

DatabaseManager::SceneData DatabaseManager::getScene(int sceneId)
{
    SceneData d;
    d.id = -1;
    if (!m_db.isOpen())
        return d;

    QSqlQuery query;
    query.prepare("SELECT id, name, description, enabled FROM scenes WHERE id = ?");
    query.addBindValue(sceneId);

    if (query.exec() && query.next())
    {
        d.id = query.value(0).toInt();
        d.name = query.value(1).toString();
        d.description = query.value(2).toString();
        d.enabled = query.value(3).toBool();
    }
    return d;
}

QList<DatabaseManager::DeviceData> DatabaseManager::getAllDevices()
{
    QList<DeviceData> list;
    if (!m_db.isOpen())
        return list;

    QSqlQuery query("SELECT id, name, type, status, params FROM devices");
    while (query.next())
    {
        DeviceData d;
        d.id = query.value(0).toInt();
        d.name = query.value(1).toString();
        d.type = query.value(2).toString();
        d.status = query.value(3).toInt();
        d.params = query.value(4).toString();
        list.append(d);
    }
    return list;
}
