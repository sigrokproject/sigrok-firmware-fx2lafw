##
## This file is part of the fx2lafw project.
##
## Copyright (C) 2012 Uwe Hermann <uwe@hermann-uwe.de>
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

all: saleae-logic cwav-usbeeax

saleae-logic:
	$(MAKE) -C hw/saleae-logic

cwav-usbeeax:
	$(MAKE) -C hw/cwav-usbeeax

clean:
	rm -rf hw/saleae-logic/build
	rm -rf hw/cwav-usbeeax/build

.PHONY: saleae-logic cwav-usbeeax

