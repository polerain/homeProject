#include "databasemanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QVariant>

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
        bool ret = initTables();
        if (ret)
            initBuiltinScenes();
        return ret;
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

    // Create Scenes table
    success = query.exec("CREATE TABLE IF NOT EXISTS scenes ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "name TEXT NOT NULL, "
                         "description TEXT, "
                         "is_builtin INTEGER DEFAULT 0, "
                         "is_enabled INTEGER DEFAULT 1, "
                         "icon TEXT, "
                         "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
                         "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP)");
    if (!success)
    {
        qDebug() << "Failed to create scenes table:" << query.lastError().text();
        return false;
    }

    // Create Scene Bindings table
    success = query.exec("CREATE TABLE IF NOT EXISTS scene_bindings ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "scene_id INTEGER NOT NULL, "
                         "device_id TEXT NOT NULL, "
                         "device_name TEXT, "
                         "device_type TEXT NOT NULL, "
                         "is_selected INTEGER DEFAULT 1, "
                         "target_power TEXT, "
                         "target_temp INTEGER, "
                         "target_mode TEXT, "
                         "target_fan TEXT, "
                         "target_brightness INTEGER, "
                         "target_light_mode TEXT, "
                         "target_position TEXT, "
                         "sort_order INTEGER DEFAULT 0)");
    if (!success)
    {
        qDebug() << "Failed to create scene_bindings table:" << query.lastError().text();
        return false;
    }

    auto addColumnIfMissing = [&](const QString &table, const QString &columnName, const QString &columnDef) -> bool
    {
        QSqlQuery colQuery;
        colQuery.exec(QString("PRAGMA table_info(%1)").arg(table));
        while (colQuery.next())
        {
            if (colQuery.value("name").toString().compare(columnName, Qt::CaseInsensitive) == 0)
            {
                return true;
            }
        }

        QSqlQuery alterQuery;
        if (!alterQuery.exec(QString("ALTER TABLE %1 ADD COLUMN %2").arg(table, columnDef)))
        {
            qDebug() << "Failed to add column" << table << columnName << ":" << alterQuery.lastError().text();
            return false;
        }
        return true;
    };

    if (!addColumnIfMissing("scenes", "is_builtin", "is_builtin INTEGER DEFAULT 0"))
        return false;
    if (!addColumnIfMissing("scenes", "is_enabled", "is_enabled INTEGER DEFAULT 1"))
        return false;
    if (!addColumnIfMissing("scenes", "icon", "icon TEXT"))
        return false;
    if (!addColumnIfMissing("scenes", "created_at", "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"))
        return false;
    if (!addColumnIfMissing("scenes", "updated_at", "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"))
        return false;

    if (!addColumnIfMissing("scene_bindings", "device_name", "device_name TEXT"))
        return false;
    if (!addColumnIfMissing("scene_bindings", "device_type", "device_type TEXT"))
        return false;
    if (!addColumnIfMissing("scene_bindings", "is_selected", "is_selected INTEGER DEFAULT 1"))
        return false;
    if (!addColumnIfMissing("scene_bindings", "target_power", "target_power TEXT"))
        return false;
    if (!addColumnIfMissing("scene_bindings", "target_temp", "target_temp INTEGER"))
        return false;
    if (!addColumnIfMissing("scene_bindings", "target_mode", "target_mode TEXT"))
        return false;
    if (!addColumnIfMissing("scene_bindings", "target_fan", "target_fan TEXT"))
        return false;
    if (!addColumnIfMissing("scene_bindings", "target_brightness", "target_brightness INTEGER"))
        return false;
    if (!addColumnIfMissing("scene_bindings", "target_light_mode", "target_light_mode TEXT"))
        return false;
    if (!addColumnIfMissing("scene_bindings", "target_position", "target_position TEXT"))
        return false;
    if (!addColumnIfMissing("scene_bindings", "sort_order", "sort_order INTEGER DEFAULT 0"))
        return false;

    return true;
}

void DatabaseManager::initBuiltinScenes()
{
    // Check if builtin scenes exist
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM scenes WHERE is_builtin = 1");
    if (query.next() && query.value(0).toInt() > 0)
    {
        return; // Already initialized
    }

    // Insert builtin scenes
    // 1. Home
    SceneInfo home;
    home.name = "回家模式";
    home.description = "开启客厅灯光和空调";
    home.isBuiltin = true;
    home.icon = "HOME";
    addScene(home);

    QList<SceneDeviceBinding> homeBindings;
    homeBindings.append({-1, home.id, "light_living", "客厅主灯", "LIGHT", true, "ON", -1, "", "", -1, "", ""});
    homeBindings.append({-1, home.id, "ac_living", "客厅空调", "AC", true, "ON", -1, "", "", -1, "", ""});
    homeBindings.append({-1, home.id, "light_bedroom", "卧室顶灯", "LIGHT", true, "ON", -1, "", "", -1, "", ""});
    homeBindings.append({-1, home.id, "light_kitchen", "厨房顶灯", "LIGHT", true, "ON", -1, "", "", -1, "", ""});
    saveSceneBindings(home.id, homeBindings);

    // 2. Away
    SceneInfo away;
    away.name = "离家模式";
    away.description = "关闭所有灯光和电器";
    away.isBuiltin = true;
    away.icon = "AWAY";
    addScene(away);

    QList<SceneDeviceBinding> awayBindings;
    awayBindings.append({-1, away.id, "light_living", "客厅主灯", "LIGHT", true, "OFF", -1, "", "", -1, "", ""});
    awayBindings.append({-1, away.id, "light_bedroom", "卧室顶灯", "LIGHT", true, "OFF", -1, "", "", -1, "", ""});
    awayBindings.append({-1, away.id, "ac_living", "客厅空调", "AC", true, "OFF", -1, "", "", -1, "", ""});
    awayBindings.append({-1, away.id, "light_kitchen", "厨房顶灯", "LIGHT", true, "OFF", -1, "", "", -1, "", ""});
    awayBindings.append({-1, away.id, "light_restroom", "卫生间灯", "LIGHT", true, "OFF", -1, "", "", -1, "", ""});
    awayBindings.append({-1, away.id, "light_dining", "餐厅吊灯", "LIGHT", true, "OFF", -1, "", "", -1, "", ""});
    saveSceneBindings(away.id, awayBindings);

    // 3. Sleep
    SceneInfo sleep;
    sleep.name = "睡眠模式";
    sleep.description = "营造舒适睡眠环境";
    sleep.isBuiltin = true;
    sleep.icon = "SLEEP";
    addScene(sleep);

    QList<SceneDeviceBinding> sleepBindings;
    sleepBindings.append({-1, sleep.id, "light_living", "客厅主灯", "LIGHT", true, "OFF", -1, "", "", -1, "", ""});
    sleepBindings.append({-1, sleep.id, "light_bedroom", "卧室顶灯", "LIGHT", true, "OFF", -1, "", "", -1, "", ""});
    sleepBindings.append({-1, sleep.id, "ac_living", "客厅空调", "AC", true, "ON", 26, "COOL", "", -1, "", ""});
    saveSceneBindings(sleep.id, sleepBindings);

    // 4. Movie
    SceneInfo movie;
    movie.name = "观影模式";
    movie.description = "调暗灯光，关闭窗帘";
    movie.isBuiltin = true;
    movie.icon = "MOVIE";
    addScene(movie);

    QList<SceneDeviceBinding> movieBindings;
    movieBindings.append({-1, movie.id, "light_living", "客厅主灯", "LIGHT", true, "ON", -1, "", "", 20, "", ""});
    movieBindings.append({-1, movie.id, "curtain_living", "客厅窗帘", "CURTAIN", true, "", -1, "", "", -1, "", "CLOSE"});
    saveSceneBindings(movie.id, movieBindings);
}

QList<SceneInfo> DatabaseManager::getAllScenes()
{
    QList<SceneInfo> list;
    QSqlQuery query;
    // Order by is_builtin DESC (builtin first), then id ASC
    query.exec("SELECT id, name, description, is_builtin, is_enabled, icon FROM scenes ORDER BY is_builtin DESC, id ASC");
    while (query.next())
    {
        SceneInfo s;
        s.id = query.value(0).toInt();
        s.name = query.value(1).toString();
        s.description = query.value(2).toString();
        s.isBuiltin = query.value(3).toBool();
        s.isEnabled = query.value(4).toBool();
        s.icon = query.value(5).toString();
        list.append(s);
    }
    return list;
}

SceneInfo DatabaseManager::getSceneById(int id)
{
    SceneInfo s;
    QSqlQuery query;
    query.prepare("SELECT id, name, description, is_builtin, is_enabled, icon FROM scenes WHERE id = ?");
    query.addBindValue(id);
    if (query.exec() && query.next())
    {
        s.id = query.value(0).toInt();
        s.name = query.value(1).toString();
        s.description = query.value(2).toString();
        s.isBuiltin = query.value(3).toBool();
        s.isEnabled = query.value(4).toBool();
        s.icon = query.value(5).toString();
    }
    return s;
}

bool DatabaseManager::addScene(SceneInfo &scene)
{
    QSqlQuery query;
    query.prepare("INSERT INTO scenes (name, description, is_builtin, is_enabled, icon) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(scene.name);
    query.addBindValue(scene.description);
    query.addBindValue(scene.isBuiltin);
    query.addBindValue(scene.isEnabled);
    query.addBindValue(scene.icon);

    if (query.exec())
    {
        scene.id = query.lastInsertId().toInt();
        return true;
    }
    qDebug() << "Add scene failed:" << query.lastError().text();
    return false;
}

bool DatabaseManager::updateScene(const SceneInfo &scene)
{
    QSqlQuery query;
    query.prepare("UPDATE scenes SET name=?, description=?, is_enabled=?, icon=?, updated_at=CURRENT_TIMESTAMP WHERE id=?");
    query.addBindValue(scene.name);
    query.addBindValue(scene.description);
    query.addBindValue(scene.isEnabled);
    query.addBindValue(scene.icon);
    query.addBindValue(scene.id);

    return query.exec();
}

bool DatabaseManager::deleteScene(int id)
{
    // Delete bindings first
    clearSceneBindings(id);

    QSqlQuery query;
    query.prepare("DELETE FROM scenes WHERE id=?");
    query.addBindValue(id);
    return query.exec();
}

QList<SceneDeviceBinding> DatabaseManager::getSceneBindings(int sceneId)
{
    QList<SceneDeviceBinding> list;
    QSqlQuery query;
    query.prepare("SELECT * FROM scene_bindings WHERE scene_id = ? ORDER BY sort_order ASC");
    query.addBindValue(sceneId);

    if (query.exec())
    {
        while (query.next())
        {
            SceneDeviceBinding b;
            b.id = query.value("id").toInt();
            b.sceneId = query.value("scene_id").toInt();
            b.deviceId = query.value("device_id").toString();
            b.deviceName = query.value("device_name").toString();
            b.deviceType = query.value("device_type").toString();
            b.isSelected = query.value("is_selected").toBool();
            b.targetPower = query.value("target_power").toString();
            b.targetTemp = query.value("target_temp").toInt();
            b.targetMode = query.value("target_mode").toString();
            b.targetFan = query.value("target_fan").toString();
            b.targetBrightness = query.value("target_brightness").toInt();
            b.targetLightColor = query.value("target_light_mode").toString();
            b.targetPosition = query.value("target_position").toString();
            b.sortOrder = query.value("sort_order").toInt();

            list.append(b);
        }
    }
    return list;
}

bool DatabaseManager::saveSceneBindings(int sceneId, const QList<SceneDeviceBinding> &bindings)
{
    clearSceneBindings(sceneId);

    QSqlQuery query;
    bool success = true;

    // Batch insert could be better, but loop is simple for now
    m_db.transaction();

    for (const auto &b : bindings)
    {
        query.prepare("INSERT INTO scene_bindings (scene_id, device_id, device_name, device_type, is_selected, "
                      "target_power, target_temp, target_mode, target_fan, target_brightness, target_light_mode, target_position, sort_order) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(sceneId);
        query.addBindValue(b.deviceId);
        query.addBindValue(b.deviceName);
        query.addBindValue(b.deviceType);
        query.addBindValue(b.isSelected);
        query.addBindValue(b.targetPower);
        query.addBindValue(b.targetTemp);
        query.addBindValue(b.targetMode);
        query.addBindValue(b.targetFan);
        query.addBindValue(b.targetBrightness);
        query.addBindValue(b.targetLightColor);
        query.addBindValue(b.targetPosition);
        query.addBindValue(b.sortOrder);

        if (!query.exec())
        {
            qDebug() << "Insert binding failed:" << query.lastError().text();
            success = false;
            break;
        }
    }

    if (success)
        m_db.commit();
    else
        m_db.rollback();

    return success;
}

bool DatabaseManager::clearSceneBindings(int sceneId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM scene_bindings WHERE scene_id = ?");
    query.addBindValue(sceneId);
    return query.exec();
}
