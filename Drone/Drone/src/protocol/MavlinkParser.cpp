#include "src/protocol/MavlinkParser.h"
#include <cstdint>
MavlinkParser::MavlinkParser(QObject *parent):QObject(parent),m_seq(0){}
    uint8_t MavlinkParser::calculateChecksum(const QByteArray &data){
        uint8_t checksum=0;
        for(int i=0;i<data.size();++i){
            checksum ^=static_cast<uint8_t>(data[i]);

        }
        return checksum;
    }
    QByteArray MavlinkParser::buildFrame(uint8_t sysId, uint8_t msgId, const QByteArray &payload){
        QByteArray frame;
        frame.append(static_cast<char>(STX));
        frame.append(static_cast<char>(payload.size()));
        frame.append(static_cast<char>(m_seq++));
        frame.append(static_cast<char>(sysId));
        frame.append(static_cast<char>(1));
        frame.append(static_cast<char>(msgId));
        frame.append(payload);
        uint8_t cka=calculateChecksum(frame);
        uint8_t ckb=static_cast<uint8_t>(~cka & 0xFF);
        frame.append(static_cast<char>(cka));
        frame.append(static_cast<char>(ckb));
        return frame;
    }
    QByteArray MavlinkParser::encodeHeartbeat(uint8_t sysId){
        QByteArray payload(10,0);
        return buildFrame(sysId,MSG_HEARTBEAT,payload);

    }
    QByteArray MavlinkParser::encodeGlobalPosition(const DroneInfo *drone){
        if(!drone) return QByteArray();
        QByteArray payload;
        int32_t lat=static_cast<int32_t>(drone->latitude * 1e7);
        payload.append(reinterpret_cast<const char*>(&lat),4);
        int32_t lon=static_cast<int32_t>(drone->longitude * 1e7);
        payload.append(reinterpret_cast<const char*>(&lon),4);
        int32_t alt=static_cast<int32_t>(drone->altitude * 1000);
        payload.append(reinterpret_cast<const char*>(&alt),4);
        uint16_t yaw=static_cast<uint16_t>(drone->heading * 100);
        payload.append(reinterpret_cast<const char*>(&yaw),2);
        int16_t vx=static_cast<int16_t>(drone->speed * 100);
        payload.append(reinterpret_cast<const char*>(&vx),2);
        return buildFrame(drone->id,MSG_GLOBAL_POS,payload);
    }
