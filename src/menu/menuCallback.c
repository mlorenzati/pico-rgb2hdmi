#include "menuCallback.h"
#include "menuGlobals.h"
#include "overlay.h"
#include "commands.h"
#include "version.h"
#include "security.h"
#include "rgbScan.h"
#include "wm8213Afe.h"

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
        } else if (status.substract && *data != 0) {
            *data   -= 1;
            if (n_data != NULL) {
                *n_data += 1;
            } else if (s_data != NULL) {
                *s_data -= 1;
            }
        }
        if (spinbox_vertical != GET_VIDEO_PROPS().vertical_front_porch) {
            rgbScannerUpdateData(GET_VIDEO_PROPS().vertical_front_porch, 0);
        }
        if (spinbox_pix_width != GET_VIDEO_PROPS().horizontal_front_porch + GET_VIDEO_PROPS().horizontal_back_porch) {
            update_sampling_rate();
            //wm8213_afe_capture_update_sampling_rate(GET_VIDEO_PROPS().sampling_rate);
        }
        spinbox_horizontal = GET_VIDEO_PROPS().horizontal_front_porch;
        spinbox_vertical = GET_VIDEO_PROPS().vertical_front_porch;
        spinbox_pix_width = GET_VIDEO_PROPS().horizontal_front_porch + GET_VIDEO_PROPS().horizontal_back_porch;
    }

    destination->base.status.data_changed = 1;
    
    return true;
}

bool on_gain_offset_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    uint *data = (uint *) origin->data;
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
        command_reboot();
    }
    return true;
}

bool on_factory_reboot_event(gui_status_t status, gui_base_t *origin, gui_object_t *destination) {
    if (!status.activated && origin->status.activated) {
        command_reboot();
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
};

void menu_scan_print(print_delegate_t printer) {
	printer("HSYNC %d, VSYNC %d\nHLines %d, Type %s", 1000000000 / rgbScannerGetVsyncNanoSec(), 1000000000 / rgbScannerGetHsyncNanoSec(), rgbScannerGetHorizontalLines(), rgbScannerGetSyncTypeStr());
};

void menu_palette_opt_print(print_delegate_t printer) {
    uint index = color_slider_option * (menu_colors_list.size - 1) / GUI_BAR_100PERCENT;
    printer("%s", gui_colors_str[index]);
}
// ---------  GUI Callbacks END  ---------

// ----------- RELATED UTILS START -----------
void menu_setup_selected_color() { 
    uint index = color_slider_option * (menu_colors_list.size-1) / GUI_BAR_100PERCENT;
    color_slider_selected = &menu_colors[index];
    bppx_split_color(menu_overlay_ctx.bppx, menu_colors[index], &color_spinbox_red, &color_spinbox_green, &color_spinbox_blue, true);
}
// ----------- RELATED UTILS END -----------