#include "src/database/GeoFenceDAO.h"
#include "src/database/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

GeoFenceDAO &GeoFenceDAO::instance() {
    static GeoFenceDAO dao;
    return dao;
}

GeoFenceZone GeoFenceDAO::fromQuery(const QSqlQuery &query) const {
    GeoFenceZone z;
    z.id = query.value("id").toInt();
    z.name = query.value("name").toString();
    z.type = static_cast<ZoneType>(query.value("type").toInt());
    z.enabled = query.value("enabled").toBool();
    z.regulation = query.value("regulation").toString();
    z.centerLat = query.value("center_lat").toDouble();
    z.centerLng = query.value("center_lng").toDouble();
    z.radius = query.value("radius").toDouble();

    QString pointsStr = query.value("points").toString();
    if (!pointsStr.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(pointsStr.toUtf8());
        QJsonArray arr = doc.array();
        for (const auto &v : arr) {
            QJsonObject obj = v.toObject();
            z.points.append(qMakePair(obj["lat"].toDouble(), obj["lng"].toDouble()));
        }
    }
    return z;
}

bool GeoFenceDAO::insert(const GeoFenceZone &zone) {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "INSERT INTO geofence_zones (id, name, type, enabled, regulation, center_lat, center_lng, radius, points) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );
    query.addBindValue(zone.id);
    query.addBindValue(zone.name);
    query.addBindValue(static_cast<int>(zone.type));
    query.addBindValue(zone.enabled);
    query.addBindValue(zone.regulation);
    query.addBindValue(zone.centerLat);
    query.addBindValue(zone.centerLng);
    query.addBindValue(zone.radius);

    QJsonArray arr;
    for (const auto &p : zone.points) {
        QJsonObject obj;
        obj["lat"] = p.first;
        obj["lng"] = p.second;
        arr.append(obj);
    }
    query.addBindValue(QJsonDocument(arr).toJson());

    if (!query.exec()) {
        qDebug() << "GeoFenceDAO insert error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool GeoFenceDAO::update(const GeoFenceZone &zone) {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "UPDATE geofence_zones SET name=?, type=?, enabled=?, regulation=?, "
        "center_lat=?, center_lng=?, radius=?, points=? WHERE id=?"
    );
    query.addBindValue(zone.name);
    query.addBindValue(static_cast<int>(zone.type));
    query.addBindValue(zone.enabled);
    query.addBindValue(zone.regulation);
    query.addBindValue(zone.centerLat);
    query.addBindValue(zone.centerLng);
    query.addBindValue(zone.radius);

    QJsonArray arr;
    for (const auto &p : zone.points) {
        QJsonObject obj;
        obj["lat"] = p.first;
        obj["lng"] = p.second;
        arr.append(obj);
    }
    query.addBindValue(QJsonDocument(arr).toJson());
    query.addBindValue(zone.id);

    if (!query.exec()) {
        qDebug() << "GeoFenceDAO update error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool GeoFenceDAO::remove(int zoneId) {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM geofence_zones WHERE id=?");
    query.addBindValue(zoneId);

    if (!query.exec()) {
        qDebug() << "GeoFenceDAO remove error:" << query.lastError().text();
        return false;
    }
    return true;
}

GeoFenceZone GeoFenceDAO::findById(int zoneId) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM geofence_zones WHERE id=?");
    query.addBindValue(zoneId);
    query.exec();

    if (query.next()) {
        return fromQuery(query);
    }
    return GeoFenceZone{0, "", Circle, false, "", 0, 0, 0, {}};
}

QList<GeoFenceZone> GeoFenceDAO::findAll() const {
    QList<GeoFenceZone> list;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT * FROM geofence_zones");
    while (query.next()) {
        list.append(fromQuery(query));
    }
    return list;
}

QList<GeoFenceZone> GeoFenceDAO::findEnabled() const {
    QList<GeoFenceZone> list;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT * FROM geofence_zones WHERE enabled=1");
    while (query.next()) {
        list.append(fromQuery(query));
    }
    return list;
}
