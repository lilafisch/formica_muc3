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
#include "battery.h"
#include "device.h"
#include "stdint.h"
#include "types.h"

#define PG (1<<3)
#define STAT1 (1<<1)
#define STAT2 (1<<2)

#define get_pg() ( P3IN & PG )
#define get_stat1() (P3IN & STAT1)
#define get_stat2() (P3IN & STAT2)

uint16_t battval = 65000;
bool pg_inverted = FALSE;

bool battery_low( void )
{
	/* Low = 3.5V. 1.75 out of divider, 1.75/2.5*1023 = 716 */
	/* tweaked slightly by experiment */
	return battval < 721 ? TRUE : FALSE;
}

bool battery_critical( void )
{
	/* Low = 3.3V. 1.65 out of divider, 1.65/2.5*1023 = 675 */
	/* tweaked slightly by experiment */
	return battval < 684 ? TRUE : FALSE;
}

void battery_init( void )
{
	P3DIR &= ~(PG | STAT1 | STAT2);
	P3REN |= (PG | STAT1 | STAT2);
	P3OUT |= (PG | STAT1 | STAT2);

	/* some robots have a hardware fault; !PG signal is inverted  */
	/* due to damaged charge controller! So at boot up (not connected */
	/* to charger) measure the !PG state. */

	if ( !get_pg() )
		pg_inverted = TRUE;
}

bool battery_charge_complete( void )
{
	/* stat1 = high, stat2 = low */
	if( get_stat1() && (!get_stat2()) )
		return TRUE;
	else
		return FALSE;		       
}

bool battery_charge_in_progress( void )
{
	/* stat1 = low, stat2 = high */
	if( (!get_stat1()) && get_stat2() )
		return TRUE;
	else
		return FALSE;
}

bool battery_charge_standby( void )
{
	/* stat1 = high, stat2 = high */
	if( ( get_stat1()) && get_stat2() )
		return TRUE;
	else
		return FALSE;
}

bool battery_power_good( void )
{
	bool retval = FALSE;
  
	if( !get_pg() )
		retval  =  TRUE;
	else
		retval = FALSE;
  
	if (pg_inverted)
		retval = !retval;

	return retval;
}
