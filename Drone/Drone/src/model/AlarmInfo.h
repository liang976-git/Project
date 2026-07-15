#ifndef ALARMINFO_H
#define ALARMINFO_H

#include <QString>
#include <QDateTime>
enum AlarmType{
    LowBattery,
    SignalLost,
    GeoFenceBreach,
    MotorError,
    ConnectionLost
};
enum AlarmLevel{
    Info,
    Warning,
    Critical,
    Emergency
};
struct AlarmInfo{
    int id;//警告ID
    int droneId;//关联的无人机ID
    AlarmType type;//警告类型
    AlarmLevel level;//警告级别
    QString message;//警告内容
    QDateTime time;//警告时间
    bool confirmed;//是否已确认
};

#endif // ALARMINFO_H
