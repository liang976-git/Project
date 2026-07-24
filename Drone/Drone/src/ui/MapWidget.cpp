#include "MapWidget.h"
#include <QVBoxLayout>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

JsBridge::JsBridge(QObject *parent) : QObject(parent) {}

void JsBridge::onMapClick(double lng, double lat) {
    emit mapClicked(lng, lat);
}

void JsBridge::onMapReady() {
    qDebug() << "[地图] JS 引擎就绪";
    emit mapReady();
}
void JsBridge::onWaypointDelete(int index){
    emit waypointDelete(index);
}
void JsBridge::onWaypointMoved(int index, double lng, double lat){
    emit onWaypointMoved(index,lng,lat);
}
void JsBridge::onWaypointEdited(int index, double altitude, double speed){
    emit onWaypointEdited(index,altitude,speed);
}
void JsBridge::onFenceDrawn(const QString &type, const QString &paramsJson){
    emit fenceDrawn(type, paramsJson);
}
MapWidget::MapWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void MapWidget::setupUI() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_view = new QWebEngineView(this);
    m_channel = new QWebChannel(this);
    m_bridge = new JsBridge(this);

    m_channel->registerObject("bridge", m_bridge);
    m_view->page()->setWebChannel(m_channel);
    connect(m_bridge, &JsBridge::mapClicked, this, &MapWidget::mapClicked);
    connect(m_bridge,&JsBridge::waypointDelete,this,&MapWidget::waypointDeleted);
    connect(m_bridge,&JsBridge::waypointMoved,this,&MapWidget::waypointMoved);
    connect(m_bridge,&JsBridge::waypointEdited,this,&MapWidget::waypointEdited);
    connect(m_bridge,&JsBridge::fenceDrawn,this,&MapWidget::fenceDrawn);
    layout->addWidget(m_view);
    initMap();
}

void MapWidget::initMap() {
    QFile file(":/html/map.html");
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "[地图] 无法加载 map.html";
        return;
    }
    QString html = file.readAll();
    file.close();
    m_view->setHtml(html, QUrl("https://localhost"));
}

void MapWidget::updateDronePosition(const DroneInfo &drone) {
    QString js = QString(
        "if (typeof updateMarker === 'function') {"
        "  updateMarker(%1, %2, %3, %4, '%5');"
        "}"
    ).arg(drone.id)
     .arg(drone.longitude, 0, 'f', 6)
     .arg(drone.latitude,  0, 'f', 6)
     .arg(drone.heading,   0, 'f', 1)
     .arg(drone.name);
    m_view->page()->runJavaScript(js);
}

void MapWidget::updateDronePositions(const QList<DroneInfo> &drones) {
    QJsonArray arr;
    for (const auto &d : drones) {
        QJsonObject obj;
        obj["id"]      = d.id;
        obj["lng"]     = d.longitude;
        obj["lat"]     = d.latitude;
        obj["heading"] = d.heading;
        obj["name"]    = d.name;
        arr.append(obj);
    }
    QString js = QString(
        "if (typeof updateAllMarkers === 'function') {"
        "  updateAllMarkers(%1);"
        "}"
    ).arg(QString(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
    m_view->page()->runJavaScript(js);
}

void MapWidget::addTrailPoint(int droneId, double lng, double lat) {
    QString js = QString(
        "if (typeof addTrailPoint === 'function') {"
        "  addTrailPoint(%1, %2, %3);"
        "}"
    ).arg(droneId)
     .arg(lng, 0, 'f', 6)
     .arg(lat, 0, 'f', 6);
    m_view->page()->runJavaScript(js);
}
void MapWidget::enterDrawMode(const QString &type){
    QString js=QString("if(typeof enterDrawMode==='function'){enterDrawMode('%1');}").arg(type);
    m_view->page()->runJavaScript(js);
}
void MapWidget::exitDrawMode(){
    m_view->page()->runJavaScript("if(typeof exitDrawMode==='function')exitDrawMode();");
}
void MapWidget::renderFenceZone(const QString &type, const QString &paramsJson){
    QString js=QString("if(typeof addFenceZone==='function'){addFenceZone(Date.now(),'%1',%2);}")
                    .arg(type, paramsJson);
    m_view->page()->runJavaScript(js);
}
void MapWidget::renderWaypoints(const QList<WayPoint> &pts){
    QJsonArray arr;
    for(const auto &p:pts){
        QJsonObject obj;
        obj["lng"]=p.longitude;
        obj["lat"]=p.latitude;
        obj["altitude"]=p.altitude;
        obj["speed"]=p.speed;
        arr.append(obj);
    }
    QString js=QString("if(typeof renderWaypoints === 'function'){"
                       "renderWaypoints(%1);}").arg(QString(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
    m_view->page()->runJavaScript(js);
}
