#include "menu.h"
#include "graphics.h"
#include "videoAdjust.h"
#include "overlay.h"

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
menu_event_t  menu_events[menu_event_max];
menu_button_group_type menu_nav_stack[MENU_TOTAL_NAV_STACK];
uint8_t menu_button_index = 0;

static graphic_ctx_t menu_graphic_ctx = {
    .bppx = MENU_OVERLAY_BBPX,
    .parent = NULL
};

static graphic_ctx_t menu_overlay_ctx;
uint menu_colors[] = { color_dark_gray, color_light_gray, color_white, color_black, color_mid_gray, color_green };
gui_list_t menu_colors_list = initalizeGuiList(menu_colors);
const gui_properties_t menu_common_nshared_props = {
    .alignment  = gui_align_center,
    .horiz_vert = gui_orientation_vertical,
    .padding    = 1,
    .shared     = 0,
    .border     = 1
};
const gui_properties_t menu_spinbox_props = {
		.alignment = gui_align_center,
		.horiz_vert = gui_orientation_horizontal,
		.padding = 1,
		.shared = 0,
		.border = 1
	};

gui_object_t *menu_focused_object = NULL;
gui_object_t menu_window;
gui_object_t menu_left_buttons_group_elements[MENU_TOTAL_LEFT_BUTTONS];
gui_list_t   menu_left_buttons_group_list;
gui_object_t menu_left_buttons_group;

// --------- Global register end --------- 

// --------- KEYBOARD API CALL START --------- 
void menu_on_keyboard_event(keyboard_status_t keys) {
    for (uint8_t cnt = 0; cnt < menu_tot_events; cnt++) {
        menu_event_t *event = &menu_events[cnt];
        event->key_up   = keyboard_get_key_status(&keys, true, cnt); 
        event->key_down = keyboard_get_key_status(&keys, false, cnt);
        switch (event->type) {
            case menu_event_action:
                if (event->key_up) {
                    if (!is_video_overlay_enabled()) {
                        // Enable Overlay, Render and Focus first element
                        video_overlay_enable(true);
                        gui_obj_draw(menu_window);
                        gui_enable(&menu_left_buttons_group);
                        gui_unfocused(menu_focused_object);
                        menu_focused_object = gui_focused(&menu_left_buttons_group_elements[0]);
                    } else if (menu_focused_object != NULL) {
                        gui_object_t *old_focus = menu_focused_object;
                        gui_object_t *new_focus = gui_deactivate(menu_focused_object);
                        if (menu_focused_object != old_focus && new_focus == old_focus) {
                            new_focus ->base.status.focused = false;
                        }
                    }
                } else if (event->key_down && is_video_overlay_enabled() && menu_focused_object != NULL) {
                    menu_focused_object = gui_activate(menu_focused_object);
                }
                break;
            case menu_event_next:
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
            case menu_event_previous:
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

// ---------  GUI EVENT SLOTS HANDLERS START  ---------
bool on_exit_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        video_overlay_enable(false);
        gui_disable(&menu_left_buttons_group);
    }
    return true;
}

bool on_alignment_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        menu_button_group_type previous = menu_nav_stack[menu_button_index - 1];
        menu_button_group_type new = menu_button_group_alignment;
        menu_left_buttons_group = menu_create_left_button_group(previous, new);
        gui_obj_draw(menu_window);
        gui_obj_draw(menu_left_buttons_group);
        menu_focused_object = gui_focused(&menu_left_buttons_group_elements[0]);
    }
    return true;
}

bool on_back_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        if (menu_button_index >= 2) {
            menu_button_group_type previous = menu_nav_stack[menu_button_index - 1];
            menu_button_group_type new = menu_nav_stack[menu_button_index - 2];
            menu_button_index -= 2;
            menu_left_buttons_group = menu_create_left_button_group(previous, new);
            gui_obj_draw(menu_window);
            gui_obj_draw(menu_left_buttons_group);
            menu_focused_object = gui_focused(&menu_left_buttons_group_elements[0]);
        }
    }
    return false;
}

uint test1, test2;
bool on_spinbox_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
		uint8_t *data = (uint8_t *) origin->data;
        if (!status.activated && origin->status.activated) {
			destination->base.status.navigable = !destination->base.status.navigable;
		} else if (status.add && *data < 100) {
			*data += 1;
		} else if (status.substract && *data != 0) {
			*data -= 1;
		}
		destination->base.status.data_changed = 1;
		
		return true;
	}

// ----------  GUI EVENT SLOTS HANDLERS END  ---------- 

void menu_elements_copy_(const gui_object_t *src, gui_object_t *dst, uint8_t size) {
    for (uint8_t cnt = 0; cnt < size; cnt++) {
        dst[cnt] = src[cnt];
    }
}

gui_object_t menu_create_left_button_group(menu_button_group_type previous, menu_button_group_type new) {
    //First unsubscribe last objects
    gui_event_unsubscribe(&(menu_left_buttons_group.base), NULL);  

    // Avoid new button groups if stack is full
    if (menu_button_index > MENU_TOTAL_LEFT_BUTTONS) { 
        return menu_left_buttons_group; 
    }
    menu_button_index++;
    
    //Generate new objects and subscriptions
    menu_nav_stack[menu_button_index - 1] = new;
    switch(new) {
        case menu_button_group_home: {
            gui_object_t elements[] = { 
                gui_create_button(&menu_overlay_ctx,  0, 0, 120, 12, &menu_colors_list, menu_common_nshared_props, "Alignment"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 120, 12, &menu_colors_list, menu_common_nshared_props, "Diagnostics"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 120, 12, &menu_colors_list, menu_common_nshared_props, "Save & reboot"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 120, 12, &menu_colors_list, menu_common_nshared_props, "Exit")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_status_t button_status = { .activated = 1 };
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[3].base, &menu_left_buttons_group_elements[3], on_exit_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[0].base, &menu_left_buttons_group_elements[0], on_alignment_event);
            }
            break;

        case menu_button_group_alignment: {
            gui_object_t elements[] = {
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 120, 12, &menu_colors_list, menu_spinbox_props, &test1),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 120, 12, &menu_colors_list, menu_spinbox_props, &test2),
                gui_create_button(&menu_overlay_ctx,  0, 0, 120, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_status_t button_status = { .activated = 1, .add = 1, .substract = 1};
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_back_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[0].base, &menu_left_buttons_group_elements[0], on_spinbox_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_spinbox_event);
            }
            break;
            
        default: {
            gui_object_t elements[] = { gui_create_button(&menu_overlay_ctx,  0, 0, 100, 12, &menu_colors_list, menu_common_nshared_props, "Exit") };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            }
            break;
    };
   
	gui_object_t left_buttons_group = gui_create_group(&menu_overlay_ctx, 0, 0,
		gui_sum(&menu_left_buttons_group_list, menu_common_nshared_props, gui_coord_width), 
		gui_sum(&menu_left_buttons_group_list, menu_common_nshared_props, gui_coord_height), 
		&menu_colors_list, menu_common_nshared_props , &menu_left_buttons_group_list);
    return left_buttons_group;
}

// --------- MENU INIT API CALL START --------- 
int menu_initialize(uint *pins, menu_event_type *events, uint8_t count) {
    if (count > KEYBOARD_MAX_COUNT) { return 1; }
    if (count > menu_event_max) { return 2; }
    
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

    //Menu System initialize
    menu_window = gui_create_window(&menu_graphic_ctx, 0, 0, menu_overlay_ctx.width, menu_overlay_ctx.height, &menu_colors_list, menu_common_nshared_props);
    menu_left_buttons_group = menu_create_left_button_group(menu_button_group_none, menu_button_group_home);
    menu_left_buttons_group.base.status.enabled = false;

    return 0;
}
// --------- MENU INIT API CALL END  ---------  