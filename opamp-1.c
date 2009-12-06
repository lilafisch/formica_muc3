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
#include "opamp-1.h"
#include "device.h"


void opamp1_init( void )
{
	OA1CTL0 = /* OANx = Don't care */
		OAP_2		/* OA1I2 to non-inverting input */
		| OAPM_3//OAPM_3	/* Fast slew rate */
		| OAADC0;	/* OA1OUT to external pins (and some ADC inputs) */

	OA1CTL1 =  OAFBR_2	/* Tap 2 - 8R/8R */
		| OAFC_3	/* Comparator mode */
		/* OANEXT = 0 -- inverting input not externally available */
		| OARRIP;

	/* Put the output on P4.4 */
	ADC10AE |= 0x20;

	/* Configure the input pins */
	/* RX is connected to P3.7 (OA1I2) */
	/* See page 70 of the MSP430F2234 datasheet */
	ADC10AE |= 0x80;
}
