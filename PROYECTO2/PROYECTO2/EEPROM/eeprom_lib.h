#ifndef EEPROM_LIB_H_
#define EEPROM_LIB_H_

#include <stdint.h>

#define MAX_POSITIONS 4// guardar maximo 4 posiciones
#define SERVOS_TOTAL 4 //cada posicion tiene 4 valores 

//prototipos
void EEPROM_savePosition(uint8_t index, uint8_t angles[4]);
void EEPROM_readPosition(uint8_t index, uint8_t angles[4]);
void EEPROM_setCount(uint8_t count);
uint8_t EEPROM_getCount(void);
void EEPROM_clear(void);

#endif