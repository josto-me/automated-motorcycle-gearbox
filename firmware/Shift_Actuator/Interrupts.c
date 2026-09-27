// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Interrupts.c
 *
 *  Author: Johannes Stockhammer
 */

#include "sam.h"
#include "global.h"

void ADC_Handler(void)		//ADC result ready
{
	drum_pot_val=ADC_FULL_SCALE-REG_ADC_RESULT;		//Invert so the value rises on upshift; reading RESULT clears RESRDY
}//end ADC_Handler

void RTC_Handler(void)		//RTC compare 0, every 1ms
{
	if(startup_timer) startup_timer--;
	REG_RTC_MODE0_INTFLAG=RTC_MODE0_INTFLAG_CMP0;	//Clear CMP0 flag (write 1 to clear)
}//end RTC_Handler

void TC3_Handler(void)		//TC3 match channel 0, every 50us
{
	control_tick=1;									//Main loop runs the control once per tick
	REG_TC3_INTFLAG=TC_INTFLAG_MC0|TC_INTFLAG_OVF;	//Clear MC0 and OVF flags (write 1 to clear)
}//end TC3_Handler
