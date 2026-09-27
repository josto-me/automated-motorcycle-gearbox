// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * test_drum_position.c
 *
 * Host test: Get_Drum_Pos() / Get_Drum_Val() against a plain if-chain for every ADC value.
 *   cc -I../firmware/Shift_Actuator -o test_drum_position test_drum_position.c ../firmware/Shift_Actuator/Drum_Position.c ../firmware/Shift_Actuator/global.c
 */

#include <stdio.h>
#include "global.h"
#include "Drum_Position.h"

static uint8_t Reference_Pos(uint16_t v)			//Reference: one comparison per range
{
	const uint16_t *g=GEAR_POS_VAL; const uint16_t t=GEAR_THRESHOLD;
	if(v<=g[0]-t) return 15;
	else if((v>g[0]-t)&&(v<=g[0]+t)) return 20;
	else if((v>g[0]+t)&&(v<=g[1]-t)) return 25;
	else if((v>g[1]-t)&&(v<=g[1]+t)) return 30;
	else if((v>g[1]+t)&&(v<=g[2]-t)) return 35;
	else if((v>g[2]-t)&&(v<=g[2]+t)) return 40;
	else if((v>g[2]+t)&&(v<=g[3]-t)) return 45;
	else if((v>g[3]-t)&&(v<=g[3]+t)) return 50;
	else if((v>g[3]+t)&&(v<=g[4]-t)) return 55;
	else if((v>g[4]-t)&&(v<=g[4]+t)) return 60;
	else if((v>g[4]+t)&&(v<=g[5]-t)) return 65;
	else if((v>g[5]-t)&&(v<=g[5]+t)) return 70;
	else if((v>g[5]+t)&&(v<=g[6]-t)) return 75;
	else if((v>g[6]-t)&&(v<=g[6]+t)) return 80;
	else if(v>=g[6]+t) return 85;
	else return 99;
}

int main(void)
{
	int fail=0;
	for(uint32_t v=0;v<=4095;v++)
	{
		uint8_t a=Get_Drum_Pos((uint16_t)v), b=Reference_Pos((uint16_t)v);
		if(a!=b) { if(fail<10) printf("ADC %u: new %u, reference %u\n",(unsigned)v,a,b); fail++; }
	}
	for(uint8_t p=20;p<=80;p+=10) if(Get_Drum_Val(p)!=GEAR_POS_VAL[(p-20)/10]) { printf("Get_Drum_Val(%u) wrong\n",p); fail++; }
	if(Get_Drum_Val(25)!=DRUM_VAL_INVALID) { printf("Get_Drum_Val(25) should be invalid\n"); fail++; }
	printf("%s: %d differences in 4096 ADC values\n", fail ? "FAIL" : "OK", fail);
	return fail ? 1 : 0;
}
