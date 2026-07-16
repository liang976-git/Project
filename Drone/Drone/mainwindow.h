#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "src/core/DataDispatcher.h"
#include "src/core/DroneManager.h"
#include "src/protocol/SimulatedLink.h"
#include "src/protocol/MavlinkParser.h"

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
private:
    Ui::MainWindow *ui;
    DroneManager *m_droneManager;
    DataDispatcher *m_dispatcher;
    SimulatedLink *m_simLink;
    MavlinkParser *m_parser;
};
#endif // MAINWINDOW_H
