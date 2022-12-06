#include "menu.h"
#include "version.h"
#include "rgbScan.h"
#include "graphics.h"
#include "videoAdjust.h"
#include "security.h"
#include "overlay.h"
#include "commands.h"

//System configuration includes
#include "common_configs.h"

#include "pico/stdlib.h"

#if DVI_SYMBOLS_PER_WORD == 2
    const uint color_black      = 0b0000000000000000;
    const uint color_dark_gray  = 0b0001100011100011;
    const uint color_mid_gray   = 0b0000000000011111;
    const uint color_light_gray = 0b1111100000000000;
    const uint color_green      = 0b0000011111100000;
    const uint color_white      = 0b1111111111111111;
    #define MENU_OVERLAY_BBPX    rgb_16
#else
    const uint color_black      = 0b00000000;
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

gui_object_t *menu_focused_object = NULL;
gui_object_t menu_window;
gui_object_t menu_left_buttons_group_elements[MENU_TOTAL_LEFT_BUTTONS];
gui_list_t   menu_left_buttons_group_list;
gui_object_t menu_left_buttons_group;
gui_object_t menu_main_view_group_elements[MENU_TOTAL_MAIN_VIEW];
gui_list_t   menu_main_view_group_list;
gui_object_t menu_main_view_group;

struct repeating_timer menu_vsync_hsync_timer;
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
                        //Fix when focused object changes outside the event system
                        if (menu_focused_object != old_focus && new_focus == old_focus) {
                            new_focus->base.status.focused = false;
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

// ---------- TIMER  CALLBACK START ----------
bool menu_hvsync_timer_callback(struct repeating_timer *t) {
    gui_object_t *label = (gui_object_t *) t->user_data;
    if (label != NULL) {
        gui_ref_draw(label);
    }
    
    return true;
}

// ----------- TIMER CALLBACK  END -----------

bool menu_change_view(gui_status_t status, gui_base_t *origin, menu_button_group_type type){
    if (!status.activated && origin->status.activated) {
        menu_left_buttons_group = menu_create_left_button_group(menu_nav_stack[menu_button_index - 1], type);
        gui_obj_draw(menu_window);
        gui_obj_draw(menu_left_buttons_group);
        if (menu_main_view_group.base.status.visible) {
           gui_obj_draw(menu_main_view_group);
        }
        menu_focused_object = gui_focused(&menu_left_buttons_group_elements[0]);
        //Once we consume this event, we dispose it due to new changes
        return false;
    }
    return true;
}

// ---------  GUI EVENT SLOTS HANDLERS START  ---------
bool on_exit_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        video_overlay_enable(false);
        gui_disable(&menu_left_buttons_group);
    }
    return true;
}

bool on_palette_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_group_palette);
}

bool on_about_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_group_about);
}

bool on_alignment_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_group_alignment);
}

bool on_diagnostic_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_group_diagnostic);
}

bool on_save_reboot_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_group_save_reboot);
}

bool on_back_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        if (menu_button_index >= 2) {
            cancel_repeating_timer(&menu_vsync_hsync_timer); //blindly stopping the timer for all back events for now
            menu_button_group_type previous = menu_nav_stack[menu_button_index - 1];
            menu_button_group_type new = menu_nav_stack[menu_button_index - 2];
            menu_button_index -= 2;
            menu_left_buttons_group = menu_create_left_button_group(previous, new);
            gui_obj_draw(menu_window);
            gui_obj_draw(menu_left_buttons_group);
            menu_focused_object = gui_focused(&menu_left_buttons_group_elements[0]);
        }
    }
    //Once we consume this event, we dispose it due to new changes
    return false;
}

uint test1, test2, test3, slider_opt;
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

bool on_palette_option_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    uint16_t *data = (uint16_t *) origin->data;
    if (!status.activated && origin->status.activated) {
        destination->base.status.navigable = !destination->base.status.navigable;
    } else if (status.add && *data < GUI_BAR_100PERCENT) {
        *data += (GUI_BAR_100PERCENT/(menu_colors_list.size-1));
    } else if (status.substract && *data != 0) {
        *data -= (GUI_BAR_100PERCENT/(menu_colors_list.size-1));
    }
    destination->base.status.data_changed = 1;
    gui_obj_draw(menu_main_view_group);
    
    return true;
}

bool on_palette_color_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
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

bool on_save_reboot_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        command_reboot();
    }
    return true;
}

// ----------  GUI EVENT SLOTS HANDLERS END  ---------- 

void menu_elements_copy_(const gui_object_t *src, gui_object_t *dst, uint8_t size) {
    for (uint8_t cnt = 0; cnt < size; cnt++) {
        dst[cnt] = src[cnt];
    }
}

void menu_diagnostic_print(print_delegate_t printer) {
	printer("%s v%s\n\nRes: %dx%d %dbits\nAFE code: %d\nScan code: %d\nId: %s\nLicense: %s",
        PROJECT_NAME, PROJECT_VER, menu_graphic_ctx.width, menu_graphic_ctx.height, bppx_to_int(menu_graphic_ctx.bppx), 
        command_info_afe_error, command_info_scanner_error, security_get_uid(),
        command_is_license_valid() ? "valid" : "invalid, please register");
};

void menu_scan_print(print_delegate_t printer) {
	printer("HSYNC %d, VSYNC %d", 1000000000 / rgbScannerGetVsyncNanoSec(), 1000000000 / rgbScannerGetHsyncNanoSec());
};

void menu_palette_opt_print(print_delegate_t printer) {
    uint index = slider_opt * (menu_colors_list.size-1) / GUI_BAR_100PERCENT;
    printer("%s", gui_colors_str[index]);
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
    gui_status_t button_status = { .activated = 1 };
    gui_status_t spinbox_status = { .activated = 1, .add = 1, .substract = 1};

    switch(new) {
        case menu_button_group_alignment: {
            gui_object_t elements[] = {
                gui_create_text(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_common_label_props, "Horizontal"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_spinbox_props, &test1),
                gui_create_text(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_common_label_props, "Vertical"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_spinbox_props, &test2),
                gui_create_button(&menu_overlay_ctx,  0, 0, 100, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[4].base, &menu_left_buttons_group_elements[4], on_back_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_spinbox_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[3].base, &menu_left_buttons_group_elements[3], on_spinbox_event);
            }
            break;
        case menu_button_group_diagnostic: {
            gui_object_t elements[] = {
                gui_create_label(&menu_overlay_ctx, 0, 0, 200, 78, &menu_colors_list, menu_common_text_props, menu_diagnostic_print),
                gui_create_label(&menu_overlay_ctx, 0, 0, 200, 12, &menu_colors_list, menu_common_text_props, menu_scan_print),
                gui_create_button(&menu_overlay_ctx,  0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_back_event);
            add_repeating_timer_ms(MENU_HV_SYNC_REFRESH, menu_hvsync_timer_callback, &menu_left_buttons_group_elements[2], &menu_vsync_hsync_timer);
            }
            break;
        case menu_button_group_palette: {
            gui_object_t elements[] = {
                gui_create_slider(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_spinbox_props, &slider_opt),
                gui_create_text(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_common_label_props, "Red"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_spinbox_props, &test1),
                gui_create_text(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_common_label_props, "Green"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_spinbox_props, &test2),
                gui_create_text(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_common_label_props, "Blue"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_spinbox_props, &test3),
                gui_create_button(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[0].base, &menu_left_buttons_group_elements[0], on_palette_option_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_palette_color_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[4].base, &menu_left_buttons_group_elements[4], on_palette_color_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[6].base, &menu_left_buttons_group_elements[6], on_palette_color_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[7].base, &menu_left_buttons_group_elements[7], on_back_event);
            }
            break;
        case menu_button_group_about: {
            gui_object_t elements[] = {
                gui_create_window(&menu_overlay_ctx, 0, 0, 200, 90, &menu_colors_list, menu_common_label_props),
                gui_create_button(&menu_overlay_ctx, 0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_back_event);
            }
            break;
        case menu_button_group_save_reboot: {
            gui_object_t elements[] = {
                gui_create_text(&menu_overlay_ctx, 0, 0, 200, 78, &menu_colors_list, menu_common_label_props, "All changes done up to\nthe moment will be saved\nin flash and used upon\nreboot as default\nAre you sure?"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Save and Reboot"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Cancel")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_save_reboot_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_back_event);
            }
            break;
            case menu_button_group_home: default: {
            gui_object_t elements[] = { 
                gui_create_button(&menu_overlay_ctx,  0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Alignment"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Diagnostics"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Palette"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Save & reboot"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "About"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Exit")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[0].base, &menu_left_buttons_group_elements[0], on_alignment_button_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_diagnostic_button_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_palette_button_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[3].base, &menu_left_buttons_group_elements[3], on_save_reboot_button_event);            
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[4].base, &menu_left_buttons_group_elements[4], on_about_button_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[5].base, &menu_left_buttons_group_elements[5], on_exit_button_event);
            }
            break;
    };
   
	gui_object_t left_buttons_group = gui_create_group(&menu_overlay_ctx, 0, 0,
		gui_sum(&menu_left_buttons_group_list, menu_common_nshared_props, gui_coord_width), 
		gui_sum(&menu_left_buttons_group_list, menu_common_nshared_props, gui_coord_height), 
		&menu_colors_list, menu_common_nshared_props , &menu_left_buttons_group_list);
    
    menu_main_view_group = menu_create_main_view_group(&(left_buttons_group.base), new);
    
    return left_buttons_group;
}

gui_object_t menu_create_main_view_group(gui_base_t *left_group, menu_button_group_type type) {
     //First unsubscribe last objects
    gui_event_unsubscribe(&(menu_main_view_group.base), NULL);
    menu_main_view_group.base.status.visible = false;

    switch(type) {
        case menu_button_group_palette: {
            gui_object_t elements[] = { gui_create_label(&menu_overlay_ctx, 0, 0, 115, 11, &menu_colors_list, menu_common_label_props, menu_palette_opt_print) };
            menu_elements_copy(elements, menu_main_view_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_main_view_group_elements, arraySize(elements));
            menu_main_view_group_list = group_list;
            }
            break;
        default:
            return menu_main_view_group;
    }
    gui_object_t main_view_group = gui_create_group(&menu_overlay_ctx, left_group->width, 0,
		gui_sum(&menu_main_view_group_list, menu_common_nshared_props, gui_coord_width), 
		gui_sum(&menu_main_view_group_list, menu_common_nshared_props, gui_coord_height), 
		&menu_colors_list, menu_common_nshared_props , &menu_main_view_group_list);

    return main_view_group;
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