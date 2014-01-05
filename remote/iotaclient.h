#ifndef IOTACLIENT_H
#define IOTACLIENT_H

#include <mosquittopp.h>
#include <QTime>
#include <inttypes.h>

//=============================================================================
/// \brief Messaging client.
///
class IotaClient : public QObject, public mosqpp::mosquittopp
{
    Q_OBJECT
public:
    IotaClient();
    virtual ~IotaClient();
    bool connect();
    void close(bool notify);
    bool isConnected() const { return _isConnected; }
    inline bool isStationActive() const;

    int msSinceLastMessage() const;

    bool setSwitchOn(int channel, bool on);
    bool setLamp(uint8_t r, uint8_t g, uint8_t b);

signals:
    void switchStatusChanged(int channel, bool on);
    void weatherStatusChanged(float celsius, int pascals, float altitudeMeters, int humidityPercent);
    void connectionStatusChanged(bool connected);
    void stationStatusChanged(bool active);

private:
    void on_connect(int rc);
    void on_disconnect(int rc);
    void on_publish(int mid);
    void on_message(const struct mosquitto_message *message);
    void on_subscribe(int mid, int qos_count, const int *granted_qos);
    void on_unsubscribe(int mid);
    void on_log(int level, const char *str);
    void on_error();

private:
    bool _isConnected;
    bool _messageRecvd;
    bool _messagePublished;
    QTime _timekeeper;
}; // MqttClient

//-----------------------------------------------------------------------------
bool IotaClient::isStationActive() const
//-----------------------------------------------------------------------------
{
    return _messageRecvd && (msSinceLastMessage() < 10000);
}

#endif // IOTACLIENT_H
