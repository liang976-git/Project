#include "src/database/FlightLogDAO.h"
#include "src/database/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

FlightLogDAO &FlightLogDAO::instance(){
    static FlightLogDAO dao;
    return dao;
}
FlightLog FlightLogDAO::fromQuery(const QSqlQuery &query) const{
    FlightLog log;
    log.id=query.value("id").toInt();
    log.droneId=query.value("drone_id").toInt();
    log.droneName=query.value("drone_name").toString();
    log.startTime=query.value("start_time").toDateTime();
    log.endTime=query.value("end_time").toDateTime();
    log.totalDistance=query.value("total_distance").toDouble();
    log.maxSpeed=query.value("max_speed").toDouble();
    log.maxAltitude=query.value("max_altitude").toDouble();
    log.batteryUsed=query.value("battery_used").toInt();
    log.status=query.value("status").toString();
    return log;
}
FlightLogPoint FlightLogDAO::fromPointQuery(const QSqlQuery &query) const{
    FlightLogPoint p;
    p.seq=query.value("seq").toInt();
    p.latitude=query.value("latitude").toDouble();
    p.longitude=query.value("longitude").toDouble();
    p.altitude=query.value("altitude").toDouble();
    p.speed=query.value("speed").toDouble();
    p.timestamp=query.value("timestamp").toString();
    return p;
}
bool FlightLogDAO::insertLog(FlightLog &log){
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("insert into flight_logs(id,drone_id,drone_name,start_time,end_time,"
                  "total_distance,max_speed,max_altitude,battery_used,status)"
                  "values(?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(log.id);
    query.addBindValue(log.droneId);
    query.addBindValue(log.droneName);
    query.addBindValue(log.startTime.toString(Qt::ISODate));
    query.addBindValue(log.endTime.toString(Qt::ISODate));
    query.addBindValue(log.totalDistance);
    query.addBindValue(log.maxSpeed);
    query.addBindValue(log.maxAltitude);
    query.addBindValue(log.batteryUsed);
    query.addBindValue(log.status);
    if(!query.exec()){
        qDebug()<<"FlightLogDAO insertLog error:"<<query.lastError().text();
        return false;
    }
    return true;
}
bool FlightLogDAO::insertPoint(int logId,const FlightLogPoint &point){
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("insert into flight_log_points(log_id,seq,latitude,longitude,altitude,speed,timestamp)"
                  "values(?,?,?,?,?,?,?)");
    query.addBindValue(logId);
    query.addBindValue(point.seq);
    query.addBindValue(point.latitude);
    query.addBindValue(point.longitude);
    query.addBindValue(point.altitude);
    query.addBindValue(point.speed);
    query.addBindValue(point.timestamp);
    if(!query.exec()){
        qDebug()<<"FlightLogDAO insertPoint error:"<<query.lastError().text();
        return false;
    }
    return true;
}
bool FlightLogDAO::removeLog(int logId){
    QSqlDatabase db = DatabaseManager::instance().database();
    db.transaction();
    QSqlQuery query(db);
    query.prepare("delete from flight_log_points where log_id=?");
    query.addBindValue(logId);
    if(!query.exec()){
        qDebug()<<"FlightLogDAO removeLog points error:"<<query.lastError().text();
        db.rollback();
        return false;
    }
    query.prepare("delete from flight_logs where id=?");
    query.addBindValue(logId);
    if(!query.exec()){
        qDebug()<<"FlightLogDAO removeLog error:"<<query.lastError().text();
        db.rollback();
        return false;
    }
    db.commit();
    return true;
}
FlightLog FlightLogDAO::findById(int logId) const{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("select * from flight_logs where id=?");
    query.addBindValue(logId);
    query.exec();
    if(query.next()){
        return fromQuery(query);
    }
    return FlightLog{0,0,"",QDateTime(),QDateTime(),QList<QPair<double,double>>(),0,0,0,0,""};
}
QList<FlightLog> FlightLogDAO::findByDroneId(int droneId) const{
    QList<FlightLog> list;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("select * from flight_logs where drone_id=?");
    query.addBindValue(droneId);
    query.exec();
    while(query.next()){
        list.append(fromQuery(query));
    }
    return list;
}
QList<FlightLog> FlightLogDAO::findByTimeRange(const QString &start,const QString &end) const{
    QList<FlightLog> list;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("select * from flight_logs where start_time>=? and start_time<=?");
    query.addBindValue(start);
    query.addBindValue(end);
    query.exec();
    while(query.next()){
        list.append(fromQuery(query));
    }
    return list;
}
QList<FlightLogPoint> FlightLogDAO::findPointsByLogId(int logId) const{
    QList<FlightLogPoint> list;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("select * from flight_log_points where log_id=? order by seq");
    query.addBindValue(logId);
    query.exec();
    while(query.next()){
        list.append(fromPointQuery(query));
    }
    return list;
}
