#ifndef _WM819X_AFE_H
#define _WM819X_AFE_H

/* Wolfson AFE wm819x Registers */
#define WM819X_REG_SETUP_TOTAL       12      //Number of registers to setup on AFE 

// Setup registers
#define WM819X_REG_REV_NUM           0x07
#define WM819X_REG_SETUP5            0x08
#define wm819x_REG_SETUP6            0x09

// Bit sizes and maximum values
#define WM819X_GAIN_BITS             (1 << 8)
#define WM819X_GAIN_MAX              (WM819X_GAIN_BITS - 1)

// Register structure
typedef struct wm819x_afe_setup_1 {
    io_rw_8  enable:1;
    io_rw_8  cds:1;   // Correlated Double Sampler
    io_rw_8  mono:1;
    io_rw_8  selpd:1; // Selective Power down
    io_rw_8  pgafs:2; // Black Level Adjust (3= positive signals)
    io_rw_8  mode_4:1;
    io_rw_8  reserved:1;
} wm819x_afe_setup_1_t;

typedef struct wm819x_afe_setup_2 {
    io_rw_8  muxop:2;       // 1-4bits multiplex anything else is 8 bits multiplexed
    io_rw_8  invop:1;       // Invert polarity of output data
    io_rw_8  vrlcext:1;     // External VRLC vias (0)
    io_rw_8  reserved:1;    // Undocumented
    io_rw_8  rlc_dac_rng:1; // Output range of the rlcdac (0 = AVdd, 1=VRT)
    io_rw_8  del:2;         // Data latency
} wm819x_afe_setup_2_t;

// Setup 3 is common

typedef struct wm819x_afe_setup_4 {
    io_rw_8  line_by_line:1; // 1.Line by line 0.Normal
    io_rw_8  acycnrlc:1;
    io_rw_8  fme:1;          // Force Mux
    io_rw_8  rlcint:1;
    io_rw_8  intm:2;         // Color selection for internal mode
    io_rw_8  fm:2;
} wm819x_afe_setup_4_t;

typedef struct wm819x_afe_setup_5 {
    io_rw_8  vsmpdet:1;
    io_rw_8  vdel:2;         // VSMP delay
    io_rw_8  posneg:1;       // 0: negative edge on VSMP 1: positive
    io_rw_8  reserved:4;
} wm819x_afe_setup_5_t;

typedef struct wm819x_afe_setup_6 {
    io_rw_8  seldis:4;       // Selective power disable
    io_rw_8  reserved:4;  
} wm819x_afe_setup_6_t;

// Afe offset dac is common

typedef struct wm819x_afe_pga_gain_rgb {
    io_rw_8 red;
    io_rw_8 green;
    io_rw_8 blue;
} wm819x_afe_pga_gain_rgb_t;

#endif