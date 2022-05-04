/**
 *  Initial tests of the rgbScan api
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "keyboard.h"
#include "common_configs.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
bool blink = true;
uint gpio_pins[3] = { KEYBOARD_PIN_UP, KEYBOARD_PIN_DOWN, KEYBOARD_PIN_ACTION };

void on_keyboard_event(keyboard_status_t keys) {
    printf("Keyboard event received \n");
    if (keys.key1_down) {
        printf("Key 1 pressed \n");
    }
    if (keys.key2_down) {
        printf("Key 2 pressed \n");
    }
    if (keys.key3_down) {
        printf("Key 3 pressed \n");
    }
    if (keys.key1_up) {
        printf("Key 1 released \n");
    }
    if (keys.key2_up) {
        printf("Key 2 released \n");
    }
    if (keys.key3_up) {
        printf("Key 3 released \n");
    }
    gpio_put(LED_PIN, blink);
    blink = !blink;
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    keyboard_initialize(gpio_pins, 3, KEYBOARD_REFRESH_RATE_MS, KEYBOARD_REPEAT_RATE_MS, on_keyboard_event);

    while (true) {
        printf("Current polled value is key 1=%d, key2=%d, key3=%d\n", 
            keyboard_get_current_value(0), keyboard_get_current_value(1), keyboard_get_current_value(2));
        sleep_ms(5000);
    }
}