/**
 */

#include "pico/stdlib.h"
#include "rgbScan.h"

uint _vsyncGPIO, _hsyncGPIO;
uint64_t timestampIrq, timestampLast;
unsigned long long  timeVsync, timeHsync;
unsigned long hsyncCounter; 


void rgbScannerIrqCallback(uint gpio, uint32_t events) {
    if (gpio == _vsyncGPIO) {
        timestampLast = time_us_64();
        timeVsync = (timestampLast - timestampIrq) * 1000;
        if (hsyncCounter > 0) {
            timeHsync = timeVsync / hsyncCounter;
        } 
        timestampIrq = timestampLast;
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

    return 0;
}

unsigned long rgbScannerGetVsyncNanoSec() {
    return timeVsync;
}

unsigned long rgbScannerGetHsyncNanoSec() {
    return timeHsync;
}