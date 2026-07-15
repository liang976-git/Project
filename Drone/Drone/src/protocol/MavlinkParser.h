#ifndef MAVLINKPARSER_H
#define MAVLINKPARSER_H

#include <QObject>
#include <QByteArray>
#include "src/protocol/MavlinkMessage.h"
#include "src/model/DroneInfo.h"

class MavlinkParser : public QObject{
    Q_OBJECT
public:
    explicit MavlinkParser(QObject *parent=nullptr);
    QByteArray encodeHeartbeat(uint8_t sysId);
    QByteArray encodeGlobalPosition(const DroneInfo *drone);
    QByteArray encodeVfrHud(const DroneInfo &drone);
    QByteArray encodeSysStatus(const DroneInfo &drone);
    DroneInfo decodeGlobalPosition(const QByteArray &data);
    DroneInfo decodeSysStatus(const QByteArray &data,DroneInfo &drone);
private:
    uint8_t calculateChecksum(const QByteArray &data);
    QByteArray buildFrame(uint8_t sysId,uint8_t msgId,const QByteArray &payload);
    uint8_t m_seq;
};


#endif MAVLINKPARSER_H
