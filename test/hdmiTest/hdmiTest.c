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

//System configuration includes
#include "version.h"
#include "common_configs.h"

// TMDS bit clock 252 MHz
// DVDD 1.2V
#define FRAME_WIDTH  640
#define FRAME_HEIGHT 480

#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

struct dvi_inst dvi0;
struct semaphore dvi_start_sem;

static inline void prepare_scanline(const uint32_t *colourbuf, uint32_t *tmdsbuf) {
	const uint pixwidth = FRAME_WIDTH;
	tmds_encode_data_channel_fullres_16bpp(colourbuf, tmdsbuf + 0 * pixwidth, pixwidth, 4, 0);
	tmds_encode_data_channel_fullres_16bpp(colourbuf, tmdsbuf + 1 * pixwidth, pixwidth, 10, 5);
	tmds_encode_data_channel_fullres_16bpp(colourbuf, tmdsbuf + 2 * pixwidth, pixwidth, 15, 11);
}

// Core 1 handles DMA IRQs and runs TMDS encode on scanline buffers it
// receives through the mailbox FIFO
void __not_in_flash("main") core1_main() {
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	sem_acquire_blocking(&dvi_start_sem);
	dvi_start(&dvi0);

	while (1) {
		const uint32_t *colourbuf = (const uint32_t*)multicore_fifo_pop_blocking();
		uint32_t *tmdsbuf = (uint32_t*)multicore_fifo_pop_blocking();
		prepare_scanline(colourbuf, tmdsbuf);
		multicore_fifo_push_blocking(0);
	}
	__builtin_unreachable();
}

uint16_t img_buf[FRAME_HEIGHT / 3][FRAME_WIDTH];

int __not_in_flash("main") main() {
	vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);
	set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

	setup_default_uart();

	printf("Configuring DVI\n");

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
    dvi0.ser_cfg.symbols_per_word = 0;
	
	printf("Copy Image pattern\n");
	for (int y=0; y< FRAME_HEIGHT/3; y++) {
		for (int x=0; x< FRAME_WIDTH; x++) {
			int red = x * 32 / FRAME_WIDTH;
			int green = y * 64 / (FRAME_HEIGHT/3);
			int blue = 31 - (x * 32) / FRAME_WIDTH;
			img_buf[y][x] = (x%8>0)&&(y%4>0) ? blue<<11 |green<<5 |red : 0xFFFF;
		}
	}

	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());
    
	printf("Core 1 start\n");
	sem_init(&dvi_start_sem, 0, 1);
	hw_set_bits(&bus_ctrl_hw->priority, BUSCTRL_BUS_PRIORITY_PROC1_BITS);
	multicore_launch_core1(core1_main);

	sem_release(&dvi_start_sem);
	printf("%s version - HDMI Test %s started!\n", PROJECT_NAME, PROJECT_VER);

	while (1) {
		for (int y = 0; y < FRAME_HEIGHT; y+=2) {
			uint32_t *our_tmds_buf, *their_tmds_buf;
			queue_remove_blocking_u32(&dvi0.q_tmds_free, &their_tmds_buf);
			multicore_fifo_push_blocking((uint32_t)img_buf[y/3]);
			multicore_fifo_push_blocking((uint32_t)their_tmds_buf);
	
			queue_remove_blocking_u32(&dvi0.q_tmds_free, &our_tmds_buf);
			prepare_scanline((const uint32_t*)(img_buf[(y+1)/3]), our_tmds_buf);
			
			multicore_fifo_pop_blocking();
			queue_add_blocking_u32(&dvi0.q_tmds_valid, &their_tmds_buf);
			queue_add_blocking_u32(&dvi0.q_tmds_valid, &our_tmds_buf);
		}
	}
	__builtin_unreachable();
}