#ifndef _COMMON_CONFIGS_H
#define _COMMON_CONFIGS_H

//SYNC edge triggered Pins
#ifdef _RGB_SCANNER_H
	#define RGB_SCAN_VSYNC_PIN  26
	#define RGB_SCAN_HSYNC_PIN  27
#endif 

//AFE (Analog Front End) Specific Config
#ifdef _WM8213_AFE_H
	//AFE SPI Pins
	#define AFE_SDO  16
	#define AFE_CS   17
	#define AFE_SCK  18
	#define AFE_SDI  19

	//AFE Paralel port pins
	#define AFE_OP		9  //Takes 8 pins starting from the beforementioned number
	#define AFE_VSMP	20 //Video Sample timing pulse
	#define AFE_RSMP	21 //Reset sample timing pulse
	#define AFE_MCLK	22 //Master ADC Clock

    //VSMP ___________________|‾|___________________________|‾|____________
	//RSMP ____|‾|___________________________|‾|___________________________
	//MCLK ____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____
	//OP   ===>.<===R===>.<===G===>.<===B===>.<===R===>.<===G===>.<===B===>

	#define AFE_SAMPLING_RATE 320*256*50

	static const wm8213_afe_config_t afec_cfg = {
		.spi = spi0,
		.baudrate = 1 * MHZ,
		.pins_spi = {AFE_SCK, AFE_SDI, AFE_SDO, AFE_CS},
		.setups = {
			.setup1 = {
				.enable = 1,
				.cds= 0,
    			.mono = 0,
				.two_chan = 0,
				.pgafs = 3,
				.mode_4_legacy = 0,
				.legacy = 0
			},
			.setup2 = {
				.opp_form = 1,
				.invop = 0,
				.opd = 0,
				.low_refs = 0,
				.rlc_dac_rng = 0,
				.del = 0
			},
			.setup3 = {
				.rlc_dac = 3,
				.cds_ref = 0,
				.chan = 0
			},
			.setup4 = {
				.line_by_line = 0,
				.acyc =  0,
				.intm = 0
			},
			.setup5 = {
				.red_pd = 0,
				.green_pd = 0,
				.blue_pd = 0,
				.adc_pd = 0,
				.vrlc_dac_pd = 0,
				.vrx_pd = 0
			},
			.setup6 = {
				.vsm_pdet = 0,
				.vdel = 0,
				.posn_neg = 0,
				.rlc_en = 0,
				.clamp_ctrl = 0
			}
		},
		.verify_retries = 3,
		.pio = pio1,
		.sm_afe_cp = 0,
		.pin_base_afe_op = AFE_OP,
		.pin_base_afe_ctrl = AFE_VSMP,
		.sampling_rate_afe = AFE_SAMPLING_RATE
	};
#endif

//DVI Specific configs
#ifdef _DVI_SERIALISER_H
	//DVI Config
	#define DVI_DEFAULT_SERIAL_CONFIG 	picodvi_dvi_cfg
	#define DVI_DEFAULT_PIO_INST 		pio0

	static const struct dvi_serialiser_cfg picodvi_dvi_cfg = {
		.pio = pio0,
		.sm_tmds = {0, 1, 2},
		.pins_tmds = {3, 5, 7},
		.pins_clk = 1,
		.invert_diffpairs = true
	};
#endif
#endif
