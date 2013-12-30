#include "publisher.h"
#include <iostream>
#include <string.h>
#include "connection_info.h"
#include "iota_messages.h"

Publisher::Publisher()
    : mosquittopp(NULL)
{
    int rc = username_pw_set(IOTA_BROKER_USER, IOTA_BROKER_PASS);

    if( MOSQ_ERR_SUCCESS != rc )
    {
        std::cout << "[Publisher::Publisher] username_pw_set failed" << std::endl;
    }

    connect(IOTA_BROKER_URL, IOTA_BROKER_PORT, /*keepalive*/60);
}


Publisher::~Publisher()
{}

void Publisher::on_connect(int rc)
{
    std::cout << "[Publisher::on_connect] code " << rc << std::endl;
}

void Publisher::on_message(const struct mosquitto_message *message)
{}

void Publisher::on_subscribe(int mid, int qos_count, const int *granted_qos)
{}
