#include "PathPlanningWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>

PathPlanningWidget::PathPlanningWidget(QWidget *parent)
    : QWidget(parent), m_currentRow(-1)
{
    setupUI();
    setupConnections();
}

void PathPlanningWidget::setupUI() {
    auto *main = new QVBoxLayout(this);

    m_table = new QTableWidget(0, 5, this);
    m_table->setHorizontalHeaderLabels({"序号", "经度", "纬度", "高度(m)", "速度(m/s)"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    main->addWidget(m_table);

    auto *group = new QGroupBox("航点编辑");
    auto *form = new QFormLayout(group);

    m_lngSpin = new QDoubleSpinBox;
    m_lngSpin->setRange(-180, 180);
    m_lngSpin->setDecimals(6);
    m_lngSpin->setPrefix("经度: ");

    m_latSpin = new QDoubleSpinBox;
    m_latSpin->setRange(-90, 90);
    m_latSpin->setDecimals(6);
    m_latSpin->setPrefix("纬度: ");

    m_altSpin = new QDoubleSpinBox;
    m_altSpin->setRange(0, 10000);
    m_altSpin->setSuffix(" m");
    m_altSpin->setValue(100);

    m_speedSpin = new QDoubleSpinBox;
    m_speedSpin->setRange(0, 50);
    m_speedSpin->setSuffix(" m/s");
    m_speedSpin->setValue(10);

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("航线名称（选填）");

    form->addRow(m_lngSpin, m_latSpin);
    form->addRow("高度:", m_altSpin);
    form->addRow("速度:", m_speedSpin);
    form->addRow("名称:", m_nameEdit);
    main->addWidget(group);

    auto *row1 = new QHBoxLayout;
    m_addBtn = new QPushButton("新增航点");
    m_updateBtn = new QPushButton("更新航点");
    m_deleteBtn = new QPushButton("删除航点");
    m_clearBtn = new QPushButton("清空全部");
    row1->addWidget(m_addBtn);
    row1->addWidget(m_updateBtn);
    row1->addWidget(m_deleteBtn);
    row1->addWidget(m_clearBtn);
    main->addLayout(row1);

    auto *row2 = new QHBoxLayout;
    m_importBtn = new QPushButton("导入 KML");
    m_exportBtn = new QPushButton("导出 KML");
    m_saveBtn = new QPushButton("保存航线");
    row2->addWidget(m_importBtn);
    row2->addWidget(m_exportBtn);
    row2->addWidget(m_saveBtn);
    main->addLayout(row2);
}

void PathPlanningWidget::setupConnections() {
    connect(m_table, &QTableWidget::cellClicked,
            this, &PathPlanningWidget::onTableClicked);
    connect(m_addBtn, &QPushButton::clicked, this, &PathPlanningWidget::onAdd);
    connect(m_updateBtn, &QPushButton::clicked, this, &PathPlanningWidget::onUpdate);
    connect(m_deleteBtn, &QPushButton::clicked, this, &PathPlanningWidget::onDelete);
    connect(m_clearBtn, &QPushButton::clicked, this, &PathPlanningWidget::onClear);
}

bool PathPlanningWidget::validateInput() const {
    if (m_lngSpin->value() == 0 && m_latSpin->value() == 0) {
        QMessageBox::warning(const_cast<PathPlanningWidget*>(this),
                             "输入错误", "经纬度不能同时为 0");
        return false;
    }
    return true;
}

void PathPlanningWidget::onAdd() {
    if (!validateInput()) return;

    WayPoint p;
    p.longitude = m_lngSpin->value();
    p.latitude = m_latSpin->value();
    p.altitude = m_altSpin->value();
    p.speed = m_speedSpin->value();

    m_points.append(p);
    refreshTable();
    m_table->scrollToBottom();
    emit waypointsChanged(m_points);
}

void PathPlanningWidget::onUpdate() {
    if (m_currentRow < 0 || m_currentRow >= m_points.size()) return;
    if (!validateInput()) return;

    m_points[m_currentRow].longitude = m_lngSpin->value();
    m_points[m_currentRow].latitude = m_latSpin->value();
    m_points[m_currentRow].altitude = m_altSpin->value();
    m_points[m_currentRow].speed = m_speedSpin->value();

    refreshTable();
    emit waypointsChanged(m_points);
}

void PathPlanningWidget::onDelete() {
    if (m_currentRow < 0 || m_currentRow >= m_points.size()) return;

    m_points.removeAt(m_currentRow);
    m_currentRow = -1;
    refreshTable();
    emit waypointsChanged(m_points);
}

void PathPlanningWidget::onClear() {
    if (m_points.isEmpty()) return;
    if (QMessageBox::question(this, "确认", "清空所有航点？") != QMessageBox::Yes)
        return;

    m_points.clear();
    m_currentRow = -1;
    refreshTable();
    emit waypointsChanged(m_points);
}

void PathPlanningWidget::onTableClicked(int row, int) {
    if (row < 0 || row >= m_points.size()) return;
    m_currentRow = row;

    const auto &p = m_points[row];
    m_lngSpin->setValue(p.longitude);
    m_latSpin->setValue(p.latitude);
    m_altSpin->setValue(p.altitude);
    m_speedSpin->setValue(p.speed);
}

void PathPlanningWidget::refreshTable() {
    m_table->setRowCount(m_points.size());
    for (int r = 0; r < m_points.size(); ++r) {
        const auto &p = m_points[r];
        m_table->setItem(r, 0, new QTableWidgetItem(QString::number(r + 1)));
        m_table->setItem(r, 1, new QTableWidgetItem(
            QString::number(p.longitude, 'f', 6)));
        m_table->setItem(r, 2, new QTableWidgetItem(
            QString::number(p.latitude, 'f', 6)));
        m_table->setItem(r, 3, new QTableWidgetItem(
            QString::number(p.altitude, 'f', 1)));
        m_table->setItem(r, 4, new QTableWidgetItem(
            QString::number(p.speed, 'f', 1)));
    }
}

void PathPlanningWidget::setWaypoints(const QList<WayPoint> &points) {
    m_points = points;
    m_currentRow = -1;
    refreshTable();
}

QList<WayPoint> PathPlanningWidget::waypoints() const {
    return m_points;
}
