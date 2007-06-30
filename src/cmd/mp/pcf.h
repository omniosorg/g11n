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
/*
 * Copyright (C) 1994 X Consortium
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNEC-
 * TION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the X Consortium shall not
 * be used in advertising or otherwise to promote the sale, use or other deal-
 * ings in this Software without prior written authorization from the X Consor-
 * tium.
 *
 * X Window System is a trademark of X Consortium, Inc.
 */
#pragma ident   "@(#)pcf.h	1.2 99/07/25 SMI"

#ifndef _PCF_H
#define _PCF_H
#include <X11/Xproto.h>
#include "general_header.h"
#define XU_IGNORE               -2
#define XU_UCS4UNDEF            -4
#define XU_MOTION_CHAR          1
#define XU_PSBM_CACHED          2
#define XU_PSBM_NOTCACHED       3
#define NOBITMAPREPL		0x0126	/* specific to splmt24.pcf.Z -sun-supplement-medium-r-normal--24-240-75-75-p-240-unicode-fontspecific */
#define FALLBACK_FONT	"splmt24.pcf.Z"
#define PPI             (72)            /* Postscript Points Per Inch */
#define DEF_XRES        PPI     /* default X resolution (72 dpi) */
#define DEF_YRES        PPI     /* default Y resolution (72 dpi) */
#define START_NEW_PAGE  -200.0

typedef unsigned char pcf_bm_t;
typedef struct fontmetric pcf_fontmet_t;
typedef struct charmetric pcf_charmet_t;
typedef struct scaled_fontmetric pcf_SCfontmet_t;
typedef struct scaled_charmetric pcf_SCcharmet_t;
typedef struct _CharInfo *CharInfoPtr;
typedef struct pcffont pcffont_t;

typedef struct _CharInfo {
	xCharInfo   metrics;        /* info preformatted for Queries */
	char       *bits;           /* pointer to glyph image */
}CharInfoRec;

struct scaled_fontmetric {
	double ascent;
	double descent;
	double linespace;
};

struct fontmetric {
	int ptsz;
	int Xres;
	int Yres;
	int ascent;
	int descent;
	int linespace;
	int firstchar;
	int firstCol;
	int lastCol;
	int lastchar;
};
struct charmetric {
	int width;
	int height;
	int widthBits;
	int widthBytes;
	int ascent;
	int descent;
	int LSBearing;
	int RSBearing;
	int origin_xoff;
};

struct scaled_charmetric {
	double width;
	double height;
	double widthBits;
	double ascent;
	double descent;
	double origin_xoff;
};

struct pcffont {
    	char *name;
    	char *file;
	char *cufsym;
	char *cufobj;
	int (*cuf)(ucs4_t);
	int loaded;
	double Xscale;
	double Yscale;
	pcf_fontmet_t Fmetrics;
	pcf_SCfontmet_t scFmetrics;
	CharInfoPtr *bitmaps;       /* ...array of CharInfoPtr */
};
extern void scaling_factors(pcffont_t *, double , int , int );

#endif /* _PCF_H */
