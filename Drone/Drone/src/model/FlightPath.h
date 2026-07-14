#ifndef FLIGHTPATH_H
#define FLIGHTPATH_H

#include <QString>
#include <cstdint>
#include <QDateTime>
#include <QList>
struct WayPoint{
    double latitude;
    double longitude;
    double altitude;
    double speed;//建议飞行速度
    double hoverTime;//悬停时间(second)
    int action;//动作：0=无，1=拍照，2=录像，3=抛投
};

struct FlightPath{
    int id;
    QString name;
    QList<WayPoint> waypoints;//航点列表
    QDateTime createTime;
    double totalDistance;//总距离(m)
    double estimatedTime;//预计飞行时间(second)
};


#endif //FLIGHTPATH_H
