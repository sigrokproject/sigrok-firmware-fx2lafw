/*
 * This file is part of the sigrok-firmware-fx2lafw project.
 *
 * Copyright (C) 2009 Ubixum, Inc.
 * Copyright (C) 2015 Jochen Hoenicke
 * Copyright (C) 2024 Steve Markgraf
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

#define SET_ANALOG_MODE()

#define SET_COUPLING(x) set_coupling_pso2020(x)

#define SET_CALIBRATION_PULSE(x)

#define TOGGLE_CALIBRATION_PIN()

#define LED_CLEAR() do { PA6 = 1; IOE |= (1 << 2); IOE &= ~(1 << 1); } while (0)
#define LED_GREEN() do { PA6 = 0; IOE |= (1 << 2); } while (0)
#define LED_RED()   do { PA6 = 1; IOE &= ~(1 << 2); } while (0)
#define LED_WHITE() do { IOE |= (1 << 1); } while (0)

#define MUX_S0 (1 << 2)
#define MUX_S1 (1 << 1)

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

/**
 * Control analog mux and relais to switch voltage ranges
 *
 */
static BOOL set_voltage(BYTE channel, BYTE val)
{
	BYTE bits = 0;
	BYTE mask = MUX_S0 | MUX_S1;

	/* ADC channel B is only used for interleaved sampling
	 * to get 96 MHz sampling rate, so it shares the
	 * analog input circuitry with channel A
	 * Instead of returning an error, silently ignore it */
	if (channel > 0)
		return TRUE;

	switch (val) {
	case 1:
		/* 10 and 20V */
		bits = MUX_S1;
		break;
	case 2:
		/* 5V */
		bits = MUX_S0;
		break;
	case 5:
		/* 2V */
		break;
	case 10:
		/* 1V */
		bits = MUX_S0 | MUX_S1;
		break;
	case 11:
		/* 500 mV */
		bits = MUX_S1;
		break;
	case 12:
		/* 200mV */
		bits = MUX_S0;
		break;
	case 13:
		/* 100mV */
		break;
	case 14:
		/* 20 and 50mV in original software */
		bits = MUX_S0 | MUX_S1;
		break;
	default:
		return FALSE;
	}

	/* switch relay for high voltage range */
	if (val <= 10)
		PA7 = 1;
	else
		PA7 = 0;

	IOC = (IOC & ~mask) | (bits & mask);

	return TRUE;
}

/**
 * Switch between AC and DC coupling
 */
static void set_coupling_pso2020(BYTE coupling_cfg)
{
	if (coupling_cfg & 0x01)
		PC4 = 0;
	else
		PC4 = 1;
}

#include <scope.inc>
