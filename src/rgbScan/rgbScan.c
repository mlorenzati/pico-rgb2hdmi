/**
 */

#include "pico/stdlib.h"
#include "rgbScan.h"
#include "nanoSystick.h"
#include "hardware/clocks.h"

uint _vsyncGPIO, _hsyncGPIO;
unsigned long timeVsync;
unsigned int  timeHsync;
unsigned int  hsyncCounter;
unsigned int  nanoSecPerTick;

scanlineCallback rgbScannerScanlineCallback = NULL;
unsigned int     rgbScannerScanlineTriggerFrontPorch = 10000;   
unsigned int     rgbScannerScanlineTriggerBackPorch = 10000; 

static inline void rgbScannerIrqCallback(uint gpio, uint32_t events) {
    if (gpio == _vsyncGPIO) {
        timeVsync = systick_mark(false) * nanoSecPerTick;
        if (hsyncCounter > 0) {
            timeHsync = (unsigned int)(timeVsync / hsyncCounter);
        } 
        hsyncCounter = 0;
    } else if (gpio == _hsyncGPIO) {
        hsyncCounter++;
        if (hsyncCounter >= rgbScannerScanlineTriggerFrontPorch && hsyncCounter <= rgbScannerScanlineTriggerBackPorch) {
            rgbScannerScanlineCallback(hsyncCounter);
        };
    }   
}

int rgbScannerSetup(uint vsyncGPIO, uint hsyncGPIO, uint frontPorch, uint backPorch, scanlineCallback callback) {
    if (callback == NULL) {
        return 1;
    }
    rgbScannerScanlineCallback = callback;
    rgbScannerScanlineTriggerFrontPorch = frontPorch;
    rgbScannerScanlineTriggerBackPorch = backPorch;
    _vsyncGPIO = vsyncGPIO;
    _hsyncGPIO = hsyncGPIO;
    gpio_init(_vsyncGPIO);
    gpio_set_dir(_vsyncGPIO, GPIO_IN);
    gpio_is_pulled_up(_vsyncGPIO);
    gpio_init(hsyncGPIO);
    gpio_set_dir(hsyncGPIO, GPIO_IN);
    gpio_is_pulled_up(hsyncGPIO);

    gpio_set_irq_enabled_with_callback(_vsyncGPIO,  GPIO_IRQ_EDGE_FALL, true, &rgbScannerIrqCallback);
    gpio_set_irq_enabled_with_callback(_hsyncGPIO,  GPIO_IRQ_EDGE_FALL, true, &rgbScannerIrqCallback);
    nanoSecPerTick = 1000000000 / clock_get_hz(clk_sys);
    systick_setup(false);
    systick_start(false, 0xFFFFFF);
    return 0;
}

unsigned long rgbScannerGetVsyncNanoSec() {
    return timeVsync;
}

unsigned int rgbScannerGetHsyncNanoSec() {
    return timeHsync;
}