/*
 * This file is part of the sigrok-firmware-fx2lafw project.
 *
 * Copyright (C) 2009 Ubixum, Inc.
 * Copyright (C) 2015 Jochen Hoenicke
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <fx2macros.h>
#include <fx2ints.h>
#include <autovector.h>
#include <delay.h>
#include <setupdat.h>

#define SET_ANALOG_MODE() do { PA7 = 1; } while (0)

#define SET_COUPLING(x) set_coupling(x)

#define SET_CALIBRATION_PULSE(x) set_calibration_pulse(x)

/* Note: There's no PE2 as IOE is not bit-addressable (see TRM 15.2). */
#define TOGGLE_CALIBRATION_PIN() do { IOE = IOE ^ 0x04; } while (0)

#define LED_CLEAR() NOP
#define LED_GREEN() NOP
#define LED_RED()   NOP

#define TIMER2_VAL 500

/* CTLx pin index (IFCLK, ADC clock input). */
#define CTL_BIT 2

#define OUT0 ((1 << CTL_BIT) << 4) /* OEx = 1, CTLx = 0 */

static const struct samplerate_info samplerates[] = {
	{ 48, 0x80,   0, 3, 0, 0x00, 0xea },
	{ 30, 0x80,   0, 3, 0, 0x00, 0xaa },
	{ 24,    1,   0, 2, 1, OUT0, 0xea },
	{ 16,    1,   1, 2, 0, OUT0, 0xea },
	{ 15,    1,   0, 2, 1, OUT0, 0xaa },
	{ 12,    2,   1, 2, 0, OUT0, 0xea },
	{ 11,    1,   1, 2, 0, OUT0, 0xaa },
	{  8,    3,   2, 2, 0, OUT0, 0xea },
	{  6,    2,   2, 2, 0, OUT0, 0xaa },
	{  5,    3,   2, 2, 0, OUT0, 0xaa },
	{  4,    6,   5, 2, 0, OUT0, 0xea },
	{  3,    5,   4, 2, 0, OUT0, 0xaa },
	{  2,   12,  11, 2, 0, OUT0, 0xea },
	{  1,   24,  23, 2, 0, OUT0, 0xea },
	{ 50,   48,  47, 2, 0, OUT0, 0xea },
	{ 20,  120, 119, 2, 0, OUT0, 0xea },
	{ 10,  240, 239, 2, 0, OUT0, 0xea },
};

/**
 * The gain stage is 2 stage approach. -6dB and -20dB on the first stage
 * (attentuator). The second stage is then doing the gain by 3 different
 * resistor values switched into the feedback loop.
 *
 * #Channel 0:
 * PC1=1; PC2=0; PC3=0 -> Gain x0.1 = -20dB
 * PC1=1; PC2=0; PC3=1 -> Gain x0.2 = -14dB
 * PC1=1; PC2=1; PC3=0 -> Gain x0.4 =  -8dB
 * PC1=0; PC2=0; PC3=0 -> Gain x0.5 =  -6dB
 * PC1=0; PC2=0; PC3=1 -> Gain x1   =   0dB
 * PC1=0; PC2=1; PC3=0 -> Gain x2   =  +6dB
 *
 * #Channel 1:
 * PE1=1; PC4=0; PC5=0 -> Gain x0.1 = -20dB 
 * PE1=1; PC4=0; PC5=1 -> Gain x0.2 = -14dB
 * PE1=1; PC4=1; PC5=0 -> Gain x0.4 =  -8dB
 * PE1=0; PC4=0; PC5=0 -> Gain x0.5 =  -6dB
 * PE1=0; PC4=0; PC5=1 -> Gain x1   =   0dB
 * PE1=0; PC4=1; PC5=0 -> Gain x2   =  +6dB
 */
static BOOL set_voltage(BYTE channel, BYTE val)
{
	BYTE bits_C, bit_E, mask_C, mask_E;

	if (channel == 0) {
		mask_C = 0x0E;
		mask_E = 0x00;
		bit_E = 0;
		switch (val) {
		case 1:
			bits_C = 0x02;
			break;
		case 2:
			bits_C = 0x06;
			break;
		case 5:
			bits_C = 0x00;
			break;
		case 10:
			bits_C = 0x04;
			break;
		case 20:
			bits_C = 0x08;
			break;
		default:
			return FALSE;
		}
	} else if (channel == 1) {
		mask_C = 0x30;
		mask_E = 0x02;
		switch (val) {
		case 1:
			bits_C = 0x00;
			bit_E = 0x02; 
			break;
		case 2:
			bits_C = 0x10;
			bit_E = 0x02; 
			break;
		case 5:
			bits_C = 0x00;
			bit_E = 0x00; 
			break;
		case 10:
			bits_C = 0x10;
			bit_E = 0x00; 
			break;
		case 20:
			bits_C = 0x20;
			bit_E = 0x00; 
			break;
		default:
			return FALSE;
		}
	} else {
		return FALSE;
	}
	IOC = (IOC & ~mask_C) | (bits_C & mask_C);
	IOE = (IOE & ~mask_E) | (bit_E & mask_E);
		
	return TRUE;
}

#include <scope.inc>
