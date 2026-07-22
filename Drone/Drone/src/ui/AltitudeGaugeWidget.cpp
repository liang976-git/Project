#include "AltitudeGaugeWidget.h"
#include <QPainter>

AltitudeGaugeWidget::AltitudeGaugeWidget(QWidget *parent)
    : QWidget(parent), m_value(0), m_min(0), m_max(500)
{
    setMinimumSize(60, 120);
}

void AltitudeGaugeWidget::setValue(double meters)
{
    m_value = qBound(m_min, meters, m_max);
    update();
}

void AltitudeGaugeWidget::setRange(double min, double max)
{
    m_min = min;
    m_max = max;
    update();
}

void AltitudeGaugeWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int pad = 8;
    int barW = 20;
    int barX = w / 2 - barW / 2;
    int barH = h - 2 * pad;
    int barY = pad;

    p.setPen(QPen(QColor("#bdc3c7"), 1));
    p.setBrush(QColor("#ecf0f1"));
    p.drawRoundedRect(barX, barY, barW, barH, 3, 3);

    double ratio = (m_value - m_min) / (m_max - m_min);
    int fillH = static_cast<int>(barH * ratio);
    int fillY = barY + barH - fillH;

    QColor fillColor = (ratio < 0.2) ? QColor("#e74c3c")
                     : (ratio < 0.5) ? QColor("#f39c12")
                     :                 QColor("#27ae60");
    p.setBrush(fillColor);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(barX, fillY, barW, fillH, 2, 2);

    p.setPen(QPen(QColor("#7f8c8d"), 1));
    p.drawLine(barX - 4, barY + barH, barX + barW + 4, barY + barH);

    p.setPen(QPen(QColor("#95a5a6"), 1, Qt::DashLine));
    int midY = barY + barH / 2;
    p.drawLine(barX - 4, midY, barX + barW + 4, midY);

    QFont f = p.font();
    f.setBold(true);
    f.setPixelSize(12);
    p.setFont(f);
    p.setPen(QColor("#2c3e50"));
    p.drawText(QRect(0, h - pad - 14, w, 16), Qt::AlignCenter,
               QString("%1 m").arg(m_value, 0, 'f', 0));
}
