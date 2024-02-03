;;
;; This file is part of the sigrok-firmware-fx2lafw project.
;;
;; Copyright (C) 2024 Steve Markgraf <steve@steve-m.de>
;;
;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, see <http://www.gnu.org/licenses/>.
;;

VID = 0x501d	; Manufacturer ID (0x1d50)
PID = 0x8e60	; Product ID (0x608e)
VER = 0x0600	; Product "version". 0x0006 == Hantek PSO2020.

.include "dscr_scope.inc"
string_descriptor_a 3,^"Hantek PSO2020"
_dev_strings_end:
	.dw	0x0000
