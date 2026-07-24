#ifndef GEOFENCEWIDGET_H
#define GEOFENCEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include "src/model/GeoFenceZone.h"

class GeoFenceWidget : public QWidget {
    Q_OBJECT
public:
    explicit GeoFenceWidget(QWidget *parent = nullptr);
    void refreshList();
    void onFenceDrawn(const QString &type, const QString &paramsJson);

signals:
    void fenceAdded(const GeoFenceZone &zone);
    void fenceUpdated(const GeoFenceZone &zone);
    void fenceRemoved(int zoneId);
    void fenceSelected(const GeoFenceZone &zone);
    void requestDrawFence(const QString &type);

private:
    void setupUI();
    void setupConnections();
    void onAdd();
    void onUpdate();
    void onDelete();
    void onTableClicked(int row, int col);
    void fillForm(const GeoFenceZone &zone);
    void clearForm();
    GeoFenceZone formToZone() const;

    QTableWidget *m_table;
    QLineEdit *m_nameEdit;
    QComboBox *m_typeCombo;
    QDoubleSpinBox *m_latSpin;
    QDoubleSpinBox *m_lngSpin;
    QDoubleSpinBox *m_radiusSpin;
    QCheckBox *m_enabledCheck;
    QPushButton *m_drawBtn;
    QComboBox *m_drawTypeCombo;
    QPushButton *m_addBtn;
    QPushButton *m_updateBtn;
    QPushButton *m_deleteBtn;
    int m_currentId;
};

#endif
