#ifndef _COMMON_CONFIGS_H
#define _COMMON_CONFIGS_H

//Keyboard
#ifdef _KEYBOARD_H
    #define KEYBOARD_N_PINS          3
    #define KEYBOARD_PIN_UP          0
    #define KEYBOARD_PIN_DOWN        1
    #define KEYBOARD_PIN_ACTION      2
    #define KEYBOARD_REFRESH_RATE_MS 25
    #define KEYBOARD_REPEAT_RATE_MS  2000
#endif

#ifdef _MENU_H
    #define MENU_VIDEO_OVERLAY_WIDTH   -200
    #define MENU_VIDEO_OVERLAY_HEIGHT  -104
#endif

//SYNC edge triggered Pins
#ifdef _RGB_SCANNER_H
    #define RGB_SCAN_VSYNC_PIN  26
    #define RGB_SCAN_HSYNC_PIN  27
#endif

//VIDEO Timing
#define V_FRONT_PORCH   42
#define V_BACK_PORCH    54
#define REFRESH_RATE    50
#if DVI_SYMBOLS_PER_WORD == 2
    #define HSYNC_FRONT_PORCH   50
    #define HSYNC_BACK_PORCH    20
#else
    #define HSYNC_FRONT_PORCH   100
    #define HSYNC_BACK_PORCH    50
#endif 

//AFE (Analog Front End) Specific Config
#ifdef _WM8213_AFE_H
    //AFE SPI Pins
    #define AFE_SDO  16
    #define AFE_CS   17
    #define AFE_SCK  18
    #define AFE_SDI  19

    //AFE Paralel port pins
    #define AFE_OP        11 //Takes 6 pins starting from the beforementioned number
    #define AFE_VSMP    20 //Video Sample timing pulse
    #define AFE_RSMP    21 //Reset sample timing pulse
    #define AFE_MCLK    22 //Master ADC Clock

    //VSMP ___________________|‾|___________________________|‾|____________
    //RSMP ____|‾|___________________________|‾|___________________________
    //MCLK ____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____|‾‾‾‾|____
    //OP   ===>.<===R===>.<===G===>.<===B===>.<===R===>.<===G===>.<===B===>

    #define AFE_PGA_GAIN_RGB          61
    #define AFE_PGA_GAIN_RGB_SOG      75
    #define AFE_RLC_DAC_NEG           1
	#define AFE_OFFSET_DAC            0
	#define AFE_OFFSET_DAC_SOG_RED    230
    #define AFE_OFFSET_DAC_SOG_GREEN  230/6
	#define AFE_OFFSET_DAC_SOG_BLUE   230

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
                .rlc_dac = AFE_RLC_DAC_NEG,
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
            },
            // .offset_dac = {
            //     .red = AFE_OFFSET_DAC,
			// 	.green = AFE_OFFSET_DAC,         // Common config
			// 	//.green = (1 * AFE_OFFSET_DAC) / 5, // SOG config
            //     .blue = AFE_OFFSET_DAC
            // },
            // .pga_gain = {
            //     .red = {
            //         .lsb =  AFE_PGA_GAIN_RGB & 0x01,
            //         .msb = (AFE_PGA_GAIN_RGB >> 1) & 0xFF
            //     },
            //     .green = {
            //         .lsb =  AFE_PGA_GAIN_RGB & 0x01,
            //         .msb = (AFE_PGA_GAIN_RGB >> 1) & 0xFF
            //     },
            //     .blue = {
            //         .lsb =  AFE_PGA_GAIN_RGB & 0x01,
            //         .msb = (AFE_PGA_GAIN_RGB >> 1) & 0xFF
            //     }
            // }
        },
        .verify_retries = 3,
        .pio = pio1,
        .sm_afe_cp = 0,
        .pin_base_afe_op = AFE_OP,
        .pin_base_afe_ctrl = AFE_VSMP,
        #if DVI_SYMBOLS_PER_WORD == 2
        .bppx = rgb_16_565
        #else
        .bppx = rgb_8_332
        #endif
    };
#endif

//DVI Specific configs
#ifdef _DVI_SERIALISER_H
    //DVI Config
    #define DVI_DEFAULT_SERIAL_CONFIG     picodvi_dvi_cfg
    #define DVI_DEFAULT_PIO_INST         pio0

    static const struct dvi_serialiser_cfg picodvi_dvi_cfg = {
        .pio = pio0,
        .sm_tmds = {0, 1, 2},
        .pins_tmds = {5, 7, 9},
        .pins_clk = 3,
        .invert_diffpairs = true
    };
#endif

#if DVI_SYMBOLS_PER_WORD == 2
    // Colors                0brrrrrggggggbbbbb
    #define color_black      0b0000000000000000
    #define color_dark_gray  0b0001100011100011
    #define color_mid_gray   0b1010010100010000
    #define color_light_gray 0b1100011000011000
    #define color_white      0b1111111111111111
    #define color_red        0b1111100000000000
    #define color_green      0b0000011111100000
    #define color_blue       0b0000000000011111
#else
    // Colors                0brrrgggbb
    #define color_black      0b00000000
    #define color_dark_gray  0b01001001
    #define color_mid_gray   0b10110110
    #define color_light_gray 0b11011011
    #define color_white      0b11111111
    #define color_red        0b11100000
    #define color_green      0b00011100
    #define color_blue       0b00000011
#endif

// Global settings 
#ifdef _SETTINGS_H
    // Factory Settings
    #define GET_FACTORY_SETTINGS() { \
            .security_key = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13 },\
            .menu_colors = { color_dark_gray, color_light_gray, color_white, color_black, color_mid_gray, color_green}, \
            .flags.auto_shut_down = 1, \
            .flags.default_display = 0, \
            .flags.scan_line = 0, \
            .displays = {{ \
                    .gain = { .red = AFE_PGA_GAIN_RGB, .green = AFE_PGA_GAIN_RGB, .blue = AFE_PGA_GAIN_RGB}, \
                    .offset = { .red = AFE_OFFSET_DAC, .green = AFE_OFFSET_DAC,   .blue = AFE_OFFSET_DAC}, \
                    .v_front_porch = V_FRONT_PORCH, .v_back_porch = V_BACK_PORCH, \
                    .h_front_porch = HSYNC_FRONT_PORCH, .h_back_porch = HSYNC_BACK_PORCH, \
                    .refresh_rate = REFRESH_RATE \
                }, { \
                    .gain = { .red = AFE_PGA_GAIN_RGB, .green = AFE_PGA_GAIN_RGB, .blue = AFE_PGA_GAIN_RGB}, \
                    .offset = { .red = AFE_OFFSET_DAC, .green = AFE_OFFSET_DAC,   .blue = AFE_OFFSET_DAC}, \
                    .v_front_porch = V_FRONT_PORCH, .v_back_porch = V_BACK_PORCH, \
                    .h_front_porch = HSYNC_FRONT_PORCH, .h_back_porch = HSYNC_BACK_PORCH, \
                    .refresh_rate = REFRESH_RATE \
                }, { \
                    .gain = { .red = AFE_PGA_GAIN_RGB, .green = AFE_PGA_GAIN_RGB, .blue = AFE_PGA_GAIN_RGB}, \
                    .offset = { .red = AFE_OFFSET_DAC, .green = AFE_OFFSET_DAC,   .blue = AFE_OFFSET_DAC}, \
                    .v_front_porch = V_FRONT_PORCH, .v_back_porch = V_BACK_PORCH, \
                    .h_front_porch = HSYNC_FRONT_PORCH, .h_back_porch = HSYNC_BACK_PORCH, \
                    .refresh_rate = REFRESH_RATE \
                }, { \
                    .gain = { .red = AFE_PGA_GAIN_RGB_SOG, .green = AFE_PGA_GAIN_RGB_SOG, .blue = AFE_PGA_GAIN_RGB_SOG}, \
                    .offset = { .red = AFE_OFFSET_DAC_SOG_RED, .green = AFE_OFFSET_DAC_SOG_GREEN, .blue = AFE_OFFSET_DAC_SOG_BLUE}, \
                    .v_front_porch = V_FRONT_PORCH, .v_back_porch = V_BACK_PORCH, \
                    .h_front_porch = HSYNC_FRONT_PORCH, .h_back_porch = HSYNC_BACK_PORCH, \
                    .refresh_rate = REFRESH_RATE \
                } \
            }, \
            .eof_canary = 0 \
        }
    #endif
#endif