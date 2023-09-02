#include <stdlib.h>
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "videoAdjust.h"
#include "rgbScan.h"
#include "overlay.h"
#include "security.h"
#include "settings.h"
#include "version.h"
#include "hardware/watchdog.h"

int command_info_afe_error;
int command_info_scanner_error;

///////////   GLOBALS   ///////////
bool command_license_is_valid;
const void *security_key_in_flash;

#define COMMAND_GET_PRINTF_WORD() command_get_current_bppx() == rgb_16_565 ? "%04X%s" : "%02X%s"
#define COMMAND_GET_WORD_FROM_BUFFER(width, height) command_get_current_bppx() == rgb_16_565 ? \
    GET_RGB16_BUFFER(GET_VIDEO_PROPS().video_buffer)[GET_VIDEO_PROPS().width * height + width] : \
    GET_RGB8_BUFFER(GET_VIDEO_PROPS().video_buffer)[GET_VIDEO_PROPS().width * height + width]

/////////// END GLOBALS ///////////

bool command_is_license_valid() {
    return command_license_is_valid;
}

void command_validate_license(const uint8_t *security_key) {
    int token = -1;
    #ifdef USE_LICENSE
	    command_license_is_valid = security_key_is_valid(security_key, token) <= 0;
    #else
        command_license_is_valid = true;
    #endif
}

void command_reboot() {
	printf("Rebooting\n");
	watchdog_reboot(0, SRAM_END, 10);
}

void command_save_settings() {
    printf("Saving settings\n");
    settings_update();
}

void command_factory_reset() {
    printf("Factory reset\n");
    settings_factory();
}

color_bppx command_get_current_bppx() {
    return settings_get()->flags.symbols_per_word ? rgb_16_565 : rgb_8_332; 
}

void command_enable_usb(bool status) {
    static bool first_time = true;
    if (first_time) {
        if (status) {
            stdio_init_all();
            first_time = false;
        }
    } else {
        // tiny USB implementation does not disables or allows to disable low level IRQs, so reboot
        //stdio_set_driver_enabled(&stdio_usb, status);
        //irq_set_enabled(low_priority_irq_num, status);
        command_reboot();
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
				printf("Capture screen: %dx%d@%dbppx", GET_VIDEO_PROPS().width, GET_VIDEO_PROPS().height, bppx_to_int(command_get_current_bppx(), color_part_all));
				rgbScannerEnable(false);
				for(int height=0; height < GET_VIDEO_PROPS().height; height++) {
					printf("\n");
					for(int width=0; width < GET_VIDEO_PROPS().width; width++) {
						printf(COMMAND_GET_PRINTF_WORD(), COMMAND_GET_WORD_FROM_BUFFER(width, height), (width < GET_VIDEO_PROPS().width -1) ? ",": "");
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
				security_str_2_hexa(data, settings_get()->security_key, SECURITY_SHA_SIZE);
				settings_update();
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
                printf("%s %dx%d@%dbppx\n", PROJECT_NAME, GET_VIDEO_PROPS().width, GET_VIDEO_PROPS().height, bppx_to_int(command_get_current_bppx(), color_part_all));
				break;
			case 'R':
				printf("Software reboot requested\n");
				command_reboot();
				break;
            case 'D': {
                bool current_state = dvi_is_started(&dvi0);
                    if (bool_value) {
                        dvi_start(&dvi0);
                    } else {
                        dvi_stop(&dvi0);
                    }
                    printf("DVI request to %s while %s\n", bool_value ? "start": "stop", current_state ? "active" : "inactive");
                }
				break;
            default:  return 1;
    }
    return 0;
}