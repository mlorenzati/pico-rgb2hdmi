#ifndef _COMMANDS_H
#define _COMMANDS_H
#include "pico.h"
#include "graphics.h"

extern int command_info_afe_error;
extern int command_info_scanner_error;
// extern graphic_ctx_t cmd_graphic_ctx;
// extern uint color_white;
// extern uint color_gray;
// extern uint color_dark;
// extern uint color_light_blue;
// extern uint color_yellow;

int  command_on_receive(int option, const void *data, bool convert);
void command_storage_initialize();
void command_validate_license();
void command_show_info(bool value);
void command_prepare_graphics();
bool command_is_license_valid();
void command_fill_blank();
void command_reboot();
#endif