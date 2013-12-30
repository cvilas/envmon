#include "mqttclient.h"
#include "connection_info.h"
#include <QDebug>

//=============================================================================
MqttClient::MqttClient()
//=============================================================================
    : mosquittopp(NULL),
      _isConnected(false)
{
    mosqpp::lib_init();
}

//-----------------------------------------------------------------------------
MqttClient::~MqttClient()
//-----------------------------------------------------------------------------
{
    mosqpp::mosquittopp::disconnect();
    mosqpp::mosquittopp::loop_stop(true);
    mosqpp::lib_cleanup();
}

//-----------------------------------------------------------------------------
bool MqttClient::connect()
//-----------------------------------------------------------------------------
{
    if( isConnected() )
    {
        mosqpp::mosquittopp::disconnect();
        mosqpp::mosquittopp::loop_stop(true);
        _isConnected = false;
    }

    // set connection credentials
    int rc = mosqpp::mosquittopp::username_pw_set(MQTT_USER, MQTT_PASS);
    if( MOSQ_ERR_SUCCESS != rc )
    {
        qDebug() << "[MqttClient::connect] username_pw_set() failed with code " << rc;
        return false;
    }

    // connect to broker
    rc = mosqpp::mosquittopp::connect(MQTT_SERVER, MQTT_PORT, /*keepalive*/60);
    if( MOSQ_ERR_SUCCESS != rc )
    {
        qDebug() << "[MqttClient::connect] connect() failed with code " << rc;
        return false;
    }

    // start loop
    rc = mosqpp::mosquittopp::loop_start();
    if( MOSQ_ERR_SUCCESS != rc )
    {
        qDebug() << "[MqttClient::connect] loop_start() failed with code " << rc;
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
void MqttClient::on_connect(int rc)
//-----------------------------------------------------------------------------
{
    qDebug() << "[MqttClient::on_connect] " << rc;
    _isConnected = (rc == MOSQ_ERR_SUCCESS);
}

//-----------------------------------------------------------------------------
void MqttClient::on_disconnect(int rc)
//-----------------------------------------------------------------------------
{
    qDebug() << "[MqttClient::on_disconnect] " << rc;
    _isConnected = false;
}

//-----------------------------------------------------------------------------
void MqttClient::on_message(const struct mosquitto_message *message)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
void MqttClient::on_subscribe(int mid, int qos_count, const int *granted_qos)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
void MqttClient::on_unsubscribe(int mid)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
void MqttClient::on_publish(int mid)
//-----------------------------------------------------------------------------
{}


//-----------------------------------------------------------------------------
void MqttClient::on_log(int level, const char *str)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
void MqttClient::on_error()
//-----------------------------------------------------------------------------
{}
