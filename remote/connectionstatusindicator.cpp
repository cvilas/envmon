#include "connectionstatusindicator.h"

//=============================================================================
ConnectionStatusIndicator::ConnectionStatusIndicator(QWidget* parent)
//=============================================================================
    : Indicator(parent)
{
    setPixmap( QPixmap(":images/cloud_disabled.png"), Indicator::INACTIVE);
    setPixmap( QPixmap(":images/cloud_standby.png"), Indicator::STANDBY);
    setPixmap( QPixmap(":images/cloud_on.png"), Indicator::ON);
    setPixmap( QPixmap(":images/cloud_off.png"), Indicator::OFF);
}
