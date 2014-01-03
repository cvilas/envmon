/// station.ino
/// Arduino control program for iota-station
/// 

#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include "connection_info.h"
#include "iota_messages.h"
#include "helper_defs.h"

#define IDLE_UPDATE_TIME_MS 10000

void setupWiFi();
void setupIotaBroker();
void initialiseIotaDevices();
void updateIotaStatus();
void iotaCallback(char* topic, byte* payload, unsigned int length);

// Devices
WiFiClient    wifiClient;
RCSwitch      switchController;
PubSubClient  iotaClient(IOTA_BROKER_URL, IOTA_BROKER_PORT, iotaCallback, wifiClient);

unsigned long startTime = 0;
int numActiveClients = 0;
switch_payload switchStatus[IOTA_NUM_SWITCHES];

//=============================================================================
void setup()
//=============================================================================
{
  Serial.begin(9600);
  DEBUGPRINTLN("Starting up");
  setupWiFi();
  setupIotaBroker(); 
  initialiseIotaDevices();
  updateIotaStatus();
}


//=============================================================================
void loop()
//=============================================================================
{
  iotaClient.loop();
  
  // update all 
  // note: using unsigned int for subtraction means this works even when
  // millis() rolls over after 50 days.
  unsigned long now = millis();
  if( now - startTime > IDLE_UPDATE_TIME_MS )
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
    DEBUGPRINT(__FUNCTION__);
    DEBUGPRINTLN(": No WiFi radio"); 
    while(true);
  } 
   
  // connect to wifi network
  int wifiConnAttempts = 1;
  int wifiStatus = WL_IDLE_STATUS;
  while( wifiStatus != WL_CONNECTED && (wifiConnAttempts < 3) )
  {
    wifiStatus = WiFi.begin(HOME_WIFI_SSID, HOME_WIFI_PASS);
    //delay(10000);
    ++wifiConnAttempts;
  }
  
  // wifi connection status
  if( wifiStatus != WL_CONNECTED )
  {
    DEBUGPRINT(__FUNCTION__);
    DEBUGPRINT(": WiFi connection failed");
    while(true);
  }
  else
  {
    // print the SSID of the network you're attached to:
    DEBUGPRINT(__FUNCTION__);
    DEBUGPRINT(": Connected to ");   
    DEBUGPRINTLN( WiFi.SSID() );
  }
}

//=============================================================================
void setupIotaBroker()
//=============================================================================
{
  boolean isConnected = false;
  int cloudConnAttempts = 1;
  while( !isConnected && (cloudConnAttempts < 4) )
  {
    isConnected = iotaClient.connect(NULL, IOTA_BROKER_USER, IOTA_BROKER_PASS);
    ++cloudConnAttempts;
  }
  
  // connection status
  DEBUGPRINT(__FUNCTION__);
  DEBUGPRINT(": Connection to ");
  DEBUGPRINT(IOTA_BROKER_URL);
  if( !isConnected )
  {
    DEBUGPRINTLN(" failed");
    DEBUGPRINTLN("Reset me and try again");
    while(true);
  }
  else
  {
    DEBUGPRINTLN(" succeeded");   
  }
  
  // setup subscriptions
  DEBUGPRINT(__FUNCTION__);
  DEBUGPRINTLN(": Subscriptions:");
  
  //special message to determine how many clients are connected
  String topicStr = IOTA_TOPIC_CLIENT_STATE;
  topicStr += "/+1";
  int len = topicStr.length()*sizeof(char);
  char* topic = (char*)malloc(len+1);
  topic[len] = '\0';
  topicStr.toCharArray(topic,len); 
  DEBUGPRINT("- "); DEBUGPRINTLN(topic);
  iotaClient.subscribe(topic);
  free(topic);
  
  DEBUGPRINT("- "); DEBUGPRINTLN(IOTA_TOPIC_SWITCH_CNTRL);
  iotaClient.subscribe(IOTA_TOPIC_SWITCH_CNTRL);
}

//=============================================================================
void initialiseIotaDevices()
//=============================================================================
{
  // ------------------- Switches -----------------------------
  // Switch tx is connected to this pin
  switchController.enableTransmit(IOTA_SWITCH_TX_PIN);
  switchController.setPulseLength(IOTA_SWITCH_PULSE_LEN);
  
  // reset all switches 
  DEBUGPRINT(__FUNCTION__);
  DEBUGPRINTLN(": Resetting switches.");   
  for(int i = 0; i < IOTA_NUM_SWITCHES; ++i)
  {
    switchController.switchOff(IOTA_SWITCH_SYSTEM_CODE,IOTA_SWITCH_UNIT_CODE[i]);
    switchStatus[i].channel = i;
    switchStatus[i].status = 0;
  }  
  
  // ------------------ Temp./Press. sensor ---------------------
  // todo: read sensor

  // ------------------ battery sensor ---------------------
  // todo: read sensor

  // ------------------ PIR sensor ---------------------
  // todo: read sensor

  // ------------------ NTP ---------------------
  // todo: read time. set rtc

  // ------------------ SD card logging ---------------------
  // todo: read sensor
  
  // note start time
  startTime = millis();
}

//=============================================================================
void updateIotaStatus()
//=============================================================================
{
  // ------------------- Switches -----------------------------
  for(int i = 0; i < IOTA_NUM_SWITCHES; ++i)
  {
    switch_payload* pReply = &(switchStatus[i]);
    if( numActiveClients )
    {
      iotaClient.publish(IOTA_TOPIC_SWITCH_STATUS, (uint8_t*)pReply, sizeof(switch_payload));
    }
  }
  
  // ------------------ Temp./Press. sensor ---------------------
  // todo: 
  // - read sensor, publish
  // - once an hour log to SD card

  // ------------------ battery sensor ---------------------
  // todo: 
  // - read sensor, publish
  // - once every 10 minutes, log to SD card

  // ------------------ PIR sensor ---------------------
  // todo: 
  // - move to loop
  // - on motion detection, publish with time
  // - log to SD card

  // ------------------ NTP ---------------------
  // todo: 
  // - every 5 hours, read and update RTC
  // - publish the event, with 'before' and 'after' time from RTC
  // - log the event, with 'before' and 'after' time

  // ------------------ SD card logging ---------------------
  // todo: read sensor
}

//=============================================================================
void iotaCallback(char* topic, byte* payload, unsigned int length)
//=============================================================================
{
 
  // ------------------- Client state -----------------------------
  
  if( strstr(topic, IOTA_TOPIC_CLIENT_STATE) )
  {   
    if( length )
    {
      client_state cmd = *(client_state*)payload;
      DEBUGPRINT(__FUNCTION__);
      DEBUGPRINT(": Client ");
      DEBUGPRINTHEX(cmd.sid);
      DEBUGPRINT(" ");
      if( cmd.state )
      {
        DEBUGPRINT("alive");
        numActiveClients++;
        updateIotaStatus();
      }
      else
      {
        DEBUGPRINT("dead");
        numActiveClients--;
        
        // bug in the client code means we can get 2 'dead' messages
        // from a clientexit
        if( numActiveClients < 0) numActiveClients = 0;
      }
      DEBUGPRINT(" [active clients = ");
      DEBUGPRINT(numActiveClients);
      DEBUGPRINTLN("]");
      if( cmd.state == 0 ) // remove redundant messages from broker
      {
        iotaClient.publish(topic, NULL, 0, true);
      }
    }
    return;
  } // client state
  
  // ------------------- Switches -----------------------------

  if( !strcmp(topic, IOTA_TOPIC_SWITCH_CNTRL) )
  {
    if( length != sizeof(switch_payload) )
    {
      DEBUGPRINT(__FUNCTION__);
      DEBUGPRINTLN(": ERROR: Payload size incorrect. Ignoring.");
      return;
    }
    
    // cast to type
    switch_payload cmd = *(switch_payload*)payload;
    DEBUGPRINT(__FUNCTION__);
    DEBUGPRINT(": Set Switch ");
    DEBUGPRINT(cmd.channel);
    DEBUGPRINT(" -> ");
    DEBUGPRINTLN(cmd.status?("ON"):("OFF"));
      
    if( cmd.channel < IOTA_NUM_SWITCHES )
    {     
      // send command to device
      if( cmd.status )
      {
        switchController.switchOn(IOTA_SWITCH_SYSTEM_CODE,IOTA_SWITCH_UNIT_CODE[cmd.channel]);
      }
      else
      {
        switchController.switchOff(IOTA_SWITCH_SYSTEM_CODE,IOTA_SWITCH_UNIT_CODE[cmd.channel]);
      }
      
      // copy status
      switchStatus[cmd.channel].status = cmd.status; 

      // respond to remote
      switch_payload* pReply = &(switchStatus[cmd.channel]);
      iotaClient.publish(IOTA_TOPIC_SWITCH_STATUS, (uint8_t*)pReply, sizeof(switch_payload));
    }
    
    return;
  } //switch control
  
  // ------------------ Temp./Press. sensor ---------------------
  // - read and publish
  
  // ------------------ battery sensor ---------------------
  // - read and publish
  
  // ------------------ PIR sensor ---------------------
  // - send last detected motion time
  
  // ------------------ device time ---------------------
  // - send current RTC time
  
  // ------------------ NTP sync ---------------------
  // - read NTC
  // - sync
  // - send 'before' and 'after' time

  // ------------------ SD card logging ---------------------

}


