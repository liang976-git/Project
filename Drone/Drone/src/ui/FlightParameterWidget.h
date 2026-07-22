#ifndef FLIGHTPARAMETERWIDGET_H
#define FLIGHTPARAMETERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include "src/model/DroneInfo.h"

class FlightParameterWidget : public QWidget {
    Q_OBJECT
public:
    explicit FlightParameterWidget(QWidget *parent=nullptr);
    void updateData(const DroneInfo &drone);
    void clearData();
private:
    void setupUI();
    QLabel *m_titleLabel;
    QLabel *m_nameLabel;
    QLabel *m_latValue;
    QLabel *m_lngValue;
    class AltitudeGaugeWidget *m_altGauge;
    QLabel *m_speedValue;
    QLabel *m_vSpeedValue;
    class CompassWidget *m_compass;
    QLabel *m_statusValue;
    QProgressBar *m_batteryBar;
    QLabel *m_signalalue;
    QLabel *m_timeValue;

};

#endif //FLIGHTPARAMETERWIDGET_H
