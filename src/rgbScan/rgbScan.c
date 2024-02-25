/**
 */

#include "pico/stdlib.h"
#include "rgbScan.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/structs/iobank0.h"
#include "hardware/sync.h"
#include "nanoSystick.h"
#include "timer.h"

uint32_t onEventTick, lastEventTick, lastVsyncTick;
float         nanoSecPerTick;
uint _vsyncGPIO, _hsyncGPIO;
unsigned long tickVsync;
unsigned int  tickHsync;
unsigned int  hsyncCounter, hsyncTotalLines;
unsigned int  tickCsyncPulse, hsyncRstValue;
bool          isHsyncLine, isVsync;
signed int    syncTypeCnt = RGB_SCANNER_SYNC_TYPE_TRIG;
int           shutdownCnt = 0;

scanlineCallback rgbScannerScanlineCallback = NULL;
volatile unsigned int     rgbScannerScanlineTriggerFrontPorch = 0;   
volatile unsigned int     rgbScannerScanlineTriggerlastLine   = 0;
const char *rgbsynctypeStr[rgbscan_sync_max] = { "none", "hvsync", "csync"};
struct  repeating_timer rgbScanner_timer;
rgbscanSyncSignalCallback rgbscanSyncNoSignalCallbackPtr = NULL;
rgbscan_signal_event_type rgbscan_signal_event_last = rgbscan_signal_none;

static inline void rgb_scanner_gpio_acknowledge_irq(uint gpio, uint32_t events) {
    iobank0_hw->intr[gpio / 8] = events << 4 * (gpio % 8);
}

static void __not_in_flash_func(rgb_scanner_gpio_irq_handler)(void) {
    io_irq_ctrl_hw_t *irq_ctrl_base = get_core_num() ? &iobank0_hw->proc1_irq_ctrl : &iobank0_hw->proc0_irq_ctrl;

    io_ro_32 *status_reg_hsync = &irq_ctrl_base->ints[_hsyncGPIO / 8];
    uint events_hsync = (*status_reg_hsync >> 4 * (_hsyncGPIO % 8)) & 0xf;
    if (events_hsync) {
        rgb_scanner_gpio_acknowledge_irq(_hsyncGPIO, events_hsync);
        if (events_hsync & GPIO_IRQ_EDGE_FALL) {
            onEventTick = systick_get_current();
            tickCsyncPulse = systick_delta(lastEventTick, onEventTick);
            isVsync = tickCsyncPulse && tickCsyncPulse <= RGB_SCANNER_CSYNC_TIMING;
            isHsyncLine = tickCsyncPulse > RGB_SCANNER_CSYNC_TIMING;
            if (isVsync) {
                hsyncRstValue = RGB_SCANNER_CSYNC_CNT;
                if (syncTypeCnt < RGB_SCANNER_SYNC_TYPE_CNT) {
                    syncTypeCnt++;
                }
            }
        } else {
            if (isHsyncLine) {
                if (hsyncCounter >= rgbScannerScanlineTriggerFrontPorch && hsyncCounter < rgbScannerScanlineTriggerlastLine) {
                    rgbScannerScanlineCallback(hsyncCounter - rgbScannerScanlineTriggerFrontPorch);
                };
                hsyncCounter++;
            }
        }
    }
    
    io_ro_32 *status_reg_vsync = &irq_ctrl_base->ints[_vsyncGPIO / 8];
    uint events_vsync = (*status_reg_vsync >> 4 * (_vsyncGPIO % 8)) & 0xf;
    if (events_vsync) {
        rgb_scanner_gpio_acknowledge_irq(_vsyncGPIO, events_vsync);
        isVsync = true;
        hsyncRstValue = 0;
        if (syncTypeCnt > -RGB_SCANNER_SYNC_TYPE_CNT) {
            syncTypeCnt--;
        }
    }

    if (isVsync) {
        if (hsyncCounter > 180) {
            tickVsync = systick_delta(lastVsyncTick, onEventTick);
            lastVsyncTick = onEventTick;
            tickHsync = (unsigned int)(tickVsync / (hsyncCounter - 1));
            hsyncTotalLines = hsyncCounter;
        }
        hsyncCounter = hsyncRstValue;
    }
    lastEventTick = onEventTick;  
}


bool rgbScanner_timer_callback(struct repeating_timer *t) {
    bool shouldTrigger = false;
    rgbscan_signal_event_type newEvent = rgbscan_signal_none;
    
    if (syncTypeCnt > 0) {
        shutdownCnt = 0;
        syncTypeCnt--;
        shouldTrigger = true;
    } else if (syncTypeCnt < 0) {
        shutdownCnt = 0;
        syncTypeCnt++;
        shouldTrigger = true;
    } else if (shutdownCnt++ >= RGB_SCANNER_SHUTDOWN_CNT) {
        shutdownCnt = RGB_SCANNER_SHUTDOWN_CNT;
        newEvent = rgbscan_signal_shutdown;
        shouldTrigger = true;
    }
    if (rgbscanSyncNoSignalCallbackPtr != NULL && shouldTrigger) {
        if (syncTypeCnt == 0 && newEvent != rgbscan_signal_shutdown) {
            newEvent = rgbscan_signal_stopped;
        } else if (syncTypeCnt >= RGB_SCANNER_SYNC_TYPE_TRIG || syncTypeCnt <= -RGB_SCANNER_SYNC_TYPE_TRIG) {
            newEvent = rgbscan_signal_started;
        }

        if (newEvent != rgbscan_signal_event_last) {
            rgbscan_signal_event_last = newEvent;
            rgbscanSyncNoSignalCallbackPtr(newEvent);
        }
    }
    return true;
}

int rgbScannerSetup(uint vsyncGPIO, uint hsyncGPIO, uint frontPorch, uint height, scanlineCallback scanCallback, rgbscanSyncSignalCallback signalCallback, void *noSignalData) {
    if (scanCallback == NULL) {
        return 1;
    }
    //blindly stopping the timer if the api is called consecutively
    cancel_repeating_timer(&rgbScanner_timer);

    rgbScannerScanlineCallback = scanCallback;
    rgbScannerUpdateData(frontPorch, height);
    _vsyncGPIO = vsyncGPIO;
    _hsyncGPIO = hsyncGPIO;
    gpio_init(_vsyncGPIO);
    gpio_set_dir(_vsyncGPIO, GPIO_IN);
    gpio_pull_up(_vsyncGPIO);
    gpio_init(hsyncGPIO);
    gpio_set_dir(hsyncGPIO, GPIO_IN);
    gpio_pull_up(hsyncGPIO);

    rgbScannerEnable(true);
    irq_set_exclusive_handler(IO_IRQ_BANK0, rgb_scanner_gpio_irq_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);
    
    nanoSecPerTick = 1000000000.0f / (float)clock_get_hz(clk_sys);
    systick_setup(false);
    systick_start(false, 0xFFFFFF);
    
    //Set the highest priority to the GPIO
    irq_set_priority(IO_IRQ_BANK0, 0);

    rgbscanSyncNoSignalCallbackPtr = NULL;
    if (signalCallback != NULL) {
        rgbscanSyncNoSignalCallbackPtr = signalCallback;
        add_repeating_timer_ms(RGB_SCANNER_NO_SIGNAL_TICK, rgbScanner_timer_callback, NULL, &rgbScanner_timer);
    }
    
    return 0;
}

unsigned long rgbScannerGetVsyncNanoSec() {
    return tickVsync * nanoSecPerTick;
}

unsigned int rgbScannerGetHsyncNanoSec() {
    return tickHsync * nanoSecPerTick;
}

static bool rgbScannerEnabled = false;
void rgbScannerEnable(bool value) {
    gpio_set_irq_enabled(_vsyncGPIO,  GPIO_IRQ_EDGE_FALL, value);
    gpio_set_irq_enabled(_hsyncGPIO,  GPIO_IRQ_EDGE_RISE, value);
    gpio_set_irq_enabled(_hsyncGPIO,  GPIO_IRQ_EDGE_FALL, value);
    rgbScannerEnabled = value;
    if (!value) {
        busy_wait_us(500);
    }
}

void rgbScannerUpdateData(uint frontPorch, uint height) {
    bool wasRgbScannerEnabled = rgbScannerEnabled;
    rgbScannerEnable(false);
    if (height == 0) {
        height = rgbScannerScanlineTriggerlastLine - rgbScannerScanlineTriggerFrontPorch;
    }

    rgbScannerScanlineTriggerFrontPorch = frontPorch;
    rgbScannerScanlineTriggerlastLine = frontPorch + height;
    rgbScannerEnable(wasRgbScannerEnabled);
}

unsigned int rgbScannerGetHorizontalLines() {
    return hsyncTotalLines;
}

rgbscan_sync_type rgbScannerGetSyncType() {
    if (syncTypeCnt >= RGB_SCANNER_SYNC_TYPE_TRIG) {
        return rgbscan_sync_c;
    } else if (syncTypeCnt <= -RGB_SCANNER_SYNC_TYPE_TRIG) {
        return rgbscan_sync_hv;
    } else {
        return rgbscan_sync_none;
    }
}

const char* rgbScannerGetSyncTypeStr() {
    return rgbsynctypeStr[rgbScannerGetSyncType()];
}

void rgbScannerWake() {
    if (syncTypeCnt < RGB_SCANNER_SYNC_TYPE_TRIG || syncTypeCnt > -RGB_SCANNER_SYNC_TYPE_TRIG) {
        syncTypeCnt = RGB_SCANNER_SYNC_TYPE_CNT;
    }
}