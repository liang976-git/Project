#ifndef ALARMWIDGET_H
#define ALARMWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include "src/model/AlarmInfo.h"

class AlarmWidget : public QWidget {
    Q_OBJECT
public:
    explicit AlarmWidget(QWidget *parent = nullptr);
    void addAlarm(const AlarmInfo &alarm);

private:
    void setupUI();
    QComboBox *m_filterCombo;
    QTableWidget *m_table;
    void applyFilter(int typeIndex);
};

#endif
