#include "servo_pwm.h"
#include <avr/interrupt.h>// interrupciones

#define SERVO_FRAME_TICKS 2000 //cada tick vale 10 micro s entonces 2000 ticks son 20ms (periodo completo)

volatile uint16_t servo_ticks[4] = {150, 150, 150, 150}; //tiempo en alto para cada servo (aprox 90°) INICIO
volatile uint16_t frame_tick = 0; //en que punto del ciclo PWM esta (0-1999)

void ServoPWM_init(void)
{
	DDRB |= (1 << PB1); // D9  Servo 1
	DDRB |= (1 << PB2); // D10 Servo 2
	DDRB |= (1 << PB3); // D11 Servo 3
	DDRD |= (1 << PD3); // D3  Servo 4

	TCCR1A = 0;
	TCCR1B = 0;

	OCR1A = 19;

	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS11);

	TIMSK1 |= (1 << OCIE1A);
} //configurar pines y timer1

void ServoPWM_setMicroseconds(uint8_t servo, uint16_t us)
{ //recibe el ancho de pulso en microsegundos (servo n, ancho pulso)
	if (servo > 3) return;

	if (us < 500) us = 500; //pulso minimo
	if (us > 2500) us = 2500;// pulso maximo

	uint16_t ticks = us / 10; //convertir microsegundo a ticks

	cli();//desactivar intrrupciones para modificar variable
	servo_ticks[servo] = ticks; //guardar nuevo valor del pulso
	sei();
}

void ServoPWM_setAngle(uint8_t servo, uint8_t angle)
{ //que servo se va a mover y cuantos grados 
	if (angle > 180) angle = 180;

	uint16_t us = 500 + ((uint32_t)angle * 2000) / 180;// angulo a pulso

	ServoPWM_setMicroseconds(servo, us); //despues de calcular microsegundos llamamos a esta funcion
}

ISR(TIMER1_COMPA_vect) //ejecucion automatica OCR1A
{ 
	if (frame_tick == 0) //(0 a 1999)
	{
		PORTB |= (1 << PB1);//poner en alto pines servos
		PORTB |= (1 << PB2);
		PORTB |= (1 << PB3);
		PORTD |= (1 << PD3);
	}

	if (frame_tick == servo_ticks[0]) PORTB &= ~(1 << PB1);// si el contados ya llego al tiemo que debe durar el pulso del servo apago ese pin
	if (frame_tick == servo_ticks[1]) PORTB &= ~(1 << PB2);
	if (frame_tick == servo_ticks[2]) PORTB &= ~(1 << PB3);
	if (frame_tick == servo_ticks[3]) PORTD &= ~(1 << PD3);

	frame_tick++; // ya apague mis servos aumento contador

	if (frame_tick >= SERVO_FRAME_TICKS)
	{
		frame_tick = 0;// reiniciar el ciclo
	}
}