#ifndef PATHPLANNINGWIDGET_H
#define PATHPLANNINGWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include "src/model/FlightPath.h"
#include <QMetaType>
Q_DECLARE_METATYPE(WayPoint)

class PathPlanningWidget:public QWidget{
    Q_OBJECT
public:
    PathPlanningWidget(QWidget *parent=nullptr);
    void setWaypoints(const QList<WayPoint> &points);
    QList<WayPoint> waypoints() const;
signals:
    void waypointsChanged(const QList<WayPoint> &points);
private:
    void setupUI();
    void setupConnections();
    void onAdd();
    void onUpdate();
    void onDelete();
    void onClear();
    void onTableClicked(int row,int col);
    void refreshTable();
    bool validateInput() const;
    QTableWidget *m_table;
    QDoubleSpinBox *m_lngSpin;
    QDoubleSpinBox *m_latSpin;
    QDoubleSpinBox *m_altSpin;
    QDoubleSpinBox *m_speedSpin;
    QLineEdit *m_nameEdit;
    QPushButton *m_addBtn;
    QPushButton *m_updateBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_clearBtn;
    QPushButton *m_importBtn;
    QPushButton *m_exportBtn;
    QPushButton *m_saveBtn;
    QList<WayPoint> m_points;
    int m_currentRow;
};


#endif //PATHPLANNINGWIDGET_H
