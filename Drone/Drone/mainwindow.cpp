#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <zmq.h>
#include <cstring>
#include <QThread>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    //SQLite测试
//    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");
//    db.setDatabaseName("test.db");
//    if(!db.open()){
//        qDebug()<<"DB Error:"<<db.lastError().text();
//        return;
//    }
//    QSqlQuery query;
//    query.exec("create table test(id int,name text)");
//    query.exec("insert into test values(1,'hello')");
//    query.exec("select * from test");
//    while(query.next()){
//        qDebug()<<query.value(0).toInt()<<query.value(1).toString();
//    }
//   db.close();
   //ZeroMQ PUB-SUB测试
   void *context = zmq_ctx_new();
   void *publisher = zmq_socket(context,ZMQ_PUB);
   zmq_bind(publisher,"tcp://*:5555");
   void *subscriber=zmq_socket(context,ZMQ_SUB);
   zmq_setsockopt(subscriber,ZMQ_SUBSCRIBE,"",0);
   zmq_connect(subscriber,"tcp://127.0.0.1:5555");
   QThread::msleep(100);
   //发送
   zmq_send(publisher,"hello ZeroMQ",12,0);
   //接收
   char buffer[256];
   int len = zmq_recv(subscriber,buffer,sizeof(buffer)-1,0);
   buffer[len]='\0';
   qDebug()<<"收到:"<<buffer;
   zmq_close(publisher);
   zmq_close(subscriber);
   zmq_ctx_destroy(context);
}

MainWindow::~MainWindow()
{
    delete ui;
}

