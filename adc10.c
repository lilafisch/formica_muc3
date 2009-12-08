/*  Copyright 2008 Stephen English, Jeffrey Gough, Alexis Johnson, 
    Robert Spanton and Joanna A. Sun.

    This file is part of the Formica robot firmware.

    The Formica robot firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The Formica robot firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the Formica robot firmware.  
    If not, see <http://www.gnu.org/licenses/>.  */
#include "adc10.h"
#include "ir-bias.h"
#include "device.h"
#include <signal.h>
#include <stdint.h>
#include "food.h"
#include "bearing.h"
#include "ir-tx.h"
#include "battery.h"
#include "leds.h"
#include "time.h"

/* Disable the ADC */
//#define adc10_dis() do { ADC10CTL0 &= ~ENC; } while (0)

void adc10_dis()
{
	bias_use2();			/* back to IR reception bias */
	while ( ADC10CTL1 & ADC10BUSY );
	ADC10CTL0 &= ~ENC;
}

/* Select a channel (0 <= x <= 15) */
#define adc10_set_channel(x) do { ADC10CTL1 &= ~INCH_15;	\
		ADC10CTL1 |= x << 12; } while (0)

uint16_t pd_value[3];

static enum {
	PD1,
	PD2,
	PD3,
	BATT,
	FOOD0,
	FOOD1
} curreading = PD1;

#define PD1_CHANNEL 1
#define PD2_CHANNEL 2
#define PD3_CHANNEL 3
#define FOOD_CHANNEL 4
#define BATT_CHANNEL 15

/* The ADC10AE0 value: Which pins are analogue inputs */
/* PD1, PD2, PD3 and FOOD are on P2.1, P2.2, P2.3 and P2.4 respectively */
/* P2.1, P2.2, P2.3, P2.4 (page 60 of the MSP430F2234 datasheet) */
/* RX is on P3.7 (A7) */
#define CHANNEL_CONFIG (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<7)

#define BATT_INTERVAL 10

void adc10_init( void )
{
	ADC10CTL0 = SREF_0 	/* Use VCC and VSS as the references */
		| ADC10SHT_DIV64 /* 64 x ADC10CLKs
				    32 us */
		/* ADC10SR = 0 -- Support 200 ksps sampling (TODO: maybe this can be set) */
		/* REFOUT = 0 -- Reference output off */
		/* REFBURST = 0 -- Reference buffer on continuously (TODO) */
		| REF2_5V
		| REFON         /* Use 2.5V reference */
		| ADC10ON	/* Peripheral on */
	        | ADC10IE;       /* Interrupt enabled */

	ADC10CTL1 = /* Select the channel later... */
		SHS_0		/* ADC10SC is the sample-and-hold selector */
		/* ADC10DF = 0 -- Straight binary format */
		/* ISSH = 0 -- No inversion on the s&h signal */
		| ADC10DIV_7	/* Divide clock by 8 */
		| ADC10SSEL_MCLK
		| CONSEQ_0; 	/* Single channel single conversion */
		
	/* Set up the pins as analogue inputs */
	ADC10AE0 = CHANNEL_CONFIG;
	/* Enable A15 (Batt) */
	ADC10AE1 = 0x80;

	ADC10DTC0 |= ADC10CT; /* DTC Not used. This makes it continuous */
	
	adc10_set_channel(PD1_CHANNEL);
}

void adc10_grab( void )
{
	if(curreading == FOOD1)
		fled_on();
	//else if( ir_transmit_is_enabled() )
	//{
	bias_use1();
	
	/* Start the conversion: */
	ADC10CTL0 |= (ADC10SC | ENC);
	//}
}

uint16_t adc10_readtemp( void )
{
	uint16_t boottemp;

	/* If the ADC is already enabled return 0 */
	if(ADC10CTL0 & ENC)
		return 0;

	/* Read the temperature to initialise a random number generator */
	ADC10CTL1 &= ~INCH_15;
	ADC10CTL1 |= INCH_TEMP; /*Temperature sensor*/

 	/* Start the conversion: */
 	ADC10CTL0 |= (ENC | ADC10SC);

	/* Wait for the conversion to finish */
	while(!(ADC10CTL0 & ADC10IFG));
	boottemp = ADC10MEM;

	/*Disable the ADC*/
	adc10_dis();
	return boottemp;
}

interrupt (ADC10_VECTOR) adc10_isr( void )
{
	static uint16_t food0; /*output from food with LED off*/
	static uint16_t food1; /*output from food with LED on*/
	static uint32_t batt_time = 0;

	adc10_dis();

	switch(curreading){
	case PD1:
		pd_value[0] = ADC10MEM;

		adc10_set_channel(PD2_CHANNEL);
		curreading = PD2;
		break;
	case PD2:
		pd_value[1] = ADC10MEM;

		adc10_set_channel(PD3_CHANNEL);
		curreading = PD3;
		break;
	case PD3:
		pd_value[2] = ADC10MEM;

		/* sample the battery voltage once in a while */
		if (the_time > batt_time)
		{
			batt_time = the_time + BATT_INTERVAL;

			adc10_set_channel(BATT_CHANNEL);
			curreading = BATT;

			/* disable other channels to prevent coupling of photocurrents */
			ADC10AE0 = 0;
			ADC10CTL1 &= ~ADC10SSEL_SMCLK; /* Goto Aux Clock */
			ADC10CTL1 |= ADC10SSEL_ACLK;   /* 12Khz */
			ADC10CTL1 &= ~ADC10DIV_7;      /* Remove clock divide */
			ADC10CTL1 |= ADC10DIV_0;
			ADC10CTL0 |= SREF_1; /* Use 2.5V Reference */
			ADC10CTL0 &= ~ADC10SHT_DIV64; /* Go from divide by 64 */
			ADC10CTL0 |= ADC10SHT_DIV4; /* to divide by 4 */
		}
		else
		{
			adc10_set_channel(FOOD_CHANNEL);
			curreading = FOOD0;
		}

		bearing_set( pd_value );

		break;
	case BATT:
		battval = ADC10MEM;

		adc10_set_channel(FOOD_CHANNEL);
		ADC10AE0 = CHANNEL_CONFIG;
		ADC10CTL1 &= ~ADC10SSEL_SMCLK;
		ADC10CTL1 |= ADC10SSEL_MCLK; /* Bacl to master clock*/
		ADC10CTL1 |= ADC10DIV_7; /* Divide by 7 */
		ADC10CTL0 &= ~SREF_7; /* Vcc - Vss rails */
		ADC10CTL0 |= ADC10SHT_DIV4; /* Divide clock by 64 */
			
		curreading = FOOD0;
		break;
	case FOOD0:
		/* FLED Off */
		food0 = ADC10MEM;

		adc10_set_channel(FOOD_CHANNEL);

		curreading = FOOD1;
		break;
	case FOOD1:
		/* Fled ON */
		food1 = ADC10MEM;

		adc10_set_channel(PD1_CHANNEL);

		foodcallback(food0, food1);
			
		curreading = PD1;
		break;
	}
	fled_off();
}
