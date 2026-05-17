#include "eeprom_lib.h"
#include <avr/eeprom.h>

#define EEPROM_COUNT_ADDR 0 //direccion 0 de EEPROM se usa para guardar contador de posiciones 
#define EEPROM_DATA_START 1 // angulos empiezan en la direccion 1 

void EEPROM_savePosition(uint8_t index, uint8_t angles[4]) // en que posicion guardar los angles recibidos
{
	if (index >= MAX_POSITIONS) return; // validar max posiciones (no escribir fuera del area)

	uint16_t base = EEPROM_DATA_START + (index * SERVOS_TOTAL); //en que direccion empieza la posicion a guardar

	for (uint8_t i = 0; i < SERVOS_TOTAL; i++) //ciclo para guardar los 4 servos
	{
		eeprom_update_byte((uint8_t *)(base + i), angles[i]); //byte en eeprom en la direccion y el dato
	}
}

void EEPROM_readPosition(uint8_t index, uint8_t angles[4]) //leer una posicion ya guardada
{
	if (index >= MAX_POSITIONS) return; 

	uint16_t base = EEPROM_DATA_START + (index * SERVOS_TOTAL); //misma formula que para guardar pero leer

	for (uint8_t i = 0; i < SERVOS_TOTAL; i++)// Lee el valor de los 4 servos
	{
		angles[i] = eeprom_read_byte((uint8_t *)(base + i)); //Leer EEPROM y guardar las posiciones en angles 

		if (angles[i] > 180)// revisar angulos validos (eeprom puede devolver valores basura)
		{
			angles[i] = 90;
		}
	}
}

void EEPROM_setCount(uint8_t count)// guardar cuantas posiciones estan ocupadas
{
	if (count > MAX_POSITIONS)// maximo posiciones limitar
	{
		count = MAX_POSITIONS;
	}

	eeprom_update_byte((uint8_t *)EEPROM_COUNT_ADDR, count);// guardar contador en la direccion 0 de la eeprom
}

uint8_t EEPROM_getCount(void)// leer cuantas posiciones hay guardadas
{
	uint8_t count = eeprom_read_byte((uint8_t *)EEPROM_COUNT_ADDR);

	if (count > MAX_POSITIONS)
	{
		count = 0;// revisar maximo si no poner 0
	}

	return count; //valor final de posiciones guardadas
}

void EEPROM_clear(void)// borrar la memoria
{
	EEPROM_setCount(0);
}