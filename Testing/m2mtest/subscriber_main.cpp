#include <iostream>
#include <mosquittopp.h>
#include "subscriber.h"

#define MQTT_SERVER "m10.cloudmqtt.com"
#define MQTT_PORT 10726
#define MQTT_USER "ymvhmlxw"
#define MQTT_PASS "Ind2uLCD4IJt"
#define MQTT_TOPIC "temp/device1"

int main(int argc, char *argv[])
{
    mosqpp::lib_init();

    Subscriber client(NULL, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS, MQTT_TOPIC);

    int mid = 0;
    while( 1 )
    {
        int rc  = client.loop();
        if( rc )
        {
            std::cout << "reconnecting on code " << rc << std::endl;
            client.reconnect();
        }
    }

    mosqpp::lib_cleanup();

    return 0;
}

