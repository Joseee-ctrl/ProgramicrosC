/*
JOSE MARTINEZ
*/


// Frecuencia del microcontrolador (16 MHz)
#define F_CPU 16000000UL

// Configuración de UART
#define BAUD 9600
#define UBRR_VALUE ((F_CPU/16/BAUD)-1)

#include <avr/io.h>
#include <stdint.h>

// PROTOTIPOS

// UART
void UART_init(void);
void UART_tx(char data);
char UART_rx(void);
void cadena(char txt[]);

// ADC
void ADC_init(void);
uint16_t ADC_read(uint8_t channel);

// Utilidad
void enviar_numero(uint16_t num);


// MAIN

int main(void)
{
	char opcion;// Guarda la opción del menú
	char dato; // Guarda el carácter recibido por UART
	uint16_t valor;// Guarda el valor leído del ADC (0–1023)

	UART_init();// Inicializa comunicación serial
	ADC_init();// Inicializa el ADC
	DDRB = 0xFF;// PORTB como salida (para LEDs)

	while (1)
	{
		// ---------- MENÚ ----------
		cadena("\r\n------ MENU ------\r\n");
		cadena("1. Leer Potenciometro\r\n");
		cadena("2. Enviar ASCII\r\n");
		cadena("Seleccione una opcion: ");

		// Espera a que el usuario escriba una opción
		opcion = UART_rx();

		// Muestra lo que el usuario presiona
		UART_tx(opcion);
		cadena("\r\n");

		// OPCIONES
		if (opcion == '1')
		{
			// Lee el canal A6 (ADC6)
			valor = ADC_read(6);

			// Envía el valor leído en formato decimal
			cadena("Valor del potenciometro: ");
			enviar_numero(valor);
			cadena("\r\n");
		}

		else if (opcion == '2')
		{
			cadena("Ingrese un caracter: ");

			// Espera un carácter desde la terminal
			dato = UART_rx();

			// Lo vuelve a mostrar (eco)
			UART_tx(dato);
			cadena("\r\n");

			// Muestra el valor ASCII en binario en PORTB (LEDs)
			PORTB = dato;

			cadena("ASCII mostrado en LEDs\r\n");
		}

		else
		{
			cadena("Opcion no valida\r\n");
		}
	}
}


// UART 9600 BAUDIOS, 8 BITS DE DATOS, SIN PARIDAD 1 BIT DE PARADA

void UART_init(void)
{
	// Carga el valor calculado del baud rate
	UBRR0H = (UBRR_VALUE >> 8);
	UBRR0L = UBRR_VALUE;

	// Habilita transmisión y recepción
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);

	// Configura formato: 8 bits
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


// Envía un carácter por UART
void UART_tx(char data)
{
	// Espera hasta que el buffer de transmisión esté vacío
	while (!(UCSR0A & (1 << UDRE0)));

	// Carga el dato para enviarlo
	UDR0 = data;
}


// Recibe un carácter por UART
char UART_rx(void)
{
	// Espera hasta que llegue un dato
	while (!(UCSR0A & (1 << RXC0)));

	// Retorna el dato recibido
	return UDR0;
}


// Envía una cadena completa (string)
void cadena(char txt[])
{
	int i = 0;

	// Recorre hasta encontrar el fin de cadena '\0'
	while (txt[i] != '\0')
	{
		UART_tx(txt[i]);
		i++;
	}
}


// ADC 

void ADC_init(void)
{
	ADMUX = (1 << REFS0); // Referencia AVcc

	ADCSRA = (1 << ADEN)   // Activa el ADC
	| (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128
}


// Lee un canal del ADC 
uint16_t ADC_read(uint8_t channel)
{
	// Limpia bits anteriores del canal y selecciona el nuevo
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

	// Inicia conversión
	ADCSRA |= (1 << ADSC);

	// Espera a que termine (ADSC vuelve a 0)
	while (ADCSRA & (1 << ADSC));

	// Retorna el resultado de 10 bits (0–1023)
	return ADC;
}


// UTILIDAD

// Convierte un número entero a ASCII y lo envía por UART

void enviar_numero(uint16_t num)
{
	char buffer[6]; // Hasta 5 dígitos + seguridad
	int i = 0;
	int j;

	// Caso especial: número 0
	if (num == 0)
	{
		UART_tx('0');
		return;
	}

	// Convierte dígito por dígito (al revés)
	while (num > 0)
	{
		buffer[i] = (num % 10) + '0'; // Convierte a ASCII
		num = num / 10;
		i++;
	}

	// Envía los dígitos en orden correcto
	for (j = i - 1; j >= 0; j--)
	{
		UART_tx(buffer[j]);
	}
}