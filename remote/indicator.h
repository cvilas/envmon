#ifndef INDICATOR_H
#define INDICATOR_H

#include <QWidget>
#include <QMap>
#include <QPixmap>

//=============================================================================
/// \brief Visual status indicator base class
///
class Indicator : public QWidget
{
    Q_OBJECT

public:
    enum state
    {
        INACTIVE,
        STANDBY,
        OFF,
        TO_ON,
        ON,
        TO_OFF
    };

public:

    explicit Indicator(QWidget *parent = 0);

    virtual ~Indicator();

    /// set the current state
    void setState(Indicator::state st);

protected:

    /// set pixmap for a state.
    void setPixmap(const QPixmap& pixmap, Indicator::state st);

    /// reimplemented from base class
    void paintEvent(QPaintEvent* ev);

public slots:

    /// timer callback. Must be tied to a periodic timer
    void onTimer();

private:
    QMap<Indicator::state, QPixmap> _map;
    Indicator::state _state;
    long int _count;
};

#endif // INDICATOR_H
