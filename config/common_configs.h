#ifndef _COMMON_CONFIGS_H
#define _COMMON_CONFIGS_H

//SYNC Pins edge triggered
#define RGB_SCAN_VSYNC_PIN  21
#define RGB_SCAN_HSYNC_PIN  22

//R2R DAC for the FAST SAR
#define FAST_DAC_PIN_BASE   16
#define FAST_DAC_BITS       5

//Comparator result pins
#define ADC_COMPARATOR_RED      26
#define ADC_COMPARATOR_GREEN    27
#define ADC_COMPARATOR_BLUE     28

#ifdef _DVI_SERIALISER_H
	//DVI Config
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
#endif
