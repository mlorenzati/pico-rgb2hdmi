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
	#define COMMAND_GETBUFFER   GET_RGB16_BUFFER
	#define COMMAND_PRINTF_WORD	"%04X%s"
    #define COMMAND_GFX_BBPX    rgb_16_565
#else
	#define COMMAND_GETBUFFER   GET_RGB8_BUFFER
	#define COMMAND_PRINTF_WORD	"%02X%s"
    #define COMMAND_GFX_BBPX    rgb_8_332
#endif
/////////// END GLOBALS ///////////

bool command_is_license_valid() {
    return command_license_is_valid;
}

void command_storage_initialize() {
	#ifdef INITIAL_LICENSE
		uint8_t security_key[SECURITY_SHA_SIZE];
		const char *security_key_str = INITIAL_LICENSE;
		security_str_2_hexa(security_key_str, security_key, 40);
		const bool force_storage = true;
	#else 
		const char security_key[SECURITY_SHA_SIZE] = "12345678901234567890";
		const bool force_storage = false;
	#endif
    
    if (storage_initialize(security_key, &security_key_in_flash, SECURITY_SHA_SIZE, force_storage) > 0) {
		printf("storage initialize failed \n");
	}
}

void command_validate_license() {
    int token = -1;
	command_license_is_valid = security_key_is_valid((const char *)security_key_in_flash, token) <= 0;
}

void command_reboot() {
	printf("Rebooting\n");
	watchdog_reboot(0, SRAM_END, 10);
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
				printf("Capture screen: %dx%d@%dbppx", GET_VIDEO_PROPS().width, GET_VIDEO_PROPS().height, bppx_to_int(COMMAND_GFX_BBPX, color_part_all));
				rgbScannerEnable(false);
				for(int height=0; height < GET_VIDEO_PROPS().height; height++) {
					printf("\n");
					for(int width=0; width < GET_VIDEO_PROPS().width; width++) {
						printf(COMMAND_PRINTF_WORD, COMMAND_GETBUFFER(GET_VIDEO_PROPS().video_buffer)[GET_VIDEO_PROPS().width * height + width], (width < GET_VIDEO_PROPS().width -1) ? ",": "");
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
                printf("%s %dx%d@%dbppx\n", PROJECT_NAME, GET_VIDEO_PROPS().width, GET_VIDEO_PROPS().height, bppx_to_int(COMMAND_GFX_BBPX, color_part_all));
				break;
			case 'R':
				printf("Software reboot requested\n");
				command_reboot();
				break;
            default:  return 1;
    }
    return 0;
}