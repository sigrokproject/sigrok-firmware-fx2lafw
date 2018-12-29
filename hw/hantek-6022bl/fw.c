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

#define SET_COUPLING(x)

#define SET_CALIBRATION_PULSE(x)

#define TOGGLE_CALIBRATION_PIN() do { PC2 = !PC2; } while (0)

#define LED_CLEAR() do { PC0 = 1; PC1 = 1; } while (0)
#define LED_GREEN() do { PC0 = 1; PC1 = 0; } while (0)
#define LED_RED()   do { PC0 = 0; PC1 = 1; } while (0)

#define TIMER2_VAL 500

/* CTLx pin index (IFCLK, ADC clock input). */
#define CTL_BIT 0

#define OUT0 ((1 << CTL_BIT) << 4) /* OEx = 1, CTLx = 0 */

static const struct samplerate_info samplerates[] = {
	{ 48, 0x80,   0, 3, 0, 0x00, 0xea },
	{ 30, 0x80,   0, 3, 0, 0x00, 0xaa },
	{ 24,    1,   0, 2, 1, OUT0, 0xca },
	{ 16,    1,   1, 2, 0, OUT0, 0xca },
	{ 12,    2,   1, 2, 0, OUT0, 0xca },
	{  8,    3,   2, 2, 0, OUT0, 0xca },
	{  4,    6,   5, 2, 0, OUT0, 0xca },
	{  2,   12,  11, 2, 0, OUT0, 0xca },
	{  1,   24,  23, 2, 0, OUT0, 0xca },
	{ 50,   48,  47, 2, 0, OUT0, 0xca },
	{ 20,  120, 119, 2, 0, OUT0, 0xca },
	{ 10,  240, 239, 2, 0, OUT0, 0xca },
};

/*
 * This sets three bits for each channel, one channel at a time.
 * For channel 0 we want to set bits 1, 2 & 3
 * For channel 1 we want to set bits 4, 5 & 6
 *
 * We convert the input values that are strange due to original
 * firmware code into the value of the three bits as follows:
 *
 * val -> bits
 * 1  -> 010b
 * 2  -> 001b
 * 5  -> 000b
 * 10 -> 011b
 *
 * The third bit is always zero since there are only four outputs connected
 * in the serial selector chip.
 *
 * The multiplication of the converted value by 0x24 sets the relevant bits in
 * both channels and then we mask it out to only affect the channel currently
 * requested.
 */
static BOOL set_voltage(BYTE channel, BYTE val)
{
	BYTE bits, mask;

	switch (val) {
	case 1:
		bits = 0x02;
		break;
	case 2:
		bits = 0x01;
		break;
	case 5:
		bits = 0x00;
		break;
	case 10:
		bits = 0x03;
		break;
	default:
		return FALSE;
	}

	bits = bits << (channel ? 4 : 1);
	mask = (channel) ? 0x70 : 0x0e;
	IOA = (IOA & ~mask) | (bits & mask);

	return TRUE;
}

#include <scope.inc>
