#include "nanoSystick.h"

systick_csr systick_control = (systick_csr) &(systick_hw->csr);
systick_rvr systick_reload  = (systick_rvr) &(systick_hw->rvr);
systick_cvr systick_current = (systick_cvr) &(systick_hw->rvr);
uint32_t nanoSystick_timestampLast;

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
    nanoSystick_timestampLast = systick_current->current;
    return 0;
}

uint32_t systick_mark(bool stop) {
    uint32_t current = systick_current->current;
    uint32_t delta = nanoSystick_timestampLast - current;
    nanoSystick_timestampLast = current;
    if (stop) {
        systick_control->enable = 0; 
    }

    if (systick_control->countflag) {
        //Countdown counter underflow
        delta = 0;
    }

    return delta;
}