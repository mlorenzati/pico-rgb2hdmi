#ifndef _COMMON_CONFIGS_H
#define _COMMON_CONFIGS_H

// This file defines the TMDS pair layouts on a handful of boards I have been
// developing on. It's not a particularly important file -- just saves some
// copy + paste.

#include "dvi_serialiser.h"

#define RGB_SCAN_VSYNC_PIN  21
#define RGB_SCAN_HSYNC_PIN  22

#define DVI_DEFAULT_SERIAL_CONFIG 	picodvi_dvi_cfg
#define DVI_DEFAULT_PIO_INST 		pio0

static const struct dvi_serialiser_cfg picodvi_dvi_cfg = {
	.pio = DVI_DEFAULT_PIO_INST,
	.sm_tmds = {0, 1, 2},
	.pins_tmds = {10, 12, 14},
	.pins_clk = 8,
	.invert_diffpairs = true
};

#endif
