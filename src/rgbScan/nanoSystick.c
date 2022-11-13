#include "nanoSystick.h"

systick_csr systick_control = (systick_csr) &(systick_hw->csr);
systick_rvr systick_reload  = (systick_rvr) &(systick_hw->rvr);
systick_cvr systick_current = (systick_cvr) &(systick_hw->cvr);
uint32_t nanoSystick_timestampLast[NANO_SYSTICK_MAX_COUNTERS];

int systick_setup(bool useInterrupts) {
    systick_control->enable = 0; 
    systick_control->clksource = 1;
    systick_control->tickint = useInterrupts;
    return 0;
}

int systick_start(bool wait, uint32_t ticks) {
    if (ticks > 0xFFFFFF) {
        return 1;
    }
    systick_reload->reload = ticks;
    systick_current->current = 0;
    systick_control->enable = 1; 
    while(wait && systick_current->current != 0);
    
    uint32_t current = systick_current->current;
    for (unsigned char cnt = 0; cnt < NANO_SYSTICK_MAX_COUNTERS; cnt++) {
        nanoSystick_timestampLast[cnt] = current;
    }
    
    return 0;
}