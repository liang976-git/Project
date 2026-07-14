#ifndef DRONEINFO_H
#define DRONEINFO_H

#include <QString>
#include <cstdint>

enum class DroneType{
    MultiRotor,//多旋翼
    FixedWing,//固定翼
    VTOL//垂直起降
};

enum class DroneStatus{
    Idle,      //空闲
    Flying,    //飞行中
    Returning, //返航中
    LowBattery,//电量低
    Disconnected,//失联
    Error       //故障
};

struct DroneInfo{
    int id;//唯一标识
    QString name;//无人机名称
    QString model;//型号
    DroneType type;//类型
    DroneStatus status;//当前状态
    double latitude;//纬度（WGS84）
    double longitude;//经度
    double altitude;//高度(m)
    double speed;//地速(m/s)
    double verticalSpeed;//垂直速度
    double heading;//航向角(0-360)
    int batteryPercent;//电量百分比
    double batteryVoltage;//电池电压(V)
    int signalStr;//信号强度(%)
    uint64_t timestamp;//时间戳
    QString lastHeartbeat;//最后心跳时间
};
#endif //DRONEINFO_H
