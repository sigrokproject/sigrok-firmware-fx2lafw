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

# We require at least autoconf 2.63 (AC_INIT format changed there).
AC_PREREQ([2.63])

AC_INIT([sigrok-firmware-fx2lafw], [0.1.0],
	[sigrok-devel@lists.sourceforge.net],
	[sigrok-firmware-fx2lafw], [http://www.sigrok.org])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([autostuff])
AC_CONFIG_AUX_DIR([autostuff])

# We require at least automake 1.11 (needed for 'silent rules').
AM_INIT_AUTOMAKE([1.11 -Wall -Werror check-news])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_INSTALL
AC_PROG_LN_S

AC_CHECK_PROGS([SDCC], [sdcc])
AC_CHECK_PROGS([SDCCLIB], [sdcclib])
AC_CHECK_PROGS([OBJCOPY], [objcopy])
AC_CHECK_PROGS([MAKEBIN], [makebin])

AM_CONDITIONAL([FOUND_OBJCOPY], [test "x$OBJCOPY" != "x"])
AM_CONDITIONAL([FOUND_MAKEBIN], [test "x$MAKEBIN" != "x"])
AM_COND_IF([FOUND_OBJCOPY], [AC_SUBST(FOUND_OBJCOPY, "yes")])
AM_COND_IF([FOUND_MAKEBIN], [AC_SUBST(FOUND_MAKEBIN, "yes")])

# The sdcc 8051 assembler binary has/had different names:
# Newer sdcc, Debian / Ubuntu: sdas8051
# Newer sdcc, Fedora: sdcc-sdas8051
# Older sdcc: asx8051
AC_CHECK_PROGS([SDAS8051], [sdas8051 sdcc-sdas8051 asx8051])

AC_SUBST(SDCC_FLAGS, "--code-size 0x1c00 --xram-size 0x0200 --xram-loc 0x1c00 -Wl\"-b DSCR_AREA=0x1e00\" -Wl\"-b INT2JT=0x1f00\"")
AC_SUBST(FIRMWARE_DIR, "$datadir/sigrok-firmware")
AC_SUBST(MAKEFLAGS, '--no-print-directory')

AC_CONFIG_FILES([Makefile
		 include/Makefile
		 fx2lib/Makefile
		 fx2lib/include/Makefile
		 fx2lib/lib/Makefile
		 fx2lib/lib/interrupts/Makefile
		 hw/Makefile
		 hw/braintechnology-usb-lps/Makefile
		 hw/cwav-usbeeax/Makefile
		 hw/cwav-usbeedx/Makefile
		 hw/cwav-usbeesx/Makefile
		 hw/cypress-fx2/Makefile
		 hw/saleae-logic/Makefile
		])

AC_OUTPUT

