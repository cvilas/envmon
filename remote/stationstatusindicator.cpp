#include "stationstatusindicator.h"

//=============================================================================
StationStatusIndicator::StationStatusIndicator(QWidget* parent)
//=============================================================================
    : Indicator(parent)
{
    setPixmap( QPixmap(":images/station_disabled.png"), Indicator::INACTIVE);
    setPixmap( QPixmap(":images/station_standby.png"), Indicator::STANDBY);
    setPixmap( QPixmap(":images/station_on.png"), Indicator::ON);
    setPixmap( QPixmap(":images/station_off.png"), Indicator::OFF);
}
