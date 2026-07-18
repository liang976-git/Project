#ifndef GEOFENCE_H
#define GEOFENCE_H

#include <QPair>
#include <QList>
#include "src/model/GeoFenceZone.h"

double haversineDistance(double lat1, double lng1, double lat2, double lng2);

bool pointInPolygon(double lat, double lng, const QList<QPair<double, double>> &polygon);

bool pointInCircle(double lat, double lng, double centerLat, double centerLng, double radiusMeters);

bool pointInRectangle(double lat, double lng, const QList<QPair<double, double>> &rect);

bool isInFence(double lat, double lng, const GeoFenceZone &zone);

bool isNearFence(double lat, double lng, const GeoFenceZone &zone, double bufferMeters);

#endif
