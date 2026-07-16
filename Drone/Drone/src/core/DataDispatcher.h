#ifndef DATADISPATCHER_H
#define DATADISPATCHER_H

#include <QObject>
#include <QTimer>
#include <zmq.h>
#include "src/model/DroneInfo.h"

class DataDispatcher : public QObject{
    Q_OBJECT
public:
    explicit DataDispatcher(QObject *parent = nullptr);
    ~DataDispatcher();
    void startPublishing(const QString &pubAddr="tcp://*:5555",const QString &subAddr="tcp://127.0.0.1:5556");
    void stopPublishing();
    void publishTelemetry(const DroneInfo &drone);
    void sendCommand(const QByteArray &cmd);
signals:
    void commandReceived(const QByteArray &cmd);
private slots:
    void pollSubscriber();
private:
    void *m_context;
    void *m_publisher;
    void *m_subscriber;
    QTimer *m_pollTimer;
};


#endif //DATADISPATCHER_H
