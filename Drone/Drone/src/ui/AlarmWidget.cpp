#include "AlarmWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>

static const char *alarmTypeName(AlarmType t) {
    switch (t) {
        case LowBattery:      return "低电量";
        case SignalLost:      return "信号丢失";
        case GeoFenceBreach:  return "围栏越界";
        case MotorError:      return "电机故障";
        case ConnectionLost:  return "连接断开";
    }
    return "未知";
}

AlarmWidget::AlarmWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void AlarmWidget::setupUI() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    auto *title = new QLabel("告警列表");
    title->setStyleSheet("font-size:14px;font-weight:bold;padding:4px;");
    layout->addWidget(title);

    m_filterCombo = new QComboBox;
    m_filterCombo->addItems({"全部", "低电量", "信号丢失", "围栏越界",
                             "电机故障", "连接断开"});
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AlarmWidget::applyFilter);
    layout->addWidget(m_filterCombo);

    m_table = new QTableWidget(0, 5, this);
    m_table->setHorizontalHeaderLabels({"类型", "无人机", "内容", "等级", "时间"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    m_table->setAlternatingRowColors(true);
    layout->addWidget(m_table);
}

void AlarmWidget::addAlarm(const AlarmInfo &alarm) {
    int row = m_table->rowCount();
    m_table->insertRow(row);

    auto *typeItem = new QTableWidgetItem(alarmTypeName(alarm.type));
    typeItem->setData(Qt::UserRole, static_cast<int>(alarm.type));

    auto *droneItem = new QTableWidgetItem(QString("UAV-%1").arg(alarm.droneId));
    auto *msgItem   = new QTableWidgetItem(alarm.message);
    auto *levelItem = new QTableWidgetItem(
        alarm.level == Info     ? "提示" :
        alarm.level == Warning  ? "警告" :
        alarm.level == Critical ? "严重" : "紧急");
    levelItem->setForeground(
        alarm.level == Emergency ? QColor("#c0392b") :
        alarm.level == Critical  ? QColor("#e74c3c") :
        alarm.level == Warning   ? QColor("#f39c12") :
                                   QColor("#3498db"));
    QFont bf = levelItem->font();
    bf.setBold(alarm.level >= Warning);
    levelItem->setFont(bf);

    auto *timeItem = new QTableWidgetItem(
        alarm.time.toString("HH:mm:ss"));

    m_table->setItem(row, 0, typeItem);
    m_table->setItem(row, 1, droneItem);
    m_table->setItem(row, 2, msgItem);
    m_table->setItem(row, 3, levelItem);
    m_table->setItem(row, 4, timeItem);

    m_table->scrollToBottom();
    applyFilter(m_filterCombo->currentIndex());
}

void AlarmWidget::applyFilter(int typeIndex) {
    for (int r = 0; r < m_table->rowCount(); ++r) {
        auto t = static_cast<AlarmType>(m_table->item(r, 0)->data(Qt::UserRole).toInt());
        bool visible = (typeIndex == 0) || (static_cast<int>(t) + 1 == typeIndex);
        m_table->setRowHidden(r, !visible);
    }
}
