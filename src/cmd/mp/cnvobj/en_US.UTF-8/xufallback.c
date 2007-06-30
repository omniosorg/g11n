/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").  
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 *
 * This is basically os_ws/src/locale/wcwidth.c plus xi18n_ws/src/nls/
 * XLC_LOCALE/conv_tables/util/UNICODE-FONTSPECIFIC.GLGR.txt-gen.c.
 * Whenever you add any new character glyphs at the fallback font or if
 * you change the fallback font encoding, you should update this file.
 */

#pragma ident	"@(#)xufallback.c	1.3	04/02/20 SMI"

#include "uwidth.h"

#define	XUFALLBACK_NOT_DEFINED_GLYPH		0
#define	XUFALLBACK_HALF_NO_GLYPH		1
#define	XUFALLBACK_FULL_NO_GLYPH		2

unsigned int
_xu2fallback(unsigned int wc) {
	int i, j;

	/* Let's first check known glyphs. */
	switch (wc) {
	case 0:  /* null character maps to "notdefined" glyph */
		return (XUFALLBACK_NOT_DEFINED_GLYPH);
	case 0x20ac:  /* euro */
		return (3);
	case 0xfffd:  /* replacement character */
		return (4);
	case 0x0152:  /* OE */
		return (5);
	case 0x0153:  /* oe */
		return (6);
	case 0x0178:  /* Y with diaeresis */
		return (7);
	case 0x0108:  /* C with circumflex */
		return (8);
	case 0x0109:  /* c with circumflex */
		return (9);
	case 0x010a:  /* C with dot above */
		return (0xa);
	case 0x010b:  /* c with dot above */
		return (0xb);
	case 0x011c:  /* G with circumflex */
		return (0xc);
	case 0x011d:  /* g with circumflex */
		return (0xd);
	case 0x0120:  /* G with dot above */
		return (0xe);
	case 0x0121:  /* g with dot above */
		return (0xf);
	case 0x0124:  /* H with circumflex */
		return (0x10);
	case 0x0125:  /* h with circumflex */
		return (0x11);
	case 0x0134:  /* J with circumflex */
		return (0x12);
	case 0x0135:  /* j with circumflex */
		return (0x13);
	case 0x015c:  /* S with circumflex */
		return (0x14);
	case 0x015d:  /* s with circumflex */
		return (0x15);
	case 0x016c:  /* U with breve */
		return (0x16);
	case 0x016d:  /* u with breve */
		return (0x17);
	}

	/*
	 * Now, it has to be one of the following three glyphs:
	 * 	"half-width no-glyph" glyph,
	 *	"full-width no-glyph" glyph, or
	 *	"not-defined" glyph.
	 */
	if (wc <= 0x00ffff) {
		/* Basic Multilingual Plane */
		i = wc / 4;
		j = wc % 4;
		switch (j) {
		case 0:
			wc = width_tbl[ucode00[i].u0];
			break;
		case 1:
			wc = width_tbl[ucode00[i].u1];
			break;
		case 2:
			wc = width_tbl[ucode00[i].u2];
			break;
		case 3:
			wc = width_tbl[ucode00[i].u3];
			break;
		}
	} else if (wc <= 0x01ffff) {
		/* Secondary Multilingual Plane */
		wc = wc & 0xffff;
		i = wc / 4;
		j = wc % 4;
		switch (j) {
		case 0:
			wc = width_tbl[ucode01[i].u0];
			break;
		case 1:
			wc = width_tbl[ucode01[i].u1];
			break;
		case 2:
			wc = width_tbl[ucode01[i].u2];
			break;
		case 3:
			wc = width_tbl[ucode01[i].u3];
			break;
		}
	} else if ((wc >= 0x020000 && wc <= 0x02a6d6) ||
		   (wc >= 0x02f800 && wc <= 0x02fa1f)) {
		/* Supplementary Plane for CJK Ideographs */
		/* CJK Unified Ideographs Extension B */
		/* CJK Compatibility Ideographs Supplement */
		return (XUFALLBACK_FULL_NO_GLYPH);
	} else if (wc >= 0x0e0100 && wc <= 0x0e01ef) {
		/* Variation Selectors Supplement */
		return (XUFALLBACK_HALF_NO_GLYPH);
	} else if ((wc >= 0x0f0000 && wc <= 0x0ffffd) ||
		   (wc >= 0x100000 && wc <= 0x10fffd)) {
		/* Private Use Planes 15 & 16 */
		return (XUFALLBACK_FULL_NO_GLYPH);
	} else {
		return (XUFALLBACK_NOT_DEFINED_GLYPH);
	}

	if (wc == 0 || wc == 1)
		return (XUFALLBACK_HALF_NO_GLYPH);
	else if (wc == 2)
		return (XUFALLBACK_FULL_NO_GLYPH);

	return (XUFALLBACK_NOT_DEFINED_GLYPH);
}
