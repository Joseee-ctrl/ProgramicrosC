#ifndef ADC_H_
#define ADC_H_
// funciones del adc 
#include <stdint.h>

void ADC_init(void);
uint16_t ADC_read(uint8_t canal);// leer canal analogico
uint8_t ADC_toAngle(uint16_t adc_value);// lectura 0 a 1023 a angulo 

#endif