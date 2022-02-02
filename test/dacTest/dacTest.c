/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "fastADC.h"
#include "common_configs.h"

#include "hardware/pio.h"

#define OUTPUT_FREQ_KHZ 5
void core1_main();

int main() {
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    multicore_launch_core1(core1_main);

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(1000);
        gpio_put(LED_PIN, 0);
        sleep_ms(1000);
    }
}

void core1_main() {
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio0, true);
    uint offset = pio_add_program(pio0, &resistor_dac_5bit_program);
    resistor_dac_5bit_program_init(pio0, sm, offset,
        OUTPUT_FREQ_KHZ * 1000 * 2 * (1 << FAST_DAC_BITS), FAST_DAC_PIN_BASE);
    while (true) {
        // Triangle wave
        for (int i = 0; i < (1 << FAST_DAC_BITS); ++i)
            pio_sm_put_blocking(pio, sm, i);
    }
}