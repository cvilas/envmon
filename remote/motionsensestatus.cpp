#include "motionsensestatus.h"
#include <QPainter>
#include <QPaintEvent>

//=============================================================================
MotionSenseStatus::MotionSenseStatus(QWidget *parent)
//=============================================================================
    : QWidget(parent),
      _intruder(":images/intruder.png"),
      _flip(false)
{
}

//-----------------------------------------------------------------------------
void MotionSenseStatus::onTimer()
//-----------------------------------------------------------------------------
{
    if( _isOnNow )
    {
        _flip = !_flip;
        repaint();
    }
    else
    {
        _flip = false;
    }
}

//-----------------------------------------------------------------------------
void MotionSenseStatus::paintEvent(QPaintEvent *ev)
//-----------------------------------------------------------------------------
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    int logoSz = _intruder.width();
    int scaledLogoSz = qMin(logoSz, qMin(w/2, h)-20);

    QFont alertFont;
    alertFont.setBold(true);
    alertFont.setPointSize(20);
    painter.setFont(alertFont);

    int top = 10 + (h-scaledLogoSz)/2;
    QRect logoRect(10,top,scaledLogoSz,scaledLogoSz);
    QRect textRect(w/2+10,top+scaledLogoSz/2-20, w/2-20, 20);

    if( _isOnNow )
    {
        (_flip) ? (painter.setBrush(Qt::lightGray)) : (painter.setBrush(Qt::red));
        painter.setPen(Qt::red);
        painter.drawText(textRect, Qt::AlignCenter, "Motion Alert");
    }
    else
    {
        painter.setBrush(Qt::darkGray);
        painter.setPen(Qt::black);
        painter.drawText(textRect, Qt::AlignCenter, "Calm");
    }
    painter.drawEllipse(logoRect);
    painter.drawPixmap(logoRect, _intruder, QRect(0,0,logoSz,logoSz));

    alertFont.setPointSize(10);
    alertFont.setItalic(true);
    alertFont.setBold(false);
    painter.setFont(alertFont);
    QString str("since ");
    (_isOnNow) ? ( str += QString::number(_lastOnTime) )
               : ( str += QString::number( _lastOffTime) );
    str += " ms";
    painter.drawText(textRect.x(), textRect.y()+22, textRect.width(), textRect.height(),
                     Qt::AlignCenter, str);
}
