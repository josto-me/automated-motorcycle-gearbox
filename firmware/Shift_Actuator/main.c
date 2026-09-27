// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * main.c
 *
 *  Author: Johannes Stockhammer
 *
 * Version:		1.0
 * Hardware:	ATSAMD21G18A (48 MHz from DFLL48M, 32.768 kHz crystal), two half bridges for the
 *				DC motor, shift drum potentiometer, upshift and downshift button
 * Software:	arm-none-eabi-gcc, CMSIS device headers for the SAMD21 (see Makefile)
 * Description:	Electromechanical shift drum actuator for a sequential motorcycle gearbox.
 *				A DC motor turns the shift drum. A button press moves the drum to the next gear
 *				position (1 - N - 2 - 3 - 4 - 5 - 6); a proportional position controller runs
 *				every 50us.
 */

//Includes
#include "sam.h"
#include "global.h"
#include "main.h"

int main(void)
{
	SystemInit();									//CMSIS system init (startup clock 1 MHz)
	Clock_Init();									//48 MHz from DFLL48M
	IO_Init();										//Pins
	Timer_Init();									//RTC 1ms, TC3 50us, TCC2 PWM
	ADC_Init();										//Drum potentiometer

	startup_timer=TIME_STARTUP_ms;					//Wait until supply and potentiometer are stable
	while(startup_timer);

	while(1)		//Main loop
	{
		if(control_tick)							//Every 50us
		{
			control_tick=0;
			Shift_Application();					//Debounce, shift sequence, position control
		}
	}//end while
}//end main
