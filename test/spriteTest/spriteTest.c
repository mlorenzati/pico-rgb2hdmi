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
#include "sprite.h"

//System configuration includes
#include "version.h"
#include "common_configs.h"
#include "math.h"

//Demo includes
#include "retro_logo_128x128_rgb.h"

// System config definitions
// TMDS bit clock 252 MHz
// DVDD 1.2V (1.1V seems ok too)

//With 2 repeated symbols per word, we go for 320 pixels width and 16 bits per pixel
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
#define REFRESH_RATE 50
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz
#define N_LOGOS 32
uint16_t framebuf[FRAME_HEIGHT][FRAME_WIDTH];

// --------- Global register start --------- 
struct dvi_inst dvi0;
uint gpio_pins[3] = { KEYBOARD_PIN_UP, KEYBOARD_PIN_DOWN, KEYBOARD_PIN_ACTION };
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;
volatile uint hdmi_scanline = 2;
bool symbols_per_word = 1; //0: 1 symbol (320x240@16), 1: 2 symbols(640x240@8)

sprite_t berry[N_LOGOS];
int vx[N_LOGOS];
int vy[N_LOGOS];
int vt[N_LOGOS];
uint8_t theta[N_LOGOS];
affine_transform_t atrans[N_LOGOS];
const int xmin = -100;
const int xmax = FRAME_WIDTH - 30;
const int ymin = -100;
const int ymax = FRAME_HEIGHT - 30;
const int vmax = 4;

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
    uint16_t *bufptr  = NULL;
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

static inline int clip(int x, int min, int max) {
    return x < min ? min : x > max ? max : x;
}

int main() {
    vreg_set_voltage(VREG_VSEL);
    sleep_ms(10);

    // Run system at TMDS bit clock
    set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

    //stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    printf("Initializing keyboard\n");
    keyboard_initialize(gpio_pins, 3, KEYBOARD_REFRESH_RATE_MS, KEYBOARD_REPEAT_RATE_MS, on_keyboard_event);

    printf("Configuring DVI\n");

    dvi0.timing = &DVI_TIMING;
    dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
    dvi0.ser_cfg.symbols_per_word = symbols_per_word;
    dvi0.scanline_callback = core1_scanline_callback;
    dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

    // Once we've given core 1 the framebuffer, it will just keep on displaying
    // it without any intervention from core 0

    //Prepare for the first time the two initial lines
    uint16_t *bufptr = NULL;

    queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
    bufptr += FRAME_WIDTH;
    queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);

    printf("Core 1 start\n");
    multicore_launch_core1(core1_main);
    
    printf("%s version - Sprite Test %s started!\n", PROJECT_NAME, PROJECT_VER);

	printf("Image Preparation\n");
    const char *logo[4] = { cbm_128x128, atari_128x128, msx_128x128, zx_128x128 };
    for (int i = 0; i < N_LOGOS; ++i) {
        berry[i].x = rand() % (xmax - xmin + 1) + xmin;
        berry[i].y = rand() % (ymax - ymin + 1) + ymin;
        berry[i].img = logo[i % 4];
        berry[i].log_size = 7;
        berry[i].has_opacity_metadata = true; // Much faster non-AT blitting
        berry[i].hflip = false;
        berry[i].vflip = false;
        vx[i] = (rand() % vmax + 1) * (rand() & 0x8000 ? 1 : -1);
        vy[i] = (rand() % vmax + 1) * (rand() & 0x8000 ? 1 : -1);
        vt[i] = (rand() % vmax + 1) * (rand() & 0x8000 ? 1 : -1);
        theta[i] = 0;
        affine_identity(atrans[i]);
    }
    printf("Start rendering\n");
    
    while (1)
    {
        sleep_ms(20);
        for (int i = 0; i < N_LOGOS; ++i) {
            berry[i].x += vx[i];
            berry[i].y += vy[i];
            theta[i] += vt[i];
            affine_identity(atrans[i]);
            affine_scale(atrans[i], 7 * AF_ONE / 8, 7 * AF_ONE / 8);
            affine_translate(atrans[i], -56, -56);
            affine_rotate(atrans[i], theta[i]);
            affine_translate(atrans[i], 60, 60);
            int xclip = clip(berry[i].x, xmin, xmax);
            int yclip = clip(berry[i].y, ymin, ymax);
            if (xclip != berry[i].x || yclip != berry[i].y) {
                berry[i].x = xclip;
                berry[i].y = yclip;
                vx[i] = (rand() % vmax + 1) * (rand() & 0x8000 ? 1 : -1);
                vy[i] = (rand() % vmax + 1) * (rand() & 0x8000 ? 1 : -1);
                vt[i] = (rand() % vmax + 1) * (rand() & 0x8000 ? 1 : -1);
                //berry[i].hflip = vx[i] < 0;
                //berry[i].vflip = vy[i] < 0;
            }
        }    
        while(hdmi_scanline != 50) {
        }

        for (uint y = 0; y < FRAME_HEIGHT; ++y) {
            sprite_fill16(&framebuf[y][0], 0x07ff, FRAME_WIDTH);
            for (int i = 0; i < N_LOGOS; ++i) {
                //sprite_asprite16(framebuf[y], &berry[i], atrans[i], y, FRAME_WIDTH);
				sprite_sprite16(framebuf[y], &berry[i], y, FRAME_WIDTH);
            }
        }  
    }
    __builtin_unreachable();
}

