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
#ifndef __BATTERY_H
#define __BATTERY_H
#include "types.h"
#include "stdint.h"

void battery_init( void );

bool battery_charge_complete( void );
bool battery_charge_in_progress( void );
bool battery_power_good( void );
bool battery_charge_standby( void );

bool battery_critical( void );
bool battery_low( void );

extern uint16_t battval;

#endif	/* __BATTERY_H */
