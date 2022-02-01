/*
*/

#ifndef _RGB_SCANNER_H
#define _RGB_SCANNER_H
int rgbScannerSetup(uint VSYNC_GPIO, uint HSYNC_GPIO);
unsigned long rgbScannerGetVsyncNanoSec();
unsigned int rgbScannerGetHsyncNanoSec();

#endif//_RGB_SCANNER_H