/// station.ino
/// Arduino control program for iota-station
/// 

#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <RCSwitch.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include "connection_info.h"
#include "iota_messages.h"
#include "helper_defs.h"

#define IDLE_UPDATE_TIME_MS 10000

boolean setupWiFi();
boolean setupIotaBroker();
void initialiseIotaDevices();
void transmitSwitchStatus();
void transmitWeather();
void transmitMotionSensorStatus();
void transmitAllStatus();
void setLampColor(uint8_t r, uint8_t g, uint8_t b);
void setSwitch(uint8_t ch, uint8_t st);
void setSwitchesFromStatus();
boolean isMotionSensorToggled();
void iotaCallback(char* topic, byte* payload, unsigned int length);
void(* resetFunc) (void) = 0;//declare reset function at address 0

// Devices
WiFiClient      wifiClient;
RCSwitch        switchController;
Adafruit_BMP085 bmp;
DHT             dht;
PubSubClient    iotaClient(IOTA_BROKER_URL, IOTA_BROKER_PORT, iotaCallback, wifiClient);

unsigned long lastTimeMs = 0;
uint8_t numActiveClients = 0;

boolean wasMotionSensorHigh = false;
boolean isMotionSensorHigh = false;

switch_payload       switchStatus[IOTA_NUM_SWITCHES];
motion_sense_payload motionSensorStatus;

//=============================================================================
void setup()
//=============================================================================
{
  Serial.begin(9600);
  DEBUGPRINTLN("Start");
  
  if( !setupWiFi() )
  {
    while(true);
  }
    
  if( !setupIotaBroker() )
  {
    while(true);
  } 
  
  initialiseIotaDevices();
  transmitAllStatus();
}

//=============================================================================
void loop()
//=============================================================================
{
  // reconnect if we lost it
  if( WiFi.status() != WL_CONNECTED ) 
  {
    DEBUGPRINTLN("WiFi lost. Retrying."); 
    WiFi.disconnect();
    if( !setupWiFi() )
    {
      delay(30000);
    }
    return;
  }

  if( !iotaClient.connected() )
  {
    DEBUGPRINTLN("Iota Broker lost. Retrying."); 
    iotaClient.disconnect();
    if( !setupIotaBroker() )
    {
      delay(30000);
    }
    return;
  }
  
  // check motion sensor
  if( isMotionSensorToggled() )
  {
    transmitMotionSensorStatus();
  }
  
  // process remote messages
  iotaClient.loop();

  // note: millis() rolls over after 50 days.
  // note: IDLE_UPDATE_TIME_MS must be greater than dht.getMinimumSamplingPeriod()
  unsigned long now = millis();
  if( now - lastTimeMs > IDLE_UPDATE_TIME_MS )
  {
    lastTimeMs = now;
    setSwitchesFromStatus();
    
    if( numActiveClients != 0)
    {
      transmitSwitchStatus();
      transmitWeather();
    } // remotes alive
    
  } // update time
  
}

//=============================================================================
boolean setupWiFi()
//=============================================================================
{ 
  // check for the presence of wifi shield:
  if (WiFi.status() == WL_NO_SHIELD) 
  {
    DEBUGPRINTLN("No WiFi radio"); 
    return false;
  } 
   
  // connect to wifi network
  int wifiConnAttempts = 1;
  int wifiStatus = WL_IDLE_STATUS;
  while( wifiStatus != WL_CONNECTED && (wifiConnAttempts < 3) )
  {
    wifiStatus = WiFi.begin(HOME_WIFI_SSID, HOME_WIFI_PASS);
    if( wifiStatus != WL_CONNECTED ) 
    {
      delay(10000);
    }
    ++wifiConnAttempts;
  }
  
  // wifi connection status
  if( wifiStatus != WL_CONNECTED )
  {
    DEBUGPRINTLN("WiFi failed");
    return false;
  }
  else
  {
    // print the SSID of the network you're attached to:
    DEBUGPRINT("Connected to ");   
    DEBUGPRINTLN( WiFi.SSID() );
  }
  return true;
}

//=============================================================================
boolean setupIotaBroker()
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
  DEBUGPRINT("Connection to ");
  DEBUGPRINT(IOTA_BROKER_URL);
  if( !isConnected )
  {
    DEBUGPRINTLN(" failed");
    return false;
  }
  else
  {
    DEBUGPRINTLN(" ok");   
  }
  
  // setup subscriptions
  DEBUGPRINTLN("Subscriptions:");
 
  DEBUGPRINT("- "); DEBUGPRINTLN(IOTA_TOPIC_CLIENT_STATE_SUBSCRIPTION);
  iotaClient.subscribe(IOTA_TOPIC_CLIENT_STATE_SUBSCRIPTION);
  
  DEBUGPRINT("- "); DEBUGPRINTLN(IOTA_TOPIC_SWITCH_CNTRL);
  iotaClient.subscribe(IOTA_TOPIC_SWITCH_CNTRL);

  DEBUGPRINT("- "); DEBUGPRINTLN(IOTA_TOPIC_LAMP_CNTRL);
  iotaClient.subscribe(IOTA_TOPIC_LAMP_CNTRL);
  
  DEBUGPRINT("- "); DEBUGPRINTLN(IOTA_TOPIC_MOTION_SENSOR_GET);
  iotaClient.subscribe(IOTA_TOPIC_MOTION_SENSOR_GET);

  return true;
}

//=============================================================================
void initialiseIotaDevices()
//=============================================================================
{
  // ------------------- Switches -----------------------------
  // Switch tx is connected to this pin
  switchController.enableTransmit(IOTA_SWITCH_TX_PIN);
  switchController.setPulseLength(IOTA_SWITCH_PULSE_LEN);
  //switchController.setRepeatTransmit(2);
  
  // reset all switches 
  DEBUGPRINTLN("Reset switches.");   
  for(int i = 0; i < IOTA_NUM_SWITCHES; ++i)
  {
    switchController.switchOff(IOTA_SWITCH_SYSTEM_CODE,IOTA_SWITCH_UNIT_CODE[i]);
    switchStatus[i].channel = i;
    switchStatus[i].status = 0;
  }  
  
  // ------------------- RGB LED -----------------------------
  pinMode(RGB_LED_R_PIN, OUTPUT);
  pinMode(RGB_LED_G_PIN, OUTPUT);
  pinMode(RGB_LED_B_PIN, OUTPUT);
  setLampColor(0,0,0);
  
  // ------------------ pressure sensor ---------------------
  bmp.begin();

  // ------------------ humidity sensor ---------------------
  pinMode(IOTA_HUMIDITY_RX_PIN, INPUT);
  dht.setup(IOTA_HUMIDITY_RX_PIN);

  // ------------------ PIR sensor ---------------------
  pinMode(IOTA_PIR_RX_PIN, INPUT);
  isMotionSensorToggled();
 
  // note start time
  lastTimeMs = millis();
}

//=============================================================================
void transmitWeather()
//=============================================================================
{
  weather_payload w;
  w.temperature10C = (int32_t)(10 * bmp.readTemperature());
  w.pressurePa = bmp.readPressure();
  w.pressureAlt10m = (int32_t)(10 * bmp.readAltitude());
  w.humidityPercent = (int32_t)(dht.getHumidity());
  if( dht.getStatus() != DHT::ERROR_NONE )
  {
    w.humidityPercent = -1;
  }
  
  iotaClient.publish(IOTA_TOPIC_WEATHER, (uint8_t*)&w, sizeof(w));
}

//=============================================================================
void transmitSwitchStatus()
//=============================================================================
{
  for(int i = 0; i < IOTA_NUM_SWITCHES; ++i)
  {
    switch_payload* pReply = &(switchStatus[i]);
    iotaClient.publish(IOTA_TOPIC_SWITCH_STATUS, (uint8_t*)pReply, sizeof(switch_payload));
  }
}

//=============================================================================
void transmitMotionSensorStatus()
//=============================================================================
{  
  iotaClient.publish(IOTA_TOPIC_MOTION_SENSOR_STATUS, 
                      (uint8_t*)&motionSensorStatus, 
                      sizeof(motionSensorStatus));
}

//=============================================================================
void transmitAllStatus()
//=============================================================================
{
  transmitWeather();
  transmitSwitchStatus();
  transmitMotionSensorStatus();
}

//=============================================================================
void setLampColor(uint8_t r, uint8_t g, uint8_t b)
//=============================================================================
{
  analogWrite(RGB_LED_R_PIN, 255 - r);
  analogWrite(RGB_LED_G_PIN, 255 - g);
  analogWrite(RGB_LED_B_PIN, 255 - b);
}

//=============================================================================
void setSwitch(uint8_t ch, uint8_t st)
//=============================================================================
{
  if( ch < IOTA_NUM_SWITCHES )
  {
    if( st )
    {
      switchController.switchOn(IOTA_SWITCH_SYSTEM_CODE,IOTA_SWITCH_UNIT_CODE[ch]);
    }
    else
    {
      switchController.switchOff(IOTA_SWITCH_SYSTEM_CODE,IOTA_SWITCH_UNIT_CODE[ch]);
    } 
  }
}

//=============================================================================
void setSwitchesFromStatus()
//=============================================================================
{
  // we do this every often because the switches are open loop, and we don't really
  // know if they got activated when they were commanded.
  for(int i = 0; i < IOTA_NUM_SWITCHES; ++i)
  {
    setSwitch(switchStatus[i].channel, switchStatus[i].status);
  }

}

//=============================================================================
boolean isMotionSensorToggled()
//=============================================================================
{
  isMotionSensorHigh = digitalRead(IOTA_PIR_RX_PIN);
  boolean tripped = (wasMotionSensorHigh != isMotionSensorHigh);
  wasMotionSensorHigh = isMotionSensorHigh;
  
  if( tripped )
  {
    motionSensorStatus.currentState = isMotionSensorHigh;
    unsigned long t = millis();

    DEBUGPRINT("Motion: ");
    DEBUGPRINT(motionSensorStatus.currentState);
    DEBUGPRINT(" [");
    DEBUGPRINT(t);
    DEBUGPRINTLN("]");

    if( isMotionSensorHigh )
    {
      motionSensorStatus.lastOnTimeMs = t;
    }
    else
    {
      motionSensorStatus.lastOffTimeMs = t;
    }    
  }
  return tripped;
}

//=============================================================================
void iotaCallback(char* topic, byte* payload, unsigned int length)
//=============================================================================
{ 
  // ------------------- Client state -----------------------------
  
  if( strstr(topic, IOTA_TOPIC_CLIENT_STATE) )
  {   
    if( length == sizeof(client_state) )
    {
      client_state* cmd = (client_state*)payload;
      DEBUGPRINT("Client ");
      DEBUGPRINTHEX(cmd->sid); DEBUGPRINT(" ");
      if( cmd->state )
      {
        DEBUGPRINT("alive");
        numActiveClients++;
        transmitAllStatus();
      }
      else
      {
        DEBUGPRINT("dead");
        numActiveClients--;
      
        // bug in the client code means we can get 2 'dead' messages
        // from a client exit
        if( numActiveClients < 0) numActiveClients = 0;
      }
    
      DEBUGPRINT(" [num clients = "); DEBUGPRINT(numActiveClients); DEBUGPRINTLN("]");
      if( cmd->state == 0 ) // remove redundant messages from broker
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
      DEBUGPRINTLN("Wrong payload. Ignoring.");
      return;
    }
    
    // cast to type
    switch_payload* cmd = (switch_payload*)payload;
    DEBUGPRINT("Set Switch ");
    DEBUGPRINT(cmd->channel); DEBUGPRINTLN(cmd->status?(" ON"):(" OFF"));
      
    if( cmd->channel < IOTA_NUM_SWITCHES )
    {
      // set it
      setSwitch(cmd->channel, cmd->status);
           
      // copy status
      switchStatus[cmd->channel].status = cmd->status; 

      // respond to remote
      switch_payload* pReply = &(switchStatus[cmd->channel]);
      iotaClient.publish(IOTA_TOPIC_SWITCH_STATUS, (uint8_t*)pReply, sizeof(switch_payload));
    }
    
    return;
  } // switch control
   
  // ------------------- lamps -----------------------------

  if( !strcmp(topic, IOTA_TOPIC_LAMP_CNTRL) )
  {
    if( length != sizeof(lamp_payload) )
    {
      DEBUGPRINTLN("Wrong payload. Ignoring.");
      return;
    }
    
    lamp_payload* pCmd = (lamp_payload*)payload;
    setLampColor(pCmd->r, pCmd->g, pCmd->b);
    
    // respond
    lamp_payload lamp;
    lamp.r = pCmd->r;
    lamp.g = pCmd->g;
    lamp.b = pCmd->b;
    iotaClient.publish(IOTA_TOPIC_LAMP_STATUS, (uint8_t*)&lamp, sizeof(lamp));
    return;
  } // lamps
  
  // ------------------ PIR sensor ---------------------
  
  if( !strcmp(topic, IOTA_TOPIC_MOTION_SENSOR_GET) )
  {
    if( length )
    {
      DEBUGPRINTLN("Wrong payload. Ignoring.");
      return;
    }
    transmitMotionSensorStatus();
    return;
  } // pir

}

