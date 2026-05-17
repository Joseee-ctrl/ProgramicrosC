#ifndef UART_H_
#define UART_H_

#include <stdint.h>

void UART_init(uint32_t baud);
void UART_sendChar(char data);
void UART_sendString(const char *str);
uint8_t UART_available(void);
char UART_readChar(void);

#endif