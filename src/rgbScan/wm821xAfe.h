#ifndef _wm821x_AFE_H
#define _wm821x_AFE_H

/* Wolfson AFE wm821x Registers */
#define WM821X_REG_SETUP_TOTAL       15      //Number of registers to setup on AFE 

// Setup registers
#define WM821X_REG_SETUP5            0x07
#define WM821X_REG_SETUP6            0x08

// Gain register per color (MSB is common)
#define WM821X_REG_PGA_GAIN_LSB_RED  0x24
#define WM821X_REG_PGA_GAIN_LSB_GRN  0x25
#define WM821X_REG_PGA_GAIN_LSB_BLU  0x26
#define wm821x_REG_PGA_GAIN_LSB_RGB  0x27

// Bit sizes and maximum values
#define wm821x_GAIN_BITS             (1 << 9)
#define WM821X_GAIN_MAX              (wm821x_GAIN_BITS - 1)
#define wm821x_SAMPLING_LIMIT        7600000 // Should be 8MSPS but proben to be less, After this value RSMP / VSMP has to be flipped

// Register structure
typedef struct wm821x_afe_setup_1 {
    io_rw_8  enable:1;
    io_rw_8  cds:1;      // Correlated Double Sampler
    io_rw_8  mono:1;
    io_rw_8  two_chan:1;
    io_rw_8  pgafs:2;    // Black Level Adjust (3= positive signals)
    io_rw_8  mode_4_legacy:1;
    io_rw_8  legacy:1;
} wm821x_afe_setup_1_t;

typedef struct wm821x_afe_setup_2 {
    io_rw_8  opp_form:2;    // x0-8bit multiplexed, 01-8 bit parallel, 11-4bits multiplex 
    io_rw_8  invop:1;       // Invert polarity of output data
    io_rw_8  opd:1;         // Output disable 
    io_rw_8  low_refs:1;    // Reduce the adc ref range
    io_rw_8  rlc_dac_rng:1; // Output range of the rlcdac (0 = AVdd, 1=VRT)
    io_rw_8  del:2;         // Data latency
} wm821x_afe_setup_2_t;

// Setup 3 is common

typedef struct wm821x_afe_setup_4 {
    io_rw_8  line_by_line:1;
    io_rw_8  acyc:1;
    io_rw_8  intm:2;
    io_rw_8  reserved:4; 
} wm821x_afe_setup_4_t;

typedef struct wm821x_afe_setup_5 {
    io_rw_8  red_pd:1;
    io_rw_8  green_pd:1;
    io_rw_8  blue_pd:1;
    io_rw_8  adc_pd:1;
    io_rw_8  vrlc_dac_pd:1; //0: VRLC defined by internal RLC DAC
    io_rw_8  adc_ref_pd:1;
    io_rw_8  vrx_pd:1;
    io_rw_8  reserved:1;  
} wm821x_afe_setup_5_t;

typedef struct wm821x_afe_setup_6 {
    io_rw_8  vsm_pdet:1;
    io_rw_8  vdel:3;
    io_rw_8  posn_neg:1;
    io_rw_8  rlc_en:1;
    io_rw_8  clamp_ctrl:1;
    io_rw_8  reserved:1;  
} wm821x_afe_setup_6_t;

// Afe offset dac is common

typedef struct wm821x_afe_pga_gain {
    io_rw_8      lsb:1;  // LSB Gain on the final V3
    io_rw_8      reserved:7;
    io_rw_8      msb;    // MSB Gain on the final V3
} wm821x_afe_pga_gain_t;

typedef struct wm821x_afe_pga_gain_rgb {
    wm821x_afe_pga_gain_t red;
    wm821x_afe_pga_gain_t green;
    wm821x_afe_pga_gain_t blue;
} wm821x_afe_pga_gain_rgb_t;

#endif