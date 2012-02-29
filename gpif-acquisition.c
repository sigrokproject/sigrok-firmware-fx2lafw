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
	// 0x01, 0x38, 0x01, 0x01, 0x01, 0x01, 0x01,
	// FIXME: For now just loop over the "sample data" state forever.
	0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
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

	/* Write the four GPIF waveforms into the WAVEDATA register. */
	gpif_write_waveforms();

	/* Initialize GPIF address pins, output initial values. */
	gpif_init_addr_pins();

	/* Initialize flowstate registers (not used by us). */
	gpif_init_flowstates();
}

void gpif_acquisition_start(void)
{
	/* Perform the initial GPIF read. */
	gpif_fifo_read(GPIF_EP2);
}
