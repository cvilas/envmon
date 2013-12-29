#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <mosquittopp.h>
#include <string>

class Subscriber : public mosqpp::mosquittopp
{
public:
    Subscriber(const char*id, const char* host, int port,
              const char* user, const char* pass, const char* topic);
    ~Subscriber();

    void on_connect(int rc);
    void on_message(const struct mosquitto_message *message);
    void on_subscribe(int mid, int qos_count, const int *granted_qos);
private:
    std::string m_topic;
};

#endif // SUBSCRIBER_H
