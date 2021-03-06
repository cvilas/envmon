#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <mosquittopp.h>

class Publisher : public mosqpp::mosquittopp
{
public:
    Publisher();
    ~Publisher();

    void on_connect(int rc);
    void on_message(const struct mosquitto_message *message);
    void on_subscribe(int mid, int qos_count, const int *granted_qos);
};

#endif // PUBLISHER_H
