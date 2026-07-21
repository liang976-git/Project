#include "DroneListWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QProgressBar>

DroneListWidget::DroneListWidget(DroneManager *manager,QWidget *parent)
    : QWidget(parent), m_manager(manager)
{
    auto *layout=new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    m_table=new QTableWidget(0,5,this);
    m_table->setHorizontalHeaderLabels({"ID","名称","状态","电量","信号"});
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0,40);
    m_table->setColumnWidth(1,80);
    m_table->setColumnWidth(2,60);
    m_table->setColumnWidth(3,80);
    m_table->setStyleSheet("QTableWidget{font-size:12px;}"
                           "QHeaderView::section{background:#2c3e50;color:white;padding:6px;}");

    layout->addWidget(m_table);
    connect(m_table,&QTableWidget::itemSelectionChanged,this,[this](){
        auto selected=m_table->selectedItems();
        if(!selected.isEmpty()){
            int row=selected.first()->row();
            int droneId=m_table->item(row,0)->data(Qt::UserRole).toInt();
            m_selectedId=droneId;
            emit droneSelected(droneId);
        }
    });
}
void DroneListWidget::refreshList(){
    auto drones=m_manager->getAllDrones();
    m_table->setRowCount(drones.size());
    for(int i=0;i<drones.size();++i){
        const auto &d=drones[i];
        auto *idItem=new QTableWidgetItem(QString::number(d.id));
        idItem->setData(Qt::UserRole,d.id);
        idItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i,0,idItem);
        m_table->setItem(i,1,new QTableWidgetItem(d.name));
        auto *statusItem=new QTableWidgetItem(statusText(d.status));
        statusItem->setForeground(statusColor(d.status));
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i,2,statusItem);
        auto *batteryBar=new QProgressBar;
        batteryBar->setRange(0,100);
        batteryBar->setValue(d.batteryPercent);
        batteryBar->setTextVisible(true);
        QString color=d.batteryPercent>60?"#27ae60":d.batteryPercent>20?"#f39c12":"#e74c3c";
        batteryBar->setStyleSheet("QProgressBar{border:1px solid #bdc3c7;border-radius:3px;"
                                  "text-align:center;height:18px;}"
                                  "QProgressBar::chunk{background:"+color+";}");
        m_table->setCellWidget(i,3,batteryBar);
        auto *signalItem=new QTableWidgetItem(QString("%1%").arg(d.signalStr));
        signalItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i,4,signalItem);
        if(d.id==m_selectedId){
            m_table->selectRow(i);
        }
    }
}
int DroneListWidget::currentSelectedId() const{

    return m_selectedId;
}
QString DroneListWidget::statusText(DroneStatus s) const{
    switch(s){
    case DroneStatus::Idle:return "空闲";
    case DroneStatus::Flying:return "飞行中";
        case DroneStatus::Returning:return "返航中";
        case DroneStatus::LowBattery:return "电量低";
        case DroneStatus::Disconnected:return "失联";
        case DroneStatus::Error:return "故障";
    }
    return "未知";
}
QColor DroneListWidget::statusColor(DroneStatus s) const{
    switch (s) {
             case DroneStatus::Flying:
               return QColor("#27ae60");
             case DroneStatus::Idle:
               return QColor("#3498db");
             case DroneStatus::Returning:
                return QColor("#f39c12");
             case DroneStatus::LowBattery:
                return QColor("#e67e22");
             case DroneStatus::Disconnected:
                return QColor("#95a5a6");
             case DroneStatus::Error:
               return QColor("#e74c3c");
             }
             return QColor("#000000");
}
