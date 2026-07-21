#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include "src/database/DatabaseManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,m_droneManager(new DroneManager(this))
    ,m_dispatcher(new DataDispatcher(this))
    ,m_simLink(new SimulatedLink(this))
    ,m_parser(new MavlinkParser(this))
{
    ui->setupUi(this);
    setWindowTitle("无人机管控平台");
    resize(1280,800);
    //1.初始化数据库
    DatabaseManager::instance().initialize("drone.db");
    //2.构建UI布局
    setupLayout();
    setupNavMenu();
    //3.连接信号槽
    setupConnections();

    connect(m_simLink,&SimulatedLink::telemetryReceived,this,&MainWindow::onTelemetryReceived);
//    connect(m_droneManager,&DroneManager::droneStatusChanged,this,&MainWindow::onDroneStatusChanged);
//    connect(m_droneManager,&DroneManager::alarmTriggered,this,&MainWindow::onAlarmTriggered);
//    connect(m_dispatcher,&DataDispatcher::commandReceived,this,&MainWindow::onCommandReceived);
    m_dispatcher->startPublishing();
    m_simLink->startSimulation(5);

     qDebug()<<"===无人机管控平台启动===";
     qDebug()<<"模拟5架无人机，遥测数据通过ZeroMQ分发";


}

MainWindow::~MainWindow()
{
    m_simLink->stopSimulation();
    m_dispatcher->stopPublishing();
    //delete ui;
}
void MainWindow::setupLayout(){
    auto *leftPanel=new QWidget(this);
    auto *leftLayout=new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0,0,0,0);
    leftLayout->setSpacing(0);
    //导航标题
    auto *navTitle=new QLabel("功能导航");
    navTitle->setAlignment(Qt::AlignCenter);
    navTitle->setFixedHeight(30);
    navTitle->setStyleSheet("background: #2c3e50;color:white;font-weight:bold;");
    leftLayout->addWidget(navTitle);
    //导航列表
    m_navList=new QListWidget;
    m_navList->setIconSize(QSize(24,24));
    m_navList->setFixedWidth(160);
    m_navList->setStyleSheet("QListWidget{background:#34495e;color:white;border:none;font-size:13px;}"
                             "QListWidget::item{padding:10px;border-bottom:1px solid #2c3e50;}"
                             "QListWidget::item:selected{background:#3498db;}"
                             "QListWidget::item:hover{background:#2980b9;}");
    leftLayout->addWidget(m_navList);
    //无人机列表标题
    auto *droneTitle=new QLabel("无人机列表");
    droneTitle->setAlignment(Qt::AlignCenter);
    droneTitle->setFixedHeight(30);
    droneTitle->setStyleSheet("background:#2c3e50;color:white;font-weight:bold;");
    leftLayout->addWidget(droneTitle);
    //无人机表格
    m_droneList=new DroneListWidget(m_droneManager);
    leftLayout->addWidget(m_droneList);
    //主分割器（左|中|右）
    m_mainSplitter=new QSplitter(Qt::Horizontal,this);
    //===中心区域：
    m_centerStack=new QStackedWidget;
    auto *mapPlaceholder=new QLabel("地图区域（后续集成)");
    mapPlaceholder->setAlignment(Qt::AlignCenter);
    mapPlaceholder->setStyleSheet("background:#ecf0f1;font-size:18px;color:#7f8c8d");
    m_centerStack->addWidget(mapPlaceholder);//page 0:监控
    auto *pathPlaceholder=new QLabel("路径规划（后续开发)");
    pathPlaceholder->setAlignment(Qt::AlignCenter);
    pathPlaceholder->setStyleSheet("background:#ecf0f1;font-size:18px;color:#7f8c8d;");
    m_centerStack->addWidget(pathPlaceholder);//page 1:路径规划
    auto *fencePlaceholder=new QLabel("禁飞区管理（后续开发)");
    fencePlaceholder->setAlignment(Qt::AlignCenter);
    fencePlaceholder->setStyleSheet("background:#ecf0f1;font-size:18px;color:#7f8c8d;");
    m_centerStack->addWidget(fencePlaceholder);//page 2:禁飞去
    auto *historyPlaceholder=new QLabel("历史回放（后续开发）");
    historyPlaceholder->setAlignment(Qt::AlignCenter);
    historyPlaceholder->setStyleSheet("background:#ecf0f1;font-size:18px;color:#7f8c8d");
    m_centerStack->addWidget(historyPlaceholder);//page 3:历史回放
    auto *permPlaceholder=new QLabel("权限管理（后续开发）");
    permPlaceholder->setAlignment(Qt::AlignCenter);
    permPlaceholder->setStyleSheet("background:#ecf0f1;font-size:18px;color:#7f8c8d;");
    m_centerStack->addWidget(permPlaceholder);//page 4:权限管理
    // === 右侧面板：飞行参数 ===
    m_flightParam=new FlightParameterWidget;
    //组装主分割器
    m_mainSplitter->addWidget(leftPanel);
    m_mainSplitter->addWidget(m_centerStack);
    m_mainSplitter->addWidget(m_flightParam);
    m_mainSplitter->setStretchFactor(0,0);//左侧不伸缩
    m_mainSplitter->setStretchFactor(1,1);//中心伸缩
    m_mainSplitter->setStretchFactor(2,0);//右侧不伸缩
    m_mainSplitter->setSizes({200,700,280});
    setCentralWidget(m_mainSplitter);
    // === 底部状态栏 ===
    m_statusBar=new StatusBarWidget(this);
    statusBar()->addPermanentWidget(m_statusBar);
}
//=== 导航菜单===
void MainWindow::setupNavMenu(){
    QList<QString> items={
        "实时监控","路径规划","禁飞区管理","历史回放","权限管理"
    };
    for(int i=0;i<items.size();++i){
        m_navList->addItem(items[i]);

    }
    m_navList->setCurrentRow(0);
}
//===信号槽连接===
void MainWindow::setupConnections(){
    connect(m_navList,&QListWidget::currentRowChanged,this,&MainWindow::onNavChanged);
    connect(m_droneList,&DroneListWidget::droneSelected,this,&MainWindow::onDroneSelected);
}
//===导航切换===
void MainWindow::onNavChanged(int row){
    m_centerStack->setCurrentIndex(row);
}
//===无人机选中→右侧面板更新===
void MainWindow::onDroneSelected(int droneId){
    auto drones = m_droneManager->getAllDrones();
    for (const auto &d : drones) {
        if (d.id == droneId) {
            m_flightParam->updateData(d);
            break;
        }
    }
}

void MainWindow::onTelemetryReceived(const DroneInfo &drone){
    m_droneManager->updateDroneStatus(drone.id,drone);
    m_droneList->refreshList();
    m_dispatcher->publishTelemetry(drone);
    if (drone.id == m_droneList->currentSelectedId()) {
        m_flightParam->updateData(drone);
    }
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
