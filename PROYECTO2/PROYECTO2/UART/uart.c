#include "uart.h"
#include <avr/io.h>

#define F_CPU 16000000UL //16Mhz para calcular BAUDRATE

void UART_init(uint32_t baud)// configurar la UART
{
	uint16_t ubrr = (F_CPU / (16UL * baud)) - 1;// ubrr 103

	UBRR0H = (uint8_t)(ubrr >> 8);//0
	UBRR0L = (uint8_t)ubrr;//aprox 103

	UCSR0B = (1 << RXEN0) | (1 << TXEN0);// activar recepcion por RX y transmision por TX
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);// formato tipico 8 bits de datos sin paridad 1 stop bit
}

void UART_sendChar(char data)// caracter a enviar
{
	while (!(UCSR0A & (1 << UDRE0)));// esperar a que UART este lista para enviar otro dato

	UDR0 = data;//registro datos UART escribir caracter y enviar por TX
}

void UART_sendString(const char *str)// recibe un puntero a texto y envia cada caracter y luego avanza 
{
	while (*str)
	{
		UART_sendChar(*str++);
	}
}

uint8_t UART_available(void)// revisar si llego caracter por UART
{
	return (UCSR0A & (1 << RXC0));// 1 si ya recibio
}

char UART_readChar(void)// leer el caracter recibido 
{
	return UDR0;// registro donde se guarda el dato que llega por RX
}