#include "FlightParameterWidget.h"
#include "AltitudeGaugeWidget.h"
#include "CompassWidget.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

FlightParameterWidget::FlightParameterWidget(QWidget *parent):QWidget(parent){
    setupUI();
    //clearData();
}
void FlightParameterWidget::setupUI(){
    auto *mainLayout=new QVBoxLayout(this);
    mainLayout->setContentsMargins(8,8,8,8);
    //标题
    m_titleLabel =new QLabel("飞行参数");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size:16px;font-weight:bold;"
                                "background:#2c3e50;color:white;padding:8px;border-radius:4px;");
    mainLayout->addWidget(m_titleLabel);
    //无人机名称
    m_nameLabel=new QLabel;
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setStyleSheet("font-size:14px;font-weight:bold;color:#2c3e50;padding:6px;");
    mainLayout->addWidget(m_nameLabel);
    //==位置==
    auto *posGroup=new QGroupBox("位置");
    auto *posForm=new QFormLayout(posGroup);
    m_latValue=new QLabel;
    m_lngValue=new QLabel;
    m_altGauge = new AltitudeGaugeWidget;
    m_altGauge->setFixedSize(60, 100);
    posForm->addRow("纬度:",m_latValue);
    posForm->addRow("经度:",m_lngValue);
    posForm->addRow("高度:",m_altGauge);
    mainLayout->addWidget(posGroup);
    //==运动==
    auto *moveGroup=new QGroupBox("运动");
    auto *moveForm=new QFormLayout(moveGroup);
    m_speedValue=new QLabel;
    m_vSpeedValue=new QLabel;
    m_compass = new CompassWidget;
    m_compass->setFixedSize(80, 80);
    moveForm->addRow("地速:",m_speedValue);
    moveForm->addRow("垂速:",m_vSpeedValue);
    moveForm->addRow("航向:",m_compass);
    mainLayout->addWidget(moveGroup);
    //==状态==
    auto *statGroup=new QGroupBox("状态");
    auto *statForm=new QFormLayout(statGroup);
    m_statusValue=new QLabel;
    m_batteryBar=new QProgressBar;
    m_batteryBar->setTextVisible(true);
    m_signalalue=new QLabel;
    m_timeValue=new QLabel;
    statForm->addRow("状态:",m_statusValue);
    statForm->addRow("电量:",m_batteryBar);
    statForm->addRow("信号:",m_signalalue);
    statForm->addRow("时间:",m_timeValue);
    mainLayout->addWidget(statGroup);
    mainLayout->addStretch();
    setStyleSheet("QGroupBox{font-weight:bold;border:1px solid #bdc3c7;"
                 "border-radius:4px;margin-top:8px;padding-top:14px;}"
                 "QGroupBox::title{subcontrol-origin:margin;left:10px;padding:0 4px;}"
                 "QLabel{font-size:12px;}"
             );
}
void FlightParameterWidget::updateData(const DroneInfo &drone){
    m_nameLabel->setText(QString("<span style='color:#2c3e50;'>%1</span>"
                                 "<span style='color:#7f8c8d;'>(ID:%2)</span>")
                         .arg(drone.name).arg(drone.id));
    m_latValue->setText(QString::number(drone.latitude,'f',6)+"°");
    m_lngValue->setText(QString::number(drone.longitude, 'f', 6) + "°");
    m_altGauge->setValue(drone.altitude);
    m_speedValue->setText(QString::number(drone.speed,'f',1)+"m/s");
    m_vSpeedValue->setText(QString::number(drone.verticalSpeed,'f',1)+"m/s");
    m_compass->setHeading(drone.heading);
    switch(drone.status){
    case DroneStatus::Flying:m_statusValue->setText("飞行中"); break;
    case DroneStatus::Idle:m_statusValue->setText("空闲"); break;
    case DroneStatus::Returning:m_statusValue->setText("返航中"); break;
    case DroneStatus::LowBattery:m_statusValue->setText("电量低"); break;
    case DroneStatus::Disconnected:m_statusValue->setText("失联"); break;
    case DroneStatus::Error:m_statusValue->setText("故障"); break;
    }

    m_batteryBar->setValue(drone.batteryPercent);
    QString barColor=drone.batteryPercent > 60 ? "#27ae60" : drone.batteryPercent >20 ? "#f39c12":"#e74c3c";
    m_batteryBar->setStyleSheet("QProgressBar{border:1px solid #bdc3c7;border-radius:3px;"
                                "text-align:center;height:20px;}"
                                "QProgressBar::chunk{background:"+barColor+";}");
    m_signalalue->setText(QString("%1%").arg(drone.signalStr));
    m_timeValue->setText(drone.lastHeartbeat);

}
void FlightParameterWidget::clearData(){
    m_nameLabel->setText("未选择");
    m_latValue->setText("--");
    m_lngValue->setText("--");
    m_altGauge->setValue(0);
    m_speedValue->setText("--");
    m_vSpeedValue->setText("--");
    m_compass->setHeading(0);
    m_statusValue->setText("--");
    m_batteryBar->setValue(0);
    m_signalalue->setText("--");
    m_timeValue->setText("--");
}
