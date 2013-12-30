#include <iostream>
#include <mosquittopp.h>
#include "publisher.h"
#include <stdlib.h>
#include "iota_messages.h"

int main(int argc, char *argv[])
{
    mosqpp::lib_init();

    Publisher client;

    int mid = 0;
    while( 1 )
    {
        int rc  = client.loop();
        if( rc )
        {
            std::cout << "reconnecting on code " << rc << std::endl;
            client.reconnect();
        }

        switch_payload cmd;
        cmd.channel = 0;
        cmd.status = 1;
        rc = client.publish(NULL, IOTA_TOPIC_SWITCH_STATUS, sizeof(cmd), &cmd, /*qos*/0, /*retain*/false);
        if( MOSQ_ERR_SUCCESS != rc)
        {
            std::cout << "publish returned error " << rc << std::endl;
        }
        else
        {
            std::cout << "published " << cmd.channel << " " << cmd.status << std::endl;
        }
    }

    mosqpp::lib_cleanup();

    return 0;
}

