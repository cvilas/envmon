#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

//=============================================================================
MainWindow::MainWindow(QWidget *parent)
//=============================================================================
    :
    QMainWindow(parent),
    _pUi(new Ui::MainWindow)
{
    _pUi->setupUi(this);
    _connStatus.setMinimumWidth(_pUi->statusBar->height());
    _stationStatus.setMinimumWidth(_pUi->statusBar->height());

    _pUi->statusBar->addPermanentWidget(&_connStatus, 0);
    _pUi->statusBar->addPermanentWidget(&_stationStatus, 0);

    _pUi->centralWidget->setEnabled(false);

    QObject::connect(&_timer, SIGNAL(timeout()), _pUi->swStatus0, SLOT(onTimer()));
    QObject::connect(&_timer, SIGNAL(timeout()), _pUi->swStatus1, SLOT(onTimer()));
    QObject::connect(&_timer, SIGNAL(timeout()), _pUi->swStatus2, SLOT(onTimer()));
    QObject::connect(&_timer, SIGNAL(timeout()), &_connStatus, SLOT(onTimer()));
    QObject::connect(&_timer, SIGNAL(timeout()), &_stationStatus, SLOT(onTimer()));
    QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    QObject::connect(&_iotaClient, SIGNAL(switchStatusChanged(int,bool)), this, SLOT(onSwitchStatusChanged(int,bool)));
    QObject::connect(&_iotaClient, SIGNAL(weatherStatusChanged(float,int,float,int)), this, SLOT(onWeatherStatusChanged(float, int, float, int)));
    QObject::connect(&_iotaClient, SIGNAL(connectionStatusChanged(bool)), this, SLOT(onConnectionStatusChanged(bool)));

    on_actionReconnect_triggered();

    this->_timer.start(500);
}

//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
//-----------------------------------------------------------------------------
{
    this->_timer.stop();
    delete _pUi;
}

//-----------------------------------------------------------------------------
void MainWindow::on_actionReconnect_triggered()
//-----------------------------------------------------------------------------
{
    _iotaClient.connect();
    _connStatus.setState( Indicator::OFF );
}

//-----------------------------------------------------------------------------
void MainWindow::onConnectionStatusChanged(bool connected)
//-----------------------------------------------------------------------------
{
    _connStatus.setState( connected ? Indicator::ON : Indicator::OFF );
    _pUi->swStatus0->setState( Indicator::INACTIVE );
    _pUi->swStatus1->setState( Indicator::INACTIVE );
    _pUi->swStatus2->setState( Indicator::INACTIVE );
    _pUi->centralWidget->setEnabled(connected);
}

//-----------------------------------------------------------------------------
void MainWindow::onTimer()
//-----------------------------------------------------------------------------
{
    bool isStationActive = _iotaClient.isStationActive();
    _stationStatus.setState( isStationActive ? Indicator::ON : Indicator::OFF );
    _pUi->switchGroup->setEnabled( isStationActive );
    if( !isStationActive )
    {
        _pUi->swStatus0->setState( Indicator::INACTIVE );
        _pUi->swStatus1->setState( Indicator::INACTIVE );
        _pUi->swStatus2->setState( Indicator::INACTIVE );
    }
}

//-----------------------------------------------------------------------------
void MainWindow::on_offBtn0_clicked()
//-----------------------------------------------------------------------------
{
    _iotaClient.setSwitchOn(0,false);
    _pUi->swStatus0->setState(Indicator::TO_OFF);
}

//-----------------------------------------------------------------------------
void MainWindow::on_onBtn0_clicked()
//-----------------------------------------------------------------------------
{
    _iotaClient.setSwitchOn(0,true);
    _pUi->swStatus0->setState(Indicator::TO_ON);
}

//-----------------------------------------------------------------------------
void MainWindow::on_offBtn1_clicked()
//-----------------------------------------------------------------------------
{
    _iotaClient.setSwitchOn(1,false);
    _pUi->swStatus1->setState(Indicator::TO_OFF);
}

//-----------------------------------------------------------------------------
void MainWindow::on_onBtn1_clicked()
//-----------------------------------------------------------------------------
{
    _iotaClient.setSwitchOn(1,true);
    _pUi->swStatus1->setState(Indicator::TO_ON);
}

//-----------------------------------------------------------------------------
void MainWindow::on_offBtn2_clicked()
//-----------------------------------------------------------------------------
{
    _iotaClient.setSwitchOn(2,false);
    _pUi->swStatus2->setState(Indicator::TO_OFF);
}

//-----------------------------------------------------------------------------
void MainWindow::on_onBtn2_clicked()
//-----------------------------------------------------------------------------
{
    _iotaClient.setSwitchOn(2,true);
    _pUi->swStatus2->setState(Indicator::TO_ON);
}

//-----------------------------------------------------------------------------
void MainWindow::onSwitchStatusChanged(int channel, bool on)
//-----------------------------------------------------------------------------
{
    Indicator::state state = ((on) ? (Indicator::ON) : (Indicator::OFF));
    switch(channel)
    {
    case 0:
        _pUi->swStatus0->setState(state);
        break;
    case 1:
        _pUi->swStatus1->setState(state);
        break;
    case 2:
        _pUi->swStatus2->setState(state);
        break;
    default:
        break;
    };//switch channel
}

//-----------------------------------------------------------------------------
void MainWindow::onWeatherStatusChanged(float celsius, int pascals, float altitudeMeters, int humidityPercent)
//-----------------------------------------------------------------------------
{
    _pUi->tempValue->setText( QString::number(celsius) );
    _pUi->pressValue->setText( QString::number(pascals) );
    _pUi->altValue->setText( QString::number(altitudeMeters) );
    _pUi->humidValue->setText( QString::number(humidityPercent) );
}

//-----------------------------------------------------------------------------
void MainWindow::on_redLampValue_valueChanged(int value)
//-----------------------------------------------------------------------------
{
    _iotaClient.setLamp(_pUi->redLampValue->value(),
                        _pUi->greenLampValue->value(),
                        _pUi->blueLampValue->value());
}

//-----------------------------------------------------------------------------
void MainWindow::on_greenLampValue_valueChanged(int value)
//-----------------------------------------------------------------------------
{
    _iotaClient.setLamp(_pUi->redLampValue->value(),
                        _pUi->greenLampValue->value(),
                        _pUi->blueLampValue->value());
}

//-----------------------------------------------------------------------------
void MainWindow::on_blueLampValue_valueChanged(int value)
//-----------------------------------------------------------------------------
{
    _iotaClient.setLamp(_pUi->redLampValue->value(),
                        _pUi->greenLampValue->value(),
                        _pUi->blueLampValue->value());
}
