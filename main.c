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
#include "device.h"
#include <signal.h>
#include "ir-rx.h"
#include "ir-tx.h"
#include "ir-bias.h"
#include "opamp-1.h"
#include "adc10.h"
#include "random.h"
#include "motor.h"
#include "leds.h"
#include "virus.h"
#include "net-rx.h"
#include "net-tx.h"
#include "battery.h"
#include "food.h"
#include "bearing.h"
#include "flash.h"
#include "time.h"
#include "behav/braitenberg.h"
#include "behav/parking.h"
#include "behav/watchdog.h"

/* when food_level rises above this amount, go to charger */
#define FOOD_THRESHOLD (20 * 60)


/* Initialises everything. */
void init(void);

/* Performs emergency power off.
   Never returns. */
void low_power(void);

int i = 0;

int main( void )
{
        uint32_t chargeopportunity = 0;
	init();

	random_walk_disable();
	
	time_wait(5);

	/* start charging if touching charger within 1 second */
	chargeopportunity = the_time + 20;
	leds_set(ORANGE);
	while (the_time < chargeopportunity)
	  {
	    if ( battery_power_good() )
	      now_parking = !charge_complete;
	  }
	leds_set(NONE);
	
	while(1)
	  {
	    leds_update_mood();
	    
	    if (battery_critical())
	      low_power();
	    
	    /* We may have finished charging */
	    if( charge_complete )
	      {
		food_level = 0;
		charge_complete = FALSE;
		now_parking = 0;
		
		/* reverse out of the charger */
		random_walk_disable();
		motor_r = motor_l = 6;
		motor_mode = MOTOR_BK;
		
		time_wait(5);
		continue;
	      }
		
	    /* Go to the charger if... */
		if( battery_low()
		    /* Or we've reached a defficiency of food */
		    || ( food_level > FOOD_THRESHOLD ) 
		    || ( now_parking )  ) 
		{
		  if (battery_low() )
		    mood = MOOD_DRIVING_TO_CHARGER_FLATBATT;
		  else if ( food_level > FOOD_THRESHOLD ) 
		    mood = MOOD_DRIVING_TO_CHARGER_NOFOOD;
		   
		  now_parking = !charge_complete;
		  parking_update();
		  continue;
		}

		/* Parking involves a static situation, which is incompatible 
		   with the watchdog - hence leave it here. */
		watchdog_update();

		if( hasfood() )
		{
		  mood = MOOD_GOT_FOOD;

			/* Are we at the light source? */
			if(light_intensity == 0)
			{
				/* Deposit food here */
			  mood = MOOD_AT_LAMP;
			  leds_update_mood();
				random_walk_disable();
				motor_r = motor_l = 6;
				motor_mode = MOTOR_BK;

				time_wait(10);
			}

			/* Do we have a reasonable bearing? */
			else if(bearing_strength > 10)
			{
				random_walk_disable();
				braitenberg_update();
			}
			else
			  /* Random Walk */
			  random_walk_enable();

		}
		else
		{
			/* Not got food, just do random walk */
		  mood = MOOD_NONE;
		  random_walk_enable();
		}
	}
}

void init(void)
{
	/* Disable the watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	/* GPIO: All inputs */
	P1DIR = P2DIR = P3DIR = P4DIR = 0;

	/* Use a 16 MHz clock (DCO) */
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 &= ~0x0f;
	BCSCTL1 |= 
		/*XT2O=0: XT2 is on*/
		/*XTS=0: LFXT1 mode select. 0 -Low frequency mode*/
		DIVA_0 /* ACLK Divider 0: /1 */
		|CALBC1_16MHZ; /* BCSCTL1 Calibration Data for 16MHz */

	BCSCTL2 = SELM_DCOCLK	/* MCLK from DCO */
		/* DIVMx = 0 : No MCLK divider */
		/* SELS = 0: SMCLK from DCO */
		/* DIVSx = 0 : No SMCLK divider */
		/* DCOR = 0 : DCO internal resistor */;

	BCSCTL3 = LFXT1S1; /*VLOCLK */
	
	flash_init();
	opamp1_init();
	bias_init();

	adc10_init(); /* The order here matters. This configures the ADC */
	random_init(); /* Grab some random data */

	
	net_rx_init();
	net_tx_init();
	ir_receive_init();
	ir_transmit_init();
	motor_init();
	leds_init();
	battery_init();
	food_init();

	eint();
	
/* 	virus_init(); */
}

void low_power(void)
{
	/* Use as little power as possible */
	random_walk_disable();

	motor_off();
	fled_off();
			
	/* IR Led off */
	P4OUT &= ~1;
	P4DIR |= 1;
	P4SEL &= ~1;
	
	dint();

	_BIS_SR(LPM3_bits);
	while(1);
}
