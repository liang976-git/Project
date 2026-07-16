#include "src/core/DroneManager.h"

DroneManager::DroneManager(QObject *parent)
    : QObject(parent)
{
}

void DroneManager::registerDrone(const DroneInfo &info)
{
    m_drone.insert(info.id, info);
    emit droneAdded(info.id);
}

void DroneManager::unregisterDrone(int droneId)
{
    if (m_drone.remove(droneId) > 0) {
        emit droneRemoved(droneId);
    }
}

QList<DroneInfo> DroneManager::getAllDrones() const
{
    return m_drone.values();
}

QList<DroneInfo> DroneManager::getDronesByStatus(DroneStatus status) const
{
    QList<DroneInfo> result;
    for (const DroneInfo &drone : m_drone) {
        if (drone.status == status) {
            result.append(drone);
        }
    }
    return result;
}

void DroneManager::updateDroneStatus(int droneId, const DroneInfo &info)
{
    if (!m_drone.contains(droneId)) {
        m_drone.insert(droneId, info);
        emit droneAdded(droneId);
        return;
    }

    DroneStatus oldStatus = m_drone[droneId].status;
    m_drone[droneId] = info;

    if (oldStatus != info.status) {
        emit droneStatusChanged(droneId, info.status);
    }

    if (info.batteryPercent < 20) {
        emit alarmTriggered(droneId, QString("无人机 %1 电量过低: %2%").arg(info.name).arg(info.batteryPercent));
    }

    if (info.signalStr < 10) {
        emit alarmTriggered(droneId, QString("无人机 %1 信号弱: %2%").arg(info.name).arg(info.signalStr));
    }
}

int DroneManager::droneCount() const
{
    return m_drone.size();
}
