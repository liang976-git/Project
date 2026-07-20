#include "PathPlanner.h"
#include "GeoFence.h"
#include <cmath>
#include <queue>
#include <functional>
#include <QMap>

static double toRad(double deg) {
    return deg * M_PI / 180.0;
}

static double haversineMeters(double lat1, double lng1, double lat2, double lng2) {
    double dLat = toRad(lat2 - lat1);
    double dLng = toRad(lng2 - lng1);
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(toRad(lat1)) * cos(toRad(lat2)) *
               sin(dLng / 2) * sin(dLng / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return 6371000.0 * c;
}

static double heuristicMeters(double lat1, double lng1, double lat2, double lng2) {
    return haversineMeters(lat1, lng1, lat2, lng2);
}

struct GridNode {
    int row;
    int col;
    double gCost;
    double fCost;
    bool operator>(const GridNode &other) const {
        return fCost > other.fCost;
    }
};

static bool isBlocked(int row, int col,
                      const QList<GeoFenceZone> &fences,
                      double minLat, double minLng,
                      double cellSizeLat, double cellSizeLng) {
    double cellCenterLat = minLat + (row + 0.5) * cellSizeLat;
    double cellCenterLng = minLng + (col + 0.5) * cellSizeLng;

    for (const auto &fence : fences) {
        if (isInFence(cellCenterLat, cellCenterLng, fence)) {
            return true;
        }
    }
    return false;
}

static double metersToLatDegrees(double meters) {
    return meters / 111320.0;
}

static double metersToLngDegrees(double meters, double refLat) {
    return meters / (111320.0 * cos(toRad(refLat)));
}

QList<QPair<double, double>> aStarSearch(
    double startLat, double startLng,
    double goalLat, double goalLng,
    const QList<GeoFenceZone> &fences,
    const GridConfig &grid) {

    double cellSizeM = grid.resolutionMeters;
    double cellSizeLat = metersToLatDegrees(cellSizeM);
    double cellSizeLng = metersToLngDegrees(cellSizeM, (grid.minLat + grid.maxLat) / 2.0);

    int totalRows = (int)((grid.maxLat - grid.minLat) / cellSizeLat) + 1;
    int totalCols = (int)((grid.maxLng - grid.minLng) / cellSizeLng) + 1;

    int startRow = (int)((startLat - grid.minLat) / cellSizeLat);
    int startCol = (int)((startLng - grid.minLng) / cellSizeLng);
    int goalRow = (int)((goalLat - grid.minLat) / cellSizeLat);
    int goalCol = (int)((goalLng - grid.minLng) / cellSizeLng);

    startRow = std::max(0, std::min(totalRows - 1, startRow));
    startCol = std::max(0, std::min(totalCols - 1, startCol));
    goalRow = std::max(0, std::min(totalRows - 1, goalRow));
    goalCol = std::max(0, std::min(totalCols - 1, goalCol));

    int dRow[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dCol[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    double dCost[] = {1.414, 1.0, 1.414, 1.0, 1.0, 1.414, 1.0, 1.414};

    auto key = [](int r, int c) -> qint64 {
        return ((qint64)r << 32) | (qint64)c;
    };

    QMap<qint64, qint64> cameFrom;
    QMap<qint64, double> gScore;

    std::priority_queue<GridNode, std::vector<GridNode>, std::greater<GridNode>> openSet;

    qint64 startKey = key(startRow, startCol);
    double startH = heuristicMeters(
        startLat, startLng,
        grid.minLat + goalRow * cellSizeLat + cellSizeLat / 2,
        grid.minLng + goalCol * cellSizeLng + cellSizeLng / 2);

    gScore[startKey] = 0.0;
    openSet.push({startRow, startCol, 0.0, startH});

    QMap<qint64, bool> closedSet;

    while (!openSet.empty()) {
        GridNode current = openSet.top();
        openSet.pop();

        qint64 currentKey = key(current.row, current.col);

        if (current.row == goalRow && current.col == goalCol) {
            QList<QPair<double, double>> path;
            qint64 ck = currentKey;
            while (cameFrom.contains(ck)) {
                int r = (int)(ck >> 32);
                int c = (int)(ck & 0xFFFFFFFF);
                double lat = grid.minLat + (r + 0.5) * cellSizeLat;
                double lng = grid.minLng + (c + 0.5) * cellSizeLng;
                path.prepend({lat, lng});
                ck = cameFrom[ck];
            }
            double slat = grid.minLat + (startRow + 0.5) * cellSizeLat;
            double slng = grid.minLng + (startCol + 0.5) * cellSizeLng;
            path.prepend({slat, slng});
            return path;
        }

        if (closedSet.contains(currentKey)) continue;
        closedSet[currentKey] = true;

        for (int d = 0; d < 8; ++d) {
            int nr = current.row + dRow[d];
            int nc = current.col + dCol[d];

            if (nr < 0 || nr >= totalRows || nc < 0 || nc >= totalCols)
                continue;

            qint64 neighborKey = key(nr, nc);
            if (closedSet.contains(neighborKey)) continue;

            if (isBlocked(nr, nc, fences, grid.minLat, grid.minLng,
                          cellSizeLat, cellSizeLng))
                continue;

            double cellCenterLat = grid.minLat + (nr + 0.5) * cellSizeLat;
            double cellCenterLng = grid.minLng + (nc + 0.5) * cellSizeLng;

            double moveCost = dCost[d] * cellSizeM;
            double tentativeG = current.gCost + moveCost;

            if (!gScore.contains(neighborKey) || tentativeG < gScore[neighborKey]) {
                gScore[neighborKey] = tentativeG;
                cameFrom[neighborKey] = currentKey;

                double h = heuristicMeters(
                    cellCenterLat, cellCenterLng,
                    grid.minLat + goalRow * cellSizeLat + cellSizeLat / 2,
                    grid.minLng + goalCol * cellSizeLng + cellSizeLng / 2);

                openSet.push({nr, nc, tentativeG, tentativeG + h});
            }
        }
    }

    return QList<QPair<double, double>>();
}

FlightPath generateFlightPath(
    const QList<QPair<double, double>> &routePoints,
    const QString &name,
    double defaultSpeed) {

    FlightPath path;
    path.name = name;
    path.createTime = QDateTime::currentDateTime();
    path.totalDistance = 0.0;

    for (int i = 0; i < routePoints.size(); ++i) {
        WayPoint wp;
        wp.latitude = routePoints[i].first;
        wp.longitude = routePoints[i].second;
        wp.altitude = 100.0;
        wp.speed = defaultSpeed;
        wp.hoverTime = 0.0;
        wp.action = 0;
        path.waypoints.append(wp);

        if (i > 0) {
            path.totalDistance += haversineMeters(
                routePoints[i - 1].first, routePoints[i - 1].second,
                routePoints[i].first, routePoints[i].second);
        }
    }

    path.estimatedTime = (defaultSpeed > 0.0) ? (path.totalDistance / defaultSpeed) : 0.0;

    return path;
}

bool checkPathFenceConflict(
    const QList<QPair<double, double>> &routePoints,
    const QList<GeoFenceZone> &fences,
    QList<int> &conflictedFenceIds) {

    bool hasConflict = false;

    for (int i = 0; i < routePoints.size(); ++i) {
        for (const auto &fence : fences) {
            if (!fence.enabled) continue;

            if (isInFence(routePoints[i].first, routePoints[i].second, fence)) {
                if (!conflictedFenceIds.contains(fence.id)) {
                    conflictedFenceIds.append(fence.id);
                }
                hasConflict = true;
            }
        }

        if (i < routePoints.size() - 1) {
            int segments = 5;
            for (int s = 1; s < segments; ++s) {
                double t = (double)s / segments;
                double interpLat = routePoints[i].first +
                    t * (routePoints[i + 1].first - routePoints[i].first);
                double interpLng = routePoints[i].second +
                    t * (routePoints[i + 1].second - routePoints[i].second);

                for (const auto &fence : fences) {
                    if (!fence.enabled) continue;
                    if (isInFence(interpLat, interpLng, fence)) {
                        if (!conflictedFenceIds.contains(fence.id)) {
                            conflictedFenceIds.append(fence.id);
                        }
                        hasConflict = true;
                    }
                }
            }
        }
    }

    return hasConflict;
}

QList<QPair<double, double>> autoAvoidFences(
    double startLat, double startLng,
    double goalLat, double goalLng,
    const QList<GeoFenceZone> &fences,
    const GridConfig &grid) {

    QList<int> directConflicts;
    QList<QPair<double, double>> directRoute = {{startLat, startLng}, {goalLat, goalLng}};

    if (!checkPathFenceConflict(directRoute, fences, directConflicts)) {
        return directRoute;
    }

    return aStarSearch(startLat, startLng, goalLat, goalLng, fences, grid);
}
