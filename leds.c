#include "leds.h"
#include "time.h"
#include "device.h"

/* produce a single flash of a colour, as defined in leds.h */

mood_t mood = MOOD_NONE;
mood_t tempmood = MOOD_NONE;
#define FLASHTIME 3

void leds_flash(uint8_t colour)
{
	uint8_t prev_led_state = 0;

	prev_led_state = P4OUT & 0x06;
	P4OUT &= ~(0x06);
	P4OUT |= (colour & 0x06);
	time_wait(FLASHTIME);
	P4OUT &= ~(0x06);
	P4OUT |= prev_led_state;
}

void leds_set(uint8_t colour)
{
	P4OUT &= ~ORANGE;
	P4OUT |= (colour & ORANGE);
}

void leds_update_mood()
{
	static mood_t oldmood;
	static uint32_t temptime=0;

	if (tempmood != MOOD_NONE)
	{
		oldmood = mood;
		mood = tempmood;

		if (temptime == 0)
			temptime = the_time + FLASHTIME;
		else if (the_time > temptime)
		{
			mood = oldmood;
			tempmood = MOOD_NONE;
			temptime = 0;
		}
	}
    
	switch (mood)
	{
	case MOOD_NONE:
		leds_set(NONE);
		break;
	case MOOD_DRIVING_TO_CHARGER_NOFOOD:
		leds_flash(GREEN);
		leds_flash(NONE);
		break;
	case MOOD_DRIVING_TO_CHARGER_FLATBATT:
		leds_flash(RED);
		leds_flash(NONE);
		break;
	case MOOD_CHARGING:
		leds_flash(ORANGE);
		leds_flash(NONE);
		break;
	case MOOD_GOT_FOOD:
		leds_set(GREEN);
		break;
	case MOOD_AT_LAMP:
		leds_set(RED);
		break;
	case MOOD_HEARD_ABOUT_FOOD:
		leds_flash(ORANGE);
		break;
	case MOOD_CHARGED:
		leds_flash(GREEN);
		leds_flash(NONE);
		break;
	case MOOD_BORED_CHARGING:
		leds_flash(RED);
		leds_flash(NONE);
		break;
	case MOOD_AM_STUCK:
		leds_flash(RED);
		leds_flash(NONE);
		break;

	default:
		mood = MOOD_NONE;
		leds_set(NONE);
	}
}



