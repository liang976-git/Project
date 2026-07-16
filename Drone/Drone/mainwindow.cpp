#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,m_droneManager(new DroneManager(this))
    ,m_dispatcher(new DataDispatcher(this))
    ,m_simLink(new SimulatedLink(this))
    ,m_parser(new MavlinkParser(this))
{
    ui->setupUi(this);
    connect(m_simLink,&SimulatedLink::telemetryReceived,this,&MainWindow::onTelemetryReceived);
    connect(m_droneManager,&DroneManager::droneStatusChanged,this,&MainWindow::onDroneStatusChanged);
    connect(m_droneManager,&DroneManager::alarmTriggered,this,&MainWindow::onAlarmTriggered);
    connect(m_dispatcher,&DataDispatcher::commandReceived,this,&MainWindow::onCommandReceived);
    m_dispatcher->startPublishing();
    m_simLink->startSimulation(5);

     qDebug()<<"===无人机管控平台启动===";
     qDebug()<<"模拟5架无人机，遥测数据通过ZeroMQ分发";


}

MainWindow::~MainWindow()
{
    m_simLink->stopSimulation();
    m_dispatcher->stopPublishing();
    delete ui;
}
void MainWindow::onTelemetryReceived(const DroneInfo &drone){
    m_droneManager->updateDroneStatus(drone.id,drone);
    m_dispatcher->publishTelemetry(drone);
    static int count=0;
    if(++count%50==0){
        qDebug()<<QString("[%1] %2 | 位置:(%3,%4,%5)m | 速度:%6m/s | 电量:%7%")
                    .arg(drone.lastHeartbeat)
                    .arg(drone.name)
                    .arg(drone.latitude,0,'f',6)
                    .arg(drone.longitude,0,'f',6)
                    .arg(drone.altitude,0,'f',1)
                    .arg(drone.speed,0,'f',1)
                    .arg(drone.batteryPercent);
    }
}
void MainWindow::onDroneStatusChanged(int droneId, DroneStatus status ){
    QString statusStr;
    switch(status){
    case DroneStatus::Idle:statusStr="空闲";
        break;
    case DroneStatus::Flying:statusStr="飞行中";
        break;
    case DroneStatus::Returning:statusStr="返航中";
        break;
    case DroneStatus::LowBattery:statusStr="电量低";
        break;
    case DroneStatus::Disconnected:statusStr="失联";
        break;
    case DroneStatus::Error:statusStr="故障";
        break;
    }
    qDebug()<<QString("[状态变更]无人机%1->%2").arg(droneId).arg(statusStr);
}
void MainWindow::onAlarmTriggered(int droneId, const QString &message){
    qDebug()<<QString("[告警]无人机%1：%2").arg(droneId).arg(message);
}
void MainWindow::onCommandReceived(const QByteArray &cmd){
    qDebug()<<QString("[收到指令]%1").arg(QString (cmd));
    //TODO:解析指令并执行对应的操作
}
