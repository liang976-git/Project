#ifndef ALTITUDEGAUGEWIDGET_H
#define ALTITUDEGAUGEWIDGET_H

#include <QWidget>

class AltitudeGaugeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AltitudeGaugeWidget(QWidget *parent = nullptr);

    void setValue(double meters);
    void setRange(double min, double max);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_value;
    double m_min;
    double m_max;
};

#endif
