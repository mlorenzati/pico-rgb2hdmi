#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "pico.h"
#include "security.h"

#define GUI_MENU_COLORS 6
typedef struct settings {
    uint8_t security_key[SECURITY_SHA_SIZE];
    uint menu_colors[GUI_MENU_COLORS];
    uint menu_reserved[2];
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