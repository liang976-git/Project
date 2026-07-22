#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebChannel>
#include "src/model/DroneInfo.h"

class JsBridge : public QObject {
    Q_OBJECT
public:
    explicit JsBridge(QObject *parent = nullptr);

public slots:
    void onMapClick(double lng, double lat);
    void onMapReady();

signals:
    void mapClicked(double lng, double lat);
    void mapReady();
};

class MapWidget : public QWidget {
    Q_OBJECT
public:
    explicit MapWidget(QWidget *parent = nullptr);
    void updateDronePosition(const DroneInfo &drone);
    void updateDronePositions(const QList<DroneInfo> &drones);
    void addTrailPoint(int droneId, double lng, double lat);

private:
    void setupUI();
    void initMap();
    QWebEngineView *m_view;
    QWebChannel *m_channel;
    JsBridge *m_bridge;
};

#endif
