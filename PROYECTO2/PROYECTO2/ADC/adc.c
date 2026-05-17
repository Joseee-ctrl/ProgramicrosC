#include "adc.h"
#include <avr/io.h>

void ADC_init(void)
{
	ADMUX = (1 << REFS0);// voltaje referencia
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);// encender adc con prescaler de 128
}

uint16_t ADC_read(uint8_t canal)// seleccionar canal analogico, leer, conversion ADC
{
	canal &= 0x0F;

	ADMUX = (ADMUX & 0xF0) | canal;

	ADCSRA |= (1 << ADSC);

	while (ADCSRA & (1 << ADSC));

	return ADC;
}

uint8_t ADC_toAngle(uint16_t adc_value)// ángulo = valor_ADC × 180 / 1023
{
	return (uint8_t)(((uint32_t)adc_value * 180) / 1023);
}