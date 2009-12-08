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
#include "ir-rx.h"
#include "ir.h"
#include "ir-bias.h"
#include "device.h"
#include "freq.h"
#include "net-rx.h"
#include "types.h"
#include <signal.h>
#include "smbus_pec.h"

/* Number of samples to average out */
#define AVERAGE 2

/* The stability threshold for when to accept a new frequency */
#define STABLE_THRESH (110 * AVERAGE)

#define timera_en() do { TACTL |= MC_CONT; } while(0)

static uint8_t cb_pos = 0;
static uint8_t curbyte = 0;

/* The last symbol received */
static uint8_t last_sym = INV_SYM;

#define DATA_LEN 32
static uint8_t data[DATA_LEN];
static uint8_t d_pos = 0;

/* Debug flag pins */
/* #define debug_symbol_lock() do { P1OUT |= 1; } while (0) */
/* #define debug_symbol_nolock() do { P1OUT &= ~1; } while (0) */
/* #define debug_flag_flip() do { P1OUT ^= 2; } while (0) */
#define debug_symbol_lock() do { ; } while (0)
#define debug_symbol_nolock() do { ; } while (0)
#define debug_flag_flip() do { ; } while (0)

/* Resets/initialises the bit decoder.
   For calling on initialisation or when the signal is lost. */
static void decoder_reset( void );

/* Decode the new data.
   'period' is period that's just been received. */
static inline void decoder_newdata( uint16_t period );

void ir_receive_init( void )
{
	/*** Configure Timer A ***/
	TACTL = TASSEL_SMCLK	/* SMCLK clock source */
		/* No clock divider */
		| MC_STOP	/* Timer stopped for now */
		/* Don't reset the timer */
		| TAIE		/* Interrupt enabled */;

	/* Clear the timer */
	TAR = 0;

	/* Set up CCR1 to trigger off the comparator */
	TACCTL1 = CM_POS	/* Trigger on positive edges */
		| CCIS_0	/* CCI0A trigger source (COMP) */
		| SCS		/* Synchronize with timer clock */
		| CAP		/* Capture mode */
		| CCIE;		/* Interrupt enabled */

	/* Set P1.2 to be the CCR1 input CCI0A */
	/* Page 57 of the MSP430F2234 datasheet */
	P1DIR &= ~0x04;
	P1SEL |= 0x04;

	/* Make the PD1,2,3 signals low outputs, so they are clamped when */
	/* their analogue channels are deselected. This prevents photocurrent */
	/* from flowing into the battery measurement input. Insane! */
	P2DIR |= 0x0f;
	P2OUT &= 0xf0;

	/*** Bias resistor config ***/
	bias_comms();

	decoder_reset();

	/* Start the timer */
	timera_en();
}

interrupt (TIMERA1_VECTOR) timer_a_isr(void)
{
	uint16_t taiv_l = TAIV;
	/* Number of times the timer has wrapped since the last edge */
	static uint8_t nwraps = 0;

	switch( taiv_l )
	{
		/* CCR1: */
	case 0x02: {
		/* Previous periods */
		static int16_t prev[AVERAGE];
		/* Position in the prev buffer */
		static uint8_t pos = 0;
		/* The last recorded period */
		static uint16_t last_period = 0;
		uint16_t reading = TACCR1;

		/* The last TACCR1 reading: */
		static uint16_t last_taccr1 = 0;
		uint32_t sum = 0;
		uint8_t i;

		/* The timer may have wrapped around since the last event */
		if( nwraps == 0 )
			reading -= last_taccr1;
		else if( nwraps == 1 )
			/* The timer wrapper - compensate */
			reading = ((uint32_t)(reading) + 0xffff) - last_taccr1;
		last_taccr1 = TACCR1;

		/* Currently triggering on positive edges: */
		reading >>= 1;
		/* Reset the receiver if the recorded period was too long */
		if( nwraps > 1 || reading > (MAX_PERIOD + RANGE) ) {
			/* Lost the signal */
			decoder_reset();
			last_period = 0;
		}

		/*** Determine whether we've reached a stable period ***/
		/* Store the difference between this period and the last recorded */
		prev[pos] = reading - last_period;
		pos = (pos == AVERAGE-1)?0:pos+1;

		/* Sum the absolute values of the last AVERAGE derivatives */
		for(i=0; i<AVERAGE; i++)
			if( prev[i] < 0 )
				sum += (-prev[i]);
			else
				sum += prev[i];

		/* Only accept this frequency if it's stable */
		if( sum < STABLE_THRESH ) {
			/* Got a stable frequency */
			/* It might be the same as the last, but we can't tell until it's been
			   decoded as a symbol */
			decoder_newdata( reading );
			debug_symbol_lock();

		}
		else {
			debug_symbol_nolock();
		}

		last_period = reading;
		nwraps = 0;
		break;
	}

		/* Timer overflow */
	case 0x0A: {
		nwraps++;
		break;
	}

	default:
		/* Should never get here. */
		nop();
	}

}

interrupt (TIMERA0_VECTOR) timer_a1_isr(void)
{

}

static void decoder_reset( void )
{
	cb_pos = 0;
	last_sym = INV_SYM;
}

static inline void decoder_newdata( uint16_t period )
{
	uint8_t sym;

	/* Check that the frequency reaches the minimum */
	if( period < (period_lut[0] - RANGE) ) {
		decoder_reset();
		return;
	}

	for( sym=0;
	     sym < NFREQ;
	     sym++ )
		if( period < (period_lut[sym] + RANGE) )
			break;

	if( sym == NFREQ ) {
		/* Invalid frequency - reset the decoder */
		decoder_reset();
		return;
	}

	/* Ignore repeated symbols */
	if( sym == last_sym )
		return;

	/*** Decode the symbol ***/

	if( sym == 0 ) {
		cb_pos = 0;
		curbyte = 0;

		last_sym = 0;
		return;
	}

	/* Remove the step over the last symbol */
	if( sym > last_sym ) {
		last_sym = sym;
		sym--;
	} else
		last_sym = sym;
	
	/* Start of the byte? */
	sym--;

	curbyte |= sym << (cb_pos * NBITS);
	cb_pos++;

	if( cb_pos == SYMBOLS_PER_BYTE ) {
		static uint8_t checksum;
		static bool escaped = FALSE;

		/* We now have a full byte */
		cb_pos = 0;

		if( curbyte == 0x7e ) {
			/* Start of frame */
			d_pos = 0;
			checksum = 0;
			data[0] = 0x7e;
			escaped = FALSE;
		} else if( data[0] == 0x7e ) {
			if( curbyte == 0x7d ) {
				escaped = TRUE;
				return;
			}
			else if( escaped ) {
				curbyte ^= 0x20;
				escaped = FALSE;
			}				

			data[d_pos] = curbyte;


			/* data[1] is the length if we've got that far */
			if( d_pos > 1 && d_pos == (data[1] + 2) ) {
				/* Frame reception complete -- check the checksum */
				if( checksum == curbyte )
					net_rx_proc_incoming( data+2, data[1] );
			}
			else
				checksum_add( checksum, curbyte );

		}

		if( data[0] == 0x7e )
			d_pos = (d_pos==(DATA_LEN-1))?0:(d_pos+1);

	}
}

void ir_receive_en( void )
{
	/* Enable the capture interrupt */
	TACCTL1 |= CCIE;
}

void ir_receive_dis( void )
{
	/* Disable the capture interrupt */
	TACCTL1 &= ~CCIE;
}
