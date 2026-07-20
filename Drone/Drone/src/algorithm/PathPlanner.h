#ifndef PATHPLANNER_H
#define PATHPLANNER_H

#include <QList>
#include <QPair>
#include "src/model/FlightPath.h"
#include "src/model/GeoFenceZone.h"

struct GridConfig {
    double minLat;
    double minLng;
    double maxLat;
    double maxLng;
    double resolutionMeters;
};

QList<QPair<double, double>> aStarSearch(
    double startLat, double startLng,
    double goalLat, double goalLng,
    const QList<GeoFenceZone> &fences,
    const GridConfig &grid);

FlightPath generateFlightPath(
    const QList<QPair<double, double>> &routePoints,
    const QString &name,
    double defaultSpeed);

bool checkPathFenceConflict(
    const QList<QPair<double, double>> &routePoints,
    const QList<GeoFenceZone> &fences,
    QList<int> &conflictedFenceIds);

QList<QPair<double, double>> autoAvoidFences(
    double startLat, double startLng,
    double goalLat, double goalLng,
    const QList<GeoFenceZone> &fences,
    const GridConfig &grid);

#endif
