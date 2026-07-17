#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager{
public:
    static DatabaseManager &instance();
    bool initialize(const QString &dbPath="drone.db");
    QSqlDatabase database() const;
    void close();
private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager &)=delete;
    DatabaseManager &operator=(const DatabaseManager &)=delete;
    bool createTables();
    QSqlDatabase m_db;
};


#endif //DATABASEMANAGER_H
