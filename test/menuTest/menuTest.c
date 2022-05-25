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

//HW Configuration includes
#include "common_configs.h"
#include "math.h"

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
uint16_t framebuf[FRAME_WIDTH * FRAME_HEIGHT];
uint gpio_pins[3] = { KEYBOARD_PIN_UP, KEYBOARD_PIN_DOWN, KEYBOARD_PIN_ACTION };
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;

static const graphic_ctx_t graphic_ctx = {
	.width = FRAME_WIDTH,
	.height = FRAME_HEIGHT,
	.video_buffer = framebuf,
	.bppx = rgb_16,
	.parent = NULL
};

// --------- Global register end --------- 

void __not_in_flash_func(core1_main)() {
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);
	dvi_scanbuf_main_16bpp(&dvi0);
	__builtin_unreachable();
}

static volatile uint scanline = 2;
static inline void core1_scanline_callback() {
	uint16_t *bufptr;
	while (queue_try_remove_u32(&dvi0.q_colour_free, &bufptr));	
	bufptr = &framebuf[FRAME_WIDTH * scanline];
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	scanline = (scanline + 1) % FRAME_HEIGHT;
}

void on_keyboard_event(keyboard_status_t keys) {
    printf("Keyboard event received \n");
}

int __not_in_flash_func(main)() {
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
	for (int n=0; n < FRAME_WIDTH * FRAME_HEIGHT; n++) {
		framebuf[n] = 0x0000;
	}

	//Prepare for the first time the two initial lines
	uint16_t *bufptr = framebuf;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	bufptr += FRAME_WIDTH;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);

	printf("Core 1 start\n");
	multicore_launch_core1(core1_main);

	printf("Start rendering\n");

	uint x, y, a;
	uint sizex = 160;
	uint sizey = 120;
	uint color1 = 0b1111100000000000;
	uint color2 = 0b0000011111100000;
	uint color3 = 0b0000000000011111;
	uint color4 = 0b1111111111111111;
	uint rad = 18;

	while (1)
	{
		while (scanline < (190)) {};
		fill_rect(&graphic_ctx, 32, 32, 287, 198, 0);
		for (a = 0; a < 16; a++) {
			x = sizex + sizex/2 * sin(2*M_PI*a/16);
			y = sizey + sizey/2 * cos(2*M_PI*a/16);
			draw_circle(&graphic_ctx, x, y, rad, color1);
			rad = ((rad - 1) % 16 ) + 2;
		}
		rad = ((rad - 1) % 16 ) + 2;
		
		x = 159;
		y = 119;
		
		fill_rect(&graphic_ctx, x - 16, y - 16, x + 16, y + 16, color2);
		
		draw_line(&graphic_ctx, 0, 0, 319, 239, color3);
		draw_line(&graphic_ctx, 319, 0, 0, 239, color3);

		draw_rect(&graphic_ctx, 20, 20, 299, 219, color2);

		draw_textf(&graphic_ctx, 58, 196, color4, color4, "This is a test of LorenTek\nRGB2HDMI %d", 2022);
		sleep_ms(50);
	}
	__builtin_unreachable();
}

