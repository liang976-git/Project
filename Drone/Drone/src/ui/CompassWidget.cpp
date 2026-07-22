#include "CompassWidget.h"
#include <QPainter>
#include <QtMath>

CompassWidget::CompassWidget(QWidget *parent)
    : QWidget(parent), m_heading(0)
{
    setMinimumSize(100, 100);
}

void CompassWidget::setHeading(double degrees)
{
    m_heading = degrees;
    update();
}

void CompassWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int side = qMin(width(), height());
    int cx = width() / 2;
    int cy = height() / 2;
    int r = side / 2 - 8;

    painter.setBrush(QColor("#ecf0f1"));
    painter.setPen(QPen(QColor("#2c3e50"), 2));
    painter.drawEllipse(QPoint(cx, cy), r, r);

    for (int i = 0; i < 8; ++i) {
        double a = qDegreesToRadians(i * 45.0 - 90);
        int ox = cx + r * cos(a);
        int oy = cy + r * sin(a);
        int ix = cx + (r - 10) * cos(a);
        int iy = cy + (r - 10) * sin(a);
        painter.setPen(QPen(QColor("#7f8c8d"), 1));
        painter.drawLine(ox, oy, ix, iy);
    }

    QFont f = painter.font();
    f.setBold(true);
    f.setPixelSize(12);
    painter.setFont(f);
    painter.setPen(QColor("#2c3e50"));
    painter.drawText(cx - 6, cy - r + 16, "N");
    painter.drawText(cx + r - 18, cy + 5, "E");
    painter.drawText(cx - 6, cy + r - 4, "S");
    painter.drawText(cx - r + 4, cy + 5, "W");

    double rad = qDegreesToRadians(m_heading - 90);
    int alen = r - 14;
    int tx = cx + alen * cos(rad);
    int ty = cy + alen * sin(rad);

    painter.setPen(QPen(QColor("#e74c3c"), 3));
    painter.drawLine(cx, cy, tx, ty);

    double ha = qDegreesToRadians(20.0);
    int hl = 12;
    int lx = tx - hl * cos(rad - ha);
    int ly = ty - hl * sin(rad - ha);
    int rx2 = tx - hl * cos(rad + ha);
    int ry = ty - hl * sin(rad + ha);

    QPolygon head;
    head << QPoint(tx, ty) << QPoint(lx, ly) << QPoint(rx2, ry);
    painter.setBrush(QColor("#e74c3c"));
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(head);

    painter.setBrush(QColor("#2c3e50"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPoint(cx, cy), 4, 4);

    f.setBold(false);
    f.setPixelSize(11);
    painter.setFont(f);
    painter.setPen(QColor("#2c3e50"));
    painter.drawText(cx - 20, cy + r + 14,
                     QString("%1°").arg(m_heading, 0, 'f', 1));
}
