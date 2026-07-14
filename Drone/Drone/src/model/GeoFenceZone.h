#ifndef GEOFENCEZONE_H
#define GEOFENCEZONE_H

#include <QString>
#include <cstdint>
#include <QList>

enum ZoneType{
    Circle,
    Rectangle,
    Polygon
};

struct GeoFenceZone{
    int id;
    QString name;
    ZoneType type;
    bool enabled;
    QString regulation;//关联法规
    double centerLat;
    double centerLng;//(圆心，圆形时用）
    double radius;//半径
    QList<QPair<double,double>> points;//多边形/矩形顶点列表

};

#endif //GEOFENCEZONE_H
