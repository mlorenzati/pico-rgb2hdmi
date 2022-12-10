#ifndef MENU_GLOBALS_H
#define MENU_GLOBALS_H
#include "pico.h"
#include "graphics.h"
#include "gui.h"

// ------------ Defines Start ------------
#if DVI_SYMBOLS_PER_WORD == 2
    #define MENU_OVERLAY_BBPX    rgb_16_565
#else
    #define MENU_OVERLAY_BBPX    rgb_8_332
#endif
#define MENU_TOTAL_NAV_STACK    4
#define MENU_TOTAL_LEFT_BUTTONS 8
#define MENU_TOTAL_MAIN_VIEW    4
// ------------ Defines End ------------

// ------------ Enum / Struct Start ------------
typedef enum  {
    menu_event_action = 0,
    menu_event_next,
    menu_event_previous,
    menu_event_max
} menu_event_type;

typedef enum  {
    menu_button_group_none = 0,
    menu_button_group_home,
    menu_button_group_alignment,
    menu_button_group_diagnostic,
    menu_button_group_gain_offset,
    menu_button_group_save_reboot,
    menu_button_group_factory_opts,
    menu_button_group_palette,
    menu_button_group_about
} menu_button_group_type;

typedef struct menu_event {
   menu_event_type type;
   uint8_t key_down:1;
   uint8_t key_up:1;
} menu_event_t;
// ------------ Enum / Struct End ------------



// --------- Global register start ---------
extern const uint color_black;
extern const uint color_dark_gray;
extern const uint color_mid_gray;
extern const uint color_light_gray;
extern const uint color_green;
extern const uint color_white;

extern uint8_t menu_tot_events; 
extern menu_event_t  menu_events[menu_event_max];
extern menu_button_group_type menu_nav_stack[MENU_TOTAL_NAV_STACK];
extern graphic_ctx_t menu_graphic_ctx, menu_overlay_ctx;
extern uint8_t menu_button_index;
extern uint menu_colors[];
extern gui_list_t menu_colors_list;
extern const gui_properties_t menu_common_nshared_props;
extern const gui_properties_t menu_common_label_props;
extern const gui_properties_t menu_common_text_props;
extern const gui_properties_t menu_spinbox_props;
extern gui_object_t *menu_focused_object;
extern gui_object_t menu_window;
extern gui_object_t menu_left_buttons_group_elements[MENU_TOTAL_LEFT_BUTTONS];
extern gui_list_t   menu_left_buttons_group_list;
extern gui_object_t menu_left_buttons_group;
extern gui_object_t menu_main_view_group_elements[MENU_TOTAL_MAIN_VIEW];
extern gui_list_t   menu_main_view_group_list;
extern gui_object_t menu_main_view_group;
extern struct repeating_timer menu_vsync_hsync_timer;
extern const gui_status_t button_status;
extern const gui_status_t spinbox_status;

extern uint spinbox_vertical, spinbox_horizontal, spinbox_pix_width;
extern uint spinbox_gain, spinbox_offset;
extern uint color_slider_option, color_spinbox_red, color_spinbox_green, color_spinbox_blue, *color_slider_selected;
#endif