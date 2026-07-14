#ifndef FLIGHTLOG_H
#define FLIGHTLOG_H

#include <QString>
#include <cstdint>
#include <QDateTime>
#include <QList>
#include <QPair>

struct FlightLog {
    int id;
    int droneId;
    QString droneName;
    QDateTime startTime;
    QDateTime endTime;
    QList<QPair<double,double>> trajectory;
    double totalDistance;
    double maxSpeed;
    double maxAltitude;
    int batteryUsed;
    QString status;
};

#endif // FLIGHTLOG_H
