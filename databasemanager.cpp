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

    return true;
}
