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

#define ERRF(fmt, ...)		do { debugf("E " fmt, __VA_ARGS__); \
				while(1); } while(0)
#define ERR(msg)		ERRF("%s", msg)

#define WARNF(fmt, ...)		debugf("W " fmt, __VA_ARGS__)
#define WARN(msg)		WARNF("%s", msg)

#define INFOF(fmt, ...)		debugf("I " fmt, __VA_ARGS__)
#define INFO(msg)		INFOF("%s", msg)

/**
 * A printf that prints messages through EP6.
 */
void debugf(const char *format, ...);

#else

#define ERRF(fmt, ...)		((void)0)
#define ERR(msg)		((void)0)

#define WARNF(fmt, ...)		((void)0)
#define WARN(msg)		((void)0)

#define INFOF(fmt, ...)		((void)0)
#define INFO(msg)		((void)0)

#endif
