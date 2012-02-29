/*
 * This file is part of the fx2lafw project.
 *
 * Copyright (C) 2011-2012 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

/*
 * fx2lafw is an open-source firmware for Cypress FX2 based logic analyzers.
 *
 * It is written in C, using fx2lib as helper library, and sdcc as compiler.
 * The code is licensed under the terms of the GNU GPL, version 2 or later.
 *
 * Technical notes:
 *
 *  - We use the FX2 in GPIF mode to sample the data (asynchronously).
 *  - We use the internal 48MHz clock for GPIF.
 *  - The 8 channels/pins we sample (the GPIF data bus) are PB0-PB7.
 *    Support for 16 channels is not yet included, but might be added later.
 *  - Endpoint 2 is used for data transfers from FX2 to host.
 *  - The endpoint is quad-buffered.
 *
 * Documentation:
 *
 *  - See http://sigrok.org/wiki/Fx2lafw
 */

#include <fx2regs.h>
#include <fx2macros.h>
#include <delay.h>
#include <setupdat.h>
#include <eputils.h>
#include <gpif.h>

#include <fx2lafw.h>
#include <gpif-acquisition.h>

/* Protocol commands */
#define CMD_SET_SAMPLERATE	0xb0
#define CMD_START		0xb1
#define CMD_STOP		0xb2
#define CMD_GET_FW_VERSION	0xb3

/* ... */
volatile bit got_sud;

static void setup_endpoints(void)
{
	/* Setup EP2 (IN). */
	EP2CFG = (1 << 7) |		  /* EP is valid/activated */
		 (1 << 6) |		  /* EP direction: IN */
		 (1 << 5) | (0 << 4) |	  /* EP Type: bulk */
		 (0 << 3) |		  /* EP buffer size: 512 */
		 (0 << 2) |		  /* Reserved. */
		 (0 << 1) | (0 << 0);	  /* EP buffering: quad buffering */
	SYNCDELAY();

	/* Setup EP6 (IN) in the debug build. */
#ifdef DEBUG
	EP6CFG = (1 << 7) |		  /* EP is valid/activated */
		 (1 << 6) |		  /* EP direction: IN */
		 (1 << 5) | (0 << 4) |	  /* EP Type: bulk */
		 (0 << 3) |		  /* EP buffer size: 512 */
		 (0 << 2) |		  /* Reserved */
		 (1 << 1) | (0 << 0);	  /* EP buffering: double buffering */
#else
	EP6CFG &= ~bmVALID;
#endif
	SYNCDELAY();

	/* Disable all other EPs (EP1, EP4, and EP8). */
	EP1INCFG &= ~bmVALID;
	SYNCDELAY();
	EP1OUTCFG &= ~bmVALID;
	SYNCDELAY();
	EP4CFG &= ~bmVALID;
	SYNCDELAY();
	EP8CFG &= ~bmVALID;
	SYNCDELAY();

	/* EP2: Reset the FIFOs. */
	/* Note: RESETFIFO() gets the EP number WITHOUT bit 7 set/cleared. */
	RESETFIFO(0x02)
#ifdef DEBUG
	/* Reset the FIFOs of EP6 when in debug mode. */
	RESETFIFO(0x06)
#endif

	/* EP2: Enable AUTOIN mode. Set FIFO width to 8bits. */
	EP2FIFOCFG = bmAUTOIN | ~bmWORDWIDE;
	SYNCDELAY();

	/* EP2: Auto-commit 512 (0x200) byte packets (due to AUTOIN = 1). */
	EP2AUTOINLENH = 0x02;
	SYNCDELAY();
	EP2AUTOINLENL = 0x00;
	SYNCDELAY();

	/* EP2: Set the GPIF flag to 'full'. */
	EP2GPIFFLGSEL = (1 << 1) | (0 << 1);
	SYNCDELAY();
}

BOOL handle_vendorcommand(BYTE cmd)
{
	/* Protocol implementation */

	switch (cmd) {
	case CMD_SET_SAMPLERATE:
		/* TODO */
		break;
	case CMD_START:
		gpif_acquisition_start();
		return TRUE;
	case CMD_STOP:
		GPIFABORT = 0xff;
		/* TODO */
		return TRUE;
		break;
	case CMD_GET_FW_VERSION:
		/* TODO */
		break;
	default:
		/* Unimplemented command. */
		break;
	}

	return FALSE;
}

BOOL handle_get_interface(BYTE ifc, BYTE *alt_ifc)
{
	/* We only support interface 0, alternate interface 0. */
	if (ifc != 0)
		return FALSE;

	*alt_ifc = 0;
	return TRUE;
}

BOOL handle_set_interface(BYTE ifc, BYTE alt_ifc)
{
	/* We only support interface 0, alternate interface 0. */
	if (ifc != 0 || alt_ifc != 0)
		return FALSE;
	
	/* Perform procedure from TRM, section 2.3.7: */

	/* (1) TODO. */

	/* (2) Reset data toggles of the EPs in the interface. */
	/* Note: RESETTOGGLE() gets the EP number WITH bit 7 set/cleared. */
	RESETTOGGLE(0x82);
#ifdef DEBUG
	RESETTOGGLE(0x86);
#endif

	/* (3) Restore EPs to their default conditions. */
	/* Note: RESETFIFO() gets the EP number WITHOUT bit 7 set/cleared. */
	RESETFIFO(0x02);
	/* TODO */
#ifdef DEBUG
	RESETFIFO(0x06);
#endif

	/* (4) Clear the HSNAK bit. Not needed, fx2lib does this. */

	return TRUE;
}

BYTE handle_get_configuration(void)
{
	/* We only support configuration 1. */
	return 1;
}

BOOL handle_set_configuration(BYTE cfg)
{
	/* We only support configuration 1. */
	return (cfg == 1) ? TRUE : FALSE;
}

void sudav_isr(void) interrupt SUDAV_ISR
{
	got_sud = TRUE;
	CLEAR_SUDAV();
}

void sof_isr(void) interrupt SOF_ISR using 1
{
	CLEAR_SOF();
}

void usbreset_isr(void) interrupt USBRESET_ISR
{
	handle_hispeed(FALSE);
	CLEAR_USBRESET();
}

void hispeed_isr(void) interrupt HISPEED_ISR
{
	handle_hispeed(TRUE);
	CLEAR_HISPEED();
}

void fx2lafw_init(void)
{
	/* Set DYN_OUT and ENH_PKT bits, as recommended by the TRM. */
	REVCTL = bmNOAUTOARM | bmSKIPCOMMIT;

	got_sud = FALSE;

	/* Renumerate. */
	RENUMERATE_UNCOND();

	SETCPUFREQ(CLK_48M);

	USE_USB_INTS();

	/* TODO: Does the order of the following lines matter? */
	ENABLE_SUDAV();
	ENABLE_SOF();
	ENABLE_HISPEED();
	ENABLE_USBRESET();

	/* Global (8051) interrupt enable. */
	EA = 1;

	/* Setup the endpoints. */
	setup_endpoints();

	/* Put the FX2 into GPIF master mode and setup the GPIF. */
	gpif_init_la();
}

void fx2lafw_run(void)
{
	if (got_sud) {
		handle_setupdata();
		got_sud = FALSE;
	}
}
