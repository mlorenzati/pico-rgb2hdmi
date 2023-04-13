#ifndef _COMMANDS_H
#define _COMMANDS_H
#include "pico.h"
#include "graphics.h"
#include "dvi.h"

extern struct dvi_inst dvi0;
extern int command_info_afe_error;
extern int command_info_scanner_error;
int  command_on_receive(int option, const void *data, bool convert);
void command_validate_license(const uint8_t *security_key);

// Command show info is externally served
void command_show_info(bool value);
bool command_is_license_valid();
void command_reboot();
void command_enable_usb(bool status);
#endif