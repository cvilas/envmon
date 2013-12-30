#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <mosquittopp.h>

class Subscriber : public mosqpp::mosquittopp
{
public:
    Subscriber();
    ~Subscriber();

    void on_connect(int rc);
    void on_message(const struct mosquitto_message *message);
    void on_subscribe(int mid, int qos_count, const int *granted_qos);
};

#endif // SUBSCRIBER_H
