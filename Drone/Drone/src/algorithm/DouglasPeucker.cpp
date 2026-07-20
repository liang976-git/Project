#include "DouglasPeucker.h"
#include <cmath>
#include <algorithm>

static double toRad(double deg) {
    return deg * M_PI / 180.0;
}

double pointToSegmentDistanceMeters(double lat, double lng,
                                    double lat1, double lng1,
                                    double lat2, double lng2) {
    double dLat1 = lat - lat1;
    double dLng1 = lng - lng1;
    double dLat2 = lat2 - lat1;
    double dLng2 = lng2 - lng1;

    double dot = dLat1 * dLat2 + dLng1 * dLng2;
    double lenSq = dLat2 * dLat2 + dLng2 * dLng2;

    double param = (lenSq < 1e-12) ? 0.0 : dot / lenSq;
    param = std::max(0.0, std::min(1.0, param));

    double projLat = lat1 + param * dLat2;
    double projLng = lng1 + param * dLng2;

    double dLat = toRad(lat - projLat);
    double dLng = toRad(lng - projLng);
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(toRad(projLat)) * cos(toRad(lat)) *
               sin(dLng / 2) * sin(dLng / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return 6371000.0 * c;
}

static double maxPointToSegmentDistance(const QList<WayPoint> &points,
                                        int start, int end,
                                        int &farthestIndex) {
    double maxDist = 0.0;
    farthestIndex = start;

    double lat1 = points[start].latitude;
    double lng1 = points[start].longitude;
    double lat2 = points[end].latitude;
    double lng2 = points[end].longitude;

    for (int i = start + 1; i < end; ++i) {
        double dist = pointToSegmentDistanceMeters(
            points[i].latitude, points[i].longitude,
            lat1, lng1, lat2, lng2);
        if (dist > maxDist) {
            maxDist = dist;
            farthestIndex = i;
        }
    }
    return maxDist;
}

static void dpSimplify(const QList<WayPoint> &points,
                        int start, int end,
                        double epsilon,
                        QList<WayPoint> &result) {
    if (end - start <= 1) return;

    int farthestIndex = 0;
    double maxDist = maxPointToSegmentDistance(points, start, end, farthestIndex);

    if (maxDist > epsilon) {
        dpSimplify(points, start, farthestIndex, epsilon, result);
        dpSimplify(points, farthestIndex, end, epsilon, result);
    } else {
        result.append(points[end]);
    }
}

QList<WayPoint> douglasPeuckerSimplify(const QList<WayPoint> &waypoints,
                                       double epsilonMeters) {
    if (waypoints.size() <= 2) return waypoints;

    QList<WayPoint> result;
    result.append(waypoints.first());

    dpSimplify(waypoints, 0, waypoints.size() - 1, epsilonMeters, result);

    return result;
}
