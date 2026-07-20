#include "src/protocol/MavlinkParser.h"
#include <cstdint>
#include <cstring>

MavlinkParser::MavlinkParser(QObject *parent) : QObject(parent), m_seq(0) {}

uint8_t MavlinkParser::calculateChecksum(const QByteArray &data) {
    uint8_t checksum = 0;
    for (int i = 0; i < data.size(); ++i) {
        checksum ^= static_cast<uint8_t>(data[i]);
    }
    return checksum;
}

QByteArray MavlinkParser::buildFrame(uint8_t sysId, uint8_t msgId, const QByteArray &payload) {
    QByteArray frame;
    frame.append(static_cast<char>(STX));
    frame.append(static_cast<char>(payload.size()));
    frame.append(static_cast<char>(m_seq++));
    frame.append(static_cast<char>(sysId));
    frame.append(static_cast<char>(1));
    frame.append(static_cast<char>(msgId));
    frame.append(payload);
    uint8_t cka = calculateChecksum(frame);
    uint8_t ckb = static_cast<uint8_t>(~cka & 0xFF);
    frame.append(static_cast<char>(cka));
    frame.append(static_cast<char>(ckb));
    return frame;
}

QByteArray MavlinkParser::encodeHeartbeat(uint8_t sysId) {
    QByteArray payload(10, 0);
    return buildFrame(sysId, MSG_HEARTBEAT, payload);
}

QByteArray MavlinkParser::encodeGlobalPosition(const DroneInfo *drone) {
    if (!drone) return QByteArray();
    QByteArray payload;
    int32_t lat = static_cast<int32_t>(drone->latitude * 1e7);
    payload.append(reinterpret_cast<const char*>(&lat), 4);
    int32_t lon = static_cast<int32_t>(drone->longitude * 1e7);
    payload.append(reinterpret_cast<const char*>(&lon), 4);
    int32_t alt = static_cast<int32_t>(drone->altitude * 1000);
    payload.append(reinterpret_cast<const char*>(&alt), 4);
    uint16_t yaw = static_cast<uint16_t>(drone->heading * 100);
    payload.append(reinterpret_cast<const char*>(&yaw), 2);
    int16_t vx = static_cast<int16_t>(drone->speed * 100);
    payload.append(reinterpret_cast<const char*>(&vx), 2);
    return buildFrame(drone->id, MSG_GLOBAL_POS, payload);
}

QByteArray MavlinkParser::encodeVfrHud(const DroneInfo &drone) {
    QByteArray payload;
    int32_t airspeed = static_cast<int32_t>(drone.speed * 100);
    payload.append(reinterpret_cast<const char*>(&airspeed), 4);
    int32_t groundspeed = static_cast<int32_t>(drone.speed * 100);
    payload.append(reinterpret_cast<const char*>(&groundspeed), 4);
    uint16_t heading = static_cast<uint16_t>(drone.heading * 100);
    payload.append(reinterpret_cast<const char*>(&heading), 2);
    int16_t throttle = static_cast<int16_t>(drone.batteryPercent);
    payload.append(reinterpret_cast<const char*>(&throttle), 2);
    int32_t alt = static_cast<int32_t>(drone.altitude * 1000);
    payload.append(reinterpret_cast<const char*>(&alt), 4);
    int16_t climb = static_cast<int16_t>(drone.verticalSpeed * 100);
    payload.append(reinterpret_cast<const char*>(&climb), 2);
    return buildFrame(drone.id, MSG_VFR_HUD, payload);
}

QByteArray MavlinkParser::encodeSysStatus(const DroneInfo &drone) {
    QByteArray payload;
    uint16_t batteryRemaining = static_cast<uint16_t>(drone.batteryPercent);
    payload.append(reinterpret_cast<const char*>(&batteryRemaining), 2);
    int16_t voltage = static_cast<int16_t>(drone.batteryVoltage * 10);
    payload.append(reinterpret_cast<const char*>(&voltage), 2);
    int16_t current = static_cast<int16_t>(0);
    payload.append(reinterpret_cast<const char*>(&current), 2);
    int16_t signal = static_cast<int16_t>(drone.signalStr);
    payload.append(reinterpret_cast<const char*>(&signal), 2);
    return buildFrame(drone.id, MSG_SYS_STATUS, payload);
}

DroneInfo MavlinkParser::decodeGlobalPosition(const QByteArray &data) {
    DroneInfo drone;
    drone.id = 0;
    if (data.size() < 22) return drone;
    int32_t lat, lon, alt;
    uint16_t yaw;
    int16_t vx;
    memcpy(&lat, data.constData() + 6, 4);
    memcpy(&lon, data.constData() + 10, 4);
    memcpy(&alt, data.constData() + 14, 4);
    memcpy(&yaw, data.constData() + 18, 2);
    memcpy(&vx, data.constData() + 20, 2);
    drone.latitude = lat / 1e7;
    drone.longitude = lon / 1e7;
    drone.altitude = alt / 1000.0;
    drone.heading = yaw / 100.0;
    drone.speed = vx / 100.0;
    return drone;
}

DroneInfo MavlinkParser::decodeSysStatus(const QByteArray &data, DroneInfo &drone) {
    if (data.size() < 8) return drone;
    uint16_t batteryRemaining;
    int16_t signal;
    memcpy(&batteryRemaining, data.constData() + 6, 2);
    memcpy(&signal, data.constData() + 12, 2);
    drone.batteryPercent = static_cast<int>(batteryRemaining);
    drone.signalStr = static_cast<int>(signal);
    return drone;
}
