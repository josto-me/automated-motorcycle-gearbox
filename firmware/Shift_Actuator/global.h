// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * global.h
 *
 *  Author: Johannes Stockhammer
 *
 * Constants, pin assignment and variables shared between the modules.
 */


#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdint.h>
#include <stdbool.h>

//Defines
	//Clock and timers
#define RTC_TICK_CYCLES 48000						//RTC compare value: 1ms * 48 MHz = 48000
#define CONTROL_TICK_CYCLES 2400					//TC3 top value: 50us * 48 MHz = 2400
#define TIME_STARTUP_ms 100							//Wait after reset until supply and potentiometer are stable
	//PWM (idle state: both high side switches closed, no voltage at the motor)
#define PWM_TOP 1499								//TCC2 period: 48 MHz / (1499+1) = 32 kHz
	//Pins (PORT A = group 0, PORT B = group 1)
#define PIN_MOTOR_MINUS 16							//PA16 TCC2 WO[0] -> half bridge IN, motor terminal -
#define PIN_MOTOR_PLUS 17							//PA17 TCC2 WO[1] -> half bridge IN, motor terminal +
#define PIN_UPSHIFT 20								//PA20 upshift button, external pull-up, LOW = pressed
#define PIN_DOWNSHIFT 21							//PA21 downshift button, external pull-up, LOW = pressed
#define PIN_DRUM_POT 8								//PB08 AIN[2] shift drum potentiometer
#define ADC_FULL_SCALE 4095							//12 bit, reference INT1V = 1.0 V
	//Switch debounce
#define LOW 0
#define HIGH 1
#define UNDEF 2										//Level not yet stable
#define DEBOUNCE_TICKS 255							//255 * 50us = 12.75ms stable level
	//Shift drum position
#define NUM_GEAR_POS 7								//1st, neutral, 2nd ... 6th
#define GEAR_THRESHOLD 57							//+-57 digits around a gear position (360deg*57/4095 = +-5deg)
#define DRUM_VAL_INVALID 0xFFFF						//No gear position for this code
	//Drum position codes of Get_Drum_Pos(): 20 = 1st, 30 = neutral, 40 = 2nd ... 80 = 6th,
	//odd tens (15, 25 ... 85) = between two positions
#define DRUM_POS_FIRST 20
#define DRUM_POS_LAST 80
#define DRUM_POS_STEP 10

//Variables
	//Written in the ISRs
extern volatile uint16_t drum_pot_val;				//Shift drum potentiometer, rises on upshift
extern volatile uint32_t startup_timer;				//1ms software timer
extern volatile bool control_tick;					//Set every 50us by TC3
	//Gear positions
extern const uint16_t GEAR_POS_VAL[NUM_GEAR_POS];	//ADC values of the gear positions (placeholders, calibrate)

#endif /* GLOBAL_H_ */
