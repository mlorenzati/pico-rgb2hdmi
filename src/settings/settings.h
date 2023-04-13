#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "pico.h"
#include "security.h"

typedef struct settings {
    const char security_key[SECURITY_SHA_SIZE];

} settings_t;

extern settings_t ram_settings;

int settings_initialize();
int settings_update();
int settings_factory();

#endif