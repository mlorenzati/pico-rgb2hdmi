/*
*/

#ifndef _RGB_SCANNER_H
#define _RGB_SCANNER_H

#define RGB_SCANNER_CSYNC_TIMING    9000
#define RGB_SCANNER_CSYNC_CNT       5
#define RGB_SCANNER_SYNC_TYPE_CNT   200
#define RGB_SCANNER_SYNC_TYPE_TRIG  50

typedef enum {
	rgbscan_sync_none = 0,
    rgbscan_sync_c,
    rgbscan_sync_hv,
    rgbscan_sync_max
} rgbscan_sync_type;

typedef void (*scanlineCallback)(unsigned int render_line_number);

int rgbScannerSetup(uint vsyncGPIO, uint hsyncGPIO, uint frontPorch, uint height, scanlineCallback callback);
unsigned long rgbScannerGetVsyncNanoSec();
unsigned int rgbScannerGetHsyncNanoSec();
unsigned int rgbScannerGetHorizontalLines();
void         rgbScannerEnable(bool value);
void         rgbScannerUpdateData(uint frontPorch, uint height);
rgbscan_sync_type rgbScannerGetSyncType();
const char* rgbScannerGetSyncTypeStr();

#endif//_RGB_SCANNER_H