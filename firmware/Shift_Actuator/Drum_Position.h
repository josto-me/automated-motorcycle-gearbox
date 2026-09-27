// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Drum_Position.h
 *
 *  Author: Johannes Stockhammer
 */


#ifndef DRUM_POSITION_H_
#define DRUM_POSITION_H_

#include <stdint.h>
uint8_t Get_Drum_Pos(uint16_t adc_val);				//ADC value -> drum position code
uint16_t Get_Drum_Val(uint8_t drum_pos);			//Drum position code -> ADC value, DRUM_VAL_INVALID if no gear position

#endif /* DRUM_POSITION_H_ */
