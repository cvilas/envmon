#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <mosquittopp.h>

//=============================================================================
/// \brief Messaging client.
///
class MqttClient : public mosqpp::mosquittopp
{
public:
    MqttClient();
    virtual ~MqttClient();
    bool connect();
    bool isConnected() const { return _isConnected; }


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
}; // MqttClient

#endif // MQTTCLIENT_H
