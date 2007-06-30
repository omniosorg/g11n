#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
#
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

#
#	@(#)extension.src.m4	1.11	97/12/04 SMI
#
# localedef source file for ja locale
#

#
#	BIND (Binary Compatibility for old ja/xctype.h)
#
LCBIND
	_U   = 0x00000001
	_L   = 0x00000002
	_N   = 0x00000004
	_S   = 0x00000008
	_P   = 0x00000010
	_C   = 0x00000020
	_B   = 0x00000040
	_X   = 0x00000080
	_E1  = 0x00000100
	_E2  = 0x00000200
	_E3  = 0x00000400
	_E4  = 0x00000800
	_E5  = 0x00001000
	_E6  = 0x00002000
	_E7  = 0x00004000
	_E8  = 0x00008000
	_E9  = 0x00010000
	_E10 = 0x00020000
	_E11 = 0x00040000
	_E12 = 0x00080000
	_E13 = 0x00100000
	_E14 = 0x00200000
	_E15 = 0x00400000
	_E16 = 0x00800000
	_E17 = 0x01000000
	_E18 = 0x02000000
	_E19 = 0x04000000
	_E20 = 0x08000000
	_E21 = 0x10000000
	_E22 = 0x20000000
	_E23 = 0x40000000
	_E24 = 0x80000000

	phonogram	charclass	_E1
	jkanji		charclass	_E2
	jalpha		charclass	_E3
	jdigit		charclass	_E4
	jspecial	charclass	_E5
	jisx0208	charclass	_E9
	jparen		charclass	_E10
	jhira		charclass	_E11
	jkata		charclass	_E12
	jgreek 		charclass	_E13
	jisx0201r 	charclass	_E14
	jrussian	charclass	_E15
	line		charclass	_E16
	junit		charclass	_E17
	jsci		charclass	_E18
	jgen		charclass	_E19
	jpunct		charclass	_E20
	jisx0212	charclass	_E21

	tojhira		chartrans	_E12
	tojkata		chartrans	_E11
END LCBIND

METHODS

process_code	euc
cswidth		2:2,1:1,2:2

eucpctowc	"__eucpctowc_eucjp" "methods" "/usr/lib/locale/ja_JP.eucJP" "methods_ja_JP.eucJP.so.3"
wctoeucpc	"__wctoeucpc_eucjp"
fgetwc		"__fgetwc_euc_eucjp"
fgetwc@native	"__fgetwc_dense_eucjp"
mbftowc		"__mbftowc_euc_eucjp"
mbftowc@native	"__mbftowc_dense_eucjp"
mblen		"__mblen_eucjp"
mbstowcs	"__mbstowcs_euc_eucjp"
mbstowcs@native	"__mbstowcs_dense_eucjp"
mbtowc		"__mbtowc_euc_eucjp"
mbtowc@native	"__mbtowc_dense_eucjp"
wcstombs	"__wcstombs_euc_eucjp"
wcstombs@native	"__wcstombs_dense_eucjp"
wcswidth	"__wcswidth_euc_eucjp"
wcswidth@native	"__wcswidth_dense_eucjp"
wctomb		"__wctomb_euc_eucjp"
wctomb@native	"__wctomb_dense_eucjp"
wcwidth		"__wcwidth_euc_eucjp"
wcwidth@native	"__wcwidth_dense_eucjp"
mbrlen		"__mbrlen_eucjp"
btowc		"__btowc_euc_eucjp"
btowc@native	"__btowc_dense_eucjp"
wctob		"__wctob_euc_eucjp"
wctob@native	"__wctob_dense_eucjp"
mbrtowc		"__mbrtowc_euc_eucjp"
mbrtowc@native	"__mbrtowc_dense_eucjp"
wcrtomb		"__wcrtomb_euc_eucjp"
wcrtomb@native	"__wcrtomb_dense_eucjp"
mbsrtowcs	"__mbsrtowcs_euc_eucjp"
mbsrtowcs@native	"__mbsrtowcs_dense_eucjp"
wcsrtombs	"__wcsrtombs_euc_eucjp"
wcsrtombs@native	"__wcsrtombs_dense_eucjp"
mbsinit		"__mbsinit_eucjp"
fnmatch		"__fnmatch_std" "libc" "/usr/lib" "libc.so.1"
getdate		"__getdate_std"
iswctype	"__iswctype_bc"
iswctype@native	"__iswctype_std"
regcomp		"__regcomp_std"
regerror	"__regerror_std"
regexec		"__regexec_std"
regfree		"__regfree_std"
strcoll		"__strcoll_C"
strfmon		"__strfmon_std"
strftime	"__strftime_std"
strptime	"__strptime_std"
strxfrm		"__strxfrm_C"
towctrans	"__towctrans_bc"
towctrans@native	"__towctrans_std"
towlower	"__towlower_bc"
towlower@native	"__towlower_std"
towupper	"__towupper_bc"
towupper@native	"__towupper_std"
trwctype	"__trwctype_std"
wcscoll		"__wcscoll_C"
wcscoll@native	"__wcscoll_C"
wcsftime	"__wcsftime_std"
wcsxfrm		"__wcsxfrm_C"
wcsxfrm@native	"__wcsxfrm_C"
wctrans		"__wctrans_std"
wctype		"__wctype_std"

END METHODS
