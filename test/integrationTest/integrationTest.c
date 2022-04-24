#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/vreg.h"
#include "pico/sem.h"

#include "dvi.h"
#include "dvi_serialiser.h"
#include "rgbScan.h"
#include "wm8213Afe.h"

#include "common_configs.h"

// TMDS bit clock 252 MHz
// DVDD 1.2V (1.1V seems ok too)
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

int vFrontPorch = 24;
int vBackPorch = 239 + 25;
int hFrontPorch = 60;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;

struct dvi_inst dvi0;
uint16_t framebuf[FRAME_WIDTH * FRAME_HEIGHT];
wm8213_afe_config_t afec_cfg_2 = afec_cfg;
int samplingRate = (FRAME_WIDTH+50+50)*(FRAME_HEIGHT+30+30)*50;    

void __not_in_flash("core1_main")  core1_main() {
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);
	dvi_scanbuf_main_16bpp(&dvi0);
	__builtin_unreachable();
}

static inline void core1_scanline_callback() {
	// Discard any scanline pointers passed back
	uint16_t *bufptr;
	while (queue_try_remove_u32(&dvi0.q_colour_free, &bufptr));
	// // Note first two scanlines are pushed before DVI start
	static uint scanline = 2;
	bufptr = &framebuf[FRAME_WIDTH * scanline];
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	scanline = (scanline + 1) % FRAME_HEIGHT;
}

static inline void scanLineTriggered(unsigned int scanlineNumber) {
    wm8213_afe_capture_run(hFrontPorch, (uintptr_t)&framebuf[FRAME_WIDTH * (scanlineNumber - vFrontPorch)], 320);
	gpio_put(LED_PIN, blink);
    blink = !blink;
}

void __not_in_flash("main") main() {
	vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);
	// Run system at TMDS bit clock
	set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

    stdio_init_all();
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	afec_cfg_2.sampling_rate_afe = samplingRate;

	if (wm8213_afe_setup(&afec_cfg_2) > 0) {
         printf("AFE initialize failed \n");
		 gpio_put(LED_PIN, false);
    } else {
         printf("AFE initialize succeded \n");
		 gpio_put(LED_PIN, true);
    }

	int error = rgbScannerSetup(RGB_SCAN_VSYNC_PIN, RGB_SCAN_HSYNC_PIN, vFrontPorch, vBackPorch, scanLineTriggered);
	if (error > 0) {
        printf("rgbScannerSetup failed with code %d\n", error);
		 gpio_put(LED_PIN, false);
    }

	printf("Configuring DVI\n");

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
	dvi0.scanline_callback = core1_scanline_callback;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

	// Once we've given core 1 the framebuffer, it will just keep on displaying
	// it without any intervention from core 0
	for (int n=0; n < FRAME_WIDTH * FRAME_HEIGHT; n++) {
		framebuf[n] = 0xFFFF;
	}

	//Prepare for the first time the two initial lines
	uint16_t *bufptr = framebuf;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	bufptr += FRAME_WIDTH;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);

	printf("Core 1 start\n");
	multicore_launch_core1(core1_main);

	printf("Start rendering\n");

	for (int y = 0; y < FRAME_HEIGHT; ++y) {
		for (int x = 0; x < FRAME_WIDTH; ++x) {
			int red = x * 32 / FRAME_WIDTH;
			int green = y * 64 / FRAME_HEIGHT;
			int blue = 31 - (x * 32) / FRAME_WIDTH;
			framebuf[y * FRAME_WIDTH + x] = (x%8>0)&&(y%8>0) ? blue<<11 |green<<5 | red : 0xFFFF;
		}
	}
	while (1)
	{
		sleep_ms(1000);
	}
}

