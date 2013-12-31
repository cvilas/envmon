#include "indicator.h"
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>

//=============================================================================
Indicator::Indicator(QWidget *parent)
//=============================================================================
    :
    QWidget(parent),
    _state(Indicator::INACTIVE),
    _count(0)
{
}

//-----------------------------------------------------------------------------
Indicator::~Indicator()
//-----------------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------------
void Indicator::setPixmap(const QPixmap& pixmap, Indicator::state st)
//-----------------------------------------------------------------------------
{
    _map[st] = pixmap;
}

//-----------------------------------------------------------------------------
void Indicator::setState(Indicator::state st)
//-----------------------------------------------------------------------------
{
    _state = st;
}

//-----------------------------------------------------------------------------
void Indicator::onTimer()
//-----------------------------------------------------------------------------
{
    ++_count;
    repaint();
}

//-----------------------------------------------------------------------------
void Indicator::paintEvent(QPaintEvent* ev)
//-----------------------------------------------------------------------------
{
    // Behavior
    // INACTIVE, STANDBY, OFF, ON: show the corresponding pixmap
    // TO_ON  : switch between ON and STANDBY pixmaps
    // TO_OFF : switch between OFF and STANDBY pixmaps

    // todo:
    // - set pixmaps only on TO_.. states
    // - show STANDBY pixmap for even numbered calls

    QPixmap pixmap;
    switch( _state )
    {
    case Indicator::INACTIVE:
    case Indicator::STANDBY:
    case Indicator::OFF:
    case Indicator::ON:
    {
        pixmap = _map.value(_state);
        break;
    }
    case Indicator::TO_OFF:
    {
        if (_count%2==0 )
        {
            pixmap = _map.value(Indicator::STANDBY);
        }
        else
        {
            pixmap = _map.value(Indicator::OFF);
        }
        break;
    }
    case Indicator::TO_ON:
    {
        if (_count%2==0 )
        {
            pixmap = _map.value(Indicator::STANDBY);
        }
        else
        {
            pixmap = _map.value(Indicator::ON);
        }
        break;
    }
    default:
        break;
    };

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QSize sz = pixmap.size(); // original pixmap size
    QSize wsz = ev->rect().size(); // window size

    sz.scale( wsz.boundedTo(sz), Qt::KeepAspectRatio ); // shrink, but don't grow beyond pixmap size

    QPixmap scaledPixmap = pixmap.scaled( sz, Qt::KeepAspectRatio, Qt::SmoothTransformation );

    painter.drawPixmap(QPoint( (wsz.width() - sz.width())/2, (wsz.height() - sz.height())/2 ), scaledPixmap); // draw in the center
}

