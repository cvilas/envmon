#include "subscriber.h"
#include <iostream>
#include <string.h>
#include "connection_info.h"
#include "iota_messages.h"

Subscriber::Subscriber()
    : mosquittopp(NULL)
{
    int rc = username_pw_set(IOTA_BROKER_USER, IOTA_BROKER_PASS);

    if( MOSQ_ERR_SUCCESS != rc )
    {
        std::cout << "[Subscriber::Subscriber] username_pw_set failed" << std::endl;
    }

    connect(IOTA_BROKER_URL, IOTA_BROKER_PORT, /*keepalive*/60);
}

Subscriber::~Subscriber()
{}

void Subscriber::on_connect(int rc)
{
    std::cout << "[Subscriber::on_connect] code " << rc << std::endl;
    if( rc == MOSQ_ERR_SUCCESS )
    {
        subscribe(NULL, IOTA_TOPIC_SWITCH_CNTRL, /*qos*/0);
    }
}

void Subscriber::on_message(const struct mosquitto_message *message)
{
    if( !strcmp(message->topic, IOTA_TOPIC_SWITCH_CNTRL) )
    {
        switch_payload cmd = *(switch_payload*)message->payload;
        std::cout << "switch: " << cmd.channel << " " << cmd.status << std::endl;

        //reply
        switch_payload reply;
        reply.channel = cmd.channel;
        reply.status = cmd.status;
        publish(NULL, IOTA_TOPIC_SWITCH_STATUS, sizeof(reply), &reply);
    }
}

void Subscriber::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    std::cout << "Subscription succeeded. " << mid << " " << qos_count << std::endl;

}
