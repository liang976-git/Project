#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QListWidget>
#include <QStackedWidget>
#include "src/core/DataDispatcher.h"
#include "src/core/DroneManager.h"
#include "src/protocol/SimulatedLink.h"
#include "src/protocol/MavlinkParser.h"
#include "src/ui/FlightParameterWidget.h"
#include "src/ui/StatusBarWidget.h"
#include "src/ui/DroneListWidget.h"
#include "src/ui/AlarmWidget.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void onTelemetryReceived(const DroneInfo &drone);
    void onDroneStatusChanged(int droneId,DroneStatus status);
    void onAlarmTriggered(int droneId,const QString &message);
    void onCommandReceived(const QByteArray &cmd);
    void onNavChanged(int droneId);
    void onDroneSelected(int row);
private:
    Ui::MainWindow *ui;
    void setupLayout();
    void setupNavMenu();
    void setupConnections();
    //核心业务对象
    DroneManager *m_droneManager;
    DataDispatcher *m_dispatcher;
    SimulatedLink *m_simLink;
    MavlinkParser *m_parser;
    //UI组件
    QSplitter *m_mainSplitter;//主三栏分割器
    QListWidget *m_navList;//左侧导航栏菜单
    QStackedWidget *m_centerStack;//中心区域页面容器
    DroneListWidget *m_droneList;//左下的无人机列表
    FlightParameterWidget *m_flightParam;//右侧参数面板
    StatusBarWidget *m_statusBar;//底部状态栏
    AlarmWidget *m_alarmWidget;
};
#endif // MAINWINDOW_H
