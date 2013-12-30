#include "iotaclient.h"
#include "iota_messages.h"
#include "connection_info.h"
#include <QDebug>

//=============================================================================
IotaClient::IotaClient()
//=============================================================================
    : mosquittopp(NULL),
      _isConnected(false),
      _messageRecvd(false)
{
    mosqpp::lib_init();
    _timekeeper.start();
}

//-----------------------------------------------------------------------------
IotaClient::~IotaClient()
//-----------------------------------------------------------------------------
{
    mosqpp::mosquittopp::disconnect();
    mosqpp::mosquittopp::loop_stop(true);
    mosqpp::lib_cleanup();
}

//-----------------------------------------------------------------------------
bool IotaClient::connect()
//-----------------------------------------------------------------------------
{
    if( isConnected() )
    {
        mosqpp::mosquittopp::disconnect();
        mosqpp::mosquittopp::loop_stop(true);
        _isConnected = false;
    }

    // set connection credentials
    int rc = mosqpp::mosquittopp::username_pw_set(IOTA_BROKER_USER, IOTA_BROKER_PASS);
    if( MOSQ_ERR_SUCCESS != rc )
    {
        qDebug() << "[IotaClient::connect] username_pw_set() failed with code " << rc;
        return false;
    }

    // connect to broker
    rc = mosqpp::mosquittopp::connect(IOTA_BROKER_URL, IOTA_BROKER_PORT, /*keepalive*/60);
    if( MOSQ_ERR_SUCCESS != rc )
    {
        qDebug() << "[IotaClient::connect] connect() failed with code " << rc;
        return false;
    }

    // start loop
    rc = mosqpp::mosquittopp::loop_start();
    if( MOSQ_ERR_SUCCESS != rc )
    {
        qDebug() << "[IotaClient::connect] loop_start() failed with code " << rc;
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
int IotaClient::msSinceLastMessage() const
//-----------------------------------------------------------------------------
{
    return _timekeeper.elapsed();
}


//-----------------------------------------------------------------------------
bool IotaClient::setSwitchOn(int channel, bool on)
//-----------------------------------------------------------------------------
{
    if( !isConnected() )
    {
        return false;
    }

    switch_payload st;
    st.channel = channel;
    st.status = (on?1:0);
    return ( MOSQ_ERR_SUCCESS == mosqpp::mosquittopp::publish(NULL,
                                                              IOTA_TOPIC_SWITCH_CNTRL,
                                                              sizeof(st),
                                                              &st,
                                                              /*qos*/0,
                                                              /*retain*/false) );
}

//-----------------------------------------------------------------------------
void IotaClient::on_connect(int rc)
//-----------------------------------------------------------------------------
{
    _isConnected = (rc == MOSQ_ERR_SUCCESS);
    if( _isConnected )
    {
        rc = mosqpp::mosquittopp::subscribe(NULL, IOTA_TOPIC_SWITCH_STATUS,/*qos*/0);
    }
    emit connectionStatusChanged(_isConnected);
}

//-----------------------------------------------------------------------------
void IotaClient::on_disconnect(int rc)
//-----------------------------------------------------------------------------
{
    _isConnected = false;
    emit connectionStatusChanged(_isConnected);
}

//-----------------------------------------------------------------------------
void IotaClient::on_message(const struct mosquitto_message *message)
//-----------------------------------------------------------------------------
{
    _messageRecvd = true;
    _timekeeper.restart();

    qDebug() << "[IotaClient::on_message] " << message->topic;

    if( !strcmp(message->topic, IOTA_TOPIC_SWITCH_STATUS) )
    {
        switch_payload value = *(switch_payload*)message->payload;
        emit switchStatusChanged(value.channel, value.status);
    }//switch status reply
}

//-----------------------------------------------------------------------------
void IotaClient::on_subscribe(int mid, int qos_count, const int *granted_qos)
//-----------------------------------------------------------------------------
{
    qDebug() << "[IotaClient::on_subscribe] mid " << mid
             << " qos_count " << qos_count
             << " granted qos " << *granted_qos;
}

//-----------------------------------------------------------------------------
void IotaClient::on_unsubscribe(int mid)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
void IotaClient::on_publish(int mid)
//-----------------------------------------------------------------------------
{}


//-----------------------------------------------------------------------------
void IotaClient::on_log(int level, const char *str)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
void IotaClient::on_error()
//-----------------------------------------------------------------------------
{}
