/*
*/
#ifndef _NANO_SYSTICK_H
#define _NANO_SYSTICK_H

#include "hardware/structs/systick.h"
// Systick Info: https://developer.arm.com/documentation/dui0497/a/cortex-m0-peripherals/optional-system-timer--systick/systick-control-and-status-register

typedef struct {
    io_rw_32 enable:1;      // Enable SysTick counter
    io_rw_32 tickint:1;     // Enables SysTick exception request
    io_rw_32 clksource:1;   // 0: external, 1: internal
    io_rw_32 reserved:13;
    io_rw_32 countflag:1;   // Returns 1 if timer counted to 0 since last time this was read
} *systick_csr;

typedef struct {
    io_rw_32 reload:24;
} *systick_rvr;

typedef struct {
    io_rw_32 current:24;
} *systick_cvr;

typedef struct {
    io_rw_32 tenms:24;
    io_rw_32 reserved:6;
    io_rw_32 skew:1;
    io_rw_32 noref:1;
} *systick_calibration_r;

int systick_setup(bool useInterrupts);
int systick_start(bool wait, uint32_t ticks);
uint32_t systick_mark(bool stop);
uint32_t systick_get_current();

#endif//_NANO_SYSTICK_H
