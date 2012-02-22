/*
 * This file is part of the fx2lafw project.
 *
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

#ifdef DEBUG

#include <stdarg.h>
#include <stdio.h>
#include <delay.h>
#include <fx2regs.h>
#include <fx2macros.h>
#include "debug.h"

#define MESSAGE_LENGTH_MAX 64

void debugf(const char *format, ...)
{
	va_list arg;
	int count;

	/* Format the string. */
	va_start(arg, format);
	count = vsprintf(EP6FIFOBUF, format, arg);
	va_end(arg);

	/* Zero out the rest of the buffer. */
	while (count < MESSAGE_LENGTH_MAX)
		EP6FIFOBUF[count++] = '\0';

	/* Send the buffer. */
	count = 32;
	EP6BCH = MSB(count);
	SYNCDELAY4;
	EP6BCL = LSB(count);
	SYNCDELAY4;
}

void _assert(char *expr, const char *filename, unsigned int linenumber)
{
	debugf("Assert(%s) failed at line %u in file %s.\n",
	       expr, linenumber, filename);
	while (1);
}

#endif
