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
/* Macros for configuring the IR receiver biasing */
#ifndef __IR_BIAS_H
#define __IR_BIAS_H
#include "device.h"

#define BIAS1 (1<<6)
#define BIAS2 (1<<7)

/* Must be called before ir-receive is initialised */
#define bias_init() do { P2DIR &= ~(BIAS1|BIAS2); P2SEL &= ~(BIAS1|BIAS2); } while (0)

/* Use bias resistor 1 - this is (high sensitivity?) for daylight bearing */
#define bias_use1() do { P2DIR &= ~(BIAS1|BIAS2); P2OUT |= BIAS1; P2DIR |= BIAS1; } while (0)

/* Use bias resistor 2  - this is (low sensitivity?) for IR reception*/
#define bias_use2() do { P2DIR &= ~(BIAS1|BIAS2); P2OUT |= BIAS2; P2DIR |= BIAS2; } while (0)

#endif	/* __IR_BIAS_H */
