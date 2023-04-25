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
#define FRAME_WIDTH_8_BITS  640
#define FRAME_WIDTH_16_BITS 320
uint8_t genbuf[FRAME_HEIGHT][FRAME_WIDTH_8_BITS];
uint8_t  *framebuf_8  = GET_RGB8_BUFFER(genbuf);
uint16_t *framebuf_16 = GET_RGB16_BUFFER(genbuf);

#define REFRESH_RATE 50
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

// --------- Global register start ---------
bool symbols_per_word = 0; //0: 1 symbol (640x240@8), 1: 2 symbols(320x240@16)
struct dvi_inst dvi0;
uint gpio_pins[3] = { KEYBOARD_PIN_UP, KEYBOARD_PIN_DOWN, KEYBOARD_PIN_ACTION };
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;
static uint hdmi_scanline = 2;
static graphic_ctx_t graphic_ctx = {
	.height       = FRAME_HEIGHT,
	.video_buffer = genbuf,
	.parent       = NULL
};
const uint color_8_list[]  = {color_8_red,  color_8_green,  color_8_blue,  color_8_white,  color_8_mid_gray,  color_8_black};
const uint color_16_list[] = {color_16_red, color_16_green, color_16_blue, color_16_white, color_16_mid_gray, color_16_black};
// --------- Global register end --------- 

void __not_in_flash_func(core1_main)() {
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);

    if (symbols_per_word) {
        dvi_scanbuf_main_16bpp(&dvi0); 
    } else {
        dvi_scanbuf_main_8bpp(&dvi0);
    }

	__builtin_unreachable();
}

static inline void core1_scanline_callback() {
	void *bufptr  = NULL;

	while (queue_try_remove_u32(&dvi0.q_colour_free, &bufptr));
    if (dvi0.ser_cfg.symbols_per_word) {
        bufptr = &framebuf_16[graphic_ctx.width * hdmi_scanline];
    } else {
        bufptr = &framebuf_8[graphic_ctx.width * hdmi_scanline];
    }

	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	if (++hdmi_scanline >= FRAME_HEIGHT) {
    	hdmi_scanline = 0;
	}
}

void on_keyboard_event(keyboard_status_t keys) {
    printf("Keyboard event received \n");
}

int main() {
    uint color_red = symbols_per_word ? color_16_red : color_8_red;
    uint color_blue = symbols_per_word ? color_16_blue : color_8_blue;
    uint color_mid_gray = symbols_per_word ? color_16_mid_gray : color_8_mid_gray;
    uint color_white = symbols_per_word ? color_16_white : color_8_white;
    const uint *color_list = symbols_per_word ? color_16_list : color_8_list;
    graphic_ctx.bppx  = symbols_per_word ? rgb_16_565 : rgb_8_332;
    graphic_ctx.width = symbols_per_word ? FRAME_WIDTH_16_BITS : FRAME_WIDTH_8_BITS;

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
    dvi0.ser_cfg.symbols_per_word = symbols_per_word;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

	// Once we've given core 1 the framebuffer, it will just keep on displaying
	// it without any intervention from core 0

	//Prepare for the first time the two initial lines
	void *bufptr = NULL;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	bufptr += graphic_ctx.width;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);

	printf("Core 1 start\n");
	multicore_launch_core1(core1_main);
	
	printf("%s version - Graphic Test %s started!\n", PROJECT_NAME, PROJECT_VER);

	printf("Start rendering\n");
	uint x, y, a;
	uint sizex = graphic_ctx.width / 2;
	uint sizey = graphic_ctx.height / 2;

	//Draw boxes
	for (int i = 0; i < 6; i++) {
		int valx = (graphic_ctx.width * i)  / 30;
		int valy = (graphic_ctx.height * i) / 15;
		fill_rect(&graphic_ctx, valx, valy, graphic_ctx.width - (2 * valx), graphic_ctx.height - (2 * valy), color_list[i]);
	}

	//Draw circles
	for (a = 0; a < 16; a++) {
		x = sizex + sizex/2 * sin(2*M_PI*a/16);
		y = sizey + sizey/2 * cos(2*M_PI*a/16);
		draw_circle(&graphic_ctx, x, y, 16, color_red);
		draw_circle(&graphic_ctx, x, y, 8, color_red);
		draw_flood(&graphic_ctx, x + 10, y, color_blue, color_red, true);
	}
	
	//Draw lines
	draw_line(&graphic_ctx, 0, 0, graphic_ctx.width - 1, graphic_ctx.height - 1, color_blue);
	draw_line(&graphic_ctx, graphic_ctx.width - 1, 0, 0, graphic_ctx.height - 1, color_blue);

	//Draw rectangle
	draw_rect(&graphic_ctx, graphic_ctx.width / 16, graphic_ctx.height / 12, graphic_ctx.width - graphic_ctx.width / 8, graphic_ctx.height - graphic_ctx.height / 8, color_mid_gray);

	//Draw text
	draw_textf(&graphic_ctx, graphic_ctx.width / 6, (graphic_ctx.height * 63) / 100, color_mid_gray, color_white, false, "This is a test of RGB%s %d", symbols_per_word ? "565" : "332", 2023);

	while (1)
	{
		sleep_ms(1000);
	}
	__builtin_unreachable();
}

