#ifndef _MENU_H
#define _MENU_H

#include "hardware/address_mapped.h"
#include "keyboard.h"
#include "menuGlobals.h"

#define MENU_HV_SYNC_REFRESH    500
#define MENU_ABOUT_REFRESH     1500

#define menu_elements_copy(src, dst) menu_elements_copy_(src, dst, sizeof(src) / sizeof(src[0]))
int menu_initialize(uint *pins, menu_event_type *events, uint8_t count);
void menu_elements_copy_(const gui_object_t *src, gui_object_t *dst, uint8_t size);
gui_object_t menu_create_left_button_group(menu_button_group_type previous, menu_button_group_type new);
gui_object_t menu_create_main_view_group(gui_base_t *left_group, menu_button_group_type type);
void menu_no_signal_callback(void *data);
#endif