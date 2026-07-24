#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebChannel>
#include "src/model/DroneInfo.h"
#include "src/model/FlightPath.h"
class JsBridge : public QObject {
    Q_OBJECT
public:
    explicit JsBridge(QObject *parent = nullptr);

public slots:
    void onMapClick(double lng, double lat);
    void onMapReady();
    void onWaypointDelete(int index);
    void onWaypointMoved(int index,double lng,double lat);
    void onWaypointEdited(int index,double altitude,double speed);
    void onFenceDrawn(const QString &type, const QString &paramsJson);

signals:
    void mapClicked(double lng, double lat);
    void mapReady();
    void waypointDelete(int index);
    void waypointMoved(int index,double lng,double lat);
    void waypointEdited(int index,double altitude,double speed);
    void fenceDrawn(const QString &type, const QString &paramsJson);

};

class MapWidget : public QWidget {
    Q_OBJECT
public:
    explicit MapWidget(QWidget *parent = nullptr);
    void updateDronePosition(const DroneInfo &drone);
    void updateDronePositions(const QList<DroneInfo> &drones);
    void addTrailPoint(int droneId, double lng, double lat);
    QWebEngineView *view() const { return m_view; }
    void renderWaypoints(const QList<WayPoint> &point);
    void enterDrawMode(const QString &type);
    void exitDrawMode();
    void renderFenceZone(const QString &type, const QString &paramsJson);

signals:
    void mapClicked(double lng, double lat);
    void waypointDeleted(int index);
    void waypointMoved(int index,double lng,double lat);
    void waypointEdited(int index,double altitude,double speed);
    void fenceDrawn(const QString &type, const QString &paramsJson);

private:
    void setupUI();
    void initMap();
    QWebEngineView *m_view;
    QWebChannel *m_channel;
    JsBridge *m_bridge;
};

#endif
