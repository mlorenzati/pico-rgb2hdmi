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
#include "rgbScan.h"
#include "wm8213Afe.h"
#include "videoAdjust.h"
#include "overlay.h"
#include "graphics.h"

#include "menu.h"
#include "cmdParser.h"
#include "settings.h"
#include "commands.h"
#include "storage.h"

//System configuration includes
#include "common_configs.h"
#include "system.h"

// System config definitions
#define FRAME_HEIGHT     240
#if DVI_SYMBOLS_PER_WORD == 2
    //With 2 repeated symbols per word, we go for 320 pixels width and 16 bits per pixel
    #define FRAME_WIDTH         320
    uint16_t            framebuf[FRAME_HEIGHT][FRAME_WIDTH];
#else
    //With no repeated symbols per word, we go for 640 pixels width and 8 bits per pixel
    #define FRAME_WIDTH     640
    uint8_t            framebuf[FRAME_HEIGHT][FRAME_WIDTH];
#endif 

#define VREG_VSEL       VREG_VOLTAGE_1_20
#define DVI_TIMING      dvi_timing_640x480p_60hz

// --------- Global register start --------- 
struct dvi_inst     dvi0;
static uint         hdmi_scanline = 2;
const uint          LED_PIN       = PICO_DEFAULT_LED_PIN;
uint                keyboard_gpio_pins[KEYBOARD_N_PINS] = { KEYBOARD_PIN_UP, KEYBOARD_PIN_DOWN, KEYBOARD_PIN_ACTION };
menu_event_type     menu_event_map[KEYBOARD_N_PINS]     = { menu_event_next, menu_event_previous, menu_event_action };
const settings_t factory_settings = GET_FACTORY_SETTINGS();
cmd_parser_option_t options[] =
{
    {"up",      TRUE,  NULL,  'u'},
    {"down",    TRUE,  NULL,  'd'},
    {"left",    TRUE,  NULL,  'l'},
    {"mode",    FALSE, NULL,  'm'},
    {"right",   TRUE,  NULL,  'r'},
    {"info",    TRUE,  NULL,  'i'},
    {"capture", FALSE, NULL,  'c'},
    {"id",      FALSE, NULL,  'I'},
    {"version", FALSE, NULL,  'v'},
#ifdef TEST_MODE
    {"keyset",  TRUE,  NULL,  'k'},
    {"keyget",  FALSE, NULL,  'K'},
#endif
    {"reboot",  FALSE, NULL,  'R'},
    {"display", TRUE,  NULL,  'D'},
    {NULL,      TRUE, NULL,    0 }
};
// ----------- Global register end ----------- 

// *********** IRQ API CALL END   ************
// ----------- DVI API CALL START ------------ 
void __not_in_flash_func(core1_main)() {
    storage_report_core1_use();
    dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
    dvi_start(&dvi0);
    #if DVI_SYMBOLS_PER_WORD == 2
        dvi_scanbuf_main_16bpp(&dvi0);
    #else
        dvi_scanbuf_main_8bpp(&dvi0);
    #endif
    
    __builtin_unreachable();
}

static inline void core1_scanline_callback() {
    #if DVI_SYMBOLS_PER_WORD == 2
        uint16_t *bufptr = NULL;
    #else
        uint8_t *bufptr  = NULL;
    #endif
    while (queue_try_remove_u32(&dvi0.q_colour_free, &bufptr));
    
    bufptr = &framebuf[hdmi_scanline][0];
    queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
    if (++hdmi_scanline >= FRAME_HEIGHT) {
        hdmi_scanline = 0;
    }
}
// ---------  DVI API CALL END  --------- 

// ---------  RGB SCAN API CALL START  --------- 
static void __not_in_flash_func(scanLineTriggered)(unsigned int render_line_number) {
    gpio_put(LED_PIN, true);
    if (wm8213_afe_capture_run(VIDEO_OVERLAY_GET_COMPUTED_FRONT_PORCH(), (uintptr_t)&framebuf[render_line_number][VIDEO_OVERLAY_GET_COMPUTED_OFFSET()], VIDEO_OVERLAY_GET_COMPUTED_WIDTH())) {
        //Nothing is done here so far
    }
    video_overlay_scanline_prepare(render_line_number);
    gpio_put(LED_PIN, false);
}
// ---------  RGB SCAN API CALL END  --------- 

// ***********  IRQ API CALL END  ************

void parse_command(int argc, char *const argv[]) {
    int option_result = 0;
    int option_index = 0;
    int arg_index = 0;
    char *strValue = NULL;
    while ((option_result = cmd_parser_get_cmd(argc, argv, options, &option_index, &arg_index)) != -1) {
        strValue = options[option_index].strval;
        printf ("Request %s<%c>(%s)\n", options[option_index].name, option_result, strValue);

        if (command_on_receive(option_result, strValue, true) > 0) {
            printf("Unknown command: "); cmd_parser_print_cmd(options);
        }
    }
}

void command_line_loop() {
    char inputStr[48];
    char *argv[6];
    int argc;
    while (1)
    {
        gets(inputStr);
        cmd_parser_get_argv_argc(inputStr, &argc, argv);
        parse_command(argc, argv);
    }
}

int main() {
    vreg_set_voltage(VREG_VSEL);
    sleep_ms(10);

    // Run system at TMDS bit clock
    set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);
    #ifdef RGB2HDMI_DEBUG
        //system_delayed_write_disable();
    #endif

    // Validate license prior starting second core
    if (settings_initialize(&factory_settings) > 0) {
        printf("storage initialize failed \n");
    };
    command_validate_license(settings_get()->security_key);

    // Configure scan video properties
    display_t *current_display = &(settings_get()->displays[settings_get()->flags.default_display]);
    set_video_props(current_display->v_front_porch, current_display->v_back_porch, current_display->h_front_porch, current_display->h_back_porch, FRAME_WIDTH, FRAME_HEIGHT, current_display->refresh_rate, framebuf);
    
    // Do early init of config and update Gain & offset from stored settings
    wm8213_afe_init(&afec_cfg);
    wm8213_afe_update_offset(current_display->offset.red, current_display->offset.green, current_display->offset.blue, false);
    wm8213_afe_update_gain(current_display->gain.red, current_display->gain.green, current_display->gain.blue, false);

    // Configure AFE Capture System from afe config local
    command_info_afe_error = wm8213_afe_setup(NULL, GET_VIDEO_PROPS().sampling_rate);
    if ( command_info_afe_error > 0) {
         printf("AFE initialize failed with error %d\n", command_info_afe_error);
    } else {
         printf("AFE initialize succeded \n");
    }

    // Menu System Initialize
    menu_initialize(keyboard_gpio_pins, menu_event_map, KEYBOARD_N_PINS);

    // Initialize DVI
    dvi0.timing = &DVI_TIMING;
    dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
    dvi0.scanline_callback = core1_scanline_callback;
    dvi0.scan_line = settings_get()->flags.scan_line;
    dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

    // Prepare DVI for the first time the two initial lines, passing core 1 the framebuffer
    // Start the Core1, dedicated for DVI
    #if DVI_SYMBOLS_PER_WORD == 2
        uint16_t *bufptr = GET_RGB16_BUFFER(GET_VIDEO_PROPS().video_buffer);
    #else
        uint8_t  *bufptr = GET_RGB8_BUFFER(GET_VIDEO_PROPS().video_buffer);
    #endif
    
    queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
    bufptr += FRAME_WIDTH;
    queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
    multicore_launch_core1(core1_main);

    // Initializing RGBSCAN and the leds to indicate it's activity
    sleep_ms(10);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT); 
    if (command_info_afe_error == 0) {
        io_rw_16 final_height = GET_VIDEO_PROPS().height;
        command_info_scanner_error = rgbScannerSetup(
            RGB_SCAN_VSYNC_PIN, RGB_SCAN_HSYNC_PIN, GET_VIDEO_PROPS().vertical_front_porch, final_height, scanLineTriggered, menu_video_signal_callback, NULL);
        if (command_info_scanner_error > 0) {
            printf("rgbScannerSetup failed with code %d\n", command_info_scanner_error);
        }
    }

    // Prepare first start image
    command_show_info(true);

    // Wait some seconds with visual report
    for (int i = 5; i > 0; i--) {
        sleep_ms(1000);
        printf("%d\n", i);
    }

    // Remove info screen if license is valid
    command_show_info(!command_is_license_valid());

    // Show Version
    command_on_receive('v', NULL, false);

    // Main Busy Loop
    command_line_loop();

    __builtin_unreachable();
}    
