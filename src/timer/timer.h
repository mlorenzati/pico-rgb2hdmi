#ifndef _TIMER_H
#define _TIMER_H
#include "pico.h"
#include "pico/time.h"
#include "hardware/timer.h"

#if PICO_TIME_DEFAULT_ALARM_POOL_DISABLED
alarm_pool_t *alarm_pool_get_default(void);

static inline bool add_repeating_timer_ms(int32_t delay_ms, repeating_timer_callback_t callback, void *user_data, repeating_timer_t *out) {
    return alarm_pool_add_repeating_timer_us(alarm_pool_get_default(), delay_ms * (int64_t)1000, callback, user_data, out);
}
#endif
bool timer_alarm_pool_init_default();

#endif