/*
 * Clase_SERIAL
 *
 * Created: 16/04/26
 * Author: Jose Martinez
 * Description: 
 */
/****************************************/

// Encabezado (Libraries)
#include <avr/io.h>
#include <avr/interrupt.h>

/****************************************/
// Function prototypes
void inituart();
void writechar(char c);
void writestring(char* string);

/****************************************/
// Main Function
int main (void){
	
	cli();
	DDRB |= (1<<DDB5);   // LED en PB5
	DDRD |= (1<<DDD5);   // LED en PD5
	PORTB &=~(1<<PORTB5);
	inituart();
	writestring("Hola seccion 10");
	sei();
	while(1){
		
	}
}

/****************************************/
// NON-Interrupt subroutines
void inituart(){
	
	// Configurar pines
	DDRD &=~(1<<DD0);// D0 es RX (entrada)
	DDRD |=(1<<DDD1); //D1 es Tx (salida)
	
	// Normal speed
	UCSR0A =0;
	
	// Habilitar int.rx; habilitar rx y tx
	UCSR0B =(1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	
	// Modo asincrono, sin paridad, 1 stop bit, 8 data bits
	UCSR0C= (1<<UCSZ01)|(1<<UCSZ00);
	
	// Cargar UBRR0 (calculado en excel para 16Mhz)
	UBRR0= 103;
	

}

void writechar(char c){
	
	while(!(UCSR0A& (1<<UDRE0))); //Mientras no este vacio sigue el loop
	UDR0= c;
}

void writestring(char* string){ //puntero para ir moviendome entre letras o casillas de writestring (no cuentan espacios)
	
	for (uint8_t i=0; *(string +i) != '\0'; i++)
	{
		writechar(*(string +i));
	}
	
}

/****************************************/
// Interrupt routines

ISR(USART_RX_vect){
	uint8_t bufferRX=UDR0; //guardar dato (recibir) lo envio (color azul)
	writechar(bufferRX); //enviar de vuelta (de regreso color verde)
	
	//encender led 
	if (bufferRX== 'a')
	{
		PORTB|= (1<<PORTB5);
		PORTD|= (1<<PORTD5);
	}
	if (bufferRX== 'b')
	{
		PORTB &= ~(1<<PORTB5);
		PORTD &= ~(1<<PORTD5);
	}
		
}