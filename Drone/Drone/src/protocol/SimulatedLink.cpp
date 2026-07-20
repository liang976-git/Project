#include "src/protocol/SimulatedLink.h"
#include <QRandomGenerator>
#include <QDateTime>

SimulatedLink::SimulatedLink(QObject *parent):QObject(parent),m_timer(new QTimer(this)),m_tick(0){
    connect(m_timer,&QTimer::timeout,this,&SimulatedLink::generateTelemetry);

}
void SimulatedLink::startSimulation(int droneCount){
        initDrones(droneCount);
        m_tick=0;
        m_timer->start(100);
    }
void SimulatedLink::stopSimulation(){
        m_timer->stop();
    }
void SimulatedLink::initDrones(int count){
    m_drones.clear();
    for(int i=0;i<count;++i){
        DroneInfo drone;
        drone.id=i+1;
        drone.name=QString("UAV-%1").arg(i+1,2,10,QChar('0'));
        drone.model="DJIMatrice 300";
        drone.type=DroneType::MultiRotor;
        drone.status=DroneStatus::Flying;
        drone.latitude=30.0+ QRandomGenerator::global()->bounded(50)*0.001;
        drone.longitude=120.0+QRandomGenerator::global()->bounded(50)*0.001;
        drone.altitude=50.0+QRandomGenerator::global()->bounded(100);
        drone.speed=5.0+QRandomGenerator::global()->bounded(10);
        drone.verticalSpeed=0.0;
        drone.heading=QRandomGenerator::global()->bounded(360);
        drone.batteryPercent=80+QRandomGenerator::global()->bounded(20);
        drone.signalStr=70+QRandomGenerator::global()->bounded(30);
        drone.timestamp=QDateTime::currentMSecsSinceEpoch();
        drone.lastHeartbeat=QDateTime::currentDateTime().toString("HH:mm:ss");
        m_drones.append(drone);

    }
}
void SimulatedLink::generateTelemetry(){
    ++m_tick;
    for(DroneInfo &drone:m_drones){
        if(drone.status==DroneStatus::Disconnected){
            continue;
        }
        drone.latitude+=(QRandomGenerator::global()->bounded(100)-50)*0.00001;
        drone.longitude+=(QRandomGenerator::global()->bounded(100)-50)*0.00001;
        drone.speed=qBound(0.0,drone.speed+(QRandomGenerator::global()->bounded(20)-10)*0.1,20.0);
        drone.heading+=(QRandomGenerator::global()->bounded(10)-5)*0.5;
        if(drone.heading<0) drone.heading+=360;
        if(drone.heading>=360) drone.heading-=360;
        drone.verticalSpeed=(QRandomGenerator::global()->bounded(20)-10)*0.1;
        drone.altitude=qBound(10.0,drone.altitude+drone.verticalSpeed*0.1,200.0);
        if(m_tick % 100==0 && drone.batteryPercent>0){
            drone.batteryPercent-=1;
        }
        drone.signalStr=qBound(0,drone.signalStr+QRandomGenerator::global()->bounded(5)-2,100);
        drone.timestamp=QDateTime::currentMSecsSinceEpoch();
        drone.lastHeartbeat=QDateTime::currentDateTime().toString("HH:mm:ss");
        if(drone.batteryPercent<=20 && drone.status!=DroneStatus::LowBattery){
            drone.status=DroneStatus::LowBattery;
        }
        emit telemetryReceived(drone);
    }
}
