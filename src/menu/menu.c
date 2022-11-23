#include "menu.h"
#include "graphics.h"
#include "videoAdjust.h"
#include "overlay.h"
#include "gui.h"

//System configuration includes
#include "common_configs.h"

#if DVI_SYMBOLS_PER_WORD == 2
    const uint color_black      = 0b0000000000000000;
    const uint color_dark_gray  = 0b0001100011100011;
    const uint color_mid_gray   = 0b0000000000011111;
    const uint color_light_gray = 0b1111100000000000;
    const uint color_green      = 0b0000011111100000;
    const uint color_white      = 0b1111111111111111;
    #define MENU_OVERLAY_BBPX    rgb_16
#else
    const uint color_black         = 0b00000000;
    const uint color_dark_gray  = 0b01001001;
    const uint color_mid_gray   = 0b10110110;
    const uint color_light_gray = 0b11011011;
    const uint color_green      = 0b00011100;
    const uint color_white      = 0b11111111;
    #define MENU_OVERLAY_BBPX    rgb_8
#endif

// --------- Global register start ---------
uint8_t menu_tot_events; 
menu_event_t  menu_events[menu_max];

static graphic_ctx_t menu_graphic_ctx = {
    .bppx = MENU_OVERLAY_BBPX,
    .parent = NULL
};

static graphic_ctx_t menu_overlay_ctx;

const uint menu_colors[] = { color_dark_gray, color_light_gray, color_white, color_black, color_mid_gray, color_green };
const gui_list_t menu_colors_list = initalizeGuiList(menu_colors);
const gui_properties_t menu_common_nshared_props = {
    .alignment  = gui_align_center,
    .horiz_vert = gui_orientation_vertical,
    .padding    = 1,
    .shared     = 0,
    .border     = 1
};

gui_object_t *menu_focused_object = NULL;

// --------- Global register end --------- 


// --------- KEYBOARD API CALL START --------- 
void menu_on_keyboard_event(keyboard_status_t keys) {
    for (uint8_t cnt = 0; cnt < menu_tot_events; cnt++) {
        menu_event_t *event = &menu_events[cnt];
        event->key_up   = keyboard_get_key_status(&keys, true, cnt); 
        event->key_down = keyboard_get_key_status(&keys, false, cnt);
        switch (event->type) {
            case menu_action:
                if (event->key_up) {
                    if (!is_video_overlay_enabled()) {
                        // Enable Overlay, Render and Focus first element
                        video_overlay_enable(true);
                        //render all
                    } else if (menu_focused_object != NULL) {
                        menu_focused_object = gui_deactivate(menu_focused_object);
                    }
                } else if (event->key_down && is_video_overlay_enabled() && menu_focused_object != NULL) {
                    menu_focused_object = gui_activate(menu_focused_object);
                }
                break;
            case menu_next:
                if (!is_video_overlay_enabled() || menu_focused_object == NULL) {
                    continue;
                }
                if (event->key_up) {
                    if (!menu_focused_object->base.status.navigable) {
                        gui_clear_add(menu_focused_object);
                    }
                } else if (event->key_down) {
                    if (menu_focused_object->base.status.navigable) {
                        menu_focused_object = gui_next_focus(menu_focused_object);
                    } else {
                        gui_set_add(menu_focused_object);
                    }
                }
                break;
            case menu_previous:
                if (!is_video_overlay_enabled() || menu_focused_object == NULL) {
                    continue;
                }
                if (event->key_up) {
                    if (!menu_focused_object->base.status.navigable) {
                        gui_clear_sub(menu_focused_object);
                    }
                } else if (event->key_down) {
                    if (menu_focused_object->base.status.navigable) {
                        menu_focused_object = gui_previous_focus(menu_focused_object);
                    } else {
                        gui_set_sub(menu_focused_object);
                    }
                }
                break;
            default:
                break;
        }
    }
}
// ---------  KEYBOARD API CALL END  ---------  


int menu_initialize(uint *pins, menu_event_type *events, uint8_t count) {
    if (count > KEYBOARD_MAX_COUNT) { return 1; }
    if (count > menu_max) { return 2; }
    
    // Mapping of keys to known events
    menu_tot_events = count;
    for (uint8_t cnt = 0; cnt < menu_tot_events; cnt++) {
        menu_events[cnt].type = events[cnt];
     }

    // Initialize Keyboard
    keyboard_initialize(pins, count, KEYBOARD_REFRESH_RATE_MS, KEYBOARD_REPEAT_RATE_MS, menu_on_keyboard_event);

    //Initialize Menu Context
    assert(GET_VIDEO_PROPS().width != 0);
    assert(GET_VIDEO_PROPS().height != 0);
    assert(GET_VIDEO_PROPS().video_buffer != 0);
    menu_graphic_ctx.width = GET_VIDEO_PROPS().width;
    menu_graphic_ctx.height = GET_VIDEO_PROPS().height;
    menu_graphic_ctx.video_buffer = GET_VIDEO_PROPS().video_buffer;

    set_video_overlay(MENU_VIDEO_OVERLAY_WIDTH, MENU_VIDEO_OVERLAY_HEIGHT, false);
    menu_overlay_ctx = get_sub_graphic_ctx(&menu_graphic_ctx, video_overlay_get_startx(), video_overlay_get_starty(), video_overlay.width, video_overlay.height);

    /**
     * 
    */
    //Menu System initialize
     // gui_object_t window = gui_create_window(&graphic_ctx, 0, 0, FRAME_WIDTH, FRAME_HEIGHT, &colors_list, common_nshared_props);
    
    // gui_object_t group_elements[] = { 
    //     gui_create_button(&graphic_ctx,  0, 0, 100, 12, &colors_list, common_nshared_props, "Button 1"),
    //     gui_create_button(&graphic_ctx,  0, 0, 100, 12, &colors_list, common_nshared_props, "Button 2"),
    //     gui_create_slider(&graphic_ctx,  0, 0, 100, 16, &colors_list, common_nshared_props, &slider_value),
    //     gui_create_button(&graphic_ctx,  0, 0, 100, 12, &colors_list, common_nshared_props, "Button 3"),
    //     gui_create_button(&graphic_ctx,  0, 0, 100, 12, &colors_list, common_nshared_props, "Button 4"),
    //     gui_create_spinbox(&graphic_ctx, 0, 0, 100, 12, &colors_list, spinbox_props, &spinbox_value)
    // };
    return 0;
}