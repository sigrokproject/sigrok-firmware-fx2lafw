##
## This file is part of the fx2lafw project.
##
## Copyright (C) 2013 Uwe Hermann <uwe@hermann-uwe.de>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
##

ACLOCAL_AMFLAGS = -I autostuff

SUBDIRS = include fx2lib hw

EXTRA_DIST = fx2lafw.c gpif-acquisition.c

.PHONY: fx2lafw
fx2lafw: fx2lafw.rel gpif-acquisition.rel

.c.rel:
	$(AM_V_GEN)$(SDCC) -mmcs51 -I$(srcdir)/include -I${srcdir}/fx2lib/include -c $< -o $@

MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog
ChangeLog:
	$(AM_V_at)git --git-dir $(top_srcdir)/.git log > ChangeLog || touch ChangeLog

dist-hook: ChangeLog

clean-local:
	$(AM_V_at)-rm -f *.asm *.lst *.rel *.rst *.sym

