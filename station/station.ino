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
//#include <SD.h>
#include <WiFiUdp.h>
#include "connection_info.h"
#include "iota_messages.h"
#include "helper_defs.h"

#define IDLE_UPDATE_TIME_MS 10000

boolean setupWiFi();
boolean setupIotaBroker();
void initialiseIotaDevices();
void transmitStatus();
void transmitSwitchStatus();
void transmitWeather();
void setLampColor(uint8_t r, uint8_t g, uint8_t b);
void setSwitch(uint8_t ch, uint8_t st);
void setSwitchesFromStatus();
void iotaCallback(char* topic, byte* payload, unsigned int length);
unsigned long requestNtpTime();

// Devices
WiFiClient      wifiClient;
WiFiUDP         timeReader;
RCSwitch        switchController;
Adafruit_BMP085 bmp;
DHT             dht;
PubSubClient    iotaClient(IOTA_BROKER_URL, IOTA_BROKER_PORT, iotaCallback, wifiClient);

unsigned long startTime = 0;
int numActiveClients = 0;

#define NTP_LOCAL_PORT 2390      // local port to listen for UDP packets
#define NTP_PACKET_SIZE 48 // NTP time stamp is in the first 48 bytes of the message
#define NTP_SERVER_PORT 123 // NTP time servers listen on this port 
IPAddress ntpTimeServer(129, 6, 15, 28); // time.nist.gov NTP server
byte ntpPacketBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 


switch_payload switchStatus[IOTA_NUM_SWITCHES];

//=============================================================================
void setup()
//=============================================================================
{
  Serial.begin(9600);
  DEBUGPRINTLN("Starting up");
  if( !setupWiFi() )
  {
    while(true);
  }
  if( !setupIotaBroker() )
  {
    while(true);
  } 
  initialiseIotaDevices();
  transmitStatus();
}

//=============================================================================
void loop()
//=============================================================================
{
  // reconnect if we lost it
  if( WiFi.status() != WL_CONNECTED ) 
  {
    DEBUGPRINT(__FUNCTION__);
    DEBUGPRINTLN(": WiFi connection lost. Trying again."); 
    WiFi.disconnect();
    if( !setupWiFi() )
    {
      delay(30000);
    }
    return;
  }

  if( !iotaClient.connected() )
  {
    DEBUGPRINT(__FUNCTION__);
    DEBUGPRINTLN(": Iota Broker connection lost. Trying again."); 
    iotaClient.disconnect();
    if( !setupIotaBroker() )
    {
      delay(30000);
    }
    return;
  }
  
  // process messages
  iotaClient.loop();

  // update all 
  // note: using unsigned int for subtraction means this works even when
  // millis() rolls over after 50 days.
  // note: IDLE_UPDATE_TIME_MS must be greater than dht.getMinimumSamplingPeriod()
  unsigned long now = millis();
  if( now - startTime > IDLE_UPDATE_TIME_MS )
  {
    startTime = now;
    transmitStatus();
    setSwitchesFromStatus();
  }
}

//=============================================================================
boolean setupWiFi()
//=============================================================================
{ 
  // check for the presence of wifi shield:
  if (WiFi.status() == WL_NO_SHIELD) 
  {
    DEBUGPRINT(__FUNCTION__);
    DEBUGPRINTLN(": No WiFi radio"); 
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
    DEBUGPRINT(__FUNCTION__);
    DEBUGPRINTLN(": WiFi connection failed");
    return false;
  }
  else
  {
    // print the SSID of the network you're attached to:
    DEBUGPRINT(__FUNCTION__);
    DEBUGPRINT(": Connected to ");   
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
  DEBUGPRINT(__FUNCTION__);
  DEBUGPRINT(": Connection to ");
  DEBUGPRINT(IOTA_BROKER_URL);
  if( !isConnected )
  {
    DEBUGPRINTLN(" failed");
    return false;
  }
  else
  {
    DEBUGPRINTLN(" succeeded");   
  }
  
  // setup subscriptions
  DEBUGPRINT(__FUNCTION__);
  DEBUGPRINTLN(": Subscriptions:");
 
  //special message to determine how many clients are connected
  DEBUGPRINT("- "); DEBUGPRINTLN(IOTA_TOPIC_CLIENT_STATE_SUBSCRIPTION);
  iotaClient.subscribe(IOTA_TOPIC_CLIENT_STATE_SUBSCRIPTION);
  
  DEBUGPRINT("- "); DEBUGPRINTLN(IOTA_TOPIC_SWITCH_CNTRL);
  iotaClient.subscribe(IOTA_TOPIC_SWITCH_CNTRL);

  DEBUGPRINT("- "); DEBUGPRINTLN(IOTA_TOPIC_LAMP_CNTRL);
  iotaClient.subscribe(IOTA_TOPIC_LAMP_CNTRL);
  
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
  DEBUGPRINT(__FUNCTION__);
  DEBUGPRINTLN(": Resetting switches.");   
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
  // todo: read sensor

  // ------------------ NTP ---------------------
  timeReader.begin(NTP_LOCAL_PORT);

  // ------------------ SD card logging ---------------------
  // todo: read sensor
  
  // note start time
  startTime = millis();
}

//=============================================================================
void transmitStatus()
//=============================================================================
{
  if( numActiveClients == 0)
  {
    return;
  }
  
  transmitSwitchStatus();
  transmitWeather();
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
  // ------------------- Switches -----------------------------
  for(int i = 0; i < IOTA_NUM_SWITCHES; ++i)
  {
    switch_payload* pReply = &(switchStatus[i]);
    iotaClient.publish(IOTA_TOPIC_SWITCH_STATUS, (uint8_t*)pReply, sizeof(switch_payload));
  }
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
void iotaCallback(char* topic, byte* payload, unsigned int length)
//=============================================================================
{
 
  // ------------------- Client state -----------------------------
  
  if( strstr(topic, IOTA_TOPIC_CLIENT_STATE) )
  {   
    if( length )
    {
      client_state* cmd = (client_state*)payload;
      DEBUGPRINT(__FUNCTION__); DEBUGPRINT(": Client ");
      DEBUGPRINTHEX(cmd->sid); DEBUGPRINT(" ");
      if( cmd->state )
      {
        DEBUGPRINT("alive");
        numActiveClients++;
        transmitStatus();
      }
      else
      {
        DEBUGPRINT("dead");
        numActiveClients--;
        
        // bug in the client code means we can get 2 'dead' messages
        // from a clientexit
        if( numActiveClients < 0) numActiveClients = 0;
      }
      DEBUGPRINT(" [active clients = "); DEBUGPRINT(numActiveClients); DEBUGPRINTLN("]");
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
      DEBUGPRINT(__FUNCTION__); DEBUGPRINTLN(": ERROR: Payload size incorrect. Ignoring.");
      return;
    }
    
    // cast to type
    switch_payload* cmd = (switch_payload*)payload;
    DEBUGPRINT(__FUNCTION__); DEBUGPRINT(": Set Switch ");
    DEBUGPRINT(cmd->channel); DEBUGPRINT(" -> "); DEBUGPRINTLN(cmd->status?("ON"):("OFF"));
      
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
  } //switch control
   
  // ------------------- lamps -----------------------------

  if( !strcmp(topic, IOTA_TOPIC_LAMP_CNTRL) )
  {
    if( length != sizeof(lamp_payload) )
    {
      DEBUGPRINT(__FUNCTION__); DEBUGPRINTLN(": ERROR: Payload size incorrect. Ignoring.");
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
  }
  
  // ------------------ PIR sensor ---------------------
  // - send last detected motion time
   
  // ------------------ NTP sync ---------------------
  // - read NTC
  // - sync
  // - send 'before' and 'after' time

  // ------------------ SD card logging ---------------------

}


//=============================================================================
unsigned long requestNtpTime()
//=============================================================================
{
  // set all bytes in the buffer to 0
  memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
  ntpPacketBuffer[2] = 6;     // Polling Interval
  ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  ntpPacketBuffer[12]  = 49; 
  ntpPacketBuffer[13]  = 0x4E;
  ntpPacketBuffer[14]  = 49;
  ntpPacketBuffer[15]  = 52;
  
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 		   
  timeReader.beginPacket(ntpTimeServer, NTP_SERVER_PORT); //NTP requests are to port 123
  timeReader.write(ntpPacketBuffer,NTP_PACKET_SIZE);
  timeReader.endPacket(); 
  
  // wait for reply
  int replySz = timeReader.parsePacket();
  int nAttempts = 2;
  while( !replySz && nAttempts )
  {
    delay(1000);
    replySz = timeReader.parsePacket();
    --nAttempts;
  }
  
  //todo: check replySz == NTP_PACKET_SIZE
  
  if( replySz )
  {
    timeReader.read(ntpPacketBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer
    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(ntpPacketBuffer[40], ntpPacketBuffer[41]);
    unsigned long lowWord = word(ntpPacketBuffer[42], ntpPacketBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  
    return secsSince1900;
  }
  return 0;
}
