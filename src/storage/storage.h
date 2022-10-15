#ifndef _STORAGE_H
#define _STORAGE_H
#include "pico.h"

#define STORAGE_CORE_LOCKOUT_TIMEOUT 500000

uint storage_get_flash_capacity();
void storage_report_core1_use();
int storage_initialize(const void *initial_settings, const void **updated_settings, size_t size, bool force);
int storage_update(const void *settings);

#endif