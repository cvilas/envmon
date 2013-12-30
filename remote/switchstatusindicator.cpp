#include "switchstatusindicator.h"

//=============================================================================
SwitchStatusIndicator::SwitchStatusIndicator(QWidget* parent)
//=============================================================================
    : Indicator(parent)
{
    setPixmap( QPixmap(":images/indicator_disabled.png"), Indicator::INACTIVE);
    setPixmap( QPixmap(":images/indicator_standby.png"), Indicator::STANDBY);
    setPixmap( QPixmap(":images/indicator_on.png"), Indicator::ON);
    setPixmap( QPixmap(":images/indicator_off.png"), Indicator::OFF);
}
