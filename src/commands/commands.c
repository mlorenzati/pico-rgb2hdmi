#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "videoAdjust.h"
#include "rgbScan.h"
#include "overlay.h"
#include "storage.h"
#include "security.h"
#include "version.h"
#include "hardware/watchdog.h"

#define COMMAND_VIDEO_OVERLAY_WIDTH 	-148
#define COMMAND_VIDEO_OVERLAY_HEIGHT 	-100

#ifdef TEST_MODE
#define TEST_MODE_STR	"\nTest Mode!!"
#else
#define TEST_MODE_STR	""
#endif

int command_info_afe_error;
int command_info_scanner_error;

///////////   GLOBALS   ///////////
bool command_license_is_valid;
const void *security_key_in_flash;

#if DVI_SYMBOLS_PER_WORD == 2
	#define COMMANDS_OVERLAY_BPPX rgb_16
	// Colors                0brrrrrggggggbbbbb;
	uint color_white       = 0b1111111111111111;
	uint color_gray        = 0b0110001100101100;
	uint color_dark        = 0b0011000110000110;
	uint color_light_blue  = 0b0111010101011011;
	uint color_yellow	   = 0b1111110111101001;
	#define COMMAND_GETBUFFER GET_RGB16_BUFFER
	#define COMMAND_PRINTF_WORD	"%04X%s"
#else
	#define COMMANDS_OVERLAY_BPPX rgb_8
	// Colors                0brrrgggbb;
	uint color_white       = 0b11111111;
	uint color_gray        = 0b01101110;
	uint color_dark        = 0b01001001;
	uint color_light_blue  = 0b01110010;
	uint color_yellow	   = 0b11110001;
	#define COMMAND_GETBUFFER GET_RGB8_BUFFER
	#define COMMAND_PRINTF_WORD	"%02X%s"
#endif

graphic_ctx_t cmd_graphic_ctx = {
	.bppx = COMMANDS_OVERLAY_BPPX,
	.parent = NULL
};

static graphic_ctx_t overlay_ctx;

/////////// END GLOBALS ///////////

bool command_is_license_valid() {
    return command_license_is_valid;
}

void command_fill_blank() {
    fill_rect(&cmd_graphic_ctx, 0, 0, cmd_graphic_ctx.width, cmd_graphic_ctx.height, color_white);
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
    cmd_graphic_ctx.width = GET_VIDEO_PROPS().width;
    cmd_graphic_ctx.height = GET_VIDEO_PROPS().height;
    cmd_graphic_ctx.video_buffer = GET_VIDEO_PROPS().video_buffer;

    set_video_overlay(COMMAND_VIDEO_OVERLAY_WIDTH, COMMAND_VIDEO_OVERLAY_HEIGHT, true);
	overlay_ctx = get_sub_graphic_ctx(&cmd_graphic_ctx, video_overlay_get_startx(), video_overlay_get_starty(), video_overlay.width, video_overlay.height);
}

void command_reboot() {
	printf("Rebooting\n");
	watchdog_reboot(0, SRAM_END, 10);
}

void command_show_info(bool value) {
	#ifdef PURCHASE_MODE
		value = true;
	#endif
	
	video_overlay_enable(value);
	if (value) {
		fill_rect(&overlay_ctx,  0, 0, overlay_ctx.width, overlay_ctx.height, color_white);
		fill_rect(&overlay_ctx,  2, 2, overlay_ctx.width - 5, overlay_ctx.height - 5, color_gray);
		if (command_info_afe_error > 0 || command_info_scanner_error > 0) {
			draw_textf(&overlay_ctx, 2, 2, color_dark, color_dark, true, 
				"AFE error:%d\nScan error:%d\nD:%s\nmlorenzati@gmail\nVer:%s%s", command_info_afe_error, command_info_scanner_error,  security_get_uid(), PROJECT_VER, TEST_MODE_STR);
		} else {
			draw_textf(&overlay_ctx, 2, 2, color_dark, color_dark, true, 
				"LorenTek RGB2HDMI\nLicense is %s\nD:%s\nmlorenzati@gmail\nVer:%s%s", command_license_is_valid ? "valid" : "invalid", security_get_uid(), PROJECT_VER, TEST_MODE_STR);
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
				printf("Capture screen: %dx%d@%sbppx", cmd_graphic_ctx.width, cmd_graphic_ctx.height, cmd_graphic_ctx.bppx == rgb_8 ? "8" : "16");
				rgbScannerEnable(false);
				for(int height=0; height < cmd_graphic_ctx.height; height++) {
					printf("\n");
					for(int width=0; width < cmd_graphic_ctx.width; width++) {
						printf(COMMAND_PRINTF_WORD, COMMAND_GETBUFFER(cmd_graphic_ctx.video_buffer)[cmd_graphic_ctx.width * height + width], (width < cmd_graphic_ctx.width -1) ? ",": "");
					}
				}
				rgbScannerEnable(true);
                break;
			case 'I':
                printf("Device is: %s\n", security_get_uid());
				break;
#ifdef TEST_MODE
			case 'k': {
				printf("Storing key: %s\n", (const char *)data);
				int len = strlen(data);
				if (len != (SECURITY_SHA_SIZE * 2)) {
					printf("Key error received: %d chars, requires %d\n",len, SECURITY_SHA_SIZE * 2);
					return 0;
				}
				uint8_t serial_key[SECURITY_SHA_SIZE];
				security_str_2_hexa(data, serial_key, 40);
				storage_update(serial_key);
				command_reboot();
				}
			    break;
			case 'K': {
				char key_str[41];
				security_hexa_2_str(security_key_in_flash, key_str, SECURITY_SHA_SIZE);
				printf("Stored key is: %s\n", key_str);
				}
			break;
#endif
			case 'v':
                printf("%s - Integration Test - version %s\n", PROJECT_NAME, PROJECT_VER);
				break;
			case 'm':
                printf("%s %dx%d@%sbppx\n", PROJECT_NAME, cmd_graphic_ctx.width, cmd_graphic_ctx.height, cmd_graphic_ctx.bppx == rgb_8 ? "8" : "16");
				break;
			case 'R':
				printf("Software reboot requested\n");
				command_reboot();
				break;
            default:  return 1;
    }
    return 0;
}