#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "pico.h"
#include "security.h"

#define GUI_MENU_COLORS 6
#define SETTINGS_DISPLAY_MAX 4

typedef struct display {
    struct { 
        uint32_t red:6, green:6, blue:6;
    } gain;

    struct { 
        uint8_t red, green, blue;
    } offset;
    uint16_t v_front_porch, v_back_porch;
    uint16_t h_front_porch, h_back_porch;
    uint8_t refresh_rate;
} display_t;

typedef struct settings {
    uint8_t security_key[SECURITY_SHA_SIZE];
    uint menu_colors[GUI_MENU_COLORS];
    uint menu_reserved[8];
    struct {
        uint16_t auto_shut_down: 1;
        uint16_t default_display:2; // Tied to SETTINGS_DISPLAY_MAX
    } flags;
    display_t displays[SETTINGS_DISPLAY_MAX];
    uint8_t eof_canary;
} settings_t;

extern settings_t ram_settings;

int settings_initialize(const settings_t *p_factory_settings);
int settings_update();
int settings_factory();
inline settings_t *settings_get() {
    return &ram_settings;
}

#endif