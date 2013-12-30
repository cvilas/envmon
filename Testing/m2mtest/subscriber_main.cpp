#include <iostream>
#include <mosquittopp.h>
#include "subscriber.h"


int main(int argc, char *argv[])
{
    mosqpp::lib_init();

    Subscriber client;

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

