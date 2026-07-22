#ifndef COMPASSWIDGET_H
#define COMPASSWIDGET_H

#include <QWidget>

class CompassWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CompassWidget(QWidget *parent = nullptr);

    void setHeading(double degrees);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_heading;
};

#endif
