#include "menuGlobals.h"
#include "pico/stdlib.h"

// --------- Global register start ---------
uint8_t menu_tot_events; 
menu_event_t  menu_events[menu_event_max];
menu_button_group_type menu_nav_stack[MENU_TOTAL_NAV_STACK];
uint8_t menu_button_index = 0;

graphic_ctx_t menu_graphic_ctx = {
    .parent = NULL
};

graphic_ctx_t menu_overlay_ctx;
gui_list_t menu_colors_list;

const gui_properties_t menu_common_nshared_props = {
    .focusable  = 1,
    .alignment  = gui_align_center,
    .horiz_vert = gui_orientation_vertical,
    .padding    = 1,
    .shared     = 0,
    .border     = 1
};

const gui_properties_t menu_common_label_props = {
    .alignment  = gui_align_center,
    .focusable  = 0
};

const gui_properties_t menu_common_text_props = {
    .alignment  = gui_align_left_up,
    .focusable  = 0
};

const gui_properties_t menu_spinbox_props = {
    .focusable  = 1,
    .alignment  = gui_align_center,
    .horiz_vert = gui_orientation_horizontal,
    .padding    = 1,
    .shared     = 0,
    .border     = 1
};
const gui_status_t button_status = { .activated = 1 };
const gui_status_t spinbox_status = { .activated = 1, .add = 1, .substract = 1};

gui_object_t *menu_focused_object = NULL;
gui_object_t menu_window;
gui_object_t menu_left_buttons_group_elements[MENU_TOTAL_LEFT_BUTTONS];
gui_list_t   menu_left_buttons_group_list;
gui_object_t menu_left_buttons_group;
gui_object_t menu_main_view_group_elements[MENU_TOTAL_MAIN_VIEW];
gui_list_t   menu_main_view_group_list;
gui_object_t menu_main_view_group;

struct repeating_timer menu_vsync_hsync_timer;

uint spinbox_vertical, spinbox_horizontal, spinbox_pix_width;
uint gain_offset_slider_option;
uint spinbox_gain_offset_unified, spinbox_gain_offset_red, spinbox_gain_offset_green, spinbox_gain_offset_blue, spinbox_display_no;
uint spinbox_fine_tune;
uint color_slider_option, color_spinbox_red, color_spinbox_green, color_spinbox_blue, *color_slider_selected;
const char* menu_about_scroll_str;
const char* menu_about_str = "pico-rgb2hdmi is a micro-\ncontrolled RGB to HDMI\nand RGB to USB converter\nbased on a raspberry pi\npico and a cheap Analog\nFront End (AFE) aimed to\nsupport 80s and 90s comp-\nuters with the precursor\nof the VGA.\n\nSupporter list:\nFernando Bugallo\nEspacio Tec:\n Jose Francisco Manera\n Sebastian Rho\n Juan Pablo Chucair\nGabriel Garcia\nAlejandro Perez\nGaston Martin Ferreiros\nOtto\nAldo Ibanez\nEmiliano Escobar\nFrancisco Maqueira\nJose Gumersindo\nRetro computacion:\n Jorge Castillo\nLeandro Galvano\nErnesto Uriburu\nCarlos Masciarelli\nMarcelo Barbalace\nCarlos Aon\nPCB Design:\n Camilo Gomez\nFirmware and HW Design:\n Marcelo Lorenzati\nArgentina-Cordoba-2022";
bool menu_usb_enabled = false;
bool menu_display_confirmation=false;
display_t *menu_current_display;
uint color_black, color_white;
const char *menu_gain_offset_str[5] = { "unified gain", "unified offset", "negative offset", "split gain", "split offset"};
// --------- Global register end --------- 