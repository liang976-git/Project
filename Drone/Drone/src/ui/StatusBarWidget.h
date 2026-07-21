#ifndef STATUSBARWIDGET_H
#define STATUSBARWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

class StatusBarWidget : public QWidget{
    Q_OBJECT
public:
    explicit StatusBarWidget(QWidget *parent=nullptr);
    void setOnlineCount(int);
    void incrementAlarm();
private slots:
    void updateTime();
private:
    QLabel *m_connLabel;
    QLabel *m_countLabel;
    QLabel *m_timeLabel;
    QLabel *m_alarmLabel;
    QTimer *m_timer;
    int m_alarmCount;

};

#endif
