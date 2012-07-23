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

# sigrok-firmware-fx2lafw package/tarball version number.
VERSION = "0.1.0"

DESTDIR ?= /usr/local/share/sigrok-firmware

REPO = git://sigrok.git.sourceforge.net/gitroot/sigrok/fx2lafw

TARBALLDIR = sigrok-firmware-fx2lafw-$(VERSION)

all: build-all

build-all: saleae-logic cwav-usbeeax cwav-usbeesx cypress-fx2 braintechnology-usb-lps

saleae-logic:
	@$(MAKE) -C hw/saleae-logic

cwav-usbeeax:
	@$(MAKE) -C hw/cwav-usbeeax

cwav-usbeesx:
	@$(MAKE) -C hw/cwav-usbeesx

cypress-fx2:
	@$(MAKE) -C hw/cypress-fx2

braintechnology-usb-lps:
	@$(MAKE) -C hw/braintechnology-usb-lps

ChangeLog:
	@git log > ChangeLog || touch ChangeLog

dist:
	@git clone $(REPO) $(TARBALLDIR)
	@cd $(TARBALLDIR) && $(MAKE) ChangeLog && cd ..
	@rm -rf $(TARBALLDIR)/.git
	@tar -c -z -f $(TARBALLDIR).tar.gz $(TARBALLDIR)
	@rm -rf $(TARBALLDIR)

install: build-all
	@mkdir -p $(DESTDIR)
	@cp hw/saleae-logic/build/*.fw $(DESTDIR)
	@cp hw/cwav-usbeeax/build/*.fw $(DESTDIR)
	@cp hw/cwav-usbeesx/build/*.fw $(DESTDIR)
	@cp hw/cypress-fx2/build/*.fw $(DESTDIR)
	@cp hw/braintechnology-usb-lps/build/*.fw $(DESTDIR)

clean:
	@rm -rf hw/saleae-logic/build
	@rm -rf hw/cwav-usbeeax/build
	@rm -rf hw/cwav-usbeesx/build
	@rm -rf hw/cypress-fx2/build
	@rm -rf hw/braintechnology-usb-lps/build
	@$(MAKE) -C fx2lib clean

.PHONY: saleae-logic cwav-usbeeax cwav-usbeesx cypress-fx2 braintechnology-usb-lps

