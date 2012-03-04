/*
 * This file is part of the fx2lafw project.
 *
 * Copyright (C) 2011-2012 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#include <fx2regs.h>
#include <fx2macros.h>
#include <delay.h>
#include <gpif.h>

#include <fx2lafw.h>
#include <gpif-acquisition.h>

static void gpif_reset_waveforms(void)
{
	int i;

	/* Reset WAVEDATA. */
	AUTOPTRSETUP = 0x03;
	AUTOPTRH1 = 0xe4;
	AUTOPTRL1 = 0x00;
	for (i = 0; i < 128; i++)
		EXTAUTODAT1 = 0;
}

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
	 * Map index 0 in WAVEDATA to FIFORD. The rest is assigned too,
	 * but not used by us.
	 *
	 * GPIFWFSELECT: [7:6] = SINGLEWR index, [5:4] = SINGLERD index,
	 *               [3:2] = FIFOWR index, [1:0] = FIFORD index
	 */
	GPIFWFSELECT = (0x3 << 6) | (0x2 << 4) | (0x1 << 2) | (0x0 << 0);

	/* Contains RDY* pin values. Read-only according to TRM. */
	GPIFREADYSTAT = 0;
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

void gpif_init_la(void)
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

	/* Reset WAVEDATA. */
	gpif_reset_waveforms();

	/* Initialize GPIF address pins, output initial values. */
	gpif_init_addr_pins();

	/* Initialize flowstate registers (not used by us). */
	gpif_init_flowstates();
}

void gpif_acquisition_start(const struct cmd_start_acquisition *cmd)
{
	xdata volatile BYTE *pSTATE;

	IFCONFIG = (IFCONFIG & ~bm3048MHZ) |
		((cmd->flags & CMD_START_FLAGS_CLK_48MHZ) ? bm3048MHZ : 0);

	/* GPIF terminology: DP = decision point, NDP = non-decision-point */

	/* Populate WAVEDATA
	 *
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

	/* Populate S0 */
	pSTATE = &GPIF_WAVE_DATA;
	pSTATE[0] = cmd->sample_delay;
	pSTATE[8] = 0x02;
	pSTATE[16] = 0x00;
	pSTATE[24] = 0x00;

	/* Populate S1 */
	pSTATE = &GPIF_WAVE_DATA + 1;
	pSTATE[0] = 0x00;
	pSTATE[8] = 0x01;
	pSTATE[16] = 0x00;
	pSTATE[24] = 0x36;

	/* Populate Reserved Words */
	pSTATE = &GPIF_WAVE_DATA + 7;
	pSTATE[0] = 0x07;
	pSTATE[8] = 0x00;
	pSTATE[16] = 0x00;
	pSTATE[24] = 0x3f;

	SYNCDELAY();

	/* Perform the initial GPIF read. */
	gpif_fifo_read(GPIF_EP2);
}
