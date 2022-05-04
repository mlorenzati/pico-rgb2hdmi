#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "hardware/address_mapped.h"
#define KEYBOARD_MAX_COUNT 4
#define KEYBOARD_THRESHOLD 4

typedef struct keyboard_status {
    uint8_t key1_down:1;
    uint8_t key2_down:1;
    uint8_t key3_down:1;
    uint8_t key4_down:1;
    uint8_t key1_up:1;
    uint8_t key2_up:1;
    uint8_t key3_up:1;
    uint8_t key4_up:1;
    
} keyboard_status_t;

typedef void (*keyboard_callback)(keyboard_status_t keys);

int keyboard_initialize(uint *pins, uint8_t count, uint refresh_rate_ms, uint repeat_rate_ms, keyboard_callback callback);
void keyboard_stop_service();
bool keyboard_get_current_value(uint8_t index);
#endif