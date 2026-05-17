#ifndef SERVO_PWM_H_
#define SERVO_PWM_H_// solo incluir el archivo una vez

#include <avr/io.h>// registros y pines ATmega
#include <stdint.h>//tanaños de las variables

#define SERVO_1 0// nombres servo
#define SERVO_2 1
#define SERVO_3 2
#define SERVO_4 3

void ServoPWM_init(void);
void ServoPWM_setAngle(uint8_t servo, uint8_t angle);
void ServoPWM_setMicroseconds(uint8_t servo, uint16_t us);

#endif