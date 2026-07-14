#ifndef DRONEMANAGER_H
#define DRONEMANAGER_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QList>
#include "src/model/DroneInfo.h"

class DroneManager : public QObject
{
    Q_OBJECT

public:
    explicit DroneManager(QObject *parent = nullptr);

    void registerDrone(const DroneInfo &info);
    void unregisterDrone(int droneId);
    DroneInfo getDrone(int droneId) const;
    QList<DroneInfo> getAllDrones() const;
    QList<DroneInfo> getDronesByStatus(DroneStatus status) const;
    void updateDroneStatus(int droneId, const DroneInfo &info);
    int droneCount() const;

signals:
    void droneStatusChanged(int droneId, DroneStatus status);
    void droneAdded(int droneId);
    void droneRemoved(int droneId);
    void alarmTriggered(int droneId, const QString &message);

private:
    QMap<int, DroneInfo> m_drones;
};

#endif // DRONEMANAGER_H
