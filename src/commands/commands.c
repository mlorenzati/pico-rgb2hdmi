#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "videoAdjust.h"
#include "rgbScan.h"
#include "overlay.h"
#include "storage.h"
#include "security.h"
#include "graphics.h"
#include "version.h"

#define COMMAND_VIDEO_OVERLAY_WIDTH 	-148
#define COMMAND_VIDEO_OVERLAY_HEIGHT 	-100

int command_info_afe_error;
int command_info_scanner_error;

///////////   GLOBALS   ///////////
bool command_license_is_valid;
const void *security_key_in_flash;

#if DVI_SYMBOLS_PER_WORD == 2
	#define COMMANDS_OVERLAY_BPPX rgb_16
	// Colors                0brrrrrggggggbbbbb;
	uint color_white       = 0b1111111111111111;
	uint color_gray        = 0b01100 011001 01100;
	uint color_dark        = 0b0011000110000110;
	uint color_light_blue  = 0b0111010101011011;
	uint color_yellow	   = 0b1111110111101001;
#else
	#define COMMANDS_OVERLAY_BPPX rgb_8
	// Colors                0brrrgggbb;
	uint color_white       = 0b11111111;
	uint color_gray        = 0b01101110;
	uint color_dark        = 0b01001001;
	uint color_light_blue  = 0b01110010;
	uint color_yellow	   = 0b11110001;
#endif

static graphic_ctx_t graphic_ctx = {
	.bppx = COMMANDS_OVERLAY_BPPX,
	.parent = NULL
};

static graphic_ctx_t overlay_ctx = {
	.video_buffer = NULL,
	.bppx = COMMANDS_OVERLAY_BPPX,
	.parent = &graphic_ctx
};

/////////// END GLOBALS ///////////

bool command_is_license_valid() {
    return command_license_is_valid;
}

void command_fill_blank() {
    fill_rect(&graphic_ctx, 0, 0, graphic_ctx.width, graphic_ctx.height, color_white);
}

void command_storage_initialize() {
    const char security_key[20] = "12345678901234567890";
    if (storage_initialize(security_key, &security_key_in_flash, 20, false) > 0) {
		printf("storage initialize failed \n");
	}
}

void command_validate_license() {
    int token = -1;
	command_license_is_valid = security_key_is_valid((const char *)security_key_in_flash, token) <= 0;
}

void command_prepare_graphics() {
	assert(GET_VIDEO_PROPS().width != 0);
	assert(GET_VIDEO_PROPS().height != 0);
	assert(GET_VIDEO_PROPS().video_buffer != 0);
    graphic_ctx.width = GET_VIDEO_PROPS().width;
    graphic_ctx.height = GET_VIDEO_PROPS().height;
    graphic_ctx.video_buffer = GET_VIDEO_PROPS().video_buffer;

    set_video_overlay(COMMAND_VIDEO_OVERLAY_WIDTH, COMMAND_VIDEO_OVERLAY_HEIGHT, true);
	overlay_ctx.width = video_overlay.width;
	overlay_ctx.height = video_overlay.height;
	overlay_ctx.x = video_overlay_get_startx();
	overlay_ctx.y = video_overlay_get_starty();
}

void command_show_info(bool value) {
	video_overlay_enable(value);
	if (value) {
		fill_rect(&overlay_ctx,  0, 0, overlay_ctx.width, overlay_ctx.height, color_white);
		fill_rect(&overlay_ctx,  2, 2, overlay_ctx.width - 5, overlay_ctx.height - 5, color_gray);
		if (command_info_afe_error > 0 || command_info_scanner_error > 0) {
			draw_textf(&overlay_ctx, 2, 2, color_dark, color_dark, true, "AFE error:%d\nScan error:%d\nD:%s\nmlorenzati@gmail\nVer:%s", command_info_afe_error, command_info_scanner_error, security_get_uid(), PROJECT_VER);
		} else {
			draw_textf(&overlay_ctx, 2, 2, color_dark, color_dark, true, "LorenTek RGB2HDMI\nLicense is %s\n\nmlorenzati@gmail\nVer:%s", command_license_is_valid ? "valid" : "invalid", PROJECT_VER);
		}
		fill_rect(&overlay_ctx, 41, 50, 64, 13, color_light_blue);
		fill_rect(&overlay_ctx, 41, 64, 64, 14, color_white);
		fill_rect(&overlay_ctx, 41, 79, 64, 13, color_light_blue);
		draw_circle(&overlay_ctx, 73, 71, -6,   color_yellow);
	}
}

int command_on_receive(int option, const void *data, bool convert) {
    int   integer_value = (*(const int *) data);
    bool  bool_value = *((char *)data) > 0;
    
    if (convert) {
        integer_value = atoi((const char *)data);
        bool_value = strcmp((const char *)data, "true") == 0;
    }

    switch(option) {
            case 'u':
                printf("Move screen up %d positions\n", integer_value);
				GET_VIDEO_PROPS().vertical_front_porch += integer_value;
				GET_VIDEO_PROPS().vertical_back_porch  -= integer_value;

				rgbScannerUpdateData(GET_VIDEO_PROPS().vertical_front_porch, 0);
                break;
            case 'd': 
                printf("Move screen down %d positions\n", integer_value);
				GET_VIDEO_PROPS().vertical_front_porch -= integer_value;
				GET_VIDEO_PROPS().vertical_back_porch  += integer_value;

				rgbScannerUpdateData(GET_VIDEO_PROPS().vertical_front_porch, 0);
                break;
            case 'l':
                printf("Move screen right %d positions\n", integer_value);
				GET_VIDEO_PROPS().horizontal_front_porch += integer_value;
				GET_VIDEO_PROPS().horizontal_back_porch  -= integer_value;
                break;
            case 'r':
                printf("Move screen left %d positions\n", integer_value);
				GET_VIDEO_PROPS().horizontal_front_porch -= integer_value;
				GET_VIDEO_PROPS().horizontal_back_porch  += integer_value;
                break;
			case 'i':
				if (command_license_is_valid) {
					printf("Show on screen info to %s\n", bool_value > 0 ? "on" : "off");
					command_show_info(bool_value);
				} else {
					printf("Invalid license, will not change info screen\n");
				}
                break;
			case 'c':
				printf("capture screen:");
				rgbScannerEnable(false);
				for(int height=0; height < graphic_ctx.height; height++) {
					printf("\n");
					for(int width=0; width < graphic_ctx.width; width++) {
						printf("%d%s", GET_RGB16_BUFFER(graphic_ctx.video_buffer)[graphic_ctx.width * height + width], (width < graphic_ctx.width -1) ? ",": "");
					}
				}
				rgbScannerEnable(true);
                break;
			case 'I':
                printf("Device is: %s\n", security_get_uid());
				break;
			case 'k':
				printf("Storing key: %s\n", (const char *)data);
			// 	security_str_2_hexa(strValue, serial_key, 40);
			// 	storage_update(serial_key);
			    break;
			case 'v':
                printf("%s - Integration Test - version %s\n", PROJECT_NAME, PROJECT_VER);
				break;
            default:  return 1;
    }
    return 0;
}