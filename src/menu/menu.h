#ifndef _MENU_H
#define _MENU_H

#include "hardware/address_mapped.h"
#include "keyboard.h"
#include "gui.h"

#define MENU_TOTAL_LEFT_BUTTONS 8

typedef enum  {
    menu_event_action = 0,
    menu_event_next,
    menu_event_previous,
    menu_event_max
} menu_event_type;

typedef enum  {
    menu_button_group_home = 0,
    menu_button_group_max
} menu_button_group_type;

typedef struct menu_event {
   menu_event_type type;
   uint8_t key_down:1;
   uint8_t key_up:1;
} menu_event_t;

#define menu_elements_copy(src, dst) menu_elements_copy_(src, dst, sizeof(src) / sizeof(src[0]))
int menu_initialize(uint *pins, menu_event_type *events, uint8_t count);
void menu_elements_copy_(const gui_object_t *src, gui_object_t *dst, uint8_t size);
gui_object_t menu_create_left_button_group(menu_button_group_type type);

#endif