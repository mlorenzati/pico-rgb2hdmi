/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "rgbScan.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"

//System configuration includes
#include "version.h"
#include "common_configs.h"

int frontPorch = 20;
int height = 100;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;

static void __not_in_flash_func(scanLineTriggered)(unsigned int render_line_number) {
     gpio_put(LED_PIN, blink);
     blink = !blink;
}

uint noSignaltestData = 10;
void signalCallback(rgbscan_signal_event_type type) {
    printf("video signal update received, type %d\n", type);
}

int main() {
    vreg_set_voltage(VREG_VOLTAGE_1_20);
	sleep_ms(10);
	set_sys_clock_khz(250000, true);

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    int error;
    if (error = rgbScannerSetup(RGB_SCAN_VSYNC_PIN, RGB_SCAN_HSYNC_PIN, frontPorch, height, &scanLineTriggered, signalCallback, &noSignaltestData) > 0) {
        printf("rgbScannerSetup failed with code %d\n", error);
    }

    printf("%s version - RGB Scan Test %s started!\n", PROJECT_NAME, PROJECT_VER);

    while (true) {
        printf("Current Clock=%ldhz, Vysnc=%ldnSec, %ldHz, Hsync=%dnSec, %dHz\n", clock_get_hz(clk_sys), rgbScannerGetVsyncNanoSec(), 1000000000 / rgbScannerGetVsyncNanoSec(), rgbScannerGetHsyncNanoSec(), 1000000000 / rgbScannerGetHsyncNanoSec());
        sleep_ms(1000);
    }
}