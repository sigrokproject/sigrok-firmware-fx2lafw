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
#include <autovector.h>
#include <setupdat.h>
#include <eputils.h>
#include <gpif.h>

#define SYNCDELAY() SYNCDELAY4

/* ... */
volatile bit got_sud;

/* GPIF terminology: DP = decision point, NDP = non-decision-point */

/*
 * GPIF waveforms.
 *
 * See section "10.3.4 State Instructions" in the TRM for details.
 */
static const BYTE wavedata[128] = {
	/* Waveform 0: */

	/*
	 * This is the basic algorithm implemented in our GPIF state machine:
	 *
	 * State 0: NDP: Sample the FIFO data bus.
	 * State 1: DP: If EP2 is full, go to state 7 (the IDLE state), i.e.,
	 *          end the current waveform. Otherwise, go to state 0 again,
	 *          i.e., sample data until EP2 is full.
	 * State 2: Unused.
	 * State 3: Unused.
	 * State 4: Unused.
	 * State 5: Unused.
	 * State 6: Unused.
	 */

	/* S0-S6: LENGTH/BRANCH */
	/*
	 * For NDPs (LENGTH): Number of IFCLK cycles to stay in this state.
	 * For DPs (BRANCH): [7] ReExec, [5:3]: BRANCHON1, [2:0]: BRANCHON0.
	 *
	 * 0x01: Stay one IFCLK cycle in this state.
	 * 0x38: No Re-execution, BRANCHON1 = state 7, BRANCHON0 = state 0.
	 */
	0x01, 0x38, 0x01, 0x01, 0x01, 0x01, 0x01,
	/* TRM says "reserved", but GPIF designer always puts a 0x07 here. */
	0x07,

	/* S0-S6: OPCODE */
	/*
	 * 0x02: NDP, sample the FIFO data bus.
	 * 0x01: DP, don't sample the FIFO data bus.
	 */
	0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* Reserved */
	0x00,

	/* S0-S6: OUTPUT */
	/* Unused, we don't output anything, we only sample the pins. */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* Reserved */
	0x00,

	/* S0-S6: LOGIC FUNCTION (not used for NDPs) */
	/*
	 * 0x36: LFUNC = "A AND B", A = FIFO flag, B = FIFO flag.
	 * The FIFO flag (FF == full flag, in our case) is configured via
	 * EP2GPIFFLGSEL.
	 *
	 * So: If the EP2 FIFO is full and the EP2 FIFO is full, go to
	 * the state specified by BRANCHON1 (state 7), otherwise BRANCHON0
	 * (state 0). See the LENGTH/BRANCH value above for details.
	 */
	0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* TRM says "reserved", but GPIF designer always puts a 0x3f here. */
	0x3f,

	/* TODO: Must unused waveforms be "valid"? */

	/* Waveform 1 (unused): */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	/* Waveform 2 (unused): */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	/* Waveform 3 (unused): */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static void gpif_setup_registers(void)
{
	/* TODO. Value probably irrelevant, as we don't use RDY* signals? */
	GPIFREADYCFG = 0;

	/*
	 * Set TRICTL = 0, thus CTL0-CTL5 are CMOS outputs.
	 * TODO: Probably irrelevant, as we don't use CTL0-CTL5?
	 */
	GPIFCTLCFG = 0;

	/* When GPIF is idle, tri-state the data bus. */
	/* Bit 7: DONE, bit 0: IDLEDRV. TODO: Set/clear DONE bit? */
	GPIFIDLECS = (1 << 0);

	/* When GPIF is idle, set CTL0-CTL5 to 0. */
	GPIFIDLECTL = 0;

	/*
	 * Map index 0 in wavedata[] to FIFORD. The rest is assigned too,
	 * but not used by us.
	 *
	 * GPIFWFSELECT: [7:6] = SINGLEWR index, [5:4] = SINGLERD index,
	 *               [3:2] = FIFOWR index, [1:0] = FIFORD index
	 */
	GPIFWFSELECT = (0x3 << 6) | (0x2 << 4) | (0x1 << 2) | (0x0 << 0);

	/* Contains RDY* pin values. Read-only according to TRM. */
	GPIFREADYSTAT = 0;
}

static void gpif_write_waveforms(void)
{
	int i;

	/*
	 * Write the four waveforms into the respective WAVEDATA register
	 * locations (0xe400 - 0xe47f) using the FX2's autopointer feature.
	 */
	AUTOPTRSETUP = 0x07;             /* Increment autopointers 1 & 2. */
	AUTOPTRH1 = MSB((WORD)wavedata); /* Source is the 'wavedata' array. */
	AUTOPTRL1 = LSB((WORD)wavedata);
	AUTOPTRH2 = 0xe4;                /* Dest is WAVEDATA (0xe400). */
	AUTOPTRL2 = 0x00;
	for (i = 0; i < 128; i++)
		EXTAUTODAT2 = EXTAUTODAT1;
}

static void gpif_init_addr_pins(void)
{
	/*
	 * Configure the 9 GPIF address pins (GPIFADR[8:0], which consist of
	 * PORTC[7:0] and PORTE[7]), and output an initial address (zero).
	 * TODO: Probably irrelevant, the 56pin FX2 has no ports C and E.
	 */
	PORTCCFG = 0xff;    /* Set PORTC[7:0] as alt. func. (GPIFADR[7:0]). */
	OEC = 0xff;         /* Configure PORTC[7:0] as outputs. */
	PORTECFG |= 0x80;   /* Set PORTE[7] as alt. func. (GPIFADR[8]). */
	OEE |= 0x80;        /* Configure PORTE[7] as output. */
	SYNCDELAY();
	GPIFADRL = 0x00;    /* Clear GPIFADR[7:0]. */
	SYNCDELAY();
	GPIFADRH = 0x00;    /* Clear GPIFADR[8]. */
}

static void gpif_init_flowstates(void)
{
	/* Clear all flowstate registers, we don't use this functionality. */
	FLOWSTATE = 0;
	FLOWLOGIC = 0;
	FLOWEQ0CTL = 0;
	FLOWEQ1CTL = 0;
	FLOWHOLDOFF = 0;
	FLOWSTB = 0;
	FLOWSTBEDGE = 0;
	FLOWSTBHPERIOD = 0;
}

static void gpif_init_la(void)
{
	/*
	 * Setup the FX2 in GPIF master mode, using the internal clock
	 * (non-inverted) at 48MHz, and using async sampling.
	 */
	IFCONFIG = 0xee;

	/* Abort currently executing GPIF waveform (if any). */
	GPIFABORT = 0xff;

	/* Setup the GPIF registers. */
	gpif_setup_registers();

	/* Write the four GPIF waveforms into the WAVEDATA register. */
	gpif_write_waveforms();

	/* Initialize GPIF address pins, output initial values. */
	gpif_init_addr_pins();

	/* Initialize flowstate registers (not used by us). */
	gpif_init_flowstates();
}

static void setup_endpoints(void)
{
	/* Setup EP1 (OUT). */
	EP1OUTCFG = (1 << 7) |		  /* EP is valid/activated */
		    (0 << 6) |		  /* Reserved */
		    (1 << 5) | (0 << 4) | /* EP Type: bulk */
		    (0 << 3) |		  /* Reserved */
		    (0 << 2) |		  /* Reserved */
		    (0 << 1) | (0 << 0);  /* Reserved */
	SYNCDELAY();

	/* Setup EP2 (IN). */
	EP2CFG = (1 << 7) |		  /* EP is valid/activated */
		 (1 << 6) |		  /* EP direction: IN */
		 (1 << 5) | (0 << 4) |	  /* EP Type: bulk */
		 (0 << 3) |		  /* EP buffer size: 512 */
		 (0 << 2) |		  /* Reserved. */
		 (0 << 1) | (0 << 0);	  /* EP buffering: quad buffering */
	SYNCDELAY();

	/* Disable all other EPs (EP4, EP6, and EP8). */
	EP4CFG &= ~bmVALID;
	SYNCDELAY();
	EP6CFG &= ~bmVALID;
	SYNCDELAY();
	EP8CFG &= ~bmVALID;
	SYNCDELAY();

	/* Reset the FIFOs of EP1 and EP2. */
	/* Note: RESETFIFO() gets the EP number WITHOUT bit 7 set/cleared. */
	RESETFIFO(0x01)
	RESETFIFO(0x02)

	/* Set the GPIF flag for EP2 to 'full'. */
	EP2GPIFFLGSEL = (1 << 1) | (0 << 1);
	SYNCDELAY();
}

BOOL handle_vendorcommand(BYTE cmd)
{
	(void)cmd;
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
	RESETTOGGLE(0x01);
	RESETTOGGLE(0x82);

	/* (3) Restore EPs to their default conditions. */
	/* Note: RESETFIFO() gets the EP number WITHOUT bit 7 set/cleared. */
	RESETFIFO(0x01);
	/* TODO */
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

void main(void)
{
	/* Set DYN_OUT and ENH_PKT bits, as recommended by the TRM. */
	REVCTL = (1 << 1) | (1 << 0);

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

	/* TODO */
	/* Initiate a GPIF read. */
	(void)EP2GPIFTRIG;
#if 0
	/* TODO: This seems to hang? */
	gpif_fifo_read(GPIF_EP2);
#endif

	while (1) {
		if (got_sud) {
			handle_setupdata();
			got_sud = FALSE;
		}
	}
}
