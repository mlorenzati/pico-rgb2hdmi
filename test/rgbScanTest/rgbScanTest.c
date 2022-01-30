/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "common_configs.h"
#include "rgbScan.h"

int main() {
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    rgbScannerSetup(RGB_SCAN_VSYNC_PIN, RGB_SCAN_HSYNC_PIN);

    while (true) {
        gpio_put(LED_PIN, 1);
        printf("Vysnc=%ld, Hsync=%ld\n", rgbScannerGetVsyncNanoSec(), rgbScannerGetHsyncNanoSec());
        sleep_ms(1000);
        gpio_put(LED_PIN, 0);
        sleep_ms(1000);
    }
}