// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * main.h
 *
 *  Author: Johannes Stockhammer
 */


#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <stdbool.h>

//Types
struct Switch_State									//Debounce state of one button
{
	uint8_t state;									//0 = sample level, 1 = check stable time
	uint8_t count;									//Ticks with stable level
	uint8_t level;									//Level at the start of the stable time
	uint8_t result;									//Debounced level: LOW, HIGH or UNDEF
};

//Prototypes
	//initialization.c
void Clock_Init(void);								//DFLL48M from 32.768 kHz crystal, CPU 48 MHz
void IO_Init(void);									//Pins: PWM, buttons, potentiometer
void Timer_Init(void);								//RTC 1ms tick, TC3 50us tick, TCC2 PWM
void ADC_Init(void);								//Free running ADC on the drum potentiometer
	//apps.c
void Motor_Set_Pwm(int32_t pwm);					//Signed output -PWM_TOP..PWM_TOP
uint8_t Debounce_Switch(uint8_t group, uint8_t pin, struct Switch_State *sw);	//LOW, HIGH or UNDEF
void Shift_Application(void);						//Shift sequence, once per 50us tick

#endif /* MAIN_H_ */
