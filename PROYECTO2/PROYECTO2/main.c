#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>// actualizaciones potenciometros
#include <string.h>
#include <stdlib.h>// convertir texto a numeros 
#include <stdio.h>// mensajes con variables dentro 

#include "PWM/servo_pwm.h"
#include "ADC/adc.h"
#include "UART/uart.h"
#include "EEPROM/eeprom_lib.h"
#include "modos/modos.h"// conectar main con modulos 

uint8_t modo_actual = MODO_MANUAL;// variable para guardar modo sistema
uint8_t angulos[4] = {90, 90, 90, 90};// variable servos

char uart_line[30];// espacio de hasta 29 caracteres 
uint8_t uart_index = 0;// en que posicion se guarda el siguiente caracter

uint8_t esperando_posicion_eeprom = 0;//tipo bandera

void LED_init(void);
void LED_manual(void);
void LED_eeprom(void);
void LED_uart(void);
void LED_off(void);

void aplicarAngulos(void);
void modoManual(void);
void guardarPosicionEEPROM(void);
void mostrarPosicionEEPROM(uint8_t posicion);
void moverDesdeUART(char *cmd);

void procesarUART(void);
void ejecutarComando(char *cmd);
void imprimirMenu(void);//prototipos

int main(void)
{
	LED_init(); //pines leds salida 
	ADC_init(); // configurar ADC leer pots
	ServoPWM_init(); // configurar pines de servos y el timer1
	UART_init(9600);

	sei();// global int

	aplicarAngulos();
	imprimirMenu();// comandos del menu por UART

	while (1)// loop principal revisar modo
	{
		procesarUART();

		if (modo_actual == MODO_MANUAL)
		{
			LED_manual();
			modoManual();
		}
		else if (modo_actual == MODO_EEPROM)
		{
			LED_eeprom();
		}
		else if (modo_actual == MODO_UART)
		{
			LED_uart();
		}
	}
}

void LED_init(void)
{
	DDRB |= (1 << PB5); // D13 LED modo manual
	DDRB |= (1 << PB4); // D12 LED modo EEPROM
	DDRD |= (1 << PD7); // D7  LED modo UART / Adafruit

	LED_off();
}

void LED_off(void)
{
	PORTB &= ~(1 << PB5);
	PORTB &= ~(1 << PB4);
	PORTD &= ~(1 << PD7);
}

void LED_manual(void)
{
	PORTB |= (1 << PB5);
	PORTB &= ~(1 << PB4);
	PORTD &= ~(1 << PD7);
}

void LED_eeprom(void)
{
	PORTB &= ~(1 << PB5);
	PORTB |= (1 << PB4);
	PORTD &= ~(1 << PD7);
}

void LED_uart(void)
{
	PORTB &= ~(1 << PB5);
	PORTB &= ~(1 << PB4);
	PORTD |= (1 << PD7);
}

void aplicarAngulos(void)// tomar valores del indice y mandar a libreria PWM para convertir en pulsos
{
	ServoPWM_setAngle(SERVO_1, angulos[0]);
	ServoPWM_setAngle(SERVO_2, angulos[1]);
	ServoPWM_setAngle(SERVO_3, angulos[2]);
	ServoPWM_setAngle(SERVO_4, angulos[3]);
}

void modoManual(void)// controlar servos (leer, convertir, mover)
{
	angulos[0] = ADC_toAngle(ADC_read(0)); // A0
	angulos[1] = ADC_toAngle(ADC_read(1)); // A1
	angulos[2] = ADC_toAngle(ADC_read(2)); // A2
	angulos[3] = ADC_toAngle(ADC_read(3)); // A3

	aplicarAngulos();

	_delay_ms(20);
}

void guardarPosicionEEPROM(void)// guardar posicion actual 4 servos
{
	uint8_t count = EEPROM_getCount();// cuantas posiciones ya estan guardadas?
	char buffer[60];// arreglo caracteres para armar mensajes "temporales" y enviarlos por UART

	if (count >= MAX_POSITIONS)
	{
		UART_sendString("Memoria llena. Maximo 4 posiciones.\r\n");
		imprimirMenu();
		return;
	}

	EEPROM_savePosition(count, angulos);// guardar con count como indice
	EEPROM_setCount(count + 1);// otra posicion guardada

	sprintf(buffer, "Posicion %d guardada en EEPROM\r\n", count + 1);
	UART_sendString(buffer);

	imprimirMenu();
}

void mostrarPosicionEEPROM(uint8_t posicion)
{
	uint8_t count = EEPROM_getCount();
	uint8_t temp[4];// arreglo temporal para guardar lo que lee en la EEPROM
	char buffer[80];

	if (count == 0)
	{
		UART_sendString("No hay posiciones guardadas.\r\n");
		imprimirMenu();
		return;
	}

	if (posicion < 1 || posicion > count)// validar si la posicion seleccionada es valida
	{
		sprintf(buffer, "Posicion invalida. Hay %d posiciones guardadas.\r\n", count);
		UART_sendString(buffer);
		imprimirMenu();
		return;
	}

	EEPROM_readPosition(posicion - 1, temp);// leer posicion EEPROM (1-1 en el indice es posicion 1 que es 0)

	angulos[0] = temp[0];
	angulos[1] = temp[1];
	angulos[2] = temp[2];
	angulos[3] = temp[3];// copiar angulos leidos al arreglo temporal

	aplicarAngulos();

	sprintf(buffer, "Mostrando posicion %d: %d,%d,%d,%d\r\n",
	posicion,
	angulos[0],
	angulos[1],
	angulos[2],
	angulos[3]);

	UART_sendString(buffer);

	imprimirMenu();
}

void moverDesdeUART(char *cmd)
{
	int s1, s2, s3, s4;// variables para guardar valores recibidos 
	char buffer[80];

	if (sscanf(cmd, "M,%d,%d,%d,%d", &s1, &s2, &s3, &s4) == 4)// leer datos desde un string
	{
		if (s1 < 0) s1 = 0;
		if (s1 > 180) s1 = 180;

		if (s2 < 0) s2 = 0;
		if (s2 > 180) s2 = 180;

		if (s3 < 0) s3 = 0;
		if (s3 > 180) s3 = 180;

		if (s4 < 0) s4 = 0;
		if (s4 > 180) s4 = 180;

		angulos[0] = (uint8_t)s1;
		angulos[1] = (uint8_t)s2;
		angulos[2] = (uint8_t)s3;
		angulos[3] = (uint8_t)s4;

		aplicarAngulos();

		sprintf(buffer, "Servo actualizado: %d,%d,%d,%d\r\n", s1, s2, s3, s4);
		UART_sendString(buffer);
	}
	else
	{
		UART_sendString("Error formato. Use M,s1,s2,s3,s4\r\n");
	}
}

void procesarUART(void)
{
	while (UART_available())// mientras haya caracteres se leen
	{
		char c = UART_readChar();

		if (c == '\n' || c == '\r')// revisar enter
		{
			if (uart_index > 0)// solo si hay texto guardado
			{
				uart_line[uart_index] = '\0';
				ejecutarComando(uart_line);// mandar el comando completo a la funcion 
				uart_index = 0;// reiniciar indice para recibir comandos desde 0
			}
		}
		else
		{
			if (uart_index < sizeof(uart_line) - 1)// 30 espacios limite
			{
				uart_line[uart_index] = c;// guardar caracter y avanzar al siguiente espacio
				uart_index++;
			}
		}
	}
}

void ejecutarComando(char *cmd)
{
	if (esperando_posicion_eeprom)// si esta en modo E espero posicion
	{
		uint8_t posicion = atoi(cmd);// texto a numero "2" a 2

		esperando_posicion_eeprom = 0;
		mostrarPosicionEEPROM(posicion);
		return;
	}

	if (strcmp(cmd, "H") == 0)// si lo que el usuario envio es exactamente H ejecuto ese comando
	{
		modo_actual = MODO_MANUAL;
		LED_manual();

		UART_sendString("Modo manual activado.\r\n");
		imprimirMenu();
	}

	else if (strcmp(cmd, "S") == 0)
	{
		if (modo_actual == MODO_MANUAL)
		{
			guardarPosicionEEPROM();
		}
		else
		{
			UART_sendString("Solo se puede guardar en modo manual.\r\n");
			imprimirMenu();
		}
	}

	else if (strcmp(cmd, "E") == 0)
	{
		modo_actual = MODO_EEPROM;
		LED_eeprom();

		esperando_posicion_eeprom = 1;

		UART_sendString("Modo EEPROM activado.\r\n");
		UART_sendString("Ingrese posicion a mostrar: 1, 2, 3 o 4\r\n");
	}

	else if (strcmp(cmd, "U") == 0)
	{
		modo_actual = MODO_UART;
		LED_uart();

		UART_sendString("Modo UART / Adafruit activado.\r\n");
		UART_sendString("Seleccione las posiciones de los servos desde Adafruit.\r\n");
		UART_sendString("Formato manual: M,a,b,c,d\r\n");
		UART_sendString("Ejemplo: M,90,45,120,30\r\n");
	}

	else if (cmd[0] == 'M')
	{
		if (modo_actual == MODO_UART)
		{
			moverDesdeUART(cmd);
		}
		else
		{
			UART_sendString("Primero active modo UART con U.\r\n");
			imprimirMenu();
		}
	}

	else if (strcmp(cmd, "C") == 0)
	{
		EEPROM_clear();
		UART_sendString("EEPROM borrada.\r\n");
		imprimirMenu();
	}

	else if (strcmp(cmd, "MENU") == 0)
	{
		imprimirMenu();
	}

	else
	{
		UART_sendString("Comando no reconocido.\r\n");
		imprimirMenu();
	}
}

void imprimirMenu(void)
{
	uint8_t count = EEPROM_getCount();// leer posiciones
	char buffer[50];// temporal para armar el mensaje del contador

	UART_sendString("\r\n=== PROYECTO 2: GARRA 4 SERVOS ===\r\n");
	UART_sendString("H = Modo manual con potenciometros\r\n");
	UART_sendString("S = Guardar posicion actual en EEPROM\r\n");
	UART_sendString("E = Elegir posicion guardada en EEPROM\r\n");
	UART_sendString("U = Modo UART / Adafruit\r\n");
	UART_sendString("M,a,b,c,d = Mover servos desde Adafruit\r\n");
	UART_sendString("C = Borrar posiciones guardadas\r\n");
	UART_sendString("MENU = Mostrar menu\r\n");

	sprintf(buffer, "Posiciones guardadas: %d/4\r\n", count);// imprimir el mensaje
	UART_sendString(buffer);

	UART_sendString("===================================\r\n");
}