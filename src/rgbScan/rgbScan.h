/*
*/

#ifndef _RGB_SCANNER_H
#define _RGB_SCANNER_H
typedef void (*scanlineCallback)(void);

int rgbScannerSetup(uint vsyncGPIO, uint hsyncGPIO, uint scanlineTrigger, scanlineCallback callback);
unsigned long rgbScannerGetVsyncNanoSec();
unsigned int rgbScannerGetHsyncNanoSec();

#endif//_RGB_SCANNER_H