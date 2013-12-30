#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    QObject::connect(&_timer, SIGNAL(timeout()), _pUi->swStatus0, SLOT(onTimer()));
    QObject::connect(&_timer, SIGNAL(timeout()), _pUi->swStatus1, SLOT(onTimer()));
    QObject::connect(&_timer, SIGNAL(timeout()), _pUi->swStatus2, SLOT(onTimer()));
    QObject::connect(&_timer, SIGNAL(timeout()), &_connStatus, SLOT(onTimer()));
    QObject::connect(&_timer, SIGNAL(timeout()), &_stationStatus, SLOT(onTimer()));
    QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(onTimer()));

    _mqttClient.connect();
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
    _mqttClient.connect();
}

//-----------------------------------------------------------------------------
void MainWindow::onTimer()
//-----------------------------------------------------------------------------
{
    _connStatus.setState( _mqttClient.isConnected() ? Indicator::ON : Indicator::OFF );
}
