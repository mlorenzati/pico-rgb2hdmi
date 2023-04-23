#include "menuGlobals.h"
#include "menuCallback.h"
#include "overlay.h"
#include "commands.h"
#include "version.h"
#include "security.h"
#include "rgbScan.h"
#include "wm8213Afe.h"
#include <string.h>
#include "common_configs.h"

// ---------  GUI EVENT SLOTS HANDLERS START  ---------
bool on_exit_button_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        // Only remove overlay if license is valid
        video_overlay_enable(!command_is_license_valid());
        gui_disable(&menu_left_buttons_group);
    }
    return true;
}

bool on_automatic_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        // Here we use information of HSYNC / VSYNC and HLINES to calculate pix width
    }
    return true;
}

bool on_alignment_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    uint     *o_data = (uint *) origin->data;
    io_rw_16 *data   = NULL;
    io_rw_16 *n_data = NULL;
    io_rw_16 *s_data = NULL;

    if (!status.activated && origin->status.activated) {
        destination->base.status.navigable = !destination->base.status.navigable;
    } else {
        if (o_data == &spinbox_horizontal) {
            data   = &(GET_VIDEO_PROPS().horizontal_front_porch);
            n_data = &(GET_VIDEO_PROPS().horizontal_back_porch);
        } else if (o_data == &spinbox_vertical) {
            data   = &(GET_VIDEO_PROPS().vertical_front_porch);
            n_data = &(GET_VIDEO_PROPS().vertical_back_porch);
        } else if (o_data == &spinbox_pix_width) {
            data   = &(GET_VIDEO_PROPS().horizontal_front_porch);
            s_data = &(GET_VIDEO_PROPS().horizontal_back_porch);
        }
        if (status.add && *n_data != 0) {
            *data   += 1;
            if (n_data != NULL) {
                *n_data -= 1;
            } else if (s_data != NULL) {
                *s_data += 1;
            }
        } else if (status.substract && *data > 1) {
            *data   -= 1;
            if (n_data != NULL) {
                *n_data += 1;
            } else if (s_data != NULL) {
                if (*s_data != 0) {
                    *s_data -= 1;
                }
                if ((*s_data) + (*data) == 0) {
                    *data +=1;  
                }
            }
        }
        if (spinbox_vertical != GET_VIDEO_PROPS().vertical_front_porch) {
            rgbScannerUpdateData(GET_VIDEO_PROPS().vertical_front_porch, 0);
        }
        if (spinbox_pix_width != GET_VIDEO_PROPS().horizontal_front_porch + GET_VIDEO_PROPS().horizontal_back_porch) {
            update_sampling_rate();
            rgbScannerEnable(false);
            wm8213_afe_capture_update_sampling_rate(GET_VIDEO_PROPS().sampling_rate);
            rgbScannerEnable(true);
            
        }
        menu_current_display->v_front_porch = GET_VIDEO_PROPS().vertical_front_porch;
        menu_current_display->v_back_porch =  GET_VIDEO_PROPS().vertical_back_porch;
        menu_current_display->h_back_porch =  GET_VIDEO_PROPS().horizontal_back_porch;
        menu_current_display->h_front_porch = GET_VIDEO_PROPS().horizontal_front_porch;
        spinbox_horizontal = GET_VIDEO_PROPS().horizontal_front_porch;
        spinbox_vertical = GET_VIDEO_PROPS().vertical_front_porch;
        spinbox_pix_width = GET_VIDEO_PROPS().horizontal_front_porch + GET_VIDEO_PROPS().horizontal_back_porch;
    }

    destination->base.status.data_changed = 1;
    
    return true;
}

bool on_gain_offset_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    uint *o_data = (uint *) origin->data;
    if (!status.activated && origin->status.activated) {
        destination->base.status.navigable = !destination->base.status.navigable;
    } else {
        if (o_data == &spinbox_offset) {
            if (status.add && spinbox_offset < WM8213_POS_OFFSET_MAX) {
                spinbox_offset++;
            } else if (status.substract && spinbox_offset != 0) {
                spinbox_offset--;
            }
            wm8213_afe_update_offset(spinbox_offset, spinbox_offset, spinbox_offset, true);
            menu_current_display->offset.red   = wm8213_afe_get_offset(color_part_red);
            menu_current_display->offset.green = wm8213_afe_get_offset(color_part_green);
            menu_current_display->offset.blue  = wm8213_afe_get_offset(color_part_blue);
       } else if (o_data == &spinbox_gain) {
            if (status.add && spinbox_gain < WM8213_GAIN_MAX) {
                spinbox_gain++;
            } else if (status.substract && spinbox_gain != 0) {
                spinbox_gain--;
            }
            wm8213_afe_update_gain(spinbox_gain, spinbox_gain, spinbox_gain, true);
            menu_current_display->gain.red   = wm8213_afe_get_gain(color_part_red);
            menu_current_display->gain.green = wm8213_afe_get_gain(color_part_green);
            menu_current_display->gain.blue  = wm8213_afe_get_gain(color_part_blue);
       }
    }
    destination->base.status.data_changed = 1;

    return true;
}

bool on_display_selection_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        destination->base.status.navigable = !destination->base.status.navigable;
    } else {
        if (status.add && spinbox_display_no < SETTINGS_DISPLAY_MAX) {
                spinbox_display_no++;
        } else if (status.substract && spinbox_display_no > 1) {
            spinbox_display_no--;
        }
        if (spinbox_display_no != settings_get()->flags.default_display + 1) {
            settings_get()->flags.default_display = spinbox_display_no - 1; 
            menu_current_display = &(settings_get()->displays[settings_get()->flags.default_display]);

            // Update all display related configs one by one
            // Gain does not commit changes
            wm8213_afe_update_gain(menu_current_display->gain.red, menu_current_display->gain.green, menu_current_display->gain.blue, false);
            // Offset commits all changes
            wm8213_afe_update_offset(menu_current_display->offset.red, menu_current_display->offset.green, menu_current_display->offset.blue, true);
            // Timing and aligment
            set_video_props(menu_current_display->v_front_porch, menu_current_display->v_back_porch, 
                menu_current_display->h_front_porch, menu_current_display->h_back_porch, 
                GET_VIDEO_PROPS().width, GET_VIDEO_PROPS().height, menu_current_display->refresh_rate, GET_VIDEO_PROPS().video_buffer);
            rgbScannerUpdateData(GET_VIDEO_PROPS().vertical_front_porch, 0);
            rgbScannerEnable(false);
            wm8213_afe_capture_update_sampling_rate(GET_VIDEO_PROPS().sampling_rate);
            rgbScannerEnable(true);
            //Update spinBoxes
            spinbox_horizontal = menu_current_display->h_front_porch;
            spinbox_vertical   = menu_current_display->v_front_porch;
            spinbox_pix_width  = GET_VIDEO_PROPS().horizontal_front_porch + GET_VIDEO_PROPS().horizontal_back_porch;
            spinbox_offset     = wm8213_afe_get_offset(color_part_all);
            spinbox_gain       = wm8213_afe_get_gain(color_part_all);
        }
        
    }
    destination->base.status.data_changed = 1;

    return true;
}

bool on_palette_option_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    uint *data = (uint *) origin->data;
    if (!status.activated && origin->status.activated) {
        destination->base.status.navigable = !destination->base.status.navigable;
    } else if (status.add && *data < GUI_BAR_100PERCENT) {
        *data += (GUI_BAR_100PERCENT/(menu_colors_list.size - 1));
    } else if (status.substract && *data != 0) {
        *data -= (GUI_BAR_100PERCENT/(menu_colors_list.size - 1));
    }

    destination->base.status.data_changed = 1;
    menu_setup_selected_color();
   
    return true;
}

bool on_palette_color_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    uint *data = (uint *) origin->data;
    if (!status.activated && origin->status.activated) {
        destination->base.status.navigable = !destination->base.status.navigable;
    } else {
        if (*data == 255) {
            *data = 256;
        }
        if (status.add && *data < 255) {
        *data += 8;
        destination->base.status.data_changed = 1;
        } else if (status.substract && *data != 0) {
            *data -= 8;
            destination->base.status.data_changed = 1;
        }
        if (*data == 256) {
            *data = 255;
        }
    }

    if (destination->base.status.data_changed) {
        *color_slider_selected = bppx_merge_color(origin->ctx->bppx, color_spinbox_red, color_spinbox_green, color_spinbox_blue, true);
    }
  
    return true;
}

bool on_save_reboot_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        command_save_settings();
        command_reboot(); // A software reboot is required after storing the new settings
    }
    return true;
}

bool on_factory_reboot_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        command_factory_reset();
        command_reboot(); // A software reboot is required after storing the new settings
    }
    return true;
}

bool on_usb_enable_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        menu_usb_enabled = !menu_usb_enabled;
        command_enable_usb(menu_usb_enabled);
        destination->base.data = (void *) menu_get_usb_button_txt(menu_usb_enabled);
        destination->base.status.data_changed = true;
    }
    return true;
}

bool on_shutdown_display_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        bool auto_shut_down = !(settings_get()->flags.auto_shut_down);
        destination->base.data = (void *) menu_get_shutdown_opt_txt(auto_shut_down);
        destination->base.status.data_changed = true;
        settings_get()->flags.auto_shut_down = auto_shut_down;
    }
    return true;
}

bool on_scanline_display_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        bool scanline_opt = !settings_get()->flags.scan_line;
        destination->base.data = (void *) menu_get_scanline_opt_txt(scanline_opt);
        destination->base.status.data_changed = true;
        settings_get()->flags.scan_line = scanline_opt;
        dvi0.scan_line = scanline_opt;
    }
    return true;
}

// ----------  GUI EVENT SLOTS HANDLERS END  ----------

// ---------  GUI Callbacks START  ---------
void gui_draw_palette_choice(gui_base_t *base) {
    uint **selected_color = (uint **) base->data;
    uint *colors = (uint *)(base->colors->elements);
    fill_rect(base->ctx, base->x + 1, base->y + 1, base->width - 2, base->height - 2, colors[1]);
    fill_circle(base->ctx, base->x + base->width / 2, base->y + base->height / 2,  16, color_black, *(*selected_color));
};

void menu_diagnostic_print(print_delegate_t printer) {
	printer("%s v%s\n\nRes: %dx%d %dbits\nAFE code: %d\nScan code: %d\nId: %s\nLicense: %s",
        PROJECT_NAME, PROJECT_VER, menu_graphic_ctx.width, menu_graphic_ctx.height, bppx_to_int(menu_graphic_ctx.bppx, color_part_all), 
        command_info_afe_error, command_info_scanner_error, security_get_uid(),
        command_is_license_valid() ? "valid" : "invalid");
}

void menu_about_print(print_delegate_t printer) {
    static int on_start_wait_cnt = 0;
    printer("%s", menu_about_scroll_str);
    if (menu_about_scroll_str != menu_about_str || ++on_start_wait_cnt >= 4) {
        on_start_wait_cnt = 0;
        menu_about_scroll_str = strchr(menu_about_scroll_str, '\n');
        if (menu_about_scroll_str == NULL) {
            menu_about_scroll_str = menu_about_str;
        } else {
            menu_about_scroll_str++;
        }
    }
}

void menu_scan_print(print_delegate_t printer) {
	printer("VSYNC %d, HSYNC %d\nHLines %d, Type %s", 1000000000 / rgbScannerGetVsyncNanoSec(), 1000000000 / rgbScannerGetHsyncNanoSec(), rgbScannerGetHorizontalLines(), rgbScannerGetSyncTypeStr());
}

void menu_palette_opt_print(print_delegate_t printer) {
    uint index = color_slider_option * (menu_colors_list.size - 1) / GUI_BAR_100PERCENT;
    printer("%s", gui_colors_str[index]);
}
// ---------  GUI Callbacks END  ---------

// ----------- RELATED UTILS START -----------
void menu_setup_selected_color() { 
    uint index = color_slider_option * (menu_colors_list.size-1) / GUI_BAR_100PERCENT;
    color_slider_selected = &ram_settings.menu_colors[index];
    bppx_split_color(menu_overlay_ctx.bppx, ram_settings.menu_colors[index], &color_spinbox_red, &color_spinbox_green, &color_spinbox_blue, true);
}

const char *menu_get_usb_button_txt(bool status) {
    return status ? "Reboot" : "Enable USB";
}

const char *menu_get_shutdown_opt_txt(bool status) {
    return status ? "Display auto shutdown" : "Display allways on";
}

const char *menu_get_scanline_opt_txt(bool status) {
    return status ? "ScanLine on" : "ScanLine off";
}
// ----------- RELATED UTILS END -----------