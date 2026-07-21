#ifndef DRONELISTWIDGET_H
#define DRONELISTWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include "src/core/DroneManager.h"

class DroneListWidget : public QWidget {
    Q_OBJECT
public:
    explicit DroneListWidget(DroneManager *manager, QWidget *parent=nullptr);
    void refreshList();
    int currentSelectedId() const;
signals:
    void droneSelected(int droneId);
private:
    QString statusText(DroneStatus s) const;
    QColor statusColor(DroneStatus s) const;
    DroneManager *m_manager;
    QTableWidget *m_table;
    int m_selectedId;
};

#endif //DRONELISTWIDGET_H
