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
/* $XConsortium: omTextPer.c,v 1.2 94/01/20 18:08:28 rws Exp $ */
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

#include "Xlibint.h"
#include "XomGeneric.h"
#include <stdio.h>

static Status
_XomGenericTextPerCharExtents(oc, type, text, length, ink_buf, logical_buf,
			      buf_size, num_chars, overall_ink, overall_logical)
    XOC oc;
    XOMTextType type;
    XPointer text;
    int length;
    XRectangle *ink_buf;
    XRectangle *logical_buf;
    int buf_size;
    int *num_chars;
    XRectangle *overall_ink;
    XRectangle *overall_logical;
{
    XlcConv conv;
    XFontStruct *font;
    Bool is_xchar2b;
    XPointer args[2];
    XChar2b xchar2b_buf[BUFSIZ], *xchar2b_ptr;
    unsigned char *xchar_ptr;
    XCharStruct *def, *cs, overall;
    int buf_len, left, require_num;
    int logical_ascent, logical_descent;
    Bool first = True;

    conv = _XomInitConverter(oc, type);
    if (conv == NULL)
	return 0;
    
    bzero((char *) &overall, sizeof(XCharStruct));
    logical_ascent = logical_descent = require_num = *num_chars = 0;

    args[0] = (XPointer) &font;
    args[1] = (XPointer) &is_xchar2b;

    while (length > 0) {
	xchar2b_ptr = xchar2b_buf;
	left = buf_len = BUFSIZ;

	if (_XomConvert(oc, conv, (XPointer *) &text, &length,
			(XPointer *) &xchar2b_ptr, &left, args, 2) < 0)
	    break;
	buf_len -= left;

	if (require_num) {
	    require_num += buf_len;
	    continue;
	}
	if (buf_size < buf_len) {
	    require_num = *num_chars + buf_len;
	    continue;
	}
	buf_size -= buf_len;

	if (first) {
	    logical_ascent = font->ascent;
	    logical_descent = font->descent;
	} else {
	    logical_ascent = max(logical_ascent, font->ascent);
	    logical_descent = max(logical_descent, font->descent);
	}

	if (is_xchar2b) {
	    CI_GET_DEFAULT_INFO_2D(font, def)
	    xchar2b_ptr = xchar2b_buf;
	} else {
	    CI_GET_DEFAULT_INFO_1D(font, def)
		xchar_ptr = (unsigned char *)xchar2b_buf;
	}

	while (buf_len-- > 0) {
	    if (is_xchar2b) {
		CI_GET_CHAR_INFO_2D(font, xchar2b_ptr->byte1,
				    xchar2b_ptr->byte2, def, cs)
		xchar2b_ptr++;
	    } else {
		CI_GET_CHAR_INFO_1D(font, *xchar_ptr, def, cs)
		xchar_ptr++;
	    }
	    if (cs == NULL)
		continue;

	    ink_buf->x = overall.width + cs->lbearing;
	    ink_buf->y = -(cs->ascent);
	    ink_buf->width = cs->rbearing - cs->lbearing;
	    ink_buf->height = cs->ascent + cs->descent;
	    ink_buf++;

	    logical_buf->x = overall.width;
	    logical_buf->y = -(font->ascent);
	    logical_buf->width = cs->width;
	    logical_buf->height = font->ascent + font->descent;
	    logical_buf++;

	    if (first) {
		overall = *cs;
		first = False;
	    } else {
		overall.ascent = max(overall.ascent, cs->ascent);
		overall.descent = max(overall.descent, cs->descent);
		overall.lbearing = min(overall.lbearing,
				       overall.width + cs->lbearing);
		overall.rbearing = max(overall.rbearing,
				       overall.width + cs->rbearing);
		overall.width += cs->width;
	    }

	    (*num_chars)++;
	}
    }

    if (require_num) {
	*num_chars = require_num;
	return 0;
    } else {
	if (overall_ink) {
	    overall_ink->x = overall.lbearing;
	    overall_ink->y = -(overall.ascent);
	    overall_ink->width = overall.rbearing - overall.lbearing;
	    overall_ink->height = overall.ascent + overall.descent;
	}

	if (overall_logical) {
	    overall_logical->x = 0;
	    overall_logical->y = -(logical_ascent);
	    overall_logical->width = overall.width;
	    overall_logical->height = logical_ascent + logical_descent;
	}
    }

    return 1;
}

Status
#if NeedFunctionPrototypes
_XmbGenericTextPerCharExtents(XOC oc, _Xconst char *text, int length,
			      XRectangle *ink_buf, XRectangle *logical_buf,
			      int buf_size, int *num_chars,
			      XRectangle *overall_ink,
			      XRectangle *overall_logical)
#else
_XmbGenericTextPerCharExtents(oc, text, length, ink_buf, logical_buf,
			      buf_size, num_chars, overall_ink, overall_logical)
    XOC oc;
    char *text;	
    int length;
    XRectangle *ink_buf;
    XRectangle *logical_buf;
    int buf_size;
    int *num_chars;
    XRectangle *overall_ink;
    XRectangle *overall_logical;
#endif
{
    return _XomGenericTextPerCharExtents(oc, XOMMultiByte, (XPointer) text,
					 length, ink_buf, logical_buf, buf_size,
					 num_chars, overall_ink,
					 overall_logical);
}

Status
#if NeedFunctionPrototypes
_XwcGenericTextPerCharExtents(XOC oc, _Xconst wchar_t *text, int length,
			      XRectangle *ink_buf, XRectangle *logical_buf,
			      int buf_size, int *num_chars,
			      XRectangle *overall_ink,
			      XRectangle *overall_logical)
#else
_XwcGenericTextPerCharExtents(oc, text, length, ink_buf, logical_buf,
			      buf_size, num_chars, overall_ink, overall_logical)
    XOC oc;
    wchar_t *text;
    int length;
    XRectangle *ink_buf;
    XRectangle *logical_buf;
    int buf_size;
    int *num_chars;
    XRectangle *overall_ink;
    XRectangle *overall_logical;
#endif
{
    return _XomGenericTextPerCharExtents(oc, XOMWideChar, (XPointer) text,
					 length, ink_buf, logical_buf, buf_size,
					 num_chars, overall_ink,
					 overall_logical);
}
