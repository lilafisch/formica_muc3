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
#ifndef __LEDS_H
#define __LEDS_H
#include "device.h"

void leds_flash(uint8_t colour);
void leds_set(uint8_t colour);
void leds_update_mood();

#define RED (1<<6)
#define GREEN (1<<7)
#define ORANGE (RED | GREEN)
#define NONE (0)

#define leds_init() do { P4OUT |= RED | GREEN; P4DIR |= (RED | GREEN); } while (0)

typedef enum {
	MOOD_NONE,
	MOOD_DRIVING_TO_CHARGER_NOFOOD,
	MOOD_DRIVING_TO_CHARGER_FLATBATT,
	MOOD_CHARGING,
	MOOD_GOT_FOOD,
	MOOD_AT_LAMP,
	MOOD_HEARD_ABOUT_FOOD,
	MOOD_CHARGED,
	MOOD_BORED_CHARGING,
	MOOD_AM_STUCK
} mood_t;

extern mood_t mood;
extern mood_t tempmood;

#define leds_red_on() do { P4OUT &= ~RED; } while (0)
#define leds_red_off() do { P4OUT |= RED; } while (0)

#define leds_green_on() do { P4OUT &= ~GREEN; } while (0)
#define leds_green_off() do { P4OUT |= GREEN; } while (0)

#define leds_off() do { P4OUT |= (RED | GREEN); } while (0)

#endif	/* __LEDS_H */
