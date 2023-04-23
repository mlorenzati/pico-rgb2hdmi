#ifndef _DVI_SERIALISER_H
#define _DVI_SERIALISER_H

#include "hardware/pio.h"
#include "dvi_config_defs.h"

#define N_TMDS_LANES 3

struct dvi_serialiser_cfg {
	PIO pio;
	uint sm_tmds[N_TMDS_LANES];
	uint pins_tmds[N_TMDS_LANES];
	uint pins_clk;
	bool invert_diffpairs;
	uint prog_offs;
    // By default, we assume each 32-bit word written to a PIO FIFO contains 2x
    // 10-bit TMDS symbols, concatenated into the lower 20 bits, least-significant
    // first. This is convenient if you are generating two or more pixels at once,
    // e.g. using the pixel-doubling TMDS encode. You can change this value to 1
    // (so each word contains 1 symbol) for e.g. full resolution RGB encode. Note
    // that this value needs to divide the DVI horizontal timings, so is limited
    // to 1 or 2 (0/1).
    bool symbols_per_word;
};

void dvi_serialiser_init(struct dvi_serialiser_cfg *cfg);
void dvi_serialiser_enable(struct dvi_serialiser_cfg *cfg, bool enable);
uint32_t dvi_single_to_diff(uint32_t in);

#endif
