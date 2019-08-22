/*
 * This file is part of the sigrok-firmware-fx2lafw project.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <eputils.h>
#include <fx2regs.h>
#include <fx2macros.h>
#include <delay.h>
#include <gpif.h>
#include <fx2lafw.h>
#include <gpif-acquisition.h>

enum gpif_status gpif_acquiring = STOPPED;

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

	/* Set TRICTL = 0, thus CTL0-CTL5 are CMOS outputs. */
	GPIFCTLCFG = 0;

	/* When GPIF is idle, tri-state the data bus. */
	/* Bit 7: DONE, bit 0: IDLEDRV. TODO: Set/clear DONE bit? */
	GPIFIDLECS = (0 << 0);

	/* When GPIF is idle, set CTL0-CTL5 to 0. */
	GPIFIDLECTL = 0;

	/*
	 * Map index 0 in WAVEDATA to FIFORD. The rest is assigned too,
	 * but not used by us.
	 *
	 * GPIFWFSELECT: [7:6] = SINGLEWR index, [5:4] = SINGLERD index,
	 *               [3:2] = FIFOWR index, [1:0] = FIFORD index
	 */
	GPIFWFSELECT = (0x3u << 6) | (0x2u << 4) | (0x1u << 2) | (0x0u << 0);

	/* Contains RDY* pin values. Read-only according to TRM. */
	GPIFREADYSTAT = 0;

	/* Make GPIF stop on transaction count not flag. */
	EP2GPIFPFSTOP = (0 << 0);
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

	/* Reset the status. */
	gpif_acquiring = STOPPED;
}

static void gpif_make_delay_state(volatile BYTE *pSTATE, uint8_t delay, uint8_t output)
{
	/*
	 * DELAY
	 * Delay cmd->sample_delay clocks.
	 */
	pSTATE[0] = delay;

	/*
	 * OPCODE
	 * SGL=0, GIN=0, INCAD=0, NEXT=0, DATA=0, DP=0
	 */
	pSTATE[8] = 0;

	/*
	 * OUTPUT
	 * CTL[0:5]=output
	 */
	pSTATE[16] = output;

	/*
	 * LOGIC FUNCTION
	 * Not used.
	 */
	pSTATE[24] = 0x00;
}

static void gpif_make_data_dp_state(volatile BYTE *pSTATE)
{
	/*
	 * BRANCH
	 * Branch to IDLE if condition is true, back to S0 otherwise.
	 */
	pSTATE[0] = (1 << 7) | (7 << 3) | (0 << 0);

	/*
	 * OPCODE
	 * SGL=0, GIN=0, INCAD=0, NEXT=0, DATA=1, DP=1
	 */
	pSTATE[8] = (1 << 1) | (1 << 0);

	/*
	 * OUTPUT
	 * CTL[0:5]=0
	 */
	pSTATE[16] = 0x00;

	/*
	 * LOGIC FUNCTION
	 * Evaluate if the FIFO full flag is set.
	 * LFUNC=0 (AND), TERMA=6 (FIFO Flag), TERMB=6 (FIFO Flag)
	 */
	pSTATE[24] = (6 << 3) | (6 << 0);
}

bool gpif_acquisition_prepare(const struct cmd_start_acquisition *cmd)
{
	int i;
	volatile BYTE *pSTATE = &GPIF_WAVE_DATA;

	/* Ensure GPIF is idle before reconfiguration. */
	while (!(GPIFTRIG & 0x80));

	/* Configure the EP2 FIFO. */
	if (cmd->flags & CMD_START_FLAGS_SAMPLE_16BIT)
		EP2FIFOCFG = bmAUTOIN | bmWORDWIDE;
	else
		EP2FIFOCFG = bmAUTOIN;
	SYNCDELAY();

	/* Set IFCONFIG to the correct clock source. */
	if (cmd->flags & CMD_START_FLAGS_CLK_48MHZ) {
		IFCONFIG = bmIFCLKSRC | bm3048MHZ | bmIFCLKOE | bmASYNC |
			   bmGSTATE | bmIFGPIF;
	} else {
		IFCONFIG = bmIFCLKSRC | bmIFCLKOE | bmASYNC |
			   bmGSTATE | bmIFGPIF;
	}

	/* Populate delay states. */
	if (cmd->sample_delay_h >= 6)
		return false;

	if (cmd->flags & CMD_START_FLAGS_CLK_CTL2) {
		uint8_t delay_1, delay_2 = cmd->sample_delay_l;

		/* We need a pulse where the CTL1/2 pins alternate states. */
		if (cmd->sample_delay_h) {
			for (i = 0; i < cmd->sample_delay_h; i++)
				gpif_make_delay_state(pSTATE++, 0, 0x06);
		} else {
			delay_1 = delay_2 / 2;
			delay_2 -= delay_1;
			gpif_make_delay_state(pSTATE++, delay_1, 0x06);
		}

		/* sample_delay_l is always != 0 for the supported rates. */
		gpif_make_delay_state(pSTATE++, delay_2, 0x00);
	} else {
		for (i = 0; i < cmd->sample_delay_h; i++)
			gpif_make_delay_state(pSTATE++, 0, 0x00);

		if (cmd->sample_delay_l != 0)
			gpif_make_delay_state(pSTATE++, cmd->sample_delay_l, 0x00);
	}

	/* Populate S1 - the decision point. */
	gpif_make_data_dp_state(pSTATE++);

	/* Update the status. */
	gpif_acquiring = PREPARED;

	return true;
}

void gpif_acquisition_start(void)
{
	/* Execute the whole GPIF waveform once. */
	gpif_set_tc16(1);

	/* Perform the initial GPIF read. */
	gpif_fifo_read(GPIF_EP2);

	/* Update the status. */
	gpif_acquiring = RUNNING;
}

void gpif_poll(void)
{
	/* Detect if acquisition has completed. */
	if ((gpif_acquiring == RUNNING) && (GPIFTRIG & 0x80)) {
		/* Activate NAK-ALL to avoid race conditions. */
		FIFORESET = 0x80;
		SYNCDELAY();

		/* Switch to manual mode. */
		EP2FIFOCFG = 0;
		SYNCDELAY();

		/* Reset EP2. */
		FIFORESET = 0x02;
		SYNCDELAY();

		/* Return to auto mode. */
		EP2FIFOCFG = bmAUTOIN;
		SYNCDELAY();

		/* Release NAK-ALL. */
		FIFORESET = 0x00;
		SYNCDELAY();

		gpif_acquiring = STOPPED;
	}
}
