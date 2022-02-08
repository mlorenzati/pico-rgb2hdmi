#include "fastADC.h"

// Fast ADC is just a plain ramp comparator ADC, but since it paralelizes the capture of 3 channels by reusing
// the same comparator ramp and managing low resolution (4 bits) is faster in cicles than a SAR ADC due to the assembly
// logic complexity of the pio instructions.
// Thus it requires 4 SMs of the same PIO

