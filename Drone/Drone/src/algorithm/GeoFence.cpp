#include "GeoFence.h"
#include <cmath>
#include <QDebug>

const double EARTH_RADIUS = 6371000.0;

static double toRad(double deg) {
    return deg * M_PI / 180.0;
}

double haversineDistance(double lat1, double lng1, double lat2, double lng2) {
    double dLat = toRad(lat2 - lat1);
    double dLng = toRad(lng2 - lng1);
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(toRad(lat1)) * cos(toRad(lat2)) *
               sin(dLng / 2) * sin(dLng / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return EARTH_RADIUS * c;
}

bool pointInPolygon(double lat, double lng, const QList<QPair<double, double>> &polygon) {
    if (polygon.size() < 3) return false;

    int count = polygon.size();
    bool inside = false;
    for (int i = 0, j = count - 1; i < count; j = i++) {
        double xi = polygon[i].first, yi = polygon[i].second;
        double xj = polygon[j].first, yj = polygon[j].second;

        if (((yi > lng) != (yj > lng)) &&
            (lat < (xj - xi) * (lng - yi) / (yj - yi) + xi)) {
            inside = !inside;
        }
    }
    return inside;
}

bool pointInCircle(double lat, double lng, double centerLat, double centerLng, double radiusMeters) {
    double dist = haversineDistance(lat, lng, centerLat, centerLng);
    return dist <= radiusMeters;
}

bool pointInRectangle(double lat, double lng, const QList<QPair<double, double>> &rect) {
    if (rect.size() < 4) return false;

    double minLat = rect[0].first, maxLat = rect[0].first;
    double minLng = rect[0].second, maxLng = rect[0].second;
    for (const auto &p : rect) {
        if (p.first < minLat) minLat = p.first;
        if (p.first > maxLat) maxLat = p.first;
        if (p.second < minLng) minLng = p.second;
        if (p.second > maxLng) maxLng = p.second;
    }
    return (lat >= minLat && lat <= maxLat && lng >= minLng && lng <= maxLng);
}

bool isInFence(double lat, double lng, const GeoFenceZone &zone) {
    if (!zone.enabled) return false;

    switch (zone.type) {
    case Circle:
        return pointInCircle(lat, lng, zone.centerLat, zone.centerLng, zone.radius);
    case Rectangle:
        return pointInRectangle(lat, lng, zone.points);
    case Polygon:
        return pointInPolygon(lat, lng, zone.points);
    }
    return false;
}

bool isNearFence(double lat, double lng, const GeoFenceZone &zone, double bufferMeters) {
    if (!zone.enabled) return false;

    switch (zone.type) {
    case Circle:
        return pointInCircle(lat, lng, zone.centerLat, zone.centerLng, zone.radius + bufferMeters);
    case Rectangle:
    case Polygon: {
        for (const auto &p : zone.points) {
            double dist = haversineDistance(lat, lng, p.first, p.second);
            if (dist <= bufferMeters) return true;
        }
        return isInFence(lat, lng, zone);
    }
    }
    return false;
}
