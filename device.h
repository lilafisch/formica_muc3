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
#ifndef __DEVICE_H
#define __DEVICE_H
/* These constants aren't defined for the MSP430F2234, but should be */
#define __msp430_have_opamp_1
#define __msp430_have_opamp_2
#define __msp430_have_opamp_feedback_taps
#define __msp430_have_opamp_rail_to_rail
#define __msp430_have_opamp_output_select

#include <io.h>

#endif  /* __DEVICE_H */
