/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "rgbScan.h"
#include "common_configs.h"

int frontPorch = 20;
int backPorch = 30;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;

void scanLineTriggered(unsigned int scanlineNumber) {
     gpio_put(LED_PIN, blink);
     blink = !blink;
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    int error;
    if (error = rgbScannerSetup(RGB_SCAN_VSYNC_PIN, RGB_SCAN_HSYNC_PIN, frontPorch, backPorch, &scanLineTriggered) > 0) {
        printf("rgbScannerSetup failed with code %d\n", error);
    }

    while (true) {
        printf("Vysnc=%ldnSec, %ldHz, Hsync=%dnSec, %dHz\n", rgbScannerGetVsyncNanoSec(), 1000000000 / rgbScannerGetVsyncNanoSec(), rgbScannerGetHsyncNanoSec(), 1000000000 / rgbScannerGetHsyncNanoSec());
        sleep_ms(1000);
    }
}