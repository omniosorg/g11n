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
 * Copyright (c) 1999 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma	ident	"@(#)xuiso8859.c	1.5 99/03/04 SMI"

#include <sys/types.h>
#include <sys/isa_defs.h>
#include "xucuf.h"
#include "common.h"

#if defined(ISO_8859_1)
#include "ucs4_to_iso-8859-1.h"
int
_xuiso88591(ushort_t ucs2)
#elif defined(ISO_8859_2)
#include "ucs4_to_iso-8859-2.h"
int
_xuiso88592(ushort_t ucs2)
#elif defined(ISO_8859_4)
#include "ucs4_to_iso-8859-4.h"
int
_xuiso88594(ushort_t ucs2)
#elif defined(ISO_8859_5)
#include "ucs4_to_iso-8859-5.h"
int
_xuiso88595(ushort_t ucs2)
#elif defined(ISO_8859_7)
#include "ucs4_to_iso-8859-7.h"
int
_xuiso88597(ushort_t ucs2)
#elif defined(ISO_8859_9)
#include "ucs4_to_iso-8859-9.h"
int
_xuiso88599(ushort_t ucs2)
#elif defined(ISO_8859_10)
#include "ucs4_to_iso-8859-10.h"
int
_xuiso885910(ushort_t ucs2)
#else
#error "Fatal: one of ISO 8859 macros need to be defined."
#endif
{
	register int i, l, h;

	if (ucs2 <= 0x7f)
		return (uchar_t)ucs2;

	i = l = 0;
	h = (sizeof(u4_sb_tbl) / sizeof(to_sb_table_component_t)) - 1;
	while (l <= h) {
		i = (l + h) / 2;
		if (u4_sb_tbl[i].u4 == ucs2)
			break;
		else if (u4_sb_tbl[i].u4 < ucs2)
			l = i + 1;
		else
			h = i - 1;
	}
	if (u4_sb_tbl[i].u4 == ucs2)
		return u4_sb_tbl[i].sb;
	else
    		return CUF_NICH;
}
