#include "subscriber.h"
#include <iostream>

Subscriber::Subscriber(const char*id, const char* host, int port,
                     const char* user, const char* pass, const char* topic)
    : mosquittopp(id), m_topic(topic)
{
    int rc = username_pw_set(user, pass);

    if( MOSQ_ERR_SUCCESS != rc )
    {
        std::cout << "[Subscriber::Subscriber] username_pw_set failed" << std::endl;
    }

    connect(host, port, /*keepalive*/60);
}

Subscriber::~Subscriber()
{}

void Subscriber::on_connect(int rc)
{
    std::cout << "[Subscriber::on_connect] code " << rc << std::endl;
    if( rc == MOSQ_ERR_SUCCESS )
    {
        subscribe(NULL, m_topic.c_str(), /*qos*/0);
    }
}

void Subscriber::on_message(const struct mosquitto_message *message)
{
    int value = *(int*)message->payload;
    std::cout << "topic " << message->topic << " " << message->mid << " " << value << std::endl;
}

void Subscriber::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    std::cout << "Subscription succeeded. " << mid << " " << qos_count << std::endl;

}
