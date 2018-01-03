/*
 * This file is part of the sigrok-firmware-fx2lafw project.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FX2LAFW_INCLUDE_GPIF_ACQUISITION_H
#define FX2LAFW_INCLUDE_GPIF_ACQUISITION_H

#include <stdbool.h>
#include <command.h>

enum gpif_status {
	STOPPED = 0,
	PREPARED,
	RUNNING,
};
extern enum gpif_status gpif_acquiring;

void gpif_init_la(void);
bool gpif_acquisition_prepare(const struct cmd_start_acquisition *cmd);
void gpif_acquisition_start(void);
void gpif_poll(void);

#endif
