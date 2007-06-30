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
#pragma ident   "@(#)ttf.h	1.2 99/07/25 SMI"

#ifndef _TTF_H
#define _TTF_H

#include <stdio.h>
#include <sys/types.h>
#include "general_header.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define BOOL int
#define TT_NOTFOUND     -1
#define TT_BADFONT      -2
#define TT_BADFORMAT    -3
#define TT_SUCCESS       0
#define TT_MOVETO       1
#define TT_LINETO       2
#define TT_CURVETO      3

typedef short fixp;
typedef union  TTOutlineSegment tt_OutlineSegment_t;
typedef struct TTLineSegment tt_LineSegment_t;
typedef struct TTMoveSegment tt_MoveSegment_t;
typedef struct TTCurveSegment tt_CurveSegment_t;
typedef struct TTOutlineData tt_OutlineData_t;
typedef struct tt_GlyphData tt_GlyphData_t;
typedef struct tt_longHorMetric_t tt_longHorMetric_t;
typedef struct tt_FontData tt_FontData_t;
typedef struct tt_CmapData tt_CmapData_t;
typedef struct ttffont ttffont_t;
typedef struct subglyph subglyph_t;
typedef struct matrix matrix_t;

struct matrix {
	signed long  xx, xy;
	signed long  yx, yy;
} ;

struct TTLineSegment {
	int		x1;
	int		y1;
};

struct TTMoveSegment {
	int		x1;
	int		y1;
};

struct TTCurveSegment {
	double		x1;
	double		y1;
	double		x2;
	double		y2;
	double		x3;
	double		y3;
};

union TTOutlineSegment {
	tt_LineSegment_t        *lineto;
	tt_MoveSegment_t        *moveto;
	tt_CurveSegment_t       *curveto;
};

struct TTOutlineData {
	int	count;
	float	xsize;
	int	*data_type;
	tt_OutlineSegment_t	*data;
	struct TTOutlineData *next;
};
struct tt_CmapData   {
	ushort_t        segCount;
	ushort_t        *endCount;
	ushort_t        *startCount;
	ushort_t        *idDelta;
	ushort_t        *idRangeOffset;
	fpos_t          gryphIdArray;
	ushort_t        unitsPerEm;
	short           xmin;
	short           ymin;
	short           xmax;
	short           ymax;
	ushort_t        encoding;
	short           iTLF;
};

struct tt_GlyphData   {
	ushort_t         	*epts_ctr;
	ushort_t          	num_pts;
	short           	num_ctr;
	fixp           		*xcoor;
	fixp           		*ycoor;
	uchar_t          	*flag;
	struct tt_GlyphData	*next;
};

struct tt_longHorMetric_t {
	ushort_t	aw;
	short		lsb;
};

struct tt_FontData {
	ulong_t           cmap;
	ulong_t           loca;
	ulong_t           glyf;
	ulong_t           head;
	ulong_t           maxp;
	ulong_t           hhea;
	ulong_t           hmtx;
	ushort_t          numGlyphs;
	ushort_t          numberOfHMetrics;
	tt_longHorMetric_t        *hmetrics;
};
struct ttffont {
	char            *name;
	char            *file;
	char            *cufsym;
	char            *cufobj;
	int             (*cuf)(ucs4_t);
	int             loaded;
	int             ttc_num;
	FILE            *fn;
	ulong_t         ttc_offset;
	tt_FontData_t   *fontd;
	tt_CmapData_t   *cmapd;
};

struct subglyph {
	long index;
	BOOL is_scaled;
	BOOL is_hinted;
	BOOL keep_ppts;
	long file_offset;
	long arg1;
	long arg2;
	ushort_t flags;
	matrix_t transform;
	struct subglyph *next;
};


#endif /* _TTF_H */
