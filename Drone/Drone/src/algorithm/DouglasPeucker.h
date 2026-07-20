#ifndef DOUGLASPEUCKER_H
#define DOUGLASPEUCKER_H

#include <QList>
#include "src/model/FlightPath.h"

double pointToSegmentDistanceMeters(double lat, double lng,
                                    double lat1, double lng1,
                                    double lat2, double lng2);

QList<WayPoint> douglasPeuckerSimplify(const QList<WayPoint> &waypoints,
                                       double epsilonMeters);

#endif
