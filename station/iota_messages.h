#ifndef IOTA_MESSAGES_H
#define IOTA_MESSAGES_H

#include <inttypes.h>

//-----------------------------------------------------------------------------
// client state
// This is a special topic. 
// - Client publish retained message on IOTA_TOPIC_CLIENT_STATE/<client_unique_id>
//   with state set to 1 on connection and 0 on disconnect
// - Station subscribes to IOTA_TOPIC_CLIENT_STATE/+ to determine how many clients are alive
//-----------------------------------------------------------------------------

#define IOTA_TOPIC_CLIENT_STATE    "iota/client_state"
#define IOTA_TOPIC_CLIENT_STATE_SUBSCRIPTION    "iota/client_state/+"

typedef struct _client_state
{
  int32_t sid;   // client session identifier code (autogenerated and unique)
  uint8_t state; // 0: disconnected, nonzero: alive
}client_state;

//-----------------------------------------------------------------------------
// Switch payload
//-----------------------------------------------------------------------------

#define IOTA_TOPIC_SWITCH_CNTRL     "iota/switch/cntrl"
#define IOTA_TOPIC_SWITCH_STATUS    "iota/switch/status"

#define IOTA_NUM_SWITCHES           3
#define IOTA_SWITCH_TX_PIN          2
#define IOTA_SWITCH_PULSE_LEN        320
#define IOTA_SWITCH_SYSTEM_CODE     "11111"
char IOTA_SWITCH_UNIT_CODE[IOTA_NUM_SWITCHES][5] = 
{    // n units, each with 5 character code
  {'1','0','0','0','0'},
  {'0','1','0','0','0'},
  {'0','0','1','0','0'}
};

typedef struct _switch_payload
{
    uint8_t channel; // 0 to 4
    uint8_t status;  // 0 or non-zero
}switch_payload;

//-----------------------------------------------------------------------------
// LED Payload
//-----------------------------------------------------------------------------
#define IOTA_TOPIC_LAMP_CNTRL     "iota/lamp/cntrl"
#define IOTA_TOPIC_LAMP_STATUS    "iota/lamp/status"

#define RGB_LED_R_PIN 6
#define RGB_LED_G_PIN 5
#define RGB_LED_B_PIN 3

typedef struct _lamp_payload
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
}lamp_payload;

//-----------------------------------------------------------------------------
// Weather Payload
//-----------------------------------------------------------------------------
#define IOTA_TOPIC_WEATHER     "iota/weather"

typedef struct weather_payload
{
  int32_t temperature10C;  // temp. in celsiusx10
  int32_t pressurePa;      // pressure in pascals
  int32_t pressureAlt10m;    // pressure altitude in mx10 based on standard atmosphere
  int32_t humidityPercent; 
}weather_payload;

#endif // IOTA_MESSAGES_H
