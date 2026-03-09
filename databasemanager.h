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

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
