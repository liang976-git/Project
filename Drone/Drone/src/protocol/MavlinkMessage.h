#ifndef MAVLINKMESSAGE_H
#define MAVLINKMESSAGE_H

#include <cstdint>
#include <QByteArray>

struct MavlinkHeader{
    uint8_t stx;//起始标记 0xFD
    uint8_t len;//载荷长度
    uint8_t seq;//序列号
    uint8_t sysId;//系统ID
    uint8_t compId;//组件ID
    uint8_t msgId;//消息ID

};

const uint8_t MSG_HEARTBEAT=0;
const uint8_t MSG_SYS_STATUS=1;
const uint8_t MSG_GLOBAL_POS=33;
const uint8_t MSG_VFR_HUD=74;
const uint8_t STX=0XFD;

#endif MAVLINKMESSAGE_H
