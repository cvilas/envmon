#ifndef IOTA_MESSAGES_H
#define IOTA_MESSAGES_H

// Switch payload

#define IOTA_TOPIC_SWITCH_CNTRL     "iota/switch/cntrl"
#define IOTA_TOPIC_SWITCH_STATUS    "iota/switch/status"

typedef struct _switch_payload
{
    int channel;
    int status;
}switch_payload;

#endif // IOTA_MESSAGES_H
