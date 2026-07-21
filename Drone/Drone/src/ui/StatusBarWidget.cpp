#include "StatusBarWidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QDateTime>
StatusBarWidget::StatusBarWidget(QWidget *parent):QWidget(parent){
    auto *layout=new QHBoxLayout(this);
    layout->setContentsMargins(4,0,4,0);
    m_connLabel=new QLabel("● 已连接");
    m_connLabel->setStyleSheet("color:#27ae60;font-weight:bold;");
    m_countLabel=new QLabel("在线:0/5");
    m_timeLabel=new QLabel;
    m_timeLabel->setStyleSheet("color:#7f8c8d;");
    m_alarmLabel=new QLabel("告警: 0");
    m_alarmLabel->setStyleSheet("color:#e74c3c;");
    layout->addWidget(m_connLabel);
    layout->addSpacing(15);
    layout->addWidget(m_countLabel);
    layout->addStretch();
    layout->addWidget(m_timeLabel);
    layout->addSpacing(20);
    layout->addWidget(m_alarmLabel);
    m_timer=new QTimer(this);
    connect(m_timer,&QTimer::timeout,this,&StatusBarWidget::updateTime);
    m_timer->start(1000);
    updateTime();

}
void StatusBarWidget::setOnlineCount(int count){
    m_countLabel->setText(QString("在线:%1/5").arg(count));

}
void StatusBarWidget::incrementAlarm(){
    m_alarmLabel->setText(QString("告警:%1").arg(++m_alarmCount));
}
void StatusBarWidget::updateTime(){
    m_timeLabel->setText(QDateTime::currentDateTime().toString("HH:mm:ss"));
}
