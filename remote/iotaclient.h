#ifndef IOTACLIENT_H
#define IOTACLIENT_H

#include <mosquittopp.h>
#include <QTime>

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
    bool isConnected() const { return _isConnected; }
    inline bool isStationActive() const;

    int msSinceLastMessage() const;

    bool setSwitchOn(int channel, bool on);

signals:
    void switchStatusChanged(int channel, bool on);
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
    QTime _timekeeper;
}; // MqttClient

//-----------------------------------------------------------------------------
bool IotaClient::isStationActive() const
//-----------------------------------------------------------------------------
{
    return _messageRecvd && (msSinceLastMessage() < 10000);
}

#endif // IOTACLIENT_H
