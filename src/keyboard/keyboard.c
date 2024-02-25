#include "keyboard.h"
#include "timer.h"
#include "pico/stdlib.h"

struct  repeating_timer keyboard_timer;
uint8_t keyboard_count = 0;

typedef struct key_status {
    uint    io_pin;
    uint8_t value:1;
    uint8_t count:7;
} key_status_t;

typedef union keyboard_status_cast {
    keyboard_status_t bit_status;
    uint8_t           int_status;
} keyboard_status_cast_t;

key_status_t keys_status[KEYBOARD_MAX_COUNT];
keyboard_callback global_keyboard_callback;
uint global_repeat_rate = 0;
uint global_repeat_rate_countdown = 0;

bool keyboard_timer_callback(struct repeating_timer *t) {
    //Scan each key
    bool key_changed = false;
    keyboard_status_cast_t keyb_status = { .int_status = 0};
    keyboard_status_cast_t keyb_repeat_status = { .int_status = 0};

    for (int i=0; i< keyboard_count; i++) {
        key_status_t *key_s = &keys_status[i];
        // Get current value of pin (pin low is pressed)
        bool status = !gpio_get(key_s->io_pin);
        // Compare last status to build hysteresis
        key_s->count = status ? 
            (key_s->count >= KEYBOARD_THRESHOLD ? KEYBOARD_THRESHOLD : key_s->count + 1) :
            (key_s->count == 0 ? 0 : key_s->count - 1);

        if (key_s->value != status) {
            // Key changed
            if ((status && (key_s->count == KEYBOARD_THRESHOLD)) || (!status && (key_s->count == 0))) {
                key_s->value = status;
                uint8_t shift = status ? i : i + KEYBOARD_MAX_COUNT;
                keyb_status.int_status |= 1 << shift;
                key_changed = true;
                if (status) {
                    global_repeat_rate_countdown = global_repeat_rate;
                }
            }
        }
        if (key_s->value) {
            keyb_repeat_status.int_status |= 1 << i;
        }
    }

    //Key repeated event
    if ((keyb_repeat_status.int_status > 0) && (global_repeat_rate_countdown > 0)) {
        if (--global_repeat_rate_countdown == 0) {
            global_repeat_rate_countdown = global_repeat_rate;
            keyb_status.int_status |= keyb_repeat_status.int_status;
            key_changed = true;
        }
    }
    
    if (key_changed) {
        //Current detected event
        global_keyboard_callback(keyb_status.bit_status);
    }

    return true;
}

int keyboard_initialize(uint *pins, uint8_t count, uint refresh_rate_ms, uint repeat_rate_ms, keyboard_callback callback) {
    if (count > KEYBOARD_MAX_COUNT) {
        // Max number of keys exceeded
        return 1;
    }

    keyboard_count = count;
    for (int i=0; i < keyboard_count; i++) {
        key_status_t *key_s = &keys_status[i];
        key_s->io_pin = pins[i];
        if (key_s->io_pin >= NUM_BANK0_GPIOS) {
            //GPIO pin is higher than the maximum of the board
            return 2;
        }
        key_s->count = 0;
        key_s->value = 0;
        gpio_init(key_s->io_pin);
        gpio_set_dir(key_s->io_pin, GPIO_IN);
        gpio_pull_up(key_s->io_pin);
    }

    global_keyboard_callback = callback;
    global_repeat_rate = repeat_rate_ms / refresh_rate_ms;
    global_repeat_rate_countdown = 0;

    if (!add_repeating_timer_ms(-refresh_rate_ms, keyboard_timer_callback, NULL, &keyboard_timer)) {
        // No more timer slots available
        return 1;
    }

    return 0;
}

bool keyboard_get_current_value(uint8_t index) {
    if (index >= keyboard_count) {
        return false;
    }

    return keys_status[index].value;
}

void keyboard_stop_service() {
     cancel_repeating_timer(&keyboard_timer);
}

bool keyboard_get_key_status(const keyboard_status_t *status, bool up_down, uint8_t key_id) {
    if (key_id >= KEYBOARD_MAX_COUNT) {
        return false;
    }
    keyboard_status_cast_t *status_cast = (keyboard_status_cast_t *) status;

    return (status_cast->int_status >> ((up_down ? KEYBOARD_MAX_COUNT : 0) + key_id)) & 0x1;
}