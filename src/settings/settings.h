#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "pico.h"
#include "security.h"

typedef struct settings {
    uint8_t security_key[SECURITY_SHA_SIZE];
    uint menu_colors[8]; // Only 6 colors are used but left space for upgrades
} settings_t;

extern settings_t ram_settings;

int settings_initialize(const settings_t *p_factory_settings);
int settings_update();
int settings_factory();
inline settings_t *settings_get() {
    return &ram_settings;
}

#endif