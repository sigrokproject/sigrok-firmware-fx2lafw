/*
 * This file is part of the sigrok-firmware-fx2lafw project.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
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
 *  - The 8 channels/pins we sample (the GPIF data bus) are PB0-PB7,
 *    or PB0-PB7 + PD0-PD7 for 16-channel sampling. 
 *  - Endpoint 2 (quad-buffered) is used for data transfers from FX2 to host.
 *
 * Documentation:
 *
 *  - See http://sigrok.org/wiki/Fx2lafw
 */

#include <fx2regs.h>
#include <fx2macros.h>
#include <fx2ints.h>
#include <delay.h>
#include <setupdat.h>
#include <eputils.h>
#include <gpif.h>
#include <command.h>
#include <fx2lafw.h>
#include <gpif-acquisition.h>

/* ... */
volatile __bit got_sud;
BYTE vendor_command;

volatile WORD ledcounter = 1000;

extern __bit gpif_acquiring;

static void setup_endpoints(void)
{
	/* Setup EP2 (IN). */
	EP2CFG = (1 << 7) |		  /* EP is valid/activated */
		 (1 << 6) |		  /* EP direction: IN */
		 (1 << 5) | (0 << 4) |	  /* EP Type: bulk */
		 (1 << 3) |		  /* EP buffer size: 1024 */
		 (0 << 2) |		  /* Reserved. */
		 (0 << 1) | (0 << 0);	  /* EP buffering: quad buffering */
	SYNCDELAY();

	/* Disable all other EPs (EP1, EP4, EP6, and EP8). */
	EP1INCFG &= ~bmVALID;
	SYNCDELAY();
	EP1OUTCFG &= ~bmVALID;
	SYNCDELAY();
	EP4CFG &= ~bmVALID;
	SYNCDELAY();
	EP6CFG &= ~bmVALID;
	SYNCDELAY();
	EP8CFG &= ~bmVALID;
	SYNCDELAY();

	/* EP2: Reset the FIFOs. */
	/* Note: RESETFIFO() gets the EP number WITHOUT bit 7 set/cleared. */
	RESETFIFO(0x02)

	/* EP2: Enable AUTOIN mode. Set FIFO width to 8bits. */
	EP2FIFOCFG = bmAUTOIN;
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

static void send_fw_version(void)
{
	/* Populate the buffer. */
	struct version_info *const vi = (struct version_info *)EP0BUF;
	vi->major = FX2LAFW_VERSION_MAJOR;
	vi->minor = FX2LAFW_VERSION_MINOR;

	/* Send the message. */
	EP0BCH = 0;
	EP0BCL = sizeof(struct version_info);
}

static void send_revid_version(void)
{
	uint8_t *p;

	/* Populate the buffer. */
	p = (uint8_t *)EP0BUF;
	*p = REVID;

	/* Send the message. */
	EP0BCH = 0;
	EP0BCL = 1;
}

BOOL handle_vendorcommand(BYTE cmd)
{
	/* Protocol implementation */
	switch (cmd) {
	case CMD_START:
		vendor_command = cmd;
		EP0BCL = 0;
		return TRUE;
		break;
	case CMD_GET_FW_VERSION:
		send_fw_version();
		return TRUE;
		break;
	case CMD_GET_REVID_VERSION:
		send_revid_version();
		return TRUE;
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

	/* (3) Restore EPs to their default conditions. */
	/* Note: RESETFIFO() gets the EP number WITHOUT bit 7 set/cleared. */
	RESETFIFO(0x02);
	/* TODO */

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

void sudav_isr(void) __interrupt SUDAV_ISR
{
	got_sud = TRUE;
	CLEAR_SUDAV();
}

void usbreset_isr(void) __interrupt USBRESET_ISR
{
	handle_hispeed(FALSE);
	CLEAR_USBRESET();
}

void hispeed_isr(void) __interrupt HISPEED_ISR
{
	handle_hispeed(TRUE);
	CLEAR_HISPEED();
}

void timer2_isr(void) __interrupt TF2_ISR
{
	/* Blink LED during acquisition, keep it on otherwise. */
	if (gpif_acquiring) {
		if (--ledcounter == 0) {
			PA1 = !PA1;
			ledcounter = 1000;
		}
	} else {
		PA1 = 1; /* LED on. */
	}
	TF2 = 0;
}

void fx2lafw_init(void)
{
	/* Set DYN_OUT and ENH_PKT bits, as recommended by the TRM. */
	REVCTL = bmNOAUTOARM | bmSKIPCOMMIT;

	got_sud = FALSE;
	vendor_command = 0;

	/* Renumerate. */
	RENUMERATE_UNCOND();

	SETCPUFREQ(CLK_48M);

	USE_USB_INTS();

	/* TODO: Does the order of the following lines matter? */
	ENABLE_SUDAV();
	ENABLE_HISPEED();
	ENABLE_USBRESET();

	/* PA1 (LED) is an output. */
	PORTACFG = 0;
	OEA = (1 << 1);
	PA1 = 1; /* LED on. */

	/* Init timer2. */
	RCAP2L = -500 & 0xff;
	RCAP2H = (-500 & 0xff00) >> 8;
	T2CON = 0;
	ET2 = 1;
	TR2 = 1;

	/* Global (8051) interrupt enable. */
	EA = 1;

	/* Setup the endpoints. */
	setup_endpoints();

	/* Put the FX2 into GPIF master mode and setup the GPIF. */
	gpif_init_la();
}

void fx2lafw_poll(void)
{
	if (got_sud) {
		handle_setupdata();
		got_sud = FALSE;
	}

	if (vendor_command) {
		switch (vendor_command) {
		case CMD_START:
			if ((EP0CS & bmEPBUSY) != 0)
				break;

			if (EP0BCL == sizeof(struct cmd_start_acquisition)) {
				gpif_acquisition_start(
				 (const struct cmd_start_acquisition *)EP0BUF);
			}

			/* Acknowledge the vendor command. */
			vendor_command = 0;
			break;
		default:
			/* Unimplemented command. */
			vendor_command = 0;
			break;
		}
	}

	gpif_poll();
}

void main(void)
{
	fx2lafw_init();
	while (1)
		fx2lafw_poll();
}
