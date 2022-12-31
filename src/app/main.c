#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sem.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pll.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"
#include "hardware/structs/bus_ctrl.h"
#include "hardware/structs/ssi.h"
#include "hardware/vreg.h"

#include "tmds_encode.h"
#include "dvi.h"
#include "dvi_serialiser.h"
#include "rgbScan.h"
#include "wm8xxxAfe.h"

//System configuration includes
#include "version.h"
#include "common_configs.h"

// TMDS bit clock 252 MHz
// DVDD 1.2V
#define FRAME_WIDTH  640
#define FRAME_HEIGHT 480

#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;

struct dvi_inst dvi0;
struct semaphore dvi_start_sem;
uint16_t framebuf[FRAME_HEIGHT / 3][FRAME_WIDTH];

int vFrontPorch =  28;
int hFrontPorch = 50;
int samplingRate = (FRAME_WIDTH+56+56)*((FRAME_HEIGHT)+48+48)*50;

static inline void prepare_scanline(const uint32_t *colourbuf, uint32_t *tmdsbuf) {
	const uint pixwidth = FRAME_WIDTH;
	tmds_encode_data_channel_fullres_16bpp(colourbuf, tmdsbuf + 0 * pixwidth, pixwidth, 4, 0);
	tmds_encode_data_channel_fullres_16bpp(colourbuf, tmdsbuf + 1 * pixwidth, pixwidth, 10, 5);
	tmds_encode_data_channel_fullres_16bpp(colourbuf, tmdsbuf + 2 * pixwidth, pixwidth, 15, 11);
}

static inline void scanLineTriggered(unsigned int render_line_number) {
    wm8xxx_afe_capture_run(hFrontPorch, (uintptr_t)&framebuf[2*render_line_number/3], 640);
	gpio_put(LED_PIN, blink);
    blink = !blink;
}

// Core 1 handles DMA IRQs and runs TMDS encode on scanline buffers it
// receives through the mailbox FIFO
void __not_in_flash("main") core1_main() {
    dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	sem_acquire_blocking(&dvi_start_sem);
	dvi_start(&dvi0);

	if (wm8xxx_afe_setup(&afec_cfg, samplingRate) > 0) {
         printf("AFE initialize failed \n");
		 gpio_put(LED_PIN, false);
    } else {
         printf("AFE initialize succeded \n");
		 gpio_put(LED_PIN, true);
    }

	int error = rgbScannerSetup(RGB_SCAN_VSYNC_PIN, RGB_SCAN_HSYNC_PIN, vFrontPorch, FRAME_HEIGHT, scanLineTriggered, NULL, NULL);
	if (error > 0) {
        printf("rgbScannerSetup failed with code %d\n", error);
		gpio_put(LED_PIN, false);
    }

	printf("%s version - App %s started!\n", PROJECT_NAME, PROJECT_VER);

	while (1) {
		printf("Current Clock=%ldhz, Vysnc=%ldnSec, %ldHz, Hsync=%dnSec, %dHz\n", clock_get_hz(clk_sys), rgbScannerGetVsyncNanoSec(), 1000000000 / rgbScannerGetVsyncNanoSec(), rgbScannerGetHsyncNanoSec(), 1000000000 / rgbScannerGetHsyncNanoSec());
		sleep_ms(1000);
	}
	__builtin_unreachable();
}

int __not_in_flash("main") main() {
	vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);
	set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

	stdio_init_all();
    setup_default_uart();
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	printf("Configuring DVI\n");

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
	
	printf("Copy Image pattern\n");
	for (int y=0; y< FRAME_HEIGHT/3; y++) {
		for (int x=0; x< FRAME_WIDTH; x++) {
			int red = x * 32 / FRAME_WIDTH;
			int green = y * 64 / (FRAME_HEIGHT/3);
			int blue = 31 - (x * 32) / FRAME_WIDTH;
			framebuf[y][x] = (x%8>0)&&(y%4>0) ? blue<<11 |green<<5 |red : 0xFFFF;
		}
	}

	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());
    
	printf("Core 1 start\n");
	sem_init(&dvi_start_sem, 0, 1);
	hw_set_bits(&bus_ctrl_hw->priority, BUSCTRL_BUS_PRIORITY_PROC1_BITS);
	multicore_launch_core1(core1_main);

	sem_release(&dvi_start_sem);
	while (1) {
		for (int y = 0; y < FRAME_HEIGHT; y+=2) {
			uint32_t *tmds_buf;
	
			queue_remove_blocking_u32(&dvi0.q_tmds_free, &tmds_buf);
			prepare_scanline((const uint32_t*)(framebuf[y/3]), tmds_buf);
			queue_add_blocking_u32(&dvi0.q_tmds_valid, &tmds_buf);
		}
	}
	__builtin_unreachable();
}