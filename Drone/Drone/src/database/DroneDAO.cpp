#include "src/database/DroneDAO.h"
#include "src/database/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DroneDAO &DroneDAO::instance(){
    static DroneDAO dao;
    return dao;
}
DroneInfo DroneDAO::fromQuery(const QSqlQuery &query) const{
    DroneInfo d;
    d.id=query.value("id").toInt();
    d.name=query.value("name").toString();
    d.model=query.value("model").toString();
    d.type=static_cast<DroneType>(query.value("type").toInt());
    d.status=static_cast<DroneStatus>(query.value("status").toInt());
    d.latitude=query.value("latitude").toDouble();
    d.longitude=query.value("longitude").toDouble();
    d.altitude=query.value("altitude").toDouble();
    d.speed=query.value("speed").toDouble();
    d.verticalSpeed=query.value("vertical_speed").toDouble();
    d.heading=query.value("heading").toDouble();
    d.batteryPercent=query.value("battery_percent").toInt();
    d.batteryVoltage=query.value("battery_voltage").toDouble();
    d.signalStr=query.value("signal_strength").toInt();
    d.lastHeartbeat=query.value("last_heartbeat").toString();
    return d;
}
bool DroneDAO::insert(const DroneInfo &drone){
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("insert into drones(id,name,model,type,status,latitude,longitude,altitude,speed,vertical_speed,heading,battery_percent,battery_voltage,signal_strength,last_heartbeat)"
                  "values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(drone.id);
    query.addBindValue(drone.name);
    query.addBindValue(drone.model);
    query.addBindValue(static_cast<int>(drone.type));
    query.addBindValue(static_cast<int>(drone.status));
    query.addBindValue(drone.latitude);
    query.addBindValue(drone.longitude);
    query.addBindValue(drone.altitude);
    query.addBindValue(drone.speed);
    query.addBindValue(drone.verticalSpeed);
    query.addBindValue(drone.heading);
    query.addBindValue(drone.batteryPercent);
    query.addBindValue(drone.batteryVoltage);
    query.addBindValue(drone.signalStr);
    query.addBindValue(drone.lastHeartbeat);
     if(!query.exec()){
         qDebug()<<"DroneDAO insert error:"<<query.lastError().text();
         return false;
     }
     return true;
}
bool DroneDAO::update(const DroneInfo &drone){
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("update drones set name=?,model=?,type=?,status=?,latitude=?,longitude=?,"
                  "altitude=?,speed=?,vertical_speed=?,heading=?,battery_percent=?,battery_voltage=?,signal_strength=?,last_heartbeat=?"
                  " where id=?");
    query.addBindValue(drone.name);
    query.addBindValue(drone.model);
    query.addBindValue(static_cast<int>(drone.type));
    query.addBindValue(static_cast<int>(drone.status));
    query.addBindValue(drone.latitude);
    query.addBindValue(drone.longitude);
    query.addBindValue(drone.altitude);
    query.addBindValue(drone.speed);
    query.addBindValue(drone.verticalSpeed);
    query.addBindValue(drone.heading);
    query.addBindValue(drone.batteryPercent);
    query.addBindValue(drone.batteryVoltage);
    query.addBindValue(drone.signalStr);
    query.addBindValue(drone.lastHeartbeat);
    query.addBindValue(drone.id);
    if(!query.exec()){
        qDebug()<<"DroneDAO update error:"<<query.lastError().text();
        return false;
    }
    return true;
}
bool DroneDAO::remove(int droneId){
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("delete from drones where id=?");
    query.addBindValue(droneId);
    if(!query.exec()){
        qDebug()<<"DroneDAO remove error:"<<query.lastError().text();
        return false;
    }
    return true;
}
DroneInfo DroneDAO::findById(int droneId) const{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("select * from drones where id=?");
    query.addBindValue(droneId);
    query.exec();
    if(query.next()){
        return fromQuery(query);
    }
    return DroneInfo{0,"","",DroneType::MultiRotor,DroneStatus::Idle,0,0,0,0,0,0,0,0,0,0,""};
}
QList<DroneInfo> DroneDAO::findAll() const{
    QList<DroneInfo> list;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("select * from drones");
    while(query.next()){
        list.append(fromQuery(query));
    }
    return list;
}
QList<DroneInfo> DroneDAO::findByStatus(DroneStatus status) const{
    QList<DroneInfo> list;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("select * from drones where status=?");
    query.addBindValue(static_cast<int>(status));
    query.exec();
    while(query.next()){
        list.append(fromQuery(query));
    }
     return list;
}
