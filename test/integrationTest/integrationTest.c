//System defined includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sem.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/vreg.h"

//Library related includes
#include "dvi.h"
#include "dvi_serialiser.h"
#include "rgbScan.h"
#include "wm8213Afe.h"
#include "videoAdjust.h"
#include "overlay.h"
#include "graphics.h"
#include "keyboard.h"
#include "security.h"
#include "storage.h"
#include "cmdParser.h"

//System configuration includes
#include "version.h"
#include "common_configs.h"
#include "system.h"

// System config definitions
// TMDS bit clock 252 MHz
// DVDD 1.2V (1.1V seems ok too)
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
#define REFRESH_RATE 50

#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

// --------- Global register start --------- 
struct dvi_inst dvi0;
wm8213_afe_config_t afec_cfg_2 = afec_cfg;
uint16_t framebuf[FRAME_WIDTH * FRAME_HEIGHT];
uint gpio_pins[3] = { KEYBOARD_PIN_UP, KEYBOARD_PIN_DOWN, KEYBOARD_PIN_ACTION };
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;

static graphic_ctx_t graphic_ctx = {
	.width = FRAME_WIDTH,
	.height = FRAME_HEIGHT,
	.video_buffer = framebuf,
	.bppx = rgb_16,
	.parent = NULL
};

static graphic_ctx_t overlay_ctx = {
	.video_buffer = NULL,
	.bppx = rgb_16,
	.parent = &graphic_ctx
};

cmd_parser_option_t options[] =
{
    {"up",      TRUE,  NULL,  'u'},
    {"down",    TRUE,  NULL,  'd'},
    {"left",    TRUE,  NULL,  'l'},
	{"right",   TRUE,  NULL,  'r'},
	{"info",    TRUE,  NULL,  'i'},
	{"capture", FALSE, NULL,  'c'},
	{"id",      FALSE, NULL,  'I'},
	{"version", FALSE, NULL,  'v'},
	// {"key",  	TRUE, NULL,   'k'},
    {NULL,      TRUE, NULL,  0 }
};

// Colors                0brrrrrggggggbbbbb;
uint color_white       = 0b1111111111111111;
uint color_gray        = 0b0110001100101100;
uint color_dark        = 0b0011000110000110;
uint color_light_blue  = 0b0111010101011011;
uint color_yellow	   = 0b1111110111101001;

const char security_key[20] = "12345678901234567890";
const void *security_key_in_flash;
bool license_is_valid;
// --------- Global register end --------- 
void show_info(bool value) {
	video_overlay_enable(value);
	if (value) {
		fill_rect(&overlay_ctx, 0, 0, overlay_ctx.width, overlay_ctx.height, color_white);
		fill_rect(&overlay_ctx, 2, 2, overlay_ctx.width - 5, overlay_ctx.height - 5, color_gray);
		draw_textf(&overlay_ctx, 2, 2, color_dark, color_dark, "LorenTek RGB2HDMI\nLicense is %s\n\nmlorenzati@gmail\nArgentina", license_is_valid ? "valid" : "invalid");
		fill_rect(&overlay_ctx, 41, 50, 64, 13, color_light_blue);
		fill_rect(&overlay_ctx, 41, 64, 64, 14, color_white);
		fill_rect(&overlay_ctx, 41, 79, 64, 13, color_light_blue);
		draw_circle(&overlay_ctx, 73, 71, -6,   color_yellow);
	}
}

void parse_command(int argc, char *const argv[]) {
    int option_result = 0;
    int option_index = 0;
    int arg_index = 0;
    char *strValue = NULL;
    char value = 0;
	//uint8_t serial_key[20];
    while ((option_result = cmd_parser_get_cmd(argc, argv, options, &option_index, &arg_index)) != -1) {
        strValue = options[option_index].strval;
        printf ("Request %s<%c>(%s)\n", options[option_index].name, option_result, strValue);

        switch (option_result) {
            case 'u':
                value = atoi(strValue);

                printf ("Move screen up %d positions\n", value);
				GET_VIDEO_PROPS().vertical_front_porch += value;
				GET_VIDEO_PROPS().vertical_back_porch  -= value;

				rgbScannerUpdateData(GET_VIDEO_PROPS().vertical_front_porch, 0);
                break;
            case 'd': 
                value = atoi(strValue);

                printf ("Move screen down %d positions\n", value);
				GET_VIDEO_PROPS().vertical_front_porch -= value;
				GET_VIDEO_PROPS().vertical_back_porch  += value;

				rgbScannerUpdateData(GET_VIDEO_PROPS().vertical_front_porch, 0);
                break;
            case 'l':
                value = atoi(strValue);

                printf ("Move screen right %d positions\n", value);
				GET_VIDEO_PROPS().horizontal_front_porch += value;
				GET_VIDEO_PROPS().horizontal_back_porch  -= value;
                break;
            case 'r':
				value = atoi(strValue);

                printf ("Move screen left %d positions\n", value);
				GET_VIDEO_PROPS().horizontal_front_porch -= value;
				GET_VIDEO_PROPS().horizontal_back_porch  += value;
                break;
			case 'i':
				value = strcmp(strValue, "true") == 0;
				if (license_is_valid) {
					printf("Show on screen info to %s\n", value > 0 ? "on" : "off");
					show_info(value);
				} else {
					printf("Invalid license, will not change info screen\n");
				}
				
                break;
			case 'c':
				printf("capture screen:");
				rgbScannerEnable(false);
				for(int height=0; height < FRAME_HEIGHT; height++) {
					printf("\n");
					for(int width=0; width < FRAME_WIDTH; width++) {
						printf("%d%s", framebuf[FRAME_WIDTH * height + width], (width < FRAME_WIDTH -1) ? ",": "");
					}
				}
				rgbScannerEnable(true);
                break;
			case 'I':
				printf("Device is: %s\n", security_get_uid());
				break;
			// case 'k':
			// 	printf("Storing key: %s\n", strValue);
			// 	security_str_2_hexa(strValue, serial_key, 40);
			// 	storage_update(serial_key);
			// 	break;
			case 'v':
				printf("%s - Integration Test - version %s\n", PROJECT_NAME, PROJECT_VER);
				break;
            default:  printf("Unknown command: "); cmd_parser_print_cmd(options); break;
         }
    }
}

void __not_in_flash_func(core1_main)() {
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);
	dvi_scanbuf_main_16bpp(&dvi0);
	__builtin_unreachable();
}

static uint hdmi_scanline = 2;
static inline void core1_scanline_callback() {
	uint16_t *bufptr;
	while (queue_try_remove_u32(&dvi0.q_colour_free, &bufptr));
	
	bufptr = &framebuf[FRAME_WIDTH * hdmi_scanline];
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	hdmi_scanline = (hdmi_scanline + 1) % FRAME_HEIGHT;
}

static void __not_in_flash_func(scanLineTriggered)(unsigned int render_line_number) {
	gpio_put(LED_PIN, true);
    if (wm8213_afe_capture_run(VIDEO_OVERLAY_GET_COMPUTED_FRONT_PORCH(), (uintptr_t)&framebuf[FRAME_WIDTH * render_line_number + VIDEO_OVERLAY_GET_COMPUTED_OFFSET()], VIDEO_OVERLAY_GET_COMPUTED_WIDTH())) {

	}
	video_overlay_scanline_prepare(render_line_number);
	gpio_put(LED_PIN, false);
}

void on_keyboard_event(keyboard_status_t keys) {
	static bool usb_enabled = false;
	static bool move_x_y = false;
	//Three keys pressed enables USB support
	if (keys.key1_down && keys.key2_down && keys.key3_down) {
		if (!usb_enabled) {
			usb_enabled = true;
			stdio_init_all();
		}
	}

	//key 1 toggles x vs y
	if (keys.key1_up) {
		move_x_y = ! move_x_y;
	}

	if (keys.key2_up || keys.key3_up) {
		io_rw_16 *porch_1;
		io_rw_16 *porch_2;
		bool is_vertical = false;
		if (move_x_y) {
			porch_1 = &(GET_VIDEO_PROPS().vertical_front_porch);
			porch_2 = &(GET_VIDEO_PROPS().vertical_back_porch);
			is_vertical = true;
		} else {
			porch_1 = &(GET_VIDEO_PROPS().horizontal_front_porch);
			porch_2 = &(GET_VIDEO_PROPS().horizontal_back_porch);
		}
		if (keys.key3_up) {
			io_rw_16 *swap = porch_1;
			porch_1 = porch_2;
			porch_2 = swap;
		}
		(*porch_1)++;
		(*porch_2)--;
		if (is_vertical) {
			rgbScannerUpdateData(GET_VIDEO_PROPS().vertical_front_porch, 0);
		}
	}	
}

int main() {
	vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);

	// Run system at TMDS bit clock
	set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);
	#ifdef RGB2HDMI_DEBUG
	//system_delayed_write_disable();
	#endif

	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	//
	if (storage_initialize(security_key, &security_key_in_flash, 20, false) > 0) {
		printf("storage initialize failed \n");
	}

	int token = -1;
	license_is_valid = security_key_is_valid((const char *)security_key_in_flash, token) <= 0;

	//Configure video properties
	set_video_props(44, 56, 50, 20, FRAME_WIDTH, FRAME_HEIGHT, REFRESH_RATE);
	afec_cfg_2.sampling_rate_afe = GET_VIDEO_PROPS().sampling_rate;

	//Prepare video Overlay & graphics context
	set_video_overlay(-148,-100, true);
	overlay_ctx.width = video_overlay.width;
	overlay_ctx.height = video_overlay.height;
	overlay_ctx.x = video_overlay_get_startx();
	overlay_ctx.y = video_overlay_get_starty();

	if (wm8213_afe_setup(&afec_cfg_2) > 0) {
         printf("AFE initialize failed \n");
		 gpio_put(LED_PIN, false);
    } else {
         printf("AFE initialize succeded \n");
		 gpio_put(LED_PIN, true);
    }

	int error = rgbScannerSetup(
		RGB_SCAN_VSYNC_PIN, RGB_SCAN_HSYNC_PIN, GET_VIDEO_PROPS().vertical_front_porch, GET_VIDEO_PROPS().height, scanLineTriggered);
	if (error > 0) {
        printf("rgbScannerSetup failed with code %d\n", error);
		gpio_put(LED_PIN, false);
    }

	printf("Initializing keyboard\n");
	keyboard_initialize(gpio_pins, 3, KEYBOARD_REFRESH_RATE_MS, KEYBOARD_REPEAT_RATE_MS, on_keyboard_event);

	printf("Configuring DVI\n");

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
	dvi0.scanline_callback = core1_scanline_callback;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

	// Once we've given core 1 the framebuffer, it will just keep on displaying
	// it without any intervention from core 0
	fill_rect(&graphic_ctx, 0, 0, FRAME_WIDTH, FRAME_HEIGHT, color_white);
	show_info(true);

	//Prepare for the first time the two initial lines
	uint16_t *bufptr = framebuf;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	bufptr += FRAME_WIDTH;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);

	printf("Core 1 start\n");
	multicore_launch_core1(core1_main);

	for (int i = 10; i > 0; i--) {
        sleep_ms(1000);
        printf("%d\n", i);
    }

	if (license_is_valid) { 
		show_info(false);
	}
	
	printf("IntegrationTest %s - version %s started!\n", PROJECT_NAME, PROJECT_VER);
	char inputStr[64];
    char *argv[8];
    int argc;
	while (1)
	{
		gets(inputStr);
        cmd_parser_get_argv_argc(inputStr, &argc, argv);
        parse_command(argc, argv);
	}
	__builtin_unreachable();
}

