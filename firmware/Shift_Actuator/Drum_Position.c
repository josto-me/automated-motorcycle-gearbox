// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Drum_Position.c
 *
 *  Author: Johannes Stockhammer
 *
 * Conversion between the ADC value of the shift drum potentiometer and the drum position code
 * (20 = 1st, 30 = neutral, 40 = 2nd ... 80 = 6th, odd tens = between two positions).
 */

//Includes
#include "global.h"
#include "Drum_Position.h"

uint8_t Get_Drum_Pos(uint16_t adc_val)				//ADC value -> drum position code (see global.h)
{
	uint8_t k;

	if(adc_val<=GEAR_POS_VAL[0]-GEAR_THRESHOLD) return DRUM_POS_FIRST-5;						//Below 1st
	for(k=0;k<NUM_GEAR_POS;k++)
	{
		if(adc_val<=GEAR_POS_VAL[k]+GEAR_THRESHOLD) return DRUM_POS_FIRST+k*DRUM_POS_STEP;		//In gear position k
		if((k<NUM_GEAR_POS-1)&&(adc_val<=GEAR_POS_VAL[k+1]-GEAR_THRESHOLD))
			return DRUM_POS_FIRST+k*DRUM_POS_STEP+5;										//Between position k and k+1
	}
	return DRUM_POS_LAST+5;																	//Above 6th
}//end Get_Drum_Pos

uint16_t Get_Drum_Val(uint8_t drum_pos)				//Drum position code -> ADC value of the gear position
{
	if((drum_pos<DRUM_POS_FIRST)||(drum_pos>DRUM_POS_LAST)||(drum_pos%DRUM_POS_STEP)) return DRUM_VAL_INVALID;
	return GEAR_POS_VAL[(drum_pos-DRUM_POS_FIRST)/DRUM_POS_STEP];
}//end Get_Drum_Val
