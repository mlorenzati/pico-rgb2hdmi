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


void rgbScannerIrqCallback(uint gpio, uint32_t events) {
    if (gpio == _vsyncGPIO) {
        timeVsync = systick_mark(false) * nanoSecPerTick;
        if (hsyncCounter > 0) {
            timeHsync = (unsigned int)(timeVsync / hsyncCounter);
        } 
        hsyncCounter = 0;
    } else if (gpio == _hsyncGPIO) {
        hsyncCounter++;
    }   
}

int rgbScannerSetup(uint vsyncGPIO, uint hsyncGPIO) {
    _vsyncGPIO = vsyncGPIO;
    _hsyncGPIO = hsyncGPIO;
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