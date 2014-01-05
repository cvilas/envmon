#include "iotaclient.h"
#include "iota_messages.h"
#include "connection_info.h"
#include <QDateTime>
#include <QThread>
#include <QDebug>

static client_state CLIENT_LAST_WILL{0,0};

//=============================================================================
IotaClient::IotaClient()
//=============================================================================
    : mosquittopp(NULL),
      _isConnected(false),
      _messageRecvd(false),
      _messagePublished(false)
{
    // random client session id
    qsrand( QDateTime::currentDateTime().toTime_t());
    CLIENT_LAST_WILL.sid = qrand();

    mosqpp::lib_init();
    _timekeeper.start();
}

//-----------------------------------------------------------------------------
IotaClient::~IotaClient()
//-----------------------------------------------------------------------------
{
#ifdef ANDROID
    close(true);
#else
    close(false);
#endif
    mosqpp::mosquittopp::loop_stop(true);
    mosqpp::lib_cleanup();
}

//-----------------------------------------------------------------------------
void IotaClient::close(bool notify)
//-----------------------------------------------------------------------------
{
    client_state st;
    st.sid = CLIENT_LAST_WILL.sid;
    st.state = 0;
    QString topic = QString(IOTA_TOPIC_CLIENT_STATE) + QString("/")
        + QString::number(CLIENT_LAST_WILL.sid,16);

    // let station know I am quitting
    if( notify )
    {
        _messagePublished = false;
        mosqpp::mosquittopp::publish(NULL,
                                     topic.toLocal8Bit().data(),
                                     sizeof(st),
                                     &st,
                                     0,//qos
                                     true); //retain
    }

    // remove the retained messages to avoid clogging the broker
    /// \bug
    /// - This doesn't seem to work, although doing the same thing from
    ///   arduino pubsubclient does
    _messagePublished = false;
    mosqpp::mosquittopp::publish(NULL,
                                 topic.toLocal8Bit().data(),
                                 0,
                                 NULL,
                                 0,//qos
                                 true); //retain

    /// wait for publish to happen before disconnecting
    unsigned int nTries = 10;
    while( !_messagePublished && (nTries < 100) )
    {
        ++nTries;
        QThread::currentThread()->msleep(10);
    }

    mosqpp::mosquittopp::disconnect();

}

//-----------------------------------------------------------------------------
bool IotaClient::connect()
//-----------------------------------------------------------------------------
{
    if( isConnected() )
    {
        close(true);
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


    // set last will and testament - lets broker know when we have unexpectedly quit

    mosqpp::mosquittopp::will_clear();

    QString topic = QString(IOTA_TOPIC_CLIENT_STATE) + QString("/")
            + QString::number(CLIENT_LAST_WILL.sid,16);
    rc = mosqpp::mosquittopp::will_set(topic.toLocal8Bit().data(),
                                       sizeof(CLIENT_LAST_WILL),
                                       &CLIENT_LAST_WILL,
                                       0,//qos
                                       true); //retain
    if( MOSQ_ERR_SUCCESS != rc )
    {
        qDebug() << "[IotaClient::connect] will_set() failed with code " << rc;
        return false;
    }


    // connect to broker
    rc = mosqpp::mosquittopp::connect(IOTA_BROKER_URL, IOTA_BROKER_PORT, /*keepalive*/60);
    if( MOSQ_ERR_SUCCESS != rc )
    {
        qDebug() << "[IotaClient::connect] connect() failed with code " << rc;
        return false;
    }

    // start loop in a separate thread
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
bool IotaClient::setLamp(uint8_t r, uint8_t g, uint8_t b)
//-----------------------------------------------------------------------------
{
    if( !isConnected() )
    {
        return false;
    }

    lamp_payload pl;
    pl.b = b;
    pl.g = g;
    pl.r = r;
    return ( MOSQ_ERR_SUCCESS == mosqpp::mosquittopp::publish(NULL,
                                                              IOTA_TOPIC_LAMP_CNTRL,
                                                              sizeof(pl),
                                                              &pl,
                                                              /*qos*/0,
                                                              /*retain*/false) );
}

//-----------------------------------------------------------------------------
void IotaClient::on_connect(int rc)
//-----------------------------------------------------------------------------
{
    _isConnected = (rc == MOSQ_ERR_SUCCESS);

    // subscribe to messages
    if( _isConnected )
    {
        rc = mosqpp::mosquittopp::subscribe(NULL, IOTA_TOPIC_SWITCH_STATUS,/*qos*/0);
        rc = mosqpp::mosquittopp::subscribe(NULL, IOTA_TOPIC_WEATHER,/*qos*/0);
        rc = mosqpp::mosquittopp::subscribe(NULL, IOTA_TOPIC_LAMP_STATUS,/*qos*/0);
    }

    // let station know of my existence
    client_state st;
    st.sid = CLIENT_LAST_WILL.sid;
    st.state = 1;
    QString topic = QString(IOTA_TOPIC_CLIENT_STATE) + QString("/")
            + QString::number(CLIENT_LAST_WILL.sid,16);
    rc = mosqpp::mosquittopp::publish(NULL,
                                      topic.toLocal8Bit().data(),
                                      sizeof(st),
                                      &st,
                                      0,//qos
                                      true);//retain

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

    // switches
    if( !strcmp(message->topic, IOTA_TOPIC_SWITCH_STATUS) )
    {
        if( message->payloadlen != sizeof(switch_payload) )
        {
            qDebug() << "[IotaClient::on_message] Topic " << message->topic
                     << ":\nPayload size incorrect. Ignoring";
            return;
        }
        switch_payload* pValue = (switch_payload*)message->payload;
        emit switchStatusChanged(pValue->channel, pValue->status);
    }//switch status reply

    if( !strcmp(message->topic, IOTA_TOPIC_WEATHER) )
    {
        if( message->payloadlen != sizeof(weather_payload) )
        {
            qDebug() << "[IotaClient::on_message] Topic " << message->topic
                     << ":\nPayload size incorrect. Ignoring";
            return;
        }
        weather_payload* pValue = (weather_payload*)message->payload;
        emit weatherStatusChanged(pValue->temperature10C/10.f, pValue->pressurePa, pValue->pressureAlt10m/10.f, pValue->humidityPercent);
    }//weather
}

//-----------------------------------------------------------------------------
void IotaClient::on_subscribe(int mid, int qos_count, const int *granted_qos)
//-----------------------------------------------------------------------------
{
    /*
    qDebug() << "[IotaClient::on_subscribe] mid " << mid
             << " qos_count " << qos_count
             << " granted qos " << *granted_qos;
             */
}

//-----------------------------------------------------------------------------
void IotaClient::on_unsubscribe(int mid)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
void IotaClient::on_publish(int mid)
//-----------------------------------------------------------------------------
{
    _messagePublished = true;
    //qDebug() << "[IotaClient::on_publish] " << mid;
}


//-----------------------------------------------------------------------------
void IotaClient::on_log(int level, const char *str)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
void IotaClient::on_error()
//-----------------------------------------------------------------------------
{}
