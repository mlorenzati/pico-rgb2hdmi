#include "menuGlobals.h"
#include "pico/stdlib.h"

#if DVI_SYMBOLS_PER_WORD == 2
    // Colors                     0brrrrrggggggbbbbb;
    const uint color_black      = 0b0000000000000000;
    const uint color_dark_gray  = 0b0001100011100011;
    const uint color_mid_gray   = 0b0000000000011111;
    const uint color_light_gray = 0b1111100000000000;
    const uint color_green      = 0b0000011111100000;
    const uint color_white      = 0b1111111111111111;
#else
    // Colors                     0brrrgggbb;
    const uint color_black      = 0b00000000;
    const uint color_dark_gray  = 0b01001001;
    const uint color_mid_gray   = 0b10110110;
    const uint color_light_gray = 0b11011011;
    const uint color_green      = 0b00011100;
    const uint color_white      = 0b11111111;
#endif

// --------- Global register start ---------
uint8_t menu_tot_events; 
menu_event_t  menu_events[menu_event_max];
menu_button_group_type menu_nav_stack[MENU_TOTAL_NAV_STACK];
uint8_t menu_button_index = 0;

graphic_ctx_t menu_graphic_ctx = {
    .bppx = MENU_OVERLAY_BBPX,
    .parent = NULL
};

graphic_ctx_t menu_overlay_ctx;
uint menu_colors[] = { color_dark_gray, color_light_gray, color_white, color_black, color_mid_gray, color_green };
gui_list_t menu_colors_list = initalizeGuiList(menu_colors);

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
uint spinbox_gain, spinbox_offset;
uint color_slider_option, color_spinbox_red, color_spinbox_green, color_spinbox_blue, *color_slider_selected;
const char* menu_about_scroll_str;
const char* menu_about_str = "pico-rgb2hdmi is a micro-\ncontrolled RGB to HDMI\nand RGB to USB converter\nbased on a raspberry pi\npico and a cheap Analog\nFront End (AFE) aimed to\nsupport 80s and 90s comp-\nuters with the precursor\nof the VGA.\n\nSupporter list:\nFernando Bugallo\nEspacio Tec:\n Jose Francisco Manera\n Sebastian Rho\n Juan Pablo Chucair\nGabriel Garcia\nAlejandro Perez\nGaston Martin Ferreiros\nOtto\nAldo Ibanez\nEmiliano Escobar\nFrancisco Maqueira\nJose Gumersindo\nRetro computacion:\n Jorge Castillo\nLeandro Galvano\nErnesto Uriburu\nCarlos Masciarelli\nMarcelo Barbalace\nCarlos Aon\nPCB Design:\n Camilo Gomez\nFirmware and HW Design:\n Marcelo Lorenzati\nArgentina-Cordoba-2022";

// --------- Global register end --------- 