/*
*/

#ifndef _RGB_SCANNER_H
#define _RGB_SCANNER_H

typedef void (*scanlineCallback)(unsigned int render_line_number);

int rgbScannerSetup(uint vsyncGPIO, uint hsyncGPIO, uint frontPorch, uint height, scanlineCallback callback);
unsigned long rgbScannerGetVsyncNanoSec();
unsigned int rgbScannerGetHsyncNanoSec();
void         rgbScannerEnable(bool value);
void         rgbScannerUpdateData(uint frontPorch, uint height);

#endif//_RGB_SCANNER_H