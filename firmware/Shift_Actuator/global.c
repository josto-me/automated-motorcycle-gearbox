// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * global.c
 *
 *  Author: Johannes Stockhammer
 */

#include "global.h"

//Variables
	//Gear positions: ADC values of the shift drum potentiometer. PLACEHOLDERS - measure the value
	//of every gear position on your gearbox and enter it here (ascending, 1st to 6th)
const uint16_t GEAR_POS_VAL[NUM_GEAR_POS]={500, 1000, 1500, 2000, 2500, 3000, 3500};	//1st, N, 2nd, 3rd, 4th, 5th, 6th
	//Written in the ISRs
volatile uint16_t drum_pot_val=0;
volatile uint32_t startup_timer=0;
volatile bool control_tick=0;
