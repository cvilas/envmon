#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include "mqttclient.h"
#include "connectionstatusindicator.h"
#include "stationstatusindicator.h"

namespace Ui {
class MainWindow;
}

//=============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionReconnect_triggered();
    void onTimer();

private:
    Ui::MainWindow* _pUi;
    QTimer          _timer; // Single timer to control all timed events
    MqttClient      _mqttClient;
    ConnectionStatusIndicator _connStatus;
    StationStatusIndicator _stationStatus;
};

#endif // MAINWINDOW_H
