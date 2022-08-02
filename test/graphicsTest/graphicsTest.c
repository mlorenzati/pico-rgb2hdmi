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
#include "keyboard.h"
#include "graphics.h"

//System configuration includes
#include "version.h"
#include "common_configs.h"
#include "math.h"

// System config definitions
// TMDS bit clock 252 MHz
// DVDD 1.2V (1.1V seems ok too)
#define FRAME_HEIGHT 240
#if DVI_SYMBOLS_PER_WORD == 2
	//With 2 repeated symbols per word, we go for 320 pixels width and 16 bits per pixel
	#define FRAME_WIDTH 320
	uint16_t framebuf[FRAME_HEIGHT][FRAME_WIDTH];
#else
	//With no repeated symbols per word, we go for 640 pixels width and 8 bits per pixel
	#define FRAME_WIDTH 640
	uint8_t framebuf[FRAME_HEIGHT][FRAME_WIDTH];
#endif

#define REFRESH_RATE 50
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

// --------- Global register start --------- 
struct dvi_inst dvi0;
uint gpio_pins[3] = { KEYBOARD_PIN_UP, KEYBOARD_PIN_DOWN, KEYBOARD_PIN_ACTION };
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;
static uint hdmi_scanline = 2;
static const graphic_ctx_t graphic_ctx = {
	.width = FRAME_WIDTH,
	.height = FRAME_HEIGHT,
	.video_buffer = framebuf,
	#if DVI_SYMBOLS_PER_WORD == 2
	.bppx = rgb_16,
	#else
	.bppx = rgb_8,
	#endif
	.parent = NULL
};

// --------- Global register end --------- 

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

void on_keyboard_event(keyboard_status_t keys) {
    printf("Keyboard event received \n");
}

int main() {
	vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);

	// Run system at TMDS bit clock
	set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

    stdio_init_all();
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	printf("Initializing keyboard\n");
	keyboard_initialize(gpio_pins, 3, KEYBOARD_REFRESH_RATE_MS, KEYBOARD_REPEAT_RATE_MS, on_keyboard_event);

	printf("Configuring DVI\n");

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
	dvi0.scanline_callback = core1_scanline_callback;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

	// Once we've given core 1 the framebuffer, it will just keep on displaying
	// it without any intervention from core 0

	//Prepare for the first time the two initial lines
	#if DVI_SYMBOLS_PER_WORD == 2
		uint16_t *bufptr;
	#else
		uint8_t *bufptr;
	#endif
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	bufptr += FRAME_WIDTH;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);

	printf("Core 1 start\n");
	multicore_launch_core1(core1_main);
	
	printf("%s version - Graphic Test %s started!\n", PROJECT_NAME, PROJECT_VER);

	printf("Start rendering\n");
	uint x, y, a;
	uint sizex = FRAME_WIDTH / 2;
	uint sizey = FRAME_HEIGHT / 2;
	#if DVI_SYMBOLS_PER_WORD == 2
	uint color0 = 0b0001000100000010;
	uint color1 = 0b1111100000000000;
	uint color2 = 0b0000011111100000;
	uint color3 = 0b0000000000011111;
	uint color4 = 0b1111111111111111;
	#else
	char color0 = 0b00100101;
	char color1 = 0b11100000;
	char color2 = 0b00011100;
	char color3 = 0b00000011;
	char color4 = 0b11111111;
	#endif

	fill_rect(&graphic_ctx,  0, 0, graphic_ctx.width, graphic_ctx.height, color0);

	for (a = 0; a < 16; a++) {
		x = sizex + sizex/2 * sin(2*M_PI*a/16);
		y = sizey + sizey/2 * cos(2*M_PI*a/16);
		draw_circle(&graphic_ctx, x, y, 16, color1);
	}
	
	x = 159;
	y = 119;
	
	fill_rect(&graphic_ctx, x - (FRAME_WIDTH / 20), y - (FRAME_WIDTH / 20), FRAME_WIDTH / 10, FRAME_WIDTH / 10, color2);
	
	draw_line(&graphic_ctx, 0, 0, FRAME_WIDTH - 1, FRAME_HEIGHT - 1, color3);
	draw_line(&graphic_ctx, FRAME_WIDTH - 1, 0, 0, FRAME_HEIGHT - 1, color3);

	draw_rect(&graphic_ctx, FRAME_WIDTH / 16, FRAME_HEIGHT / 12, 279, 199, color2);

	draw_textf(&graphic_ctx, 58, 196, color4, color4, false, "This is a test of LorenTek\nRGB2HDMI %d", 2022);

	while (1)
	{
		sleep_ms(1000);
	}
	__builtin_unreachable();
}

