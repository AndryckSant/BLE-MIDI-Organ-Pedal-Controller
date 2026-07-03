#ifndef ADS1115_H
#define ADS1115_H

#include <stdint.h>

void adc_init();
int16_t adc_read(uint8_t channel);

#endif // ADS1115_H