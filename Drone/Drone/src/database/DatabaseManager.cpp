#include "src/database/DatabaseManager.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

DatabaseManager::DatabaseManager(){}
DatabaseManager::~DatabaseManager(){close();}
DatabaseManager &DatabaseManager::instance(){
    static DatabaseManager db;
    return db;
}
bool DatabaseManager::initialize(const QString &dbPath){
    m_db=QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    if(!m_db.open()){
        qDebug()<<"数据库打开失败:"<<m_db.lastError().text();
        return false;
    }
    qDebug()<<"数据库连接成功"<<dbPath;
    //开启外键约束
    QSqlQuery query(m_db);
    query.exec("PRAGMA foreign_keys = ON");
    if(!createTables()){
        qDebug()<<"建表失败";
        return false;
    }
    return true;

}
bool DatabaseManager::createTables(){
    QSqlQuery query(m_db);

    // 1. 无人机表
    query.exec(
        "CREATE TABLE IF NOT EXISTS drones ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "model TEXT,"
        "type INTEGER,"
        "status INTEGER,"
        "latitude REAL,"
        "longitude REAL,"
        "altitude REAL,"
        "speed REAL,"
        "vertical_speed REAL,"
        "heading REAL,"
        "battery_percent INTEGER,"
        "battery_voltage REAL,"
        "signal_strength INTEGER,"
        "last_heartbeat TEXT"
        ")"
    );

    // 2. 飞行路径表
    query.exec(
        "CREATE TABLE IF NOT EXISTS flight_paths ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "create_time TEXT,"
        "total_distance REAL,"
        "estimated_time REAL"
        ")"
    );

    // 3. 航点表
    query.exec(
        "CREATE TABLE IF NOT EXISTS waypoints ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "path_id INTEGER,"
        "seq INTEGER,"
        "latitude REAL,"
        "longitude REAL,"
        "altitude REAL,"
        "speed REAL,"
        "hover_time REAL,"
        "action INTEGER,"
        "FOREIGN KEY(path_id) REFERENCES flight_paths(id)"
        ")"
    );

    // 4. 禁飞区表
    query.exec(
        "CREATE TABLE IF NOT EXISTS geofence_zones ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "type INTEGER,"
        "enabled INTEGER,"
        "regulation TEXT,"
        "center_lat REAL,"
        "center_lng REAL,"
        "radius REAL,"
        "points TEXT"
        ")"
    );

    // 5. 飞行日志表
    query.exec(
        "CREATE TABLE IF NOT EXISTS flight_logs ("
        "id INTEGER PRIMARY KEY,"
        "drone_id INTEGER,"
        "drone_name TEXT,"
        "start_time TEXT,"
        "end_time TEXT,"
        "total_distance REAL,"
        "max_speed REAL,"
        "max_altitude REAL,"
        "battery_used INTEGER,"
        "status TEXT"
        ")"
    );

    // 6. 日志轨迹点表
    query.exec(
        "CREATE TABLE IF NOT EXISTS flight_log_points ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "log_id INTEGER,"
        "seq INTEGER,"
        "latitude REAL,"
        "longitude REAL,"
        "altitude REAL,"
        "speed REAL,"
        "timestamp TEXT,"
        "FOREIGN KEY(log_id) REFERENCES flight_logs(id)"
        ")"
    );

    // 7. 告警表
    query.exec(
        "CREATE TABLE IF NOT EXISTS alarms ("
        "id INTEGER PRIMARY KEY,"
        "drone_id INTEGER,"
        "type INTEGER,"
        "level INTEGER,"
        "message TEXT,"
        "time TEXT,"
        "confirmed INTEGER"
        ")"
    );

    qDebug() << "建表完成";
    return true;
}
QSqlDatabase DatabaseManager::database() const{
    return m_db;
}
void DatabaseManager::close(){
    if(m_db.isOpen()){
        m_db.close();
        qDebug()<<"数据库连接已关闭";
    }
}
