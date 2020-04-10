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

#define SET_ANALOG_MODE() do { PA7 = 1; } while (0) //suport for OE ISDS205C, without touch scope.inc

#define SET_COUPLING(x) set_coupling_ISDS205(x)

#define SET_CALIBRATION_PULSE(x)

#define TOGGLE_CALIBRATION_PIN() do { PA0 = !PA0; } while (0)

#define LED_CLEAR() do { __asm nop __endasm;; } while (0) //evade evelin and his dog
#define LED_GREEN()
#define LED_RED()

#define TIMER2_VAL 500

/* CTLx pin index (IFCLK, ADC clock input). */
#define CTL_BIT 0

#define OUT0 ((1 << CTL_BIT) << 4) /* OEx = 1, CTLx = 0 */

#define SRCLK PA6
#define SRIN PA4
#define STOCLK PA5

volatile BYTE vol_state = 0 ; // ISDS205C uses 74HC595 to expand ports, here we save his actual state.

static void drive_74HC595(BYTE bits)
{
	BYTE i;

	for(i=0; i<8; i++)
	{
		SRCLK = 0;
		SRIN = bits>>7;
		SRCLK = 1;
		bits=bits<<1;
	}
	STOCLK = 1;
	STOCLK = 0;
}

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
 * For channel 0 we want to set bits 0, 1 & 6
 * For channel 1 we want to set bits 2, 3 & 4
 *
 * we set directly the relevant bits in
 * both channels and then we mask it out to only affect the channel currently
 * requested.
 */
static BOOL set_voltage(BYTE channel, BYTE val)
{
	BYTE bits, mask;

	switch (val) {
	case 1:
		bits = 0b00001001;	//1V
		break;
	case 2:
		bits = 0b00000110;	//500mV
		break;
	case 3:
		bits = 0b00000000;	//200mV
		break;
	case 4:
		bits = 0b01010110;	//100mV
		break;
	case 5:
		bits = 0b01010000;	//20mV
		break;
	case 10:
		bits = 0b01011111;	//10mV
		break;
	default:
		return FALSE;
	}

	mask = (channel) ? 0b00011100 : 0b01000011;
	vol_state = (vol_state & ~mask) | (bits & mask);
	//bits=vol_state;
		
	drive_74HC595(vol_state);

	return TRUE;
}

/**
 * Bits 5 & 7 of the byte controls the coupling per channel.
 *
 * Setting bit 7 enables AC coupling relay on CH0.
 * Setting bit 5 enables AC coupling relay on CH1.
 */
static void set_coupling_ISDS205(BYTE coupling_cfg)
{
	if (coupling_cfg & 0x01)
		vol_state &= ~0x80;
	else
		vol_state |= 0x80;

	if (coupling_cfg & 0x10)
		vol_state &= ~0x20;
	else
		vol_state |= 0x20;
		
	drive_74HC595(vol_state);
}

#include <scope.inc>
