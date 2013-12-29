#include <iostream>
#include <mosquittopp.h>
#include "publisher.h"
#include <stdlib.h>

//#define MQTT_SERVER "mqtt://ymvhmlxw:Ind2uLCD4IJt@m10.cloudmqtt.com"
#define MQTT_SERVER "m10.cloudmqtt.com"
#define MQTT_PORT 10726
#define MQTT_USER "ymvhmlxw"
#define MQTT_PASS "Ind2uLCD4IJt"
#define MQTT_TOPIC "temp/device1"

int main(int argc, char *argv[])
{
    mosqpp::lib_init();

    Publisher client(NULL, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS);

    int mid = 0;
    while( 1 )
    {
        int rc  = client.loop();
        if( rc )
        {
            std::cout << "reconnecting on code " << rc << std::endl;
            client.reconnect();
        }

        int value = rand();
        ++mid;
        rc = client.publish(&mid, MQTT_TOPIC, sizeof(value), &value, /*qos*/0, /*retain*/false);
        if( MOSQ_ERR_SUCCESS != rc)
        {
            std::cout << "publish returned error " << rc << std::endl;
        }
        else
        {
            std::cout << "published " << mid << " " << value << std::endl;
        }
    }

    mosqpp::lib_cleanup();

    return 0;
}

