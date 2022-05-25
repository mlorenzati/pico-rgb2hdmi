/**
 */

#include "pico/stdlib.h"
#include "rgbScan.h"
#include "nanoSystick.h"
#include "hardware/clocks.h"

uint _vsyncGPIO, _hsyncGPIO;
unsigned long tickVsync;
unsigned int  tickHsync;
unsigned int  hsyncCounter;
float         nanoSecPerTick;

scanlineCallback rgbScannerScanlineCallback = NULL;
unsigned int     rgbScannerScanlineTriggerFrontPorch = 10000;   
unsigned int     rgbScannerScanlineTriggerBackPorch = 10000; 

void __isr __not_in_flash_func(rgbScannerIrqCallback)(uint gpio, uint32_t events) {
     if (gpio == _hsyncGPIO) {
        hsyncCounter++;
        if (hsyncCounter >= rgbScannerScanlineTriggerFrontPorch && hsyncCounter <= rgbScannerScanlineTriggerBackPorch) {
            rgbScannerScanlineCallback(hsyncCounter - rgbScannerScanlineTriggerFrontPorch);
        };
    } else if (gpio == _vsyncGPIO) {
        tickVsync = systick_mark(false);
        if (hsyncCounter > 0) {
            tickHsync = (unsigned int)(tickVsync / hsyncCounter);
        } 
        hsyncCounter = 0;
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
    gpio_pull_up(_vsyncGPIO);
    gpio_init(hsyncGPIO);
    gpio_set_dir(hsyncGPIO, GPIO_IN);
    gpio_pull_up(hsyncGPIO);

    rgbScannerEnable(true);
    nanoSecPerTick = 1000000000.0f / (float)clock_get_hz(clk_sys);
    systick_setup(false);
    systick_start(false, 0xFFFFFF);
    return 0;
}

unsigned long rgbScannerGetVsyncNanoSec() {
    return tickVsync * nanoSecPerTick;
}

unsigned int rgbScannerGetHsyncNanoSec() {
    return tickHsync * nanoSecPerTick;
}

void rgbScannerEnable(bool value) {
    gpio_set_irq_enabled_with_callback(_vsyncGPIO,  GPIO_IRQ_EDGE_FALL, value, &rgbScannerIrqCallback);
    gpio_set_irq_enabled_with_callback(_hsyncGPIO,  GPIO_IRQ_EDGE_RISE, value, &rgbScannerIrqCallback);
}