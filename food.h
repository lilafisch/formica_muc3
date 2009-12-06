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
#ifndef __FOOD_H
#define __FOOD_H
#include "device.h"
#include "types.h"

/* Called with ADC data */
void foodcallback(uint16_t dataoff, uint16_t dataon);

bool hasfood(void);

void food_init(void);

#define NOFOOD 1000
#define FLED (1<<3)

/* Turn the food leds on/off */
#define fled_on() do { P4OUT |= FLED; } while (0)
#define fled_off() do { P4OUT &= ~FLED; } while (0)

extern uint32_t food_level;

/* Add another food time from someone else */
void food_gotinfo( uint32_t *since );

#endif	/* __FOOD_H */
