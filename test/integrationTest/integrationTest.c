//System defined includes
#include <stdio.h>
#include <stdlib.h>
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
#include "cmdParser.h"
#include "commands.h"

//System configuration includes
#include "common_configs.h"
#include "system.h"

// System config definitions
#define FRAME_HEIGHT 	240
#if DVI_SYMBOLS_PER_WORD == 2
	//With 2 repeated symbols per word, we go for 320 pixels width and 16 bits per pixel
	#define FRAME_WIDTH 	320
	uint16_t            framebuf[FRAME_HEIGHT][FRAME_WIDTH];
#else
	//With no repeated symbols per word, we go for 640 pixels width and 8 bits per pixel
	#define FRAME_WIDTH 	320
	uint8_t            framebuf[FRAME_HEIGHT][FRAME_WIDTH];
#endif 

#define REFRESH_RATE	50
#define VREG_VSEL 		VREG_VOLTAGE_1_20
#define DVI_TIMING 		dvi_timing_640x480p_60hz

// --------- Global register start --------- 
struct dvi_inst 	dvi0;
wm8213_afe_config_t afec_cfg_2 	          = afec_cfg;
static uint 		hdmi_scanline         = 2;
uint 				keyboard_gpio_pins[3] = { KEYBOARD_PIN_UP, KEYBOARD_PIN_DOWN, KEYBOARD_PIN_ACTION };
const uint 			LED_PIN               = PICO_DEFAULT_LED_PIN;

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
	{"key",  	TRUE, NULL,   'k'},
    {NULL,      TRUE, NULL,  0 }
};
// ----------- Global register end ----------- 

// *********** IRQ API CALL END   ************
// ----------- DVI API CALL START ------------ 
void __not_in_flash_func(core1_main)() {
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);
	#if DVI_SYMBOLS_PER_WORD == 2
		dvi_scanbuf_main_16bpp(&dvi0);
	#else
		dvi_scanbuf_main_8bpp(&dvi0);
	#endif
	
	__builtin_unreachable();
}

static inline void core1_scanline_callback() {
	#if DVI_SYMBOLS_PER_WORD == 2
		uint16_t *bufptr;
	#else
		uint8_t *bufptr;
	#endif
	while (queue_try_remove_u32(&dvi0.q_colour_free, &bufptr));
	
	bufptr = &framebuf[hdmi_scanline][0];
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	if (++hdmi_scanline >= FRAME_HEIGHT) {
    	hdmi_scanline = 0;
	}
}
// ---------  DVI API CALL END  --------- 

// ---------  RGB SCAN API CALL START  --------- 
static void __not_in_flash_func(scanLineTriggered)(unsigned int render_line_number) {
	gpio_put(LED_PIN, true);
    if (wm8213_afe_capture_run(VIDEO_OVERLAY_GET_COMPUTED_FRONT_PORCH(), (uintptr_t)&framebuf[render_line_number][VIDEO_OVERLAY_GET_COMPUTED_OFFSET()], VIDEO_OVERLAY_GET_COMPUTED_WIDTH())) {
		//Nothing is done here so far
	}
	video_overlay_scanline_prepare(render_line_number);
	gpio_put(LED_PIN, false);
}
// ---------  RGB SCAN API CALL END  --------- 

// --------- KEYBOARD API CALL START --------- 
void on_keyboard_event(keyboard_status_t keys) {
	static bool usb_enabled = false;
	static bool move_x_y = false;
	const int steps = 1;

	//Three keys pressed enables USB support
	if (keys.key1_down && keys.key2_down && keys.key3_down) {
		if (!usb_enabled) {
			usb_enabled = true;
			stdio_init_all();
		}
	}

	//key 1 toggles x vs y
	if (keys.key1_up) {
		move_x_y = !move_x_y;
	}

	if (keys.key2_up) {
		command_on_receive(move_x_y ? 'u' : 'l', &steps, false);
	}

	if (keys.key3_up) {
		command_on_receive(move_x_y ? 'd' : 'r', &steps, false);
	}
}
// ---------  KEYBOARD API CALL END  ---------  
// ***********  IRQ API CALL END  ************

void parse_command(int argc, char *const argv[]) {
    int option_result = 0;
    int option_index = 0;
    int arg_index = 0;
    char *strValue = NULL;
    while ((option_result = cmd_parser_get_cmd(argc, argv, options, &option_index, &arg_index)) != -1) {
        strValue = options[option_index].strval;
        printf ("Request %s<%c>(%s)\n", options[option_index].name, option_result, strValue);

        if (command_on_receive(option_result, strValue, true) > 0) {
			printf("Unknown command: "); cmd_parser_print_cmd(options);
		}
    }
}

void command_line_loop() {
	char inputStr[48];
    char *argv[6];
    int argc;
	while (1)
	{
		gets(inputStr);
        cmd_parser_get_argv_argc(inputStr, &argc, argv);
        parse_command(argc, argv);
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

	// Validate license prior starting second core
	command_validate_license();

	// Configure scan video properties
	set_video_props(44, 56, 50, 20, FRAME_WIDTH, FRAME_HEIGHT, REFRESH_RATE, framebuf);
	afec_cfg_2.sampling_rate_afe = GET_VIDEO_PROPS().sampling_rate;
	#if DVI_SYMBOLS_PER_WORD == 2
		afec_cfg_2.bppx = rgb_16_565;
	#else
		afec_cfg_2.bppx = rgb_8_332;
	#endif

	// Prepare render video Overlay & graphics context
	command_prepare_graphics();
	
	// Configure AFE Capture System
	command_info_afe_error = wm8213_afe_setup(&afec_cfg_2);
	if ( command_info_afe_error > 0) {
         printf("AFE initialize failed with error %d\n", command_info_afe_error);
    } else {
         printf("AFE initialize succeded \n");
    }

	// Initialize Keyboard
	keyboard_initialize(keyboard_gpio_pins, 3, KEYBOARD_REFRESH_RATE_MS, KEYBOARD_REPEAT_RATE_MS, on_keyboard_event);

	// Initialize DVI
	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
	dvi0.scanline_callback = core1_scanline_callback;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

	// Prepare first start image
	command_fill_blank();
	command_show_info(true);

	// Prepare DVI for the first time the two initial lines, passing core 1 the framebuffer
	// Start the Core1, dedicated for DVI
	#if DVI_SYMBOLS_PER_WORD == 2
		uint16_t *bufptr = GET_RGB16_BUFFER(GET_VIDEO_PROPS().video_buffer);
	#else
		uint8_t  *bufptr = GET_RGB8_BUFFER(GET_VIDEO_PROPS().video_buffer);
	#endif
	
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	bufptr += FRAME_WIDTH;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	multicore_launch_core1(core1_main);

	// Initializing RGBSCAN and the leds to indicate it's activity
	sleep_ms(10);
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
	if (command_info_afe_error == 0) {
		command_info_scanner_error = rgbScannerSetup(
			RGB_SCAN_VSYNC_PIN, RGB_SCAN_HSYNC_PIN, GET_VIDEO_PROPS().vertical_front_porch, GET_VIDEO_PROPS().height, scanLineTriggered);
		if (command_info_scanner_error > 0) {
			printf("rgbScannerSetup failed with code %d\n", command_info_scanner_error);
		}
	}

	// Wait some seconds with visual report
	for (int i = 10; i > 0; i--) {
        sleep_ms(1000);
        printf("%d\n", i);
    }

	// Remove info screen if license is valid
	command_show_info(!command_is_license_valid());

	// Show Version
	command_on_receive('v', NULL, false);

	// Main Busy Loop
	command_line_loop();

	__builtin_unreachable();
}	
