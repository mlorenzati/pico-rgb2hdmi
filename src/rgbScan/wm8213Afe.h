/*
*/

#ifndef _WM8213_AFE_H
#define _WM8213_AFE_H

#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"

//Video Sampling in 8 bits parallel mode
//VSMP ___________________|‾|___________________________|‾|____________
//RSMP ____|‾|___________________________|‾|___________________________
//MCLK ____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____
//OP   ===>.<===R===>.<===G===>.<===B===>.<===R===>.<===G===>.<===B===>

/* Wolfson AFE WM8213 Registers */
#define WM8213_REG_SETUP_TOTAL       15      //Number of registers to setup on AFE 

// Setup registers
#define WM8213_REG_SETUP1            0x01
#define WM8213_REG_SETUP2            0x02
#define WM8213_REG_SETUP3            0x03
#define WM8213_REG_SETUP4            0x06
#define WM8213_REG_SETUP5            0x07
#define WM8213_REG_SETUP6            0x08

// Reset registers
#define WM8213_REG_SWRESET           0x04
#define WM8213_REG_AUTO_RESET        0x05

// Offset Dac registers
#define WM8213_REG_OFFSET_DAC_RED   0x20
#define WM8213_REG_OFFSET_DAC_GRN   0x21
#define WM8213_REG_OFFSET_DAC_BLU   0x22
#define WM8213_REG_OFFSET_DAC_RGB   0x23

// Gain register per color
#define WM8213_REG_PGA_GAIN_LSB_RED  0x24
#define WM8213_REG_PGA_GAIN_LSB_GRN  0x25
#define WM8213_REG_PGA_GAIN_LSB_BLU  0x26
#define WM8213_REG_PGA_GAIN_LSB_RGB  0x27
#define WM8213_REG_PGA_GAIN_MSB_RED  0x28
#define WM8213_REG_PGA_GAIN_MSB_GRN  0x29
#define WM8213_REG_PGA_GAIN_MSB_BLU  0x2a
#define WM8213_REG_PGA_GAIN_MSB_RGB  0x2b

// Read Mode Mask
#define WM8213_ADDR_READ(x)           0x10|x
#define WM8213_ADDR_WRITE(x)          0xEF&x

#define AFE_SAMPLING_LIMIT            7600000 //Should be 8MSPS but proben to be less, After this value RSMP / VSMP has to be flipped

typedef struct wm8213_afe_setup_1 {
    io_rw_8  enable:1;
    io_rw_8  cds:1; //Correlated Double Sampler
    io_rw_8  mono:1;
    io_rw_8  two_chan:1;
    io_rw_8  pgafs:2; // Black Level Adjust (3= positive signals)
    io_rw_8  mode_4_legacy:1;
    io_rw_8  legacy:1;
} wm8213_afe_setup_1_t;

typedef struct wm8213_afe_setup_2 {
    io_rw_8  opp_form:2; //x0-8bit multiplexed, 01-8 bit parallel, 11-4bits multiplex 
    io_rw_8  invop:1; //Inver polarity of output data
    io_rw_8  opd:1; //output disable 
    io_rw_8  low_refs:1; //Reduce the adc ref range
    io_rw_8  rlc_dac_rng:1; //Output range of the rlcdac (0 = AVdd, 1=VRT)
    io_rw_8  del:2; // data latency
} wm8213_afe_setup_2_t;

typedef struct wm8213_afe_setup_3 {
    io_rw_8  rlc_dac:4; //VRLC substracting DAC on input reference (VRLCSTEP*RLCDAC[0:3]+VRLCBOT)
    io_rw_8  cds_ref:2;
    io_rw_8  chan:2;
} wm8213_afe_setup_3_t;

typedef struct wm8213_afe_setup_4 {
    io_rw_8  line_by_line:1;
    io_rw_8  acyc:1;
    io_rw_8  intm:2;
    io_rw_8  reserved:4; 
} wm8213_afe_setup_4_t;

typedef struct wm8213_afe_setup_5 {
    io_rw_8  red_pd:1;
    io_rw_8  green_pd:1;
    io_rw_8  blue_pd:1;
    io_rw_8  adc_pd:1;
    io_rw_8  vrlc_dac_pd:1; //0: VRLC defined by internal RLC DAC
    io_rw_8  adc_ref_pd:1;
    io_rw_8  vrx_pd:1;
    io_rw_8  reserved:1;  
} wm8213_afe_setup_5_t;

typedef struct wm8213_afe_setup_6 {
    io_rw_8  vsm_pdet:1;
    io_rw_8  vdel:3;
    io_rw_8  posn_neg:1;
    io_rw_8  rlc_en:1;
    io_rw_8  clamp_ctrl:1;
    io_rw_8  reserved:1;  
} wm8213_afe_setup_6_t;

typedef struct wm8213_afe_offset_dac {
    io_rw_8      red;    //Offset added to the RED V1 input
    io_rw_8      green;  //Offset added to the RED V1 input
    io_rw_8      blue;   //Offset added to the RED V1 input
} wm8213_afe_offset_dac_t;

typedef struct wm8213_afe_pga_gain {
    io_rw_8      lsb:1;  // LSB Gain on the final V3
    io_rw_8      reserved:7;
    io_rw_8      msb;    // MSB Gain on the final V3
} wm8213_afe_pga_gain_t;

typedef struct wm8213_afe_pga_gain_rgb {
    wm8213_afe_pga_gain_t red;
    wm8213_afe_pga_gain_t green;
    wm8213_afe_pga_gain_t blue;
} wm8213_afe_pga_gain_rgb_t;

typedef struct wm8213_afe_setups {
    wm8213_afe_setup_1_t setup1;
    wm8213_afe_setup_2_t setup2;
    wm8213_afe_setup_3_t setup3;
    wm8213_afe_setup_4_t setup4;
    wm8213_afe_setup_5_t setup5;
    wm8213_afe_setup_6_t setup6;
    wm8213_afe_offset_dac_t   offset_dac;
    wm8213_afe_pga_gain_rgb_t pga_gain;
} wm8213_afe_setups_t;

typedef struct wm8213_afe_pins { 
        uint sck, sdi, sdo, cs;
    } wm8213_afe_pins_t;

typedef struct wm8213_afe_config {
    spi_inst_t         *spi;
    uint                baudrate;
    wm8213_afe_pins_t   pins_spi;
    wm8213_afe_setups_t setups;
    char                verify_retries;
    PIO                 pio;
	uint                sm_afe_cp;
    uint                pin_base_afe_op;
    uint                pin_base_afe_ctrl;
    uint                sampling_rate_afe;

} wm8213_afe_config_t;

int  wm8213_afe_setup(const wm8213_afe_config_t* config);
void wm8213_afe_capture_set_buffer(uintptr_t buffer, uint size);
void wm8213_afe_capture_run(uint hFrontPorch);
void wm8213_afe_capture_stop();
void wm8213_afe_capture_wait();
void wm8213_afe_capture_update_sampling_rate(uint sampling_rate);

#endif