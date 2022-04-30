/*
*/

#ifndef _RGB_SCANNER_H
#define _RGB_SCANNER_H
typedef void (*scanlineCallback)(unsigned int render_line_number);

int rgbScannerSetup(uint vsyncGPIO, uint hsyncGPIO, uint frontPorch, uint backPorch, scanlineCallback callback);
unsigned long rgbScannerGetVsyncNanoSec();
unsigned int rgbScannerGetHsyncNanoSec();

#endif//_RGB_SCANNER_H