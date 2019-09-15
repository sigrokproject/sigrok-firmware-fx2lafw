##
## This file is part of the sigrok-firmware-fx2lafw project.
##
## Copyright (C) 2013 Uwe Hermann <uwe@hermann-uwe.de>
## Copyright (C) 2015 Daniel Elstner <daniel.kitta@gmail.com>
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
## along with this program; if not, see <http://www.gnu.org/licenses/>.
##

GNUMAKEFLAGS = --no-print-directory
SUFFIXES = .a51 .c .fw .ihx .rel

# Flags for firmware hex file generation
SDCC_LINK_FLAGS = --code-size 0x1c00 --xram-size 0x0200 --xram-loc 0x1c00 -Wl"-b DSCR_AREA=0x1e00" -Wl"-b INT2JT=0x1f00"
SDCC_LINK_FLAGS_SCOPE = --code-size 0x3c00 --xram-size 0x0100 --xram-loc 0x3c00 -Wl"-b DSCR_AREA=0x3d00" -Wl"-b INT2JT=0x3f00"

# Include paths
as_includes = -I$(srcdir)/include
c_includes = -I$(srcdir)/include -I$(srcdir)/fx2lib/include

# Binary firmware tarball basename
distdir_bin = $(PACKAGE)-bin-$(VERSION)

# Basename of directory stamp files
dirstamp = .dirstamp

dirstamps = \
	fx2lib/lib/$(dirstamp) \
	fx2lib/lib/interrupts/$(dirstamp) \
	hw/braintechnology-usb-lps/$(dirstamp) \
	hw/cwav-usbeeax/$(dirstamp) \
	hw/cwav-usbeedx/$(dirstamp) \
	hw/cwav-usbeesx/$(dirstamp) \
	hw/cwav-usbeezx/$(dirstamp) \
	hw/cypress-fx2/$(dirstamp) \
	hw/saleae-logic/$(dirstamp) \
	hw/sigrok-fx2-8ch/$(dirstamp) \
	hw/sigrok-fx2-16ch/$(dirstamp) \
	hw/hantek-6022be/$(dirstamp) \
	hw/hantek-6022bl/$(dirstamp) \
	hw/sainsmart-dds120/$(dirstamp) \
	hw/yixingdianzi-mdso/$(dirstamp)

# Final firmware files
firmware_binaries = \
	hw/braintechnology-usb-lps/fx2lafw-braintechnology-usb-lps.fw \
	hw/cwav-usbeeax/fx2lafw-cwav-usbeeax.fw \
	hw/cwav-usbeedx/fx2lafw-cwav-usbeedx.fw \
	hw/cwav-usbeesx/fx2lafw-cwav-usbeesx.fw \
	hw/cwav-usbeezx/fx2lafw-cwav-usbeezx.fw \
	hw/cypress-fx2/fx2lafw-cypress-fx2.fw \
	hw/saleae-logic/fx2lafw-saleae-logic.fw \
	hw/sigrok-fx2-8ch/fx2lafw-sigrok-fx2-8ch.fw \
	hw/sigrok-fx2-16ch/fx2lafw-sigrok-fx2-16ch.fw \
	hw/hantek-6022be/fx2lafw-hantek-6022be.fw \
	hw/hantek-6022bl/fx2lafw-hantek-6022bl.fw \
	hw/sainsmart-dds120/fx2lafw-sainsmart-dds120.fw \
	hw/yixingdianzi-mdso/fx2lafw-yixingdianzi-mdso.fw

fx2lafw_headers = \
	include/command.h \
	include/dscr.inc \
	include/common.inc \
	include/fx2lafw.h \
	include/gpif-acquisition.h

fx2lafw_sources = \
	fx2lafw.c \
	gpif-acquisition.c

fx2lafw_objects = \
	fx2lafw.rel \
	gpif-acquisition.rel

scope_headers = \
	include/dscr_scope.inc \
	include/scope.inc \
	include/common.inc

hantek_6022be_headers = $(scope_headers)
hantek_6022be_sources = hw/hantek-6022be/fw.c
hantek_6022be_objects = hw/hantek-6022be/fw.rel

hantek_6022bl_headers = $(scope_headers)
hantek_6022bl_sources = hw/hantek-6022bl/fw.c
hantek_6022bl_objects = hw/hantek-6022bl/fw.rel

sainsmart_dds120_headers = $(scope_headers)
sainsmart_dds120_sources = hw/sainsmart-dds120/fw.c
sainsmart_dds120_objects = hw/sainsmart-dds120/fw.rel

yixingdianzi_mdso_headers = $(scope_headers)
yixingdianzi_mdso_sources = hw/yixingdianzi-mdso/fw.c
yixingdianzi_mdso_objects = hw/yixingdianzi-mdso/fw.rel

fx2lib_headers = \
	fx2lib/include/autovector.h \
	fx2lib/include/delay.h \
	fx2lib/include/eputils.h \
	fx2lib/include/fx2ints.h \
	fx2lib/include/fx2macros.h \
	fx2lib/include/fx2regs.h \
	fx2lib/include/fx2types.h \
	fx2lib/include/gpif.h \
	fx2lib/include/i2c.h \
	fx2lib/include/lights.h \
	fx2lib/include/serial.h \
	fx2lib/include/setupdat.h

fx2lib_sources = \
	fx2lib/lib/delay.c \
	fx2lib/lib/eputils.c \
	fx2lib/lib/gpif.c \
	fx2lib/lib/i2c.c \
	fx2lib/lib/int4av.a51 \
	fx2lib/lib/serial.c \
	fx2lib/lib/delay.c \
	fx2lib/lib/setupdat.c \
	fx2lib/lib/usbav.a51

fx2lib_objects = \
	fx2lib/lib/delay.rel \
	fx2lib/lib/eputils.rel \
	fx2lib/lib/gpif.rel \
	fx2lib/lib/i2c.rel \
	fx2lib/lib/int4av.rel \
	fx2lib/lib/serial.rel \
	fx2lib/lib/delay.rel \
	fx2lib/lib/setupdat.rel \
	fx2lib/lib/usbav.rel

fx2lib_ints_sources = \
	fx2lib/lib/interrupts/ep0ack_isr.c \
	fx2lib/lib/interrupts/ep0in_isr.c \
	fx2lib/lib/interrupts/ep0out_isr.c \
	fx2lib/lib/interrupts/ep0ping_isr.c \
	fx2lib/lib/interrupts/ep1in_isr.c \
	fx2lib/lib/interrupts/ep1out_isr.c \
	fx2lib/lib/interrupts/ep1ping_isr.c \
	fx2lib/lib/interrupts/ep2ef_isr.c \
	fx2lib/lib/interrupts/ep2ff_isr.c \
	fx2lib/lib/interrupts/ep2isoerr_isr.c \
	fx2lib/lib/interrupts/ep2_isr.c \
	fx2lib/lib/interrupts/ep2pf_isr.c \
	fx2lib/lib/interrupts/ep2ping_isr.c \
	fx2lib/lib/interrupts/ep4ef_isr.c \
	fx2lib/lib/interrupts/ep4ff_isr.c \
	fx2lib/lib/interrupts/ep4isoerr_isr.c \
	fx2lib/lib/interrupts/ep4_isr.c \
	fx2lib/lib/interrupts/ep4pf_isr.c \
	fx2lib/lib/interrupts/ep4ping_isr.c \
	fx2lib/lib/interrupts/ep6ef_isr.c \
	fx2lib/lib/interrupts/ep6ff_isr.c \
	fx2lib/lib/interrupts/ep6isoerr_isr.c \
	fx2lib/lib/interrupts/ep6_isr.c \
	fx2lib/lib/interrupts/ep6pf_isr.c \
	fx2lib/lib/interrupts/ep6ping_isr.c \
	fx2lib/lib/interrupts/ep8ef_isr.c \
	fx2lib/lib/interrupts/ep8ff_isr.c \
	fx2lib/lib/interrupts/ep8isoerr_isr.c \
	fx2lib/lib/interrupts/ep8_isr.c \
	fx2lib/lib/interrupts/ep8pf_isr.c \
	fx2lib/lib/interrupts/ep8ping_isr.c \
	fx2lib/lib/interrupts/errlimit_isr.c \
	fx2lib/lib/interrupts/gpifdone_isr.c \
	fx2lib/lib/interrupts/gpifwf_isr.c \
	fx2lib/lib/interrupts/hispeed_isr.c \
	fx2lib/lib/interrupts/ibn_isr.c \
	fx2lib/lib/interrupts/sof_isr.c \
	fx2lib/lib/interrupts/spare_isr.c \
	fx2lib/lib/interrupts/sudav_isr.c \
	fx2lib/lib/interrupts/suspend_isr.c \
	fx2lib/lib/interrupts/sutok_isr.c \
	fx2lib/lib/interrupts/usbreset_isr.c

fx2lib_ints_objects = \
	fx2lib/lib/interrupts/ep0ack_isr.rel \
	fx2lib/lib/interrupts/ep0in_isr.rel \
	fx2lib/lib/interrupts/ep0out_isr.rel \
	fx2lib/lib/interrupts/ep0ping_isr.rel \
	fx2lib/lib/interrupts/ep1in_isr.rel \
	fx2lib/lib/interrupts/ep1out_isr.rel \
	fx2lib/lib/interrupts/ep1ping_isr.rel \
	fx2lib/lib/interrupts/ep2ef_isr.rel \
	fx2lib/lib/interrupts/ep2ff_isr.rel \
	fx2lib/lib/interrupts/ep2isoerr_isr.rel \
	fx2lib/lib/interrupts/ep2_isr.rel \
	fx2lib/lib/interrupts/ep2pf_isr.rel \
	fx2lib/lib/interrupts/ep2ping_isr.rel \
	fx2lib/lib/interrupts/ep4ef_isr.rel \
	fx2lib/lib/interrupts/ep4ff_isr.rel \
	fx2lib/lib/interrupts/ep4isoerr_isr.rel \
	fx2lib/lib/interrupts/ep4_isr.rel \
	fx2lib/lib/interrupts/ep4pf_isr.rel \
	fx2lib/lib/interrupts/ep4ping_isr.rel \
	fx2lib/lib/interrupts/ep6ef_isr.rel \
	fx2lib/lib/interrupts/ep6ff_isr.rel \
	fx2lib/lib/interrupts/ep6isoerr_isr.rel \
	fx2lib/lib/interrupts/ep6_isr.rel \
	fx2lib/lib/interrupts/ep6pf_isr.rel \
	fx2lib/lib/interrupts/ep6ping_isr.rel \
	fx2lib/lib/interrupts/ep8ef_isr.rel \
	fx2lib/lib/interrupts/ep8ff_isr.rel \
	fx2lib/lib/interrupts/ep8isoerr_isr.rel \
	fx2lib/lib/interrupts/ep8_isr.rel \
	fx2lib/lib/interrupts/ep8pf_isr.rel \
	fx2lib/lib/interrupts/ep8ping_isr.rel \
	fx2lib/lib/interrupts/errlimit_isr.rel \
	fx2lib/lib/interrupts/gpifdone_isr.rel \
	fx2lib/lib/interrupts/gpifwf_isr.rel \
	fx2lib/lib/interrupts/hispeed_isr.rel \
	fx2lib/lib/interrupts/ibn_isr.rel \
	fx2lib/lib/interrupts/sof_isr.rel \
	fx2lib/lib/interrupts/spare_isr.rel \
	fx2lib/lib/interrupts/sudav_isr.rel \
	fx2lib/lib/interrupts/suspend_isr.rel \
	fx2lib/lib/interrupts/sutok_isr.rel \
	fx2lib/lib/interrupts/usbreset_isr.rel

fx2lib_libs = \
	fx2lib/lib/fx2.lib \
	fx2lib/lib/interrupts/ints.lib

hw_sources = \
	hw/braintechnology-usb-lps/dscr.a51 \
	hw/cwav-usbeeax/dscr.a51 \
	hw/cwav-usbeedx/dscr.a51 \
	hw/cwav-usbeesx/dscr.a51 \
	hw/cwav-usbeezx/dscr.a51 \
	hw/cypress-fx2/dscr.a51 \
	hw/saleae-logic/dscr.a51 \
	hw/sigrok-fx2-8ch/dscr.a51 \
	hw/sigrok-fx2-16ch/dscr.a51 \
	hw/hantek-6022be/dscr.a51 \
	hw/hantek-6022bl/dscr.a51 \
	hw/sainsmart-dds120/dscr.a51 \
	hw/yixingdianzi-mdso/dscr.a51

firmwaredir = $(datadir)/sigrok-firmware
firmware_DATA = $(firmware_binaries)

dist_noinst_DATA = \
	$(fx2lafw_headers) $(fx2lafw_sources) \
	$(hantek_6022be_headers) $(hantek_6022be_sources) \
	$(hantek_6022bl_headers) $(hantek_6022bl_sources) \
	$(sainsmart_dds120_sources) $(sainsmart_dds120_headers) \
	$(yixingdianzi_mdso_sources) $(yixingdianzi_mdso_headers) \
	$(fx2lib_headers) $(fx2lib_sources) $(fx2lib_ints_sources) $(hw_sources)
dist_noinst_SCRIPTS = autogen.sh

MOSTLYCLEANFILES = *.asm fx2lib/lib/*.asm hw/**/*.asm fx2lib/lib/*.lib fx2lib/lib/interrupts/*.asm fx2lib/lib/interrupts/*.lib
CLEANFILES = $(firmware_binaries)
DISTCLEANFILES = $(dirstamps)

# Custom silent rules prefixes
SF_V_SDAS = $(sf_v_SDAS_@AM_V@)
sf_v_SDAS_ = $(sf_v_SDAS_@AM_DEFAULT_V@)
sf_v_SDAS_0 = @echo "  SDAS    " $@;
sf_v_SDAS_1 =

SF_V_SDCC = $(sf_v_SDCC_@AM_V@)
sf_v_SDCC_ = $(sf_v_SDCC_@AM_DEFAULT_V@)
sf_v_SDCC_0 = @echo "  SDCC    " $@;
sf_v_SDCC_1 =

.PHONY: dist-bin dist-changelog

all-local: $(firmware_binaries)

mostlyclean-local:
	-$(AM_V_at)find . -type f '(' -name '*.ihx' -o -name '*.lk' \
		-o -name '*.lst' -o -name '*.map' -o -name '*.mem' \
		-o -name '*.rel' -o -name '*.rst' -o -name '*.sym' ')' \
		-exec rm -f '{}' '+'

dist-hook: dist-changelog

dist-changelog:
	$(AM_V_at)if test -d '$(top_srcdir)/.git'; then \
		if git -C '$(top_srcdir)' log >.ChangeLog.tmp; \
		then mv -f .ChangeLog.tmp "$(top_distdir)/ChangeLog"; \
		else rm -f .ChangeLog.tmp; exit 1; fi; \
	else :; fi

# Creates a convenience tarball with just the generated *.fw files, so that
# the users don't have to build them manually.
dist-bin: $(firmware_binaries)
	-$(AM_V_at)rm -fr $(distdir_bin)
	$(AM_V_at)$(MKDIR_P) $(distdir_bin)
	$(AM_V_at)$(INSTALL_DATA) $(firmware_binaries) $(distdir_bin)
	$(AM_V_at)$(INSTALL_DATA) $(srcdir)/COPYING $(srcdir)/COPYING.LESSER $(srcdir)/NEWS $(srcdir)/README $(distdir_bin)
	$(AM_V_at)if test -d '$(top_srcdir)/.git'; \
	then git -C '$(top_srcdir)' log > $(distdir_bin)/ChangeLog; \
	else $(INSTALL_DATA) $(srcdir)/ChangeLog $(distdir_bin); fi
	$(AM_V_at)tar -czf $(distdir_bin).tar.gz $(distdir_bin)
	-$(AM_V_at)rm -fr $(distdir_bin)

# List dependencies
$(fx2lafw_objects): $(fx2lafw_headers)
$(hantek_6022be_objects): $(hantek_6022be_headers)
$(fx2lib_objects): fx2lib/lib/$(dirstamp) $(fx2lib_headers)
$(fx2lib_ints_objects): fx2lib/lib/interrupts/$(dirstamp) $(fx2lib_headers)

hw/braintechnology-usb-lps/dscr.rel: hw/braintechnology-usb-lps/$(dirstamp) include/dscr.inc
hw/cwav-usbeeax/dscr.rel: hw/cwav-usbeeax/$(dirstamp) include/dscr.inc
hw/cwav-usbeedx/dscr.rel: hw/cwav-usbeedx/$(dirstamp) include/dscr.inc
hw/cwav-usbeesx/dscr.rel: hw/cwav-usbeesx/$(dirstamp) include/dscr.inc
hw/cwav-usbeezx/dscr.rel: hw/cwav-usbeezx/$(dirstamp) include/dscr.inc
hw/cypress-fx2/dscr.rel: hw/cypress-fx2/$(dirstamp) include/dscr.inc
hw/saleae-logic/dscr.rel: hw/saleae-logic/$(dirstamp) include/dscr.inc
hw/sigrok-fx2-8ch/dscr.rel: hw/sigrok-fx2-8ch/$(dirstamp) include/dscr.inc
hw/sigrok-fx2-16ch/dscr.rel: hw/sigrok-fx2-16ch/$(dirstamp) include/dscr.inc
hw/hantek-6022be/dscr.rel: hw/hantek-6022be/$(dirstamp) include/dscr_scope.inc
hw/hantek-6022bl/dscr.rel: hw/hantek-6022bl/$(dirstamp) include/dscr_scope.inc
hw/sainsmart-dds120/dscr.rel: hw/sainsmart-dds120/$(dirstamp) include/dscr_scope.inc
hw/yixingdianzi-mdso/dscr.rel: hw/yixingdianzi-mdso/$(dirstamp) include/dscr_scope.inc


# Create build subdirectories
$(dirstamps):
	@stamp='$@'; $(MKDIR_P) "$${stamp%/*}"
	@: > $@

fx2lib/lib/fx2.lib: $(fx2lib_objects)
	-$(AM_V_at)rm -f $@
	$(AM_V_GEN)$(SDAR) -rc $@ $(fx2lib_objects)

fx2lib/lib/interrupts/ints.lib: $(fx2lib_ints_objects)
	-$(AM_V_at)rm -f $@
	$(AM_V_GEN)$(SDAR) -rc $@ $(fx2lib_ints_objects)

hw/braintechnology-usb-lps/fx2lafw-braintechnology-usb-lps.ihx: hw/braintechnology-usb-lps/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS) -o $@ hw/braintechnology-usb-lps/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)

hw/cwav-usbeeax/fx2lafw-cwav-usbeeax.ihx: hw/cwav-usbeeax/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS) -o $@ hw/cwav-usbeeax/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)

hw/cwav-usbeedx/fx2lafw-cwav-usbeedx.ihx: hw/cwav-usbeedx/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS) -o $@ hw/cwav-usbeedx/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)

hw/cwav-usbeesx/fx2lafw-cwav-usbeesx.ihx: hw/cwav-usbeesx/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS) -o $@ hw/cwav-usbeesx/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)

hw/cwav-usbeezx/fx2lafw-cwav-usbeezx.ihx: hw/cwav-usbeezx/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS) -o $@ hw/cwav-usbeezx/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)

hw/cypress-fx2/fx2lafw-cypress-fx2.ihx: hw/cypress-fx2/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS) -o $@ hw/cypress-fx2/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)

hw/saleae-logic/fx2lafw-saleae-logic.ihx: hw/saleae-logic/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS) -o $@ hw/saleae-logic/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)

hw/sigrok-fx2-8ch/fx2lafw-sigrok-fx2-8ch.ihx: hw/sigrok-fx2-8ch/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS) -o $@ hw/sigrok-fx2-8ch/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)

hw/sigrok-fx2-16ch/fx2lafw-sigrok-fx2-16ch.ihx: hw/sigrok-fx2-16ch/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS) -o $@ hw/sigrok-fx2-16ch/dscr.rel $(fx2lafw_objects) $(fx2lib_libs)

hw/hantek-6022be/fx2lafw-hantek-6022be.ihx: hw/hantek-6022be/dscr.rel $(hantek_6022be_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS_SCOPE) -o $@ hw/hantek-6022be/dscr.rel $(hantek_6022be_objects) $(fx2lib_libs)

hw/hantek-6022bl/fx2lafw-hantek-6022bl.ihx: hw/hantek-6022bl/dscr.rel $(hantek_6022bl_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS_SCOPE) -o $@ hw/hantek-6022bl/dscr.rel $(hantek_6022bl_objects) $(fx2lib_libs)

hw/sainsmart-dds120/fx2lafw-sainsmart-dds120.ihx: hw/sainsmart-dds120/dscr.rel $(sainsmart_dds120_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS_SCOPE) -o $@ hw/sainsmart-dds120/dscr.rel $(sainsmart_dds120_objects) $(fx2lib_libs)

hw/yixingdianzi-mdso/fx2lafw-yixingdianzi-mdso.ihx: hw/yixingdianzi-mdso/dscr.rel $(yixingdianzi_mdso_objects) $(fx2lib_libs)
	$(AM_V_GEN)$(SDCC) -mmcs51 $(SDCC_LINK_FLAGS_SCOPE) -o $@ hw/yixingdianzi-mdso/dscr.rel $(yixingdianzi_mdso_objects) $(fx2lib_libs)

.a51.rel:
	$(SF_V_SDAS)$(SDAS8051) -glos $(as_includes) $@ $<

.c.rel:
	$(SF_V_SDCC)$(SDCC) -mmcs51 $(c_includes) -c $< -o $@

if FOUND_OBJCOPY
.ihx.fw:
	$(AM_V_GEN)$(OBJCOPY) -Iihex $< -Obinary $@
else
if FOUND_MAKEBIN
.ihx.fw:
	$(AM_V_GEN)$(MAKEBIN) -p < $< > $@
endif
endif

