/**
 */

#include "pico/stdlib.h"
#include "rgbScan.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/structs/iobank0.h"
#include "hardware/sync.h"

#ifdef RGB_SCANNER_TIMING_INFO
#include "nanoSystick.h"
float         nanoSecPerTick;
#endif

uint _vsyncGPIO, _hsyncGPIO;
unsigned long tickVsync;
unsigned int  tickHsync;
unsigned int  hsyncCounter;

scanlineCallback rgbScannerScanlineCallback = NULL;
scanlineCallback rgbScannerDetectCallback = NULL;

volatile unsigned int     rgbScannerScanlineTriggerFrontPorch = 0;   
volatile unsigned int     rgbScannerScanlineTriggerlastLine   = 0; 

static inline void rgb_scanner_gpio_acknowledge_irq(uint gpio, uint32_t events) {
    iobank0_hw->intr[gpio / 8] = events << 4 * (gpio % 8);
}

static void __not_in_flash_func(rgb_scanner_gpio_irq_handler)(void) {
    io_irq_ctrl_hw_t *irq_ctrl_base = get_core_num() ? &iobank0_hw->proc1_irq_ctrl : &iobank0_hw->proc0_irq_ctrl;

    io_ro_32 *status_reg_hsync = &irq_ctrl_base->ints[_hsyncGPIO / 8];
    uint events_hsync = (*status_reg_hsync >> 4 * (_hsyncGPIO % 8)) & 0xf;
    if (events_hsync) {
        rgb_scanner_gpio_acknowledge_irq(_hsyncGPIO, events_hsync);

        hsyncCounter++;
        if (hsyncCounter < rgbScannerScanlineTriggerFrontPorch) {
            if (rgbScannerDetectCallback) {
                rgbScannerDetectCallback(hsyncCounter);
            }
        } else if (hsyncCounter < rgbScannerScanlineTriggerlastLine) {
            rgbScannerScanlineCallback(hsyncCounter - rgbScannerScanlineTriggerFrontPorch);
        }
    }
    io_ro_32 *status_reg_vsync = &irq_ctrl_base->ints[_vsyncGPIO / 8];
    uint events_vsync = (*status_reg_vsync >> 4 * (_vsyncGPIO % 8)) & 0xf;
    if (events_vsync) {
        rgb_scanner_gpio_acknowledge_irq(_vsyncGPIO, events_vsync);
        #ifdef RGB_SCANNER_TIMING_INFO
        tickVsync = systick_mark(false);
        if (hsyncCounter > 0) {
            tickHsync = (unsigned int)(tickVsync / hsyncCounter);
        }
        #endif
        hsyncCounter = 0;
    }    
}

int rgbScannerSetup(uint vsyncGPIO, uint hsyncGPIO, uint frontPorch, uint height, scanlineCallback videoCallback, scanlineCallback detectCallback) {
    if (videoCallback == NULL) {
        return 1;
    }
    rgbScannerDetectCallback = detectCallback;
    rgbScannerScanlineCallback = videoCallback;
    rgbScannerUpdateData(frontPorch, height);
    _vsyncGPIO = vsyncGPIO;
    _hsyncGPIO = hsyncGPIO;
    gpio_init(_vsyncGPIO);
    gpio_set_dir(_vsyncGPIO, GPIO_IN);
    gpio_pull_up(_vsyncGPIO);
    gpio_init(hsyncGPIO);
    gpio_set_dir(hsyncGPIO, GPIO_IN);
    gpio_pull_up(hsyncGPIO);

    rgbScannerEnable(true);
    irq_set_exclusive_handler(IO_IRQ_BANK0, rgb_scanner_gpio_irq_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);
    
    #ifdef RGB_SCANNER_TIMING_INFO
    nanoSecPerTick = 1000000000.0f / (float)clock_get_hz(clk_sys);
    systick_setup(false);
    systick_start(false, 0xFFFFFF);
    #endif
    
    //Set the highest priority to the GPIO
    irq_set_priority(IO_IRQ_BANK0, 0);
    
    return 0;
}

unsigned long rgbScannerGetVsyncNanoSec() {
    #ifdef RGB_SCANNER_TIMING_INFO
    return tickVsync * nanoSecPerTick;
    #else
    return 0;
    #endif
}

unsigned int rgbScannerGetHsyncNanoSec() {
    #ifdef RGB_SCANNER_TIMING_INFO
    return tickHsync * nanoSecPerTick;
    #else
    return 0;
    #endif
}

static bool rgbScannerEnabled = false;
void rgbScannerEnable(bool value) {
    gpio_set_irq_enabled(_vsyncGPIO,  GPIO_IRQ_EDGE_FALL, value);
    gpio_set_irq_enabled(_hsyncGPIO,  GPIO_IRQ_EDGE_RISE, value);
    rgbScannerEnabled = value;
}

void rgbScannerUpdateData(uint frontPorch, uint height) {
    bool wasRgbScannerEnabled = rgbScannerEnabled;
    rgbScannerEnable(false);
    if (height == 0) {
        height = rgbScannerScanlineTriggerlastLine - rgbScannerScanlineTriggerFrontPorch;
    }

    rgbScannerScanlineTriggerFrontPorch = frontPorch;
    rgbScannerScanlineTriggerlastLine = frontPorch + height;
    rgbScannerEnable(wasRgbScannerEnabled);
}