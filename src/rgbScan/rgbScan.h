/*
*/

#ifndef _RGB_SCANNER_H
#define _RGB_SCANNER_H

#define RGB_SCANNER_CSYNC_TIMING    9000
#define RGB_SCANNER_CSYNC_CNT       5
#define RGB_SCANNER_SYNC_TYPE_TRIG  50
#define RGB_SCANNER_SYNC_TYPE_CNT   2 * RGB_SCANNER_SYNC_TYPE_TRIG
#define RGB_SCANNER_SHUTDOWN_CNT    RGB_SCANNER_SYNC_TYPE_TRIG
#define RGB_SCANNER_NO_SIGNAL_TICK  200

typedef enum {
	rgbscan_sync_none = 0,
    rgbscan_sync_c,
    rgbscan_sync_hv,
    rgbscan_sync_max
} rgbscan_sync_type;

typedef enum {
    rgbscan_signal_none = 0,
    rgbscan_signal_started,
    rgbscan_signal_stopped,
    rgbscan_signal_shutdown
} rgbscan_signal_event_type;

typedef void (*scanlineCallback)(unsigned int render_line_number);
typedef void (*rgbscanSyncSignalCallback)(rgbscan_signal_event_type event_type);

int rgbScannerSetup(uint vsyncGPIO, uint hsyncGPIO, uint frontPorch, uint height, scanlineCallback scanCallback, rgbscanSyncSignalCallback noSignalCallback, void *noSignalData);
unsigned long rgbScannerGetVsyncNanoSec();
unsigned int  rgbScannerGetHsyncNanoSec();
unsigned int  rgbScannerGetHorizontalLines();
void          rgbScannerEnable(bool value);
void          rgbScannerUpdateData(uint frontPorch, uint height);
rgbscan_sync_type rgbScannerGetSyncType();
const char*   rgbScannerGetSyncTypeStr();
void          rgbScannerWake();

#endif//_RGB_SCANNER_H