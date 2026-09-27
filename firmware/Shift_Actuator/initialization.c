// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * initialization.c
 *
 *  Author: Johannes Stockhammer
 *
 * Register level initialization of clock, pins, timers and ADC of the SAMD21G18A.
 * Page and table numbers refer to the SAM D21 datasheet.
 */

//Includes
#include "sam.h"
#include "global.h"
#include "main.h"

//Defines
	//ADC factory calibration in the NVM software calibration area (NVMCTRL_OTP4 = 0x00806020)
#define NVM_ADC_LINEARITY_Pos 0						//Bits 34:27 of the calibration area -> bits 7:0 of the word
#define NVM_ADC_LINEARITY_Msk 0xFF
#define NVM_ADC_BIASCAL_Pos 8						//Bits 37:35 of the calibration area -> bits 10:8 of the word
#define NVM_ADC_BIASCAL_Msk 0x700

//----------------------------------------------------------//

void Clock_Init(void)		//CPU 48 MHz from DFLL48M in closed loop mode, reference: 32.768 kHz crystal
{
	//1) Flash wait states: 1 wait state needed at 48 MHz
	REG_NVMCTRL_CTRLB |= (1 << NVMCTRL_CTRLB_RWS_Pos);

	//2) External crystal oscillator XOSC32K as reference for the DFLL48M
	REG_SYSCTRL_XOSC32K |= (0x3 << SYSCTRL_XOSC32K_STARTUP_Pos)	//STARTUP: start-up time 125092us
						| SYSCTRL_XOSC32K_EN32K						//EN32K: 32 kHz output enabled
						| SYSCTRL_XOSC32K_XTALEN					//XTALEN: crystal connected to XIN32/XOUT32
						| SYSCTRL_XOSC32K_ENABLE;					//ENABLE: oscillator on

	//3) XOSC32K as source of generic clock generator 1 (32-bit write with the ID)
	while(REG_GCLK_STATUS&GCLK_STATUS_SYNCBUSY);					//Wait for sync
	REG_GCLK_GENCTRL=GCLK_GENCTRL_RUNSTDBY							//RUNSTDBY: generator runs in standby
					| GCLK_GENCTRL_IDC								//IDC: 50:50 duty cycle
					| GCLK_GENCTRL_GENEN							//GENEN: generator on
					| (0x5 << GCLK_GENCTRL_SRC_Pos)					//SRC: XOSC32K
					| (0x1 << GCLK_GENCTRL_ID_Pos);					//ID: generator 1
	while(REG_GCLK_STATUS&GCLK_STATUS_SYNCBUSY);					//Wait for sync

	//4) Generator 1 as reference clock of the DFLL48M (16-bit write with the ID)
	REG_GCLK_CLKCTRL=(uint16_t)(GCLK_CLKCTRL_CLKEN					//CLKEN: generic clock on
					| (0x1 << GCLK_CLKCTRL_GEN_Pos)					//GEN: generator 1
					| (0 << GCLK_CLKCTRL_ID_Pos));					//ID: DFLL48M_REF

	//5) DFLL48M in closed loop mode (write only when DFLLRDY is set)
	while(!(REG_SYSCTRL_PCLKSR&SYSCTRL_PCLKSR_DFLLRDY));			//Wait until ready
	REG_SYSCTRL_DFLLCTRL=SYSCTRL_DFLLCTRL_MODE						//MODE: closed loop
						| SYSCTRL_DFLLCTRL_ENABLE;					//ENABLE: DFLL on
	while(!(REG_SYSCTRL_PCLKSR&SYSCTRL_PCLKSR_DFLLRDY));			//Wait until ready

	//6) Multiplication factor and step sizes
	REG_SYSCTRL_DFLLMUL |= (31 << SYSCTRL_DFLLMUL_CSTEP_Pos)		//CSTEP: coarse step, half of the maximum (63)
						| (511 << SYSCTRL_DFLLMUL_FSTEP_Pos)		//FSTEP: fine step, half of the maximum (1023)
						| (1465 << SYSCTRL_DFLLMUL_MUL_Pos);		//MUL: 48 MHz / 32.768 kHz = 1465

	//7) DFLL48M as source of generic clock generator 0 -> CPU runs at 48 MHz
	while(REG_GCLK_STATUS&GCLK_STATUS_SYNCBUSY);					//Wait for sync
	REG_GCLK_GENCTRL=GCLK_GENCTRL_RUNSTDBY							//RUNSTDBY: generator runs in standby
					| GCLK_GENCTRL_IDC								//IDC: 50:50 duty cycle
					| GCLK_GENCTRL_GENEN							//GENEN: generator on
					| (0x7 << GCLK_GENCTRL_SRC_Pos)					//SRC: DFLL48M
					| (0x0 << GCLK_GENCTRL_ID_Pos);					//ID: generator 0
	while(REG_GCLK_STATUS&GCLK_STATUS_SYNCBUSY);					//Wait for sync
}//end Clock_Init

void IO_Init(void)
{
	//Outputs
		//Half bridge IN pins - PWM from TCC2 (peripheral function E, table 6-1)
		//WRCONFIG: 32-bit write, HWSEL = upper half of PORT A, PINMASK = pin - 16
	REG_PORT_WRCONFIG0=PORT_WRCONFIG_HWSEL							//HWSEL: pins 16..31
					| PORT_WRCONFIG_WRPINCFG						//WRPINCFG: update PINCFG
					| PORT_WRCONFIG_WRPMUX							//WRPMUX: update PMUX
					| (0x04 << PORT_WRCONFIG_PMUX_Pos)				//PMUX: function E = TCC2
					| PORT_WRCONFIG_PMUXEN							//PMUXEN: peripheral on
					| (1 << (PIN_MOTOR_MINUS-16))					//PA16 = TCC2 WO[0], motor terminal -
					| (1 << (PIN_MOTOR_PLUS-16));					//PA17 = TCC2 WO[1], motor terminal +

	//Inputs
		//Shift buttons - PA20 upshift, PA21 downshift, external pull-ups
	PORT->Group[0].PINCFG[PIN_UPSHIFT].bit.INEN=1;					//Input buffer on
	PORT->Group[0].PINCFG[PIN_DOWNSHIFT].bit.INEN=1;				//Input buffer on
	REG_PORT_DIRCLR0=(1 << PIN_UPSHIFT)|(1 << PIN_DOWNSHIFT);		//As inputs
		//Shift drum potentiometer - PB08 = AIN[2] (peripheral function B)
	REG_PORT_WRCONFIG1=PORT_WRCONFIG_WRPINCFG						//WRPINCFG: update PINCFG (HWSEL = 0: pins 0..15)
					| PORT_WRCONFIG_WRPMUX							//WRPMUX: update PMUX
					| (0x01 << PORT_WRCONFIG_PMUX_Pos)				//PMUX: function B = ADC
					| PORT_WRCONFIG_PMUXEN							//PMUXEN: peripheral on
					| (1 << PIN_DRUM_POT);							//PB08
}//end IO_Init

void Timer_Init(void)
{
	//RTC - 1ms tick
		//1) Generator 0 (48 MHz) as clock of the RTC
	REG_GCLK_CLKCTRL=(uint16_t)(GCLK_CLKCTRL_CLKEN|(0 << GCLK_CLKCTRL_GEN_Pos)|GCLK_CLKCTRL_ID_RTC);
		//2) 32-bit counter, cleared on compare match 0
	while(REG_RTC_STATUS&RTC_STATUS_SYNCBUSY);						//Wait for sync
	REG_RTC_MODE0_CTRL |= RTC_MODE0_CTRL_MATCHCLR;					//MATCHCLR: clear on COMP0
	while(REG_RTC_STATUS&RTC_STATUS_SYNCBUSY);
		//3) Compare value and interrupt
	REG_RTC_MODE0_COMP0=RTC_TICK_CYCLES;							//1ms * 48 MHz = 48000
	while(REG_RTC_STATUS&RTC_STATUS_SYNCBUSY);
	REG_RTC_MODE0_INTENSET=RTC_MODE0_INTENSET_CMP0;					//Compare 0 interrupt on
	NVIC_EnableIRQ(RTC_IRQn);
		//4) Enable RTC (enable-protected registers are set before)
	REG_RTC_MODE0_CTRL |= RTC_MODE0_CTRL_ENABLE;

	//TC3 - 50us tick for the position control
		//1) Bus clock and generator 0 as clock of TC3
	REG_PM_APBCMASK |= PM_APBCMASK_TC3;
	REG_GCLK_CLKCTRL=(uint16_t)(GCLK_CLKCTRL_CLKEN|(0 << GCLK_CLKCTRL_GEN_Pos)|GCLK_CLKCTRL_ID_TCC2_TC3);
		//2) Match frequency mode: counter restarts at CC0 (no output pin used)
	while(REG_TC3_STATUS&TC_STATUS_SYNCBUSY);
	REG_TC3_CTRLA |= (1 << TC_CTRLA_WAVEGEN_Pos);					//WAVEGEN: MFRQ, 16-bit, prescaler 1
	while(REG_TC3_STATUS&TC_STATUS_SYNCBUSY);
	REG_TC3_COUNT16_CC0=CONTROL_TICK_CYCLES;						//50us * 48 MHz = 2400
	while(REG_TC3_STATUS&TC_STATUS_SYNCBUSY);
		//3) Match channel 0 interrupt and enable
	REG_TC3_INTENSET=TC_INTENSET_MC0;
	NVIC_EnableIRQ(TC3_IRQn);
	REG_TC3_CTRLA |= TC_CTRLA_ENABLE;

	//TCC2 - PWM for both motor terminals (two half bridges)
		//1) Bus clock and generator 0 as clock of TCC2 (shared with TC3)
	REG_PM_APBCMASK |= PM_APBCMASK_TCC2;
	REG_GCLK_CLKCTRL=(uint16_t)(GCLK_CLKCTRL_CLKEN|(0 << GCLK_CLKCTRL_GEN_Pos)|GCLK_CLKCTRL_ID_TCC2_TC3);
		//2) Normal single slope PWM
	while(REG_TCC2_SYNCBUSY&TCC_SYNCBUSY_WAVE);
	REG_TCC2_WAVE |= (0x02 << TCC_WAVE_WAVEGEN_Pos);				//WAVEGEN: NPWM
	while(REG_TCC2_SYNCBUSY&TCC_SYNCBUSY_WAVE);
		//3) Period: 48 MHz / (PWM_TOP+1) = 32 kHz, resolution 10.5 bit
	while(REG_TCC2_SYNCBUSY&TCC_SYNCBUSY_PER);
	REG_TCC2_PER=PWM_TOP;
	while(REG_TCC2_SYNCBUSY&TCC_SYNCBUSY_PER);
		//4) Idle state: both high sides closed -> no voltage at the motor (0 = low side, PWM_TOP = high side)
	REG_TCC2_CC0=PWM_TOP;											//WO[0] motor terminal -
	REG_TCC2_CC1=PWM_TOP;											//WO[1] motor terminal +
		//5) Enable TCC2
	REG_TCC2_CTRLA |= TCC_CTRLA_ENABLE;
}//end Timer_Init

void ADC_Init(void)		//Free running conversion of AIN[2], result via interrupt
{
	uint8_t adc_linearity, adc_biascal;

	//1) Generator 0 as clock of the ADC
	REG_GCLK_CLKCTRL=(uint16_t)(GCLK_CLKCTRL_CLKEN|(0 << GCLK_CLKCTRL_GEN_Pos)|GCLK_CLKCTRL_ID_ADC);

	//2) Reference: internal 1.0 V (REFSEL 0x00, table 32-7)
	REG_ADC_REFCTRL |= (0x00 << ADC_REFCTRL_REFSEL_Pos);

	//3) Averaging: 16 samples accumulated, result divided by 8
	REG_ADC_AVGCTRL |= (0x03 << ADC_AVGCTRL_ADJRES_Pos)				//ADJRES: shift result right by 3
					| (0x04 << ADC_AVGCTRL_SAMPLENUM_Pos);			//SAMPLENUM: 16 samples

	//4) Prescaler, correction, free running
	while(REG_ADC_STATUS&ADC_STATUS_SYNCBUSY);
	REG_ADC_CTRLB |= (0x03 << ADC_CTRLB_PRESCALER_Pos)				//PRESCALER: DIV32 -> ADC clock 1.5 MHz
					| ADC_CTRLB_CORREN								//CORREN: gain and offset correction on
					| ADC_CTRLB_FREERUN;							//FREERUN: next conversion starts automatically
	while(REG_ADC_STATUS&ADC_STATUS_SYNCBUSY);

	//5) Input against GND, single ended
	REG_ADC_INPUTCTRL |= (0x18 << ADC_INPUTCTRL_MUXNEG_Pos)			//MUXNEG: internal GND
					| (0x02 << ADC_INPUTCTRL_MUXPOS_Pos);			//MUXPOS: AIN[2] = PB08
	while(REG_ADC_STATUS&ADC_STATUS_SYNCBUSY);

	//6) Result ready interrupt
	REG_ADC_INTENSET=ADC_INTENSET_RESRDY;
	NVIC_EnableIRQ(ADC_IRQn);

	//7) Factory calibration values from the NVM software calibration area
	adc_linearity=((*(uint32_t*)NVMCTRL_OTP4)&NVM_ADC_LINEARITY_Msk) >> NVM_ADC_LINEARITY_Pos;
	adc_biascal=((*(uint32_t*)NVMCTRL_OTP4)&NVM_ADC_BIASCAL_Msk) >> NVM_ADC_BIASCAL_Pos;
	REG_ADC_CALIB |= (adc_biascal << ADC_CALIB_BIAS_CAL_Pos)|(adc_linearity << ADC_CALIB_LINEARITY_CAL_Pos);

	//8) Gain and offset correction (example values, measure on your board)
	REG_ADC_GAINCORR=0x402;											//Gain 1.0010 (format 1.11 bit)
	REG_ADC_OFFSETCORR=13;											//Offset in digits (two's complement)

	//9) Sample time
	ADC->SAMPCTRL.bit.SAMPLEN=15;

	//10) Enable ADC
	while(REG_ADC_STATUS&ADC_STATUS_SYNCBUSY);
	REG_ADC_CTRLA |= ADC_CTRLA_ENABLE;
	while(REG_ADC_STATUS&ADC_STATUS_SYNCBUSY);
}//end ADC_Init
