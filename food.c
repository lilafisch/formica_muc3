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
#include "types.h"
#include "food.h"
#include "time.h"
#include "leds.h"

#define AVERAGE_SIZE 3
#define FOOD_TRIGGER 10

static uint16_t average[AVERAGE_SIZE];
static bool gotfood = FALSE;

uint32_t food_level = 0;

void foodcallback(uint16_t dataoff, uint16_t dataon)
{
	static uint8_t averageindex = 0;
	uint8_t i;
	uint16_t data;

	/* Dataoff should always be higher than dataon */
	if(dataoff < dataon)
		return;
	data = dataoff - dataon;

	/* Save the data in the array */
	average[averageindex++] = data;
	if(averageindex == AVERAGE_SIZE ){
		averageindex = 0;
	}

	/*Calculating moving average*/
	uint16_t mavg = 0;
	for(i=0; i<AVERAGE_SIZE; i++)
		mavg += average[i];

	mavg = mavg/AVERAGE_SIZE;
	
	if(mavg < FOOD_TRIGGER)
	{
		gotfood = TRUE;
		food_level = 0;
	}
	else
		gotfood = FALSE;
}

bool hasfood(void)
{
	return gotfood;
}

void food_init(void)
{
	uint16_t i;
	for(i=0;i<AVERAGE_SIZE;i++)
		average[i] = 0;

	fled_off();
	P4DIR |= FLED;
}

void food_gotinfo( uint32_t *since )
{
  /* average our food level with that received from another robot */
  food_level = (food_level + *since) >> 1;
  tempmood = MOOD_HEARD_ABOUT_FOOD;
}

