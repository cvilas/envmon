#ifndef IOTA_MESSAGES_H
#define IOTA_MESSAGES_H

#include <inttypes.h>

// Switch payload

#define IOTA_TOPIC_SWITCH_CNTRL     "iota/switch/cntrl"
#define IOTA_TOPIC_SWITCH_STATUS    "iota/switch/status"
#define IOTA_NUM_SWITCHES           4

typedef struct _switch_payload
{
    uint8_t channel;
    uint8_t status;
}switch_payload;

#endif // IOTA_MESSAGES_H
