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
#ifndef __FREQ_H
#define __FREQ_H

#define NBITS _NBITS
#define NFREQ _NFREQ
#define MIN_PERIOD _MIN_PERIOD
#define MAX_PERIOD _MAX_PERIOD

#define PERIOD_SPACING ((MAX_PERIOD - MIN_PERIOD)/(NFREQ-1))
#define RANGE (PERIOD_SPACING/2)

#define SYMBOLS_PER_BYTE _SYMBOLS_PER_BYTE

#define SYM_MASK ((1 << NBITS)-1)

const uint16_t period_lut[_NFREQ];

#endif	/* __FREQ_H */
