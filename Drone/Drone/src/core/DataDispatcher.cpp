#include "src/core/DataDispatcher.h"
#include <QDebug>
#include <QThread>
#include <cstring>

DataDispatcher::DataDispatcher(QObject *parent):QObject(parent),m_context(nullptr),m_publisher(nullptr),m_subscriber(nullptr),m_pollTimer(new QTimer(this)){
    connect(m_pollTimer,&QTimer::timeout,this,&DataDispatcher::pollSubscriber);
}
DataDispatcher::~DataDispatcher(){
    stopPublishing();
}
void DataDispatcher::startPublishing(const QString &pubAddr,const QString &subAddr){
    m_context=zmq_ctx_new();
    m_publisher=zmq_socket(m_context,ZMQ_PUB);
    if(zmq_bind(m_publisher,pubAddr.toStdString().c_str())!=0){
        qDebug()<<"PUB bind 失败："<<zmq_strerror(zmq_errno());
        return;
    }
    m_subscriber=zmq_socket(m_context,ZMQ_SUB);
    zmq_setsockopt(m_subscriber,ZMQ_SUBSCRIBE,"",0);
    if(zmq_connect(m_subscriber,subAddr.toStdString().c_str())!=0){
        qDebug()<<"SUB connect 失败："<<zmq_strerror(zmq_errno());
        return;

    }
    QThread::msleep(100);
    m_pollTimer->start(50);
    qDebug()<<"DataDispatcher 启动成功 PUB："<<pubAddr<<"SUB:"<<subAddr;
}
void DataDispatcher::stopPublishing(){
    m_pollTimer->stop();
    if(m_publisher){
            zmq_close(m_publisher);
            m_publisher=nullptr;
  }
    if(m_subscriber){
        zmq_close(m_subscriber);
        m_subscriber=nullptr;
    }
    if(m_context){
        zmq_ctx_destroy(m_context);
        m_context=nullptr;
    }
}
void DataDispatcher::publishTelemetry(const DroneInfo &drone){
    if(!m_publisher) return;
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&drone.id),sizeof(int));
    data.append(reinterpret_cast<const char*>(&drone.latitude),sizeof(double));
    data.append(reinterpret_cast<const char*>(&drone.longitude),sizeof(double));
    data.append(reinterpret_cast<const char*>(&drone.altitude),sizeof(double));
    data.append(reinterpret_cast<const char*>(&drone.speed),sizeof(double));
    data.append(reinterpret_cast<const char*>(&drone.heading),sizeof(double));
    data.append(reinterpret_cast<const char*>(&drone.batteryPercent),sizeof(int));
    data.append(reinterpret_cast<const char*>(&drone.signalStr),sizeof(int));
    int status=static_cast<int>(drone.status);
    data.append(reinterpret_cast<const char*>(&status),sizeof(int));
    zmq_send(m_publisher,data.constData(),data.size(),ZMQ_DONTWAIT);

}
void DataDispatcher::sendCommand(const QByteArray &cmd){
    if(!m_publisher) return;
    zmq_send(m_publisher,cmd.constData(),cmd.size(),ZMQ_DONTWAIT);
}
void DataDispatcher::pollSubscriber(){
    if(!m_subscriber) return;
    char buffer[1024];
    zmq_pollitem_t items[]={{m_subscriber,0,ZMQ_POLLIN,0}};
    int rc=zmq_poll(items,1,0);
    if(rc>0 && (items[0].revents & ZMQ_POLLIN)){
        int len=zmq_recv(m_subscriber,buffer,sizeof(buffer)-1,ZMQ_DONTWAIT);
        if(len >0){
            buffer[len]='\0';
            QByteArray cmd(buffer,len);
            qDebug()<<"收到指令："<<cmd;
            emit commandReceived(cmd);
        }
    }
}
