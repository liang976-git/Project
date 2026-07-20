#ifndef SIMULATEDLINK_H
#define SIMULATEDLINK_H

#include <QObject>
#include <QTimer>
#include <QList>
#include "src/model/DroneInfo.h"

class SimulatedLink:public QObject
{
    Q_OBJECT

public:
    explicit SimulatedLink(QObject *parent = nullptr);
    void startSimulation(int droneCount=5);
    void stopSimulation();
signals:
    void telemetryReceived(const DroneInfo &info);
private slots:
    void generateTelemetry();
private:
    void initDrones(int count);
    QTimer *m_timer;
    QList<DroneInfo> m_drones;
    int m_tick;
};

#endif // SIMULATEDLINK_H
