#ifndef MOTIONSENSESTATUS_H
#define MOTIONSENSESTATUS_H

#include <QWidget>
#include <QPixmap>
#include <inttypes.h>

class QPaintEvent;

//=============================================================================
/// \brief UI class to show motion sensor status.
///
class MotionSenseStatus : public QWidget
{
    Q_OBJECT

public:
    explicit MotionSenseStatus(QWidget *parent = 0);
public slots:
    inline void set(unsigned int lastOffTimeMs, unsigned int lastOnTime, bool isOnNow);
    void onTimer();
private:
    void paintEvent(QPaintEvent* ev);
private:
    uint32_t _lastOffTime;
    uint32_t _lastOnTime;
    bool _isOnNow;
    QPixmap _intruder;
    bool _flip;
}; //MotionSenseStatus

//-----------------------------------------------------------------------------
void MotionSenseStatus::set(uint32_t lastOffTimeMs, uint32_t lastOnTime, bool isOnNow)
//-----------------------------------------------------------------------------
{
    _lastOffTime = lastOffTimeMs;
    _lastOnTime= lastOnTime;
    _isOnNow = isOnNow;
    update();
}

#endif // MOTIONSENSESTATUS_H
