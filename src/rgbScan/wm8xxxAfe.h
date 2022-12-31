/*
*/

#ifndef _WM8XXX_AFE_H
#define _WM8XXX_AFE_H

#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "color.h"
#include "wm819xAfe.h"
#include "wm821xAfe.h"

//Video Sampling in 8 bits parallel mode
//VSMP ___________________|‾|___________________________|‾|____________
//RSMP ____|‾|___________________________|‾|___________________________
//MCLK ____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____
//OP   ===>.<===R===>.<===G===>.<===B===>.<===R===>.<===G===>.<===B===>

/* Wolfson AFE WM8XXX common */ 

// Setup registers
#define WM8XXX_REG_SETUP1            0x01
#define WM8XXX_REG_SETUP2            0x02
#define WM8XXX_REG_SETUP3            0x03
#define WM8XXX_REG_SETUP4            0x06

// Reset registers
#define WM8XXX_REG_SWRESET           0x04
#define WM8XXX_REG_AUTO_RESET        0x05

// Offset Dac registers
#define WM8XXX_REG_OFFSET_DAC_RED    0x20
#define WM8XXX_REG_OFFSET_DAC_GRN    0x21
#define WM8XXX_REG_OFFSET_DAC_BLU    0x22
#define WM8XXX_REG_OFFSET_DAC_RGB    0x23

// Gain register per color
#define WM8XXX_REG_PGA_GAIN_MSB_RED  0x28
#define WM8XXX_REG_PGA_GAIN_MSB_GRN  0x29
#define WM8XXX_REG_PGA_GAIN_MSB_BLU  0x2a
#define WM8XXX_REG_PGA_GAIN_MSB_RGB  0x2b

// Read Mode Mask
#define WM8XXX_ADDR_READ(x)          0x10|x
#define WM8XXX_ADDR_WRITE(x)         0xEF&x
#define AFE_PIO_FIFO_FORCE_DUMP      8

// Bit sizes and maximum values
#define WM8XXX_POS_OFFSET_BITS       (1 << 8)
#define WM8XXX_POS_OFFSET_MAX        (WM8XXX_POS_OFFSET_BITS - 1)
#define WM8XXX_NEG_OFFSET_BITS       (1 << 4)
#define WM8XXX_NEG_OFFSET_MAX        (WM8XXX_NEG_OFFSET_BITS - 1)

// Enums
typedef enum {
    family_unknown = 0,
	family_wm819x,
    family_wm821x,
    family_max
} wm8xxx_family;

// Common Register structure
typedef struct wm8xxx_afe_setup_3 {
    io_rw_8  rlc_dac:4; //VRLC substracting DAC on input reference (VRLCSTEP*RLCDAC[0:3]+VRLCBOT)
    io_rw_8  cds_ref:2;
    io_rw_8  chan:2;
} wm8xxx_afe_setup_3_t;

typedef struct wm8xxx_afe_offset_dac {
    io_rw_8      red;    //Offset added to the RED V1 input
    io_rw_8      green;  //Offset added to the RED V1 input
    io_rw_8      blue;   //Offset added to the RED V1 input
} wm8xxx_afe_offset_dac_t;

typedef struct wm8xxx_afe_pins { 
        uint sck, sdi, sdo, cs;
} wm8xxx_afe_pins_t;

// Consolidated registers per AFE
typedef struct wm819x_afe_setups {
    wm819x_afe_setup_1_t setup1;
    wm819x_afe_setup_2_t setup2;
    wm8xxx_afe_setup_3_t setup3;
    wm819x_afe_setup_4_t setup4;
    wm819x_afe_setup_5_t setup5;
    wm819x_afe_setup_6_t setup6;
    wm8xxx_afe_offset_dac_t   offset_dac;
    wm819x_afe_pga_gain_rgb_t pga_gain;
} wm819x_afe_setups_t;

typedef struct wm821x_afe_setups {
    wm821x_afe_setup_1_t setup1;
    wm821x_afe_setup_2_t setup2;
    wm8xxx_afe_setup_3_t setup3;
    wm821x_afe_setup_4_t setup4;
    wm821x_afe_setup_5_t setup5;
    wm821x_afe_setup_6_t setup6;
    wm8xxx_afe_offset_dac_t   offset_dac;
    wm821x_afe_pga_gain_rgb_t pga_gain;
} wm821x_afe_setups_t;

typedef struct wm8xxx_afe_setups {
    wm819x_afe_setups_t wm819x;
    wm821x_afe_setups_t wm821x;
    wm8xxx_family afe_family;
} wm8xxx_afe_setups_t;

typedef struct wm8xxx_afe_config {
    spi_inst_t         *spi;
    uint                baudrate;
    wm8xxx_afe_pins_t   pins_spi;
    wm8xxx_afe_setups_t setups;
    char                verify_retries;
    PIO                 pio;
	uint                sm_afe_cp;
    uint                pin_base_afe_op;
    uint                pin_base_afe_ctrl;
    color_bppx          bppx;
} wm8xxx_afe_config_t;

typedef struct wm8xxx_afe_capture {
    PIO  pio;
    uint sm;
    uint capture_dma;
    uint front_porch_dma;
    uint sampling_rate;
    color_bppx bppx;
    uint op_pins, control_pins;
    uint pio_offset;
    wm8xxx_afe_config_t config;
} wm8xxx_afe_capture_t;

extern wm8xxx_afe_capture_t wm8xxx_afe_capture_global;

int  wm8xxx_afe_setup(const wm8xxx_afe_config_t* config, uint sampling_rate);

static inline void afe_capture_rx_fifo_drain(PIO  pio, uint sm) {
    for (int i = 0; i < AFE_PIO_FIFO_FORCE_DUMP; i++) pio_sm_get(pio, sm);
}

static inline bool wm8xxx_afe_capture_run(uint hFrontPorch, uintptr_t buffer, uint size) {
     //Don't interrupt running DMAs!
    uint capture_dma = wm8xxx_afe_capture_global.capture_dma;
    if (dma_channel_is_busy(capture_dma)) {
        return false;
    }
    
    dma_channel_hw_addr(capture_dma)->al1_write_addr = buffer;
    dma_channel_hw_addr(capture_dma)->transfer_count = size;

    PIO pio = wm8xxx_afe_capture_global.pio;
    uint sm = wm8xxx_afe_capture_global.sm;
    pio_sm_set_enabled(pio, sm, false);
    afe_capture_rx_fifo_drain(pio, sm);
    pio_sm_set_enabled(pio, sm, true);
    dma_channel_hw_addr(wm8xxx_afe_capture_global.front_porch_dma)->al1_transfer_count_trig = hFrontPorch;
    return true;
}

void wm8xxx_afe_capture_stop();
void wm8xxx_afe_capture_wait();
void wm8xxx_afe_capture_update_sampling_rate(uint sampling_rate);
void wm8xxx_afe_capture_update_bppx(color_bppx bppx);
void wm8xxx_afe_update_gain(uint16_t red, uint16_t green, uint16_t blue, bool commit);
uint16_t wm8xxx_afe_get_gain(color_part part); 
uint16_t wm8xxx_afe_get_gain_max();
void wm8xxx_afe_update_offset(uint8_t red, uint8_t green, uint8_t blue, bool commit);
uint8_t wm8xxx_afe_get_offset(color_part part);
void wm8xxx_afe_update_negative_offset(uint8_t value, bool commit);
uint8_t wm8xxx_afe_get_negative_offset();
wm8xxx_family wm8xxx_afe_get_current_family();
const char *wm8xxx_afe_get_current_family_str();
#endif