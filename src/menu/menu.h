#ifndef _MENU_H
#define _MENU_H

#include "hardware/address_mapped.h"
#include "keyboard.h"

typedef enum  {
    menu_action = 0,
    menu_next,
    menu_previous,
    menu_max
} menu_event_type;

typedef struct menu_event {
   menu_event_type type;
   uint8_t key_down:1;
   uint8_t key_up:1;
} menu_event_t;

int menu_initialize(uint *pins, menu_event_type *events, uint8_t count);

#endif