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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
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
/* $XConsortium: XomGeneric.h,v 1.2 94/01/20 18:03:32 rws Exp $ */
/*
 * Copyright 1992, 1993 by TOSHIBA Corp.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of TOSHIBA not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. TOSHIBA make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * TOSHIBA DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * TOSHIBA BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Katsuhisa Yano	TOSHIBA Corp.
 *			   	mopi@osa.ilab.toshiba.co.jp
 */

#ifndef _XOMGENERIC_H_
#define _XOMGENERIC_H_

#include "XlcPublic.h"

#define XOM_GENERIC(om)		(&((XOMGeneric) om)->gen)
#define XOC_GENERIC(font_set)	(&((XOCGeneric) font_set)->gen)

typedef struct _FontScopeRange {
    unsigned long start;	/* the starting code point */
    unsigned long end;		/* the ending code point */
    unsigned long shift;	/* the code point to scope the font */
} FontScopeRangeRec, *FontScopeRange;

typedef struct _FontScope {
    char *fontname;
    XFontStruct *font;
    Bool load_failed;
    XlcSide side;
    int range_count;
    FontScopeRange range_list;
} FontScopeRec, *FontScope;

typedef enum {
    XOMMultiByte,
    XOMWideChar
} XOMTextType;

typedef struct _FontDataRec {
    char *name;
    XlcSide side;
    unsigned long start;	/* the starting code point */
    unsigned long end;		/* the ending code point */
    int       	fontscope_count;
    FontScope 	fontscope;
} FontDataRec, *FontData;

typedef struct _OMDataRec {
    int charset_count;
    XlcCharSet *charset_list;
    int font_data_count;
    FontData font_data;
    Bool delay_loading;
    Bool no_checking;
    int  char_length;
} OMDataRec, *OMData;

typedef struct _XOMGenericPart {
    int data_num;
    OMData data;
    Bool on_demand_loading;
    char *object_name;
} XOMGenericPart;

typedef struct _XOMGenericRec {
    XOMMethods methods;
    XOMCoreRec core;
    XOMGenericPart gen;
} XOMGenericRec, *XOMGeneric;

/*
 * XOC dependent data
 */

typedef struct _FontSetRec {
    int charset_count;
    XlcCharSet *charset_list;
    int font_data_count;
    FontData font_data;
    char *font_name;
    XFontStruct *info;
    XFontStruct *font;
    XlcSide side;
    Bool is_xchar2b;
    int  char_length;
    Bool delay_loading;
    Bool no_checking;
} FontSetRec, *FontSet;

typedef struct _XOCGenericPart {
    XlcConv mbs_to_cs;
    XlcConv wcs_to_cs;
    int font_set_num;
    FontSet font_set;
} XOCGenericPart;

typedef struct _XOCGenericRec {
    XOCMethods methods;
    XOCCoreRec core;	
    XOCGenericPart gen;
} XOCGenericRec, *XOCGeneric;

_XFUNCPROTOBEGIN

extern XOM _XomGenericOpenOM(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    Display*		/* dpy */,
    XrmDatabase		/* rdb */,
    _Xconst char*	/* res_name */,
    _Xconst char*	/* res_class */
#endif
);

extern XlcConv _XomInitConverter(
#if NeedFunctionPrototypes
    XOC			/* oc */,
    XOMTextType		/* type */
#endif
);

extern int _XomConvert(
#if NeedFunctionPrototypes
    XOC			/* oc */,
    XlcConv		/* conv */,
    XPointer*		/* from */,
    int*		/* from_left */,
    XPointer*		/* to */,
    int*		/* to_left */,
    XPointer*		/* args */,
    int			/* num_args */
#endif
);

_XFUNCPROTOEND

#endif  /* _XOMGENERIC_H_ */
