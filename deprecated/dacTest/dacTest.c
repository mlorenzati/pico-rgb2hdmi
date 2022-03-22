/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "common_configs.h"
#include "fastADC.h"

#define TEST_ADC_SAMPLING_FREQ 5000000
void core1_main();


// A simple test to measure how the ADC works in ramp compare mode
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

struct repeating_timer timer;
volatile uint32_t adc_value;
volatile uint32_t adc_dispersion;
volatile uint32_t adc_sps=0;

bool repeating_timer_callback(struct repeating_timer *t) {
    printf("Measured Value: %u with %u sps and %d dispersion\n", adc_value, adc_sps, adc_dispersion);
    adc_sps = 0;
    return true;
}

void core1_main() {
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio0, true);
    int offset = pio_add_program(pio0, &ramp_aquire_program);
    
    ramp_aquire_program_init(pio0, sm, offset, TEST_ADC_SAMPLING_FREQ, FAST_DAC_PIN_BASE, ADC_COMPARATOR_RED);
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);

    while (true) {
        uint32_t value = pio_sm_get_blocking(pio, sm);
        adc_dispersion = ((value > adc_value ? (value - adc_value) : adc_value - value) + adc_dispersion) / 2;
        adc_value = value;
        adc_sps++;
    }
}