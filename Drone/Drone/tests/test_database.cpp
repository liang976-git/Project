#include <QCoreApplication>
#include <QDebug>
#include <QSqlQuery>
#include "src/database/DatabaseManager.h"
#include "src/database/DroneDAO.h"
#include "src/database/FlightLogDAO.h"
#include "src/database/GeoFenceDAO.h"

void testDroneDAO() {
    qDebug() << "=== DroneDAO 测试 ===";

    DroneInfo d;
    d.id = 1;
    d.name = "测试无人机A";
    d.model = "DJI M300";
    d.type = DroneType::MultiRotor;
    d.status = DroneStatus::Flying;
    d.latitude = 39.9;
    d.longitude = 116.4;
    d.altitude = 120;
    d.speed = 15;
    d.verticalSpeed = 2;
    d.heading = 90;
    d.batteryPercent = 85;
    d.batteryVoltage = 23.5;
    d.signalStr = 90;
    d.lastHeartbeat = "2026-07-16T10:00:00";

    bool ok = DroneDAO::instance().insert(d);
    qDebug() << "insert:" << ok;

    DroneInfo got = DroneDAO::instance().findById(1);
    qDebug() << "findById:" << got.name << got.latitude;

    d.status = DroneStatus::LowBattery;
    d.batteryPercent = 15;
    ok = DroneDAO::instance().update(d);
    qDebug() << "update:" << ok;

    QList<DroneInfo> all = DroneDAO::instance().findAll();
    qDebug() << "findAll count:" << all.count();

    QList<DroneInfo> filtered = DroneDAO::instance().findByStatus(DroneStatus::LowBattery);
    qDebug() << "findByStatus count:" << filtered.count();

    ok = DroneDAO::instance().remove(1);
    qDebug() << "remove:" << ok;

    qDebug() << "";
}

void testFlightLogDAO() {
    qDebug() << "=== FlightLogDAO 测试 ===";

    FlightLog log;
    log.id = 1;
    log.droneId = 10;
    log.droneName = "测试无人机B";
    log.startTime = QDateTime::fromString("2026-07-16T10:00:00", Qt::ISODate);
    log.endTime = QDateTime::fromString("2026-07-16T10:30:00", Qt::ISODate);
    log.totalDistance = 5000;
    log.maxSpeed = 20;
    log.maxAltitude = 150;
    log.batteryUsed = 30;
    log.status = "completed";

    bool ok = FlightLogDAO::instance().insertLog(log);
    qDebug() << "insertLog:" << ok;

    FlightLogPoint p1;
    p1.seq = 1;
    p1.latitude = 39.9;
    p1.longitude = 116.4;
    p1.altitude = 100;
    p1.speed = 12;
    p1.timestamp = "2026-07-16T10:05:00";
    FlightLogDAO::instance().insertPoint(1, p1);

    FlightLogPoint p2;
    p2.seq = 2;
    p2.latitude = 39.91;
    p2.longitude = 116.41;
    p2.altitude = 120;
    p2.speed = 15;
    p2.timestamp = "2026-07-16T10:10:00";
    FlightLogDAO::instance().insertPoint(1, p2);
    qDebug() << "insertPoint: done";

    FlightLog got = FlightLogDAO::instance().findById(1);
    qDebug() << "findById:" << got.droneName << got.totalDistance;

    QList<FlightLog> byDrone = FlightLogDAO::instance().findByDroneId(10);
    qDebug() << "findByDroneId count:" << byDrone.count();

    QList<FlightLog> byTime = FlightLogDAO::instance().findByTimeRange(
        "2026-07-16T00:00:00", "2026-07-16T23:59:59");
    qDebug() << "findByTimeRange count:" << byTime.count();

    QList<FlightLogPoint> points = FlightLogDAO::instance().findPointsByLogId(1);
    qDebug() << "findPointsByLogId count:" << points.count();

    ok = FlightLogDAO::instance().removeLog(1);
    qDebug() << "removeLog:" << ok;

    qDebug() << "";
}

void testGeoFenceDAO() {
    qDebug() << "=== GeoFenceDAO 测试 ===";

    GeoFenceZone z;
    z.id = 1;
    z.name = "禁飞区A";
    z.type = Circle;
    z.enabled = true;
    z.regulation = "民用机场净空区";
    z.centerLat = 40.0;
    z.centerLng = 116.0;
    z.radius = 5000;

    bool ok = GeoFenceDAO::instance().insert(z);
    qDebug() << "insert:" << ok;

    GeoFenceZone got = GeoFenceDAO::instance().findById(1);
    qDebug() << "findById:" << got.name << got.radius;

    z.enabled = false;
    ok = GeoFenceDAO::instance().update(z);
    qDebug() << "update:" << ok;

    QList<GeoFenceZone> enabled = GeoFenceDAO::instance().findEnabled();
    qDebug() << "findEnabled count:" << enabled.count();

    QList<GeoFenceZone> all = GeoFenceDAO::instance().findAll();
    qDebug() << "findAll count:" << all.count();

    ok = GeoFenceDAO::instance().remove(1);
    qDebug() << "remove:" << ok;

    qDebug() << "";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    DatabaseManager::instance().initialize("test_drone.db");

    testDroneDAO();
    testFlightLogDAO();
    testGeoFenceDAO();

    DatabaseManager::instance().close();
    qDebug() << "=== 所有测试完成 ===";

    return 0;
}
