#ifndef GEOFENCEDAO_H
#define GEOFENCEDAO_H

#include <QList>
#include <QSqlQuery>
#include "src/model/GeoFenceZone.h"

class GeoFenceDAO {
public:
    static GeoFenceDAO &instance();

    bool insert(const GeoFenceZone &zone);
    bool update(const GeoFenceZone &zone);
    bool remove(int zoneId);
    GeoFenceZone findById(int zoneId) const;
    QList<GeoFenceZone> findAll() const;
    QList<GeoFenceZone> findEnabled() const;

private:
    GeoFenceDAO() = default;
    GeoFenceZone fromQuery(const QSqlQuery &query) const;
};

#endif // GEOFENCEDAO_H
