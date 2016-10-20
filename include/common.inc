;;
;; This file is part of the sigrok-firmware-fx2lafw project.
;;
;; Copyright (C) 2016 Stefan Brüns <stefan.bruens@rwth-aachen.de>
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

.macro string_descriptor_a n,str
_string'n:
	.nchr	len,"'str"
	.db	len * 2 + 2
	.db	3
	.irpc	i,^"'str"
		.db	''i, 0
	.endm
.endm

.macro string_descriptor_lang n,l
_string'n:
	.db	4
	.db	3
	.dw	>l + (<l * 0x100)
.endm
