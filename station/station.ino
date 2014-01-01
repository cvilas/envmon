/// station.ino
/// Arduino control program for iota-station
/// 

#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "connection_info.h"
#include "iota_messages.h"

void setupWiFi();
void setupIotaBroker();
void initialiseIotaDevices();
void updateIotaStatus();
void iotaCallback(char* topic, byte* payload, unsigned int length);

WiFiClient wifiClient;
PubSubClient iotaClient(IOTA_BROKER_URL, IOTA_BROKER_PORT, iotaCallback, wifiClient);

int wifiStatus = WL_IDLE_STATUS;
unsigned long startTime = 0;
switch_payload switchStatus[IOTA_NUM_SWITCHES];

//=============================================================================
void setup()
//=============================================================================
{
  Serial.begin(9600);
  setupWiFi();
  setupIotaBroker(); 
  initialiseIotaDevices();
}


//=============================================================================
void loop()
//=============================================================================
{
  iotaClient.loop();
  
  // update all every 5 seconds
  unsigned long now = millis();
  if( now - startTime > 5000 )
  {
    startTime = now;
    updateIotaStatus();
  }
}

//=============================================================================
void setupWiFi()
//=============================================================================
{ 
  // check for the presence of wifi shield:
  if (WiFi.status() == WL_NO_SHIELD) 
  {
    Serial.println("I got no WiFi radio. FOOL"); 
    while(true);
  } 
  
  //Serial.print("\nWiFi Shield firmware version: ");
  //Serial.println( WiFi.firmwareVersion() );
  
  // connect to wifi network
  Serial.print("Contacting home WiFi Overlord.");
  int wifiConnAttempts = 1;
  while( wifiStatus != WL_CONNECTED && (wifiConnAttempts < 3) )
  {
    Serial.print(".");Serial.print(wifiConnAttempts);Serial.print(".");
    wifiStatus = WiFi.begin(HOME_WIFI_SSID, HOME_WIFI_PASS);
    //delay(10000);
    ++wifiConnAttempts;
  }
  
  // wifi connection status
  if( wifiStatus != WL_CONNECTED )
  {
    Serial.print("FAILED");
    Serial.println("Game over!");
    while(true);
  }
  else
  {
    // print the SSID of the network you're attached to:
    Serial.print("Oh hello! ");   
    Serial.print( WiFi.SSID() );
    long rssi = WiFi.RSSI();
    Serial.print(" [RSSI ");
    Serial.print(rssi);
    Serial.println(" dBm]");

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("I am : ");
    Serial.println(ip);
  }
}

//=============================================================================
void setupIotaBroker()
//=============================================================================
{
  Serial.print("Contacting iota cloud fairy.");
  boolean isConnected = false;
  int cloudConnAttempts = 1;
  while( !isConnected && (cloudConnAttempts < 4) )
  {
    Serial.print(".");Serial.print(cloudConnAttempts);Serial.print(".");
    isConnected = iotaClient.connect(NULL, IOTA_BROKER_USER, IOTA_BROKER_PASS);
    ++cloudConnAttempts;
  }
  
  // connection status
  if( !isConnected )
  {
    Serial.println("FAILED");
    Serial.println("Reset me and try again");
    while(true);
  }
  else
  {
    Serial.println("muuaahh!");   
  }
  
  // setup subscriptions
  iotaClient.subscribe(IOTA_TOPIC_SWITCH_CNTRL);
}

//=============================================================================
void initialiseIotaDevices()
//=============================================================================
{
  // reset all switches
  for(int i = 0; i < IOTA_NUM_SWITCHES; ++i)
  {
    switchStatus[i].channel = i;
    switchStatus[i].status = 0;
    
    // todo: send it to the device
  }
  
  // note start time
  startTime = millis();
}

//=============================================================================
void updateIotaStatus()
//=============================================================================
{
  // send switch status
  for(int i = 0; i < IOTA_NUM_SWITCHES; ++i)
  {
    switch_payload* pReply = &(switchStatus[i]);
    iotaClient.publish(IOTA_TOPIC_SWITCH_STATUS, (uint8_t*)pReply, sizeof(switch_payload));
  }
}

//=============================================================================
void iotaCallback(char* topic, byte* payload, unsigned int length)
//=============================================================================
{
    if( !strcmp(topic, IOTA_TOPIC_SWITCH_CNTRL) )
    {
      // cast to type
      switch_payload cmd = *(switch_payload*)payload;
      
      if( cmd.channel < IOTA_NUM_SWITCHES )
      {
        switchStatus[cmd.channel].status = cmd.status;
        
        // todo: send command to device
      }
      
      // respond to remote
      switch_payload* pReply = &(switchStatus[cmd.channel]);
      iotaClient.publish(IOTA_TOPIC_SWITCH_STATUS, (uint8_t*)pReply, sizeof(switch_payload));
    }//switch status reply

  // copy the payload
  
}


