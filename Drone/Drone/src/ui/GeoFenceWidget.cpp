#include "GeoFenceWidget.h"
#include "src/database/GeoFenceDAO.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>

GeoFenceWidget::GeoFenceWidget(QWidget *parent)
    : QWidget(parent), m_currentId(-1)
{
    setupUI();
    setupConnections();
    refreshList();
}

void GeoFenceWidget::setupUI() {
    auto *main = new QVBoxLayout(this);

    // 表格
    m_table = new QTableWidget(0, 5, this);
    m_table->setHorizontalHeaderLabels({"ID", "名称", "类型", "状态", "半径/顶点"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    main->addWidget(m_table);

    // 编辑区
    auto *formGroup = new QGroupBox("禁飞区编辑");
    auto *form = new QFormLayout(formGroup);

    m_nameEdit = new QLineEdit;
    m_typeCombo = new QComboBox;
    m_typeCombo->addItems({"圆形", "矩形", "多边形"});
    m_latSpin = new QDoubleSpinBox;
    m_latSpin->setRange(-90, 90);
    m_latSpin->setDecimals(6);
    m_lngSpin = new QDoubleSpinBox;
    m_lngSpin->setRange(-180, 180);
    m_lngSpin->setDecimals(6);
    m_radiusSpin = new QDoubleSpinBox;
    m_radiusSpin->setRange(0, 100000);
    m_radiusSpin->setSuffix(" m");
    m_enabledCheck = new QCheckBox("启用");

    form->addRow("名称:", m_nameEdit);
    form->addRow("类型:", m_typeCombo);
    form->addRow("纬度:", m_latSpin);
    form->addRow("经度:", m_lngSpin);
    form->addRow("半径:", m_radiusSpin);
    form->addRow("", m_enabledCheck);
    main->addWidget(formGroup);

    // 按钮
    auto *btnLayout = new QHBoxLayout;
    m_addBtn = new QPushButton("新增");
    m_updateBtn = new QPushButton("更新");
    m_deleteBtn = new QPushButton("删除");
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_updateBtn);
    btnLayout->addWidget(m_deleteBtn);
    main->addLayout(btnLayout);

    auto *drawGroup = new QGroupBox("地图绘制");
    auto *drawLayout = new QHBoxLayout(drawGroup);
    m_drawTypeCombo = new QComboBox;
    m_drawTypeCombo->addItems({"圆形","矩形","多边形"});
    m_drawBtn = new QPushButton("在地图上绘制");
    drawLayout->addWidget(m_drawTypeCombo);
    drawLayout->addWidget(m_drawBtn);
    main->addWidget(drawGroup);
}

void GeoFenceWidget::setupConnections() {
    connect(m_table, &QTableWidget::cellClicked, this, &GeoFenceWidget::onTableClicked);
    connect(m_addBtn, &QPushButton::clicked, this, &GeoFenceWidget::onAdd);
    connect(m_updateBtn, &QPushButton::clicked, this, &GeoFenceWidget::onUpdate);
    connect(m_deleteBtn, &QPushButton::clicked, this, &GeoFenceWidget::onDelete);
    connect(m_drawBtn, &QPushButton::clicked, this, [this]{
        QString type;
        switch(m_drawTypeCombo->currentIndex()){
            case 0: type="circle"; break;
            case 1: type="rectangle"; break;
            case 2: type="polygon"; break;
        }
        emit requestDrawFence(type);
    });
}

void GeoFenceWidget::refreshList() {
    auto zones = GeoFenceDAO::instance().findAll();
    m_table->setRowCount(zones.size());
    for (int r = 0; r < zones.size(); ++r) {
        const auto &z = zones[r];
        m_table->setItem(r, 0, new QTableWidgetItem(QString::number(z.id)));
        m_table->setItem(r, 1, new QTableWidgetItem(z.name));
        m_table->setItem(r, 2, new QTableWidgetItem(
            z.type == Circle ? "圆形" : z.type == Rectangle ? "矩形" : "多边形"));
        auto *statusItem = new QTableWidgetItem(z.enabled ? "启用" : "禁用");
        statusItem->setForeground(z.enabled ? QColor("#27ae60") : QColor("#e74c3c"));
        m_table->setItem(r, 3, statusItem);
        QString detail;
        if (z.type == Circle)
            detail = QString("半径 %1m").arg(z.radius, 0, 'f', 0);
        else
            detail = QString("%1 个顶点").arg(z.points.size());
        m_table->setItem(r, 4, new QTableWidgetItem(detail));
    }
}

void GeoFenceWidget::onTableClicked(int row, int) {
    auto id = m_table->item(row, 0)->text().toInt();
    auto zone = GeoFenceDAO::instance().findById(id);
    fillForm(zone);
    emit fenceSelected(zone);
}

void GeoFenceWidget::onAdd() {
    auto zone = formToZone();
    zone.id = 0;
    if (GeoFenceDAO::instance().insert(zone)) {
        refreshList();
        clearForm();
        emit fenceAdded(zone);
    } else {
        QMessageBox::warning(this, "错误", "新增禁飞区失败");
    }
}

void GeoFenceWidget::onUpdate() {
    if (m_currentId < 0) return;
    auto zone = formToZone();
    zone.id = m_currentId;
    if (GeoFenceDAO::instance().update(zone)) {
        refreshList();
        emit fenceUpdated(zone);
    } else {
        QMessageBox::warning(this, "错误", "更新禁飞区失败");
    }
}

void GeoFenceWidget::onDelete() {
    if (m_currentId < 0) return;
    if (QMessageBox::question(this, "确认", "确认删除此禁飞区？") == QMessageBox::Yes) {
        if (GeoFenceDAO::instance().remove(m_currentId)) {
            refreshList();
            clearForm();
            emit fenceRemoved(m_currentId);
            m_currentId = -1;
        }
    }
}

void GeoFenceWidget::fillForm(const GeoFenceZone &zone) {
    m_currentId = zone.id;
    m_nameEdit->setText(zone.name);
    m_typeCombo->setCurrentIndex(static_cast<int>(zone.type));
    m_latSpin->setValue(zone.centerLat);
    m_lngSpin->setValue(zone.centerLng);
    m_radiusSpin->setValue(zone.radius);
    m_enabledCheck->setChecked(zone.enabled);
}

void GeoFenceWidget::clearForm() {
    m_currentId = -1;
    m_nameEdit->clear();
    m_typeCombo->setCurrentIndex(0);
    m_latSpin->setValue(0);
    m_lngSpin->setValue(0);
    m_radiusSpin->setValue(0);
    m_enabledCheck->setChecked(true);
}

void GeoFenceWidget::onFenceDrawn(const QString &type, const QString &paramsJson) {
    QJsonDocument doc = QJsonDocument::fromJson(paramsJson.toUtf8());
    QJsonObject obj = doc.object();
    if (type == "circle") {
        m_typeCombo->setCurrentIndex(0);
        m_latSpin->setValue(obj["centerLat"].toDouble());
        m_lngSpin->setValue(obj["centerLng"].toDouble());
        m_radiusSpin->setValue(obj["radius"].toDouble());
    } else if (type == "rectangle") {
        m_typeCombo->setCurrentIndex(1);
        double centerLat = (obj["swLat"].toDouble() + obj["neLat"].toDouble()) / 2.0;
        double centerLng = (obj["swLng"].toDouble() + obj["neLng"].toDouble()) / 2.0;
        m_latSpin->setValue(centerLat);
        m_lngSpin->setValue(centerLng);
        m_radiusSpin->setValue(0);
    } else if (type == "polygon") {
        m_typeCombo->setCurrentIndex(2);
        m_radiusSpin->setValue(0);
    }
    m_currentId = -1;
}

GeoFenceZone GeoFenceWidget::formToZone() const {
    GeoFenceZone z;
    z.name = m_nameEdit->text();
    z.type = static_cast<ZoneType>(m_typeCombo->currentIndex());
    z.centerLat = m_latSpin->value();
    z.centerLng = m_lngSpin->value();
    z.radius = m_radiusSpin->value();
    z.enabled = m_enabledCheck->isChecked();
    return z;
}
