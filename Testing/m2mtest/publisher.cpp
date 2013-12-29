#include "publisher.h"
#include <iostream>

Publisher::Publisher(const char*id, const char* host, int port,
                     const char* user, const char* pass)
    : mosquittopp(id)
{
    int rc = username_pw_set(user, pass);

    if( MOSQ_ERR_SUCCESS != rc )
    {
        std::cout << "[Publisher::Publisher] username_pw_set failed" << std::endl;
    }

    connect(host, port, /*keepalive*/60);
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
