#include "menu.h"
#include "graphics.h"
#include "videoAdjust.h"
#include "overlay.h"
#include "rgbScan.h"
#include "menuGlobals.h"
#include "menuCallback.h"
#include "commands.h"
#include "wm8213Afe.h"
#include "dvi.h"

//System configuration includes
#include "common_configs.h"
#include "pico/stdlib.h"

// --------- KEYBOARD API CALL START --------- 
void menu_on_keyboard_event(keyboard_status_t keys) {
    for (uint8_t cnt = 0; cnt < menu_tot_events; cnt++) {
        menu_event_t *event = &menu_events[cnt];
        event->key_up   = keyboard_get_key_status(&keys, true, cnt); 
        event->key_down = keyboard_get_key_status(&keys, false, cnt);
        rgbScannerWake();
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
                            //Fix corner cases when focused object changes outside the event system
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
bool menu_update_timer_callback(struct repeating_timer *t) {
    gui_object_t *label = (gui_object_t *) t->user_data;
    if (label != NULL) {
        gui_ref_draw(label);
    }
    
    return true;
}

void menu_video_signal_callback(rgbscan_signal_event_type type) {
    bool auto_shut_down = settings_get()->flags.auto_shut_down;
    if (type == rgbscan_signal_stopped) {
        fill_rect(&menu_graphic_ctx, 0, 0, menu_graphic_ctx.width, menu_graphic_ctx.height, color_black);
        draw_textf(&menu_graphic_ctx, menu_graphic_ctx.width / 2 - 36, menu_graphic_ctx.height / 2 - 4, color_white, color_white, false, "NO SIGNAL");
        if (is_video_overlay_enabled()) {
            gui_obj_draw(menu_window);
            gui_obj_draw(menu_left_buttons_group);
            if (menu_main_view_group.base.status.visible) {
                gui_obj_draw(menu_main_view_group);
            }
        }
    } else if (type == rgbscan_signal_shutdown && auto_shut_down) {
        dvi_stop(&dvi0);
    } else if (type == rgbscan_signal_started) {
        dvi_start(&dvi0);
    }
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

// ---------  MENU BASED GUI EVENT SLOTS HANDLERS START  ---------
bool on_display_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_main_group_display);
}

bool on_configuration_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_main_group_config);
}

bool on_palette_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_sub_group_palette);
}

bool on_about_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_sub_group_about);
}

bool on_alignment_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_sub_group_alignment);
}

bool on_diagnostic_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_sub_group_diagnostic);
}

bool on_gain_offset_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_sub_group_gain_offset);
}

bool on_save_reboot_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_sub_group_save_reboot);
}

bool on_factory_opts_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    return menu_change_view(status, origin, menu_button_sub_group_factory_opts);
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

             //Once we consume this event, we reequest no more propagations
            return false;
        }
    }
   
    return true;
}
// ---------  MENU BASED GUI EVENT SLOTS HANDLERS END  ---------- 

// --------- MENU API CALL START --------- 
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
        case menu_button_sub_group_alignment: {
            gui_object_t elements[] = {
                gui_create_text(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_common_label_props, "Horizontal"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_spinbox_props, &spinbox_horizontal),
                gui_create_text(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_common_label_props, "Vertical"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_spinbox_props, &spinbox_vertical),
                gui_create_text(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_common_label_props, "Pixel width"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_spinbox_props, &spinbox_pix_width),
                gui_create_button(&menu_overlay_ctx,  0, 0, 100, 12, &menu_colors_list, menu_common_nshared_props, "Automatic"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 100, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_alignment_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[3].base, &menu_left_buttons_group_elements[3], on_alignment_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[5].base, &menu_left_buttons_group_elements[5], on_alignment_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[5].base, &menu_left_buttons_group_elements[1], NULL); // The pixel with updates the horiz front and back porch
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[6].base, &menu_left_buttons_group_elements[6], on_automatic_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[7].base, &menu_left_buttons_group_elements[7], on_back_event);
            }
            break;
        case menu_button_sub_group_gain_offset: {
            gui_object_t elements[] = {
                gui_create_text(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_common_label_props, "Gain"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_spinbox_props, &spinbox_gain),
                gui_create_text(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_common_label_props, "Offset"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 100, 12, &menu_colors_list, menu_spinbox_props, &spinbox_offset),
                gui_create_button(&menu_overlay_ctx,  0, 0, 100, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_gain_offset_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[3].base, &menu_left_buttons_group_elements[3], on_gain_offset_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[4].base, &menu_left_buttons_group_elements[4], on_back_event);
            }
            break;
        case menu_button_sub_group_diagnostic: {
            gui_object_t elements[] = {
                gui_create_label(&menu_overlay_ctx, 0, 0, 200, 59, &menu_colors_list, menu_common_text_props, menu_diagnostic_print),
                gui_create_label(&menu_overlay_ctx, 0, 0, 200, 17, &menu_colors_list, menu_common_text_props, menu_scan_print),
                gui_create_button(&menu_overlay_ctx,  0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, menu_get_usb_button_txt(menu_usb_enabled)),
                gui_create_button(&menu_overlay_ctx,  0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Back"),
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_usb_enable_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[3].base, &menu_left_buttons_group_elements[3], on_back_event);
            add_repeating_timer_ms(MENU_HV_SYNC_REFRESH, menu_update_timer_callback, &menu_left_buttons_group_elements[1], &menu_vsync_hsync_timer);
            }
            break;
        case menu_button_sub_group_palette: {
            gui_object_t elements[] = {
                gui_create_slider(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_spinbox_props, &color_slider_option),
                gui_create_text(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_common_label_props, "Red"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_spinbox_props, &color_spinbox_red),
                gui_create_text(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_common_label_props, "Green"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_spinbox_props, &color_spinbox_green),
                gui_create_text(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_common_label_props, "Blue"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_spinbox_props, &color_spinbox_blue),
                gui_create_button(&menu_overlay_ctx, 0, 0, 80, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            menu_setup_selected_color();
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[0].base, &menu_left_buttons_group_elements[0], on_palette_option_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_palette_color_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[4].base, &menu_left_buttons_group_elements[4], on_palette_color_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[6].base, &menu_left_buttons_group_elements[6], on_palette_color_event);
            gui_event_subscribe(button_status,  &menu_left_buttons_group_elements[7].base, &menu_left_buttons_group_elements[7], on_back_event);
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[0].base, &menu_left_buttons_group_elements[2], NULL); //Redraw RED spinbox on option change
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[0].base, &menu_left_buttons_group_elements[4], NULL); //Redraw GREEN spinbox on option change
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[0].base, &menu_left_buttons_group_elements[6], NULL); //Redraw BLUE spinbox on option change
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[0].base, &menu_main_view_group_elements[0],    NULL); //Redraw text hint
            }
            break;
        case menu_button_sub_group_about: {
            menu_about_scroll_str = menu_about_str;
            gui_object_t elements[] = {
                gui_create_label(&menu_overlay_ctx, 0, 0, 200, 89, &menu_colors_list, menu_common_text_props, menu_about_print),
                gui_create_button(&menu_overlay_ctx, 0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            add_repeating_timer_ms(MENU_ABOUT_REFRESH, menu_update_timer_callback, &menu_left_buttons_group_elements[0], &menu_vsync_hsync_timer);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_back_event);
            }
            break;
        case menu_button_sub_group_factory_opts: {
            gui_object_t elements[] = {
                gui_create_text(&menu_overlay_ctx, 0, 0, 200, 78, &menu_colors_list, menu_common_label_props, "Factory settings will\nbe restored, all local\nsettings will be lost\nAre you sure?"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Default and Reboot"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Cancel")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_factory_reboot_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_back_event);
            }
            break;
        case menu_button_sub_group_save_reboot: {
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
        case menu_button_main_group_config:{
            gui_object_t elements[] = { 
                gui_create_button(&menu_overlay_ctx, 0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Save & reboot"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Factory opts"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[0].base, &menu_left_buttons_group_elements[0], on_save_reboot_button_event);            
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_factory_opts_event);  
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_back_event);
            }
            break;
        case  menu_button_main_group_display:{
            bool auto_shut_down = settings_get()->flags.auto_shut_down;
            bool scanline_opt = settings_get()->flags.scan_line;
            gui_properties_t optional_prop = menu_common_nshared_props;
            optional_prop.focusable = (DVI_VERTICAL_REPEAT == 2);
            gui_object_t elements[] = { 
                gui_create_text(&menu_overlay_ctx, 0, 0,    200, 12, &menu_colors_list, menu_common_label_props, "Display Number"),
                gui_create_spinbox(&menu_overlay_ctx, 0, 0, 200, 12, &menu_colors_list, menu_spinbox_props, &spinbox_display_no),
                gui_create_button(&menu_overlay_ctx,  0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Alignment"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Gain & offset"),
                gui_create_button(&menu_overlay_ctx,  0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, menu_get_shutdown_opt_txt(auto_shut_down)),
                gui_create_button(&menu_overlay_ctx,  0, 0, 200, 12, &menu_colors_list, optional_prop, menu_get_scanline_opt_txt(scanline_opt && DVI_VERTICAL_REPEAT == 2)),
                gui_create_button(&menu_overlay_ctx,  0, 0, 200, 12, &menu_colors_list, menu_common_nshared_props, "Back")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_display_selection_event);
            gui_event_subscribe(button_status,  &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_alignment_button_event);
            gui_event_subscribe(button_status,  &menu_left_buttons_group_elements[3].base, &menu_left_buttons_group_elements[3], on_gain_offset_button_event);
            gui_event_subscribe(button_status,  &menu_left_buttons_group_elements[4].base, &menu_left_buttons_group_elements[4], on_shutdown_display_event);
            gui_event_subscribe(button_status,  &menu_left_buttons_group_elements[5].base, &menu_left_buttons_group_elements[5], on_scanline_display_event);
            gui_event_subscribe(button_status,  &menu_left_buttons_group_elements[6].base, &menu_left_buttons_group_elements[6], on_back_event);     
            }
            break;
        case menu_button_main_group_startup_info: {
            gui_object_t elements[] = {
                gui_create_label(&menu_overlay_ctx, 0, 0, 200, 66, &menu_colors_list, menu_common_text_props, menu_diagnostic_print),
                gui_create_label(&menu_overlay_ctx, 0, 0, 200, 24, &menu_colors_list, menu_common_text_props, menu_scan_print),
                gui_create_text(&menu_overlay_ctx, 0, 0, 200, 12, &menu_colors_list, menu_common_label_props, command_is_license_valid() ? "Thanks for collaborating!": "Invalid License, register")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            add_repeating_timer_ms(MENU_HV_SYNC_REFRESH, menu_update_timer_callback, &menu_left_buttons_group_elements[1], &menu_vsync_hsync_timer);
            }
            break;
        case menu_button_main_group_home: default: {
            gui_object_t elements[] = {
                gui_create_button(&menu_overlay_ctx, 0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Diagnostics"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Display"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Palette"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Configuration"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "About"),
                gui_create_button(&menu_overlay_ctx, 0, 0, 110, 12, &menu_colors_list, menu_common_nshared_props, "Exit")
            };
            menu_elements_copy(elements, menu_left_buttons_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_left_buttons_group_elements, arraySize(elements));
            menu_left_buttons_group_list = group_list;
            
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[0].base, &menu_left_buttons_group_elements[0], on_diagnostic_button_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[1].base, &menu_left_buttons_group_elements[1], on_display_button_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[2].base, &menu_left_buttons_group_elements[2], on_palette_button_event);
            gui_event_subscribe(button_status, &menu_left_buttons_group_elements[3].base, &menu_left_buttons_group_elements[3], on_configuration_button_event);
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
        case menu_button_sub_group_palette: {
            gui_object_t elements[] = { 
                gui_create_label(&menu_overlay_ctx, 0, 0, 115, 11, &menu_colors_list, menu_common_label_props, menu_palette_opt_print),
                gui_create_object(&menu_overlay_ctx, 0, 0, 115, 90, "circle_palette", &menu_colors_list, menu_common_label_props, &color_slider_selected, gui_draw_palette_choice)
             };
            menu_elements_copy(elements, menu_main_view_group_elements);
            gui_list_t group_list = initalizeGuiDynList(menu_main_view_group_elements, arraySize(elements));
            menu_main_view_group_list = group_list;
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[0].base, &menu_main_view_group_elements[1], NULL); //Redraw on selection change
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[2].base, &menu_main_view_group_elements[1], NULL); //Redraw on color change
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[4].base, &menu_main_view_group_elements[1], NULL); //Redraw on color change
            gui_event_subscribe(spinbox_status, &menu_left_buttons_group_elements[6].base, &menu_main_view_group_elements[1], NULL); //Redraw on color change
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
    menu_graphic_ctx.bppx = GET_VIDEO_PROPS().symbols_per_word ? rgb_16_565 : rgb_8_332;
    menu_graphic_ctx.width = GET_VIDEO_PROPS().width;
    menu_graphic_ctx.height = GET_VIDEO_PROPS().height;
    menu_graphic_ctx.video_buffer = GET_VIDEO_PROPS().video_buffer;

    set_video_overlay(MENU_VIDEO_OVERLAY_WIDTH, MENU_VIDEO_OVERLAY_HEIGHT, false);
    menu_overlay_ctx = get_sub_graphic_ctx(&menu_graphic_ctx, video_overlay_get_startx(), video_overlay_get_starty(), video_overlay.width, video_overlay.height);

    //Menu System initialize
    menu_window = gui_create_window(&menu_overlay_ctx, 0, 0, menu_overlay_ctx.width, menu_overlay_ctx.height, &menu_colors_list, menu_common_nshared_props);
    menu_left_buttons_group = menu_create_left_button_group(menu_button_group_none, menu_button_main_group_home);
    menu_left_buttons_group.base.status.enabled = false;

    //Variables mapping (since spinboxes uses uints)
    spinbox_horizontal = GET_VIDEO_PROPS().horizontal_front_porch;
    spinbox_vertical = GET_VIDEO_PROPS().vertical_front_porch;
    spinbox_pix_width = GET_VIDEO_PROPS().horizontal_front_porch + GET_VIDEO_PROPS().horizontal_back_porch;
    spinbox_offset = wm8213_afe_get_offset(color_part_all);
    spinbox_gain = wm8213_afe_get_gain(color_part_all);
    spinbox_display_no = settings_get()->flags.default_display + 1;

    menu_current_display = &(settings_get()->displays[settings_get()->flags.default_display]);
    
    // Colors in 16 and 8 bits modes
    color_black = settings_get()->flags.symbols_per_word ? color_16_black : color_8_black;
    color_white = settings_get()->flags.symbols_per_word ? color_16_white : color_8_white;
    gui_list_t _menu_colors_list = initalizeGuiList(ram_settings.menu_colors);
    menu_colors_list = _menu_colors_list;
    return 0;
}
// --------- MENU API CALL END  ---------

// Implementation of the command method
void command_show_info(bool value) {
    video_overlay_enable(value);
    menu_button_index = 0;
    cancel_repeating_timer(&menu_vsync_hsync_timer);
    menu_left_buttons_group = menu_create_left_button_group(menu_button_group_none, value ? menu_button_main_group_startup_info : menu_button_main_group_home);
    
    if (!value) {
        menu_left_buttons_group.base.status.enabled = false;
        cancel_repeating_timer(&menu_vsync_hsync_timer);
        gui_obj_draw(menu_window);
    }
    gui_obj_draw(menu_left_buttons_group);
}