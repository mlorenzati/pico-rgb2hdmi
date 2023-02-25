#ifndef _COMMANDS_H
#define _COMMANDS_H
#include "pico.h"
#include "graphics.h"

extern int command_info_afe_error;
extern int command_info_scanner_error;
int  command_on_receive(int option, const void *data, bool convert);
void command_storage_initialize();
void command_validate_license();
// Command show info is externally served
void command_show_info(bool value);
bool command_is_license_valid();
void command_reboot();
void command_enable_usb(bool status);
#endif