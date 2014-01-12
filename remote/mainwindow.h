#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include "iotaclient.h"
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
    void onSwitchStatusChanged(int channel, bool on);
    void onWeatherStatusChanged(float celsius, int pascals, float altitudeMeters, int humidityPercent);
    void onConnectionStatusChanged(bool connected);
    void onTimer();

    void on_offBtn0_clicked();
    void on_onBtn0_clicked();
    void on_offBtn1_clicked();
    void on_onBtn1_clicked();
    void on_offBtn2_clicked();
    void on_onBtn2_clicked();
    void on_redLampValue_valueChanged(int value);
    void on_greenLampValue_valueChanged(int value);
    void on_blueLampValue_valueChanged(int value);

private:
    Ui::MainWindow* _pUi;
    QTimer          _timer; // Single timer to control all timed events
    IotaClient      _iotaClient;
    ConnectionStatusIndicator _connStatus;
    StationStatusIndicator _stationStatus;
};

#endif // MAINWINDOW_H
