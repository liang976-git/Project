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
