// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * apps.c
 *
 *  Author: Johannes Stockhammer
 *
 * Shift sequence, run once per 50us tick: a button press moves the shift drum to the next
 * position. A DC motor drives the drum; a potentiometer on the drum gives its position.
 * A proportional position controller drives the motor PWM until the target position is
 * reached. The control values are examples and have to be tuned on the gearbox.
 */

//Includes
#include "sam.h"
#include "global.h"
#include "main.h"
#include "Drum_Position.h"

//Defines
	//Shift sequence
#define SHIFT_WAIT_RELEASE 0						//Wait until both buttons are released
#define SHIFT_READY 1								//Wait for a button, calculate target position
#define SHIFT_START 2								//Reset counters
#define SHIFT_CONTROL 3								//Position control
	//Position control (example values)
#define KP 15										//Proportional gain: PWM digits per ADC digit
#define OUTPUT_LIMIT PWM_TOP						//Output limit
#define TARGET_TICKS 600							//600 * 50us = 30ms in the target position = done
#define TIME_SHIFT_MAX 10000						//10000 * 50us = 500ms, then motor off

//Variables
struct Switch_State switch_up={0,0,LOW,UNDEF}, switch_down={0,0,LOW,UNDEF};	//Debounce state per button

//----------------------------------------------------------//

void Motor_Set_Pwm(int32_t pwm)		//The half bridge that is not pulsed keeps its high side closed
{
	if(pwm>PWM_TOP) pwm=PWM_TOP;					//Output limit positive direction
	if(pwm<-PWM_TOP) pwm=-PWM_TOP;					//Output limit negative direction
	if(pwm>=0)										//Positive direction: drum towards lower values
	{
		REG_TCC2_CC0=PWM_TOP;						//Motor terminal -: high side closed
		REG_TCC2_CC1=PWM_TOP-(uint32_t)pwm;			//Motor terminal +: duty cycle
	}
	else											//Negative direction: drum towards higher values
	{
		REG_TCC2_CC0=PWM_TOP-(uint32_t)(-pwm);		//Motor terminal -: duty cycle
		REG_TCC2_CC1=PWM_TOP;						//Motor terminal +: high side closed
	}
}//end Motor_Set_Pwm

uint8_t Debounce_Switch(uint8_t group, uint8_t pin, struct Switch_State *sw)	//Once per 50us tick, returns LOW, HIGH or UNDEF
{
	bool pin_level=(PORT->Group[group].IN.reg&(1 << pin))!=0;

	switch(sw->state)
	{
		case 0:		sw->level=pin_level;			//Remember level, start stable time
					sw->count=0;
					sw->state=1;
					break;

		case 1:		if(pin_level!=sw->level) sw->state=0;			//Level changed (bouncing) -> start again
					else if(sw->count<DEBOUNCE_TICKS) sw->count++;	//Still stable
					else sw->result=sw->level;						//Stable for 12.75ms -> take over
					break;
	}//end switch sw->state
	return sw->result;
}//end Debounce_Switch

void Shift_Application(void)
{
	static uint8_t shift_state=SHIFT_WAIT_RELEASE;
	static uint16_t next_drum_val=0, target_counter=0;
	static uint32_t shift_ticks=0;
	uint16_t current_drum_val=drum_pot_val;
	uint8_t current_drum_pos, up, down;
	int32_t output;

	up=Debounce_Switch(0, PIN_UPSHIFT, &switch_up);
	down=Debounce_Switch(0, PIN_DOWNSHIFT, &switch_down);

	switch(shift_state)
	{
		case SHIFT_WAIT_RELEASE:	if((up==HIGH)&&(down==HIGH)) shift_state=SHIFT_READY;	//Both released since the last shift
									break;

		case SHIFT_READY:			current_drum_pos=Get_Drum_Pos(current_drum_val);
									next_drum_val=DRUM_VAL_INVALID;
									if(down==LOW)														//Downshift pressed
									{
										if(current_drum_pos<=DRUM_POS_FIRST) next_drum_val=Get_Drum_Val(DRUM_POS_FIRST);
										else if(current_drum_pos>DRUM_POS_LAST) next_drum_val=Get_Drum_Val(DRUM_POS_LAST);
										else next_drum_val=Get_Drum_Val(((current_drum_pos-5)/DRUM_POS_STEP)*DRUM_POS_STEP);	//Next lower position
									}
									else if(up==LOW)													//Upshift pressed
									{
										if(current_drum_pos<DRUM_POS_FIRST) next_drum_val=Get_Drum_Val(DRUM_POS_FIRST);
										else if(current_drum_pos>=DRUM_POS_LAST) next_drum_val=Get_Drum_Val(DRUM_POS_LAST);
										else next_drum_val=Get_Drum_Val(((current_drum_pos+DRUM_POS_STEP)/DRUM_POS_STEP)*DRUM_POS_STEP);	//Next higher position
									}
									if(next_drum_val!=DRUM_VAL_INVALID) shift_state=SHIFT_START;
									break;

		case SHIFT_START:			target_counter=0;
									shift_ticks=0;
									shift_state=SHIFT_CONTROL;
									break;

		case SHIFT_CONTROL:			output=KP*((int32_t)current_drum_val-(int32_t)next_drum_val);		//Proportional controller
									if(output>OUTPUT_LIMIT) output=OUTPUT_LIMIT;
									if(output<-OUTPUT_LIMIT) output=-OUTPUT_LIMIT;
									Motor_Set_Pwm(output);
									if(Get_Drum_Pos(current_drum_val)==Get_Drum_Pos(next_drum_val))	//Target position reached?
									{
										target_counter++;
										if(target_counter>TARGET_TICKS)									//Stable -> done
										{
											Motor_Set_Pwm(0);
											shift_state=SHIFT_WAIT_RELEASE;
										}
									}
									else target_counter=0;
									shift_ticks++;
									if(shift_ticks>TIME_SHIFT_MAX)										//Shift does not finish -> motor off
									{
										Motor_Set_Pwm(0);
										shift_state=SHIFT_WAIT_RELEASE;
									}
									break;
	}//end switch shift_state
}//end Shift_Application
