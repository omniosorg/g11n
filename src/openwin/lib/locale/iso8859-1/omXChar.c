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
/* $XConsortium: omXChar.c,v 1.3 94/02/06 15:10:11 rws Exp $ */
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

static FontSet
_XomGetFontSetFromCharSet(oc, charset)
    XOC oc;
    XlcCharSet charset;
{
    FontSet font_set = XOC_GENERIC(oc)->font_set;
    int num = XOC_GENERIC(oc)->font_set_num;
    XlcCharSet *charset_list;
    int charset_count;

    for ( ; num-- > 0; font_set++) {
	charset_count = font_set->charset_count;
	charset_list = font_set->charset_list;
	for ( ; charset_count-- > 0; charset_list++)
	    if (*charset_list == charset)
		return font_set;
    }

    return (FontSet) NULL;
}

static void
shift_to_gl(text, length)
    char *text;
    int length;
{
    while (length-- > 0)
	*text++ &= 0x7f;
}

static void
shift_to_gr(text, length)
    char *text;
    int length;
{
    while (length-- > 0)
	*text++ |= 0x80;
}

static Bool
load_font(oc, font_set)
    XOC oc;
    FontSet font_set;
{
    if (font_set->delay_loading == False) {
	/* Give up displaying any characters of this missing charset. */
	return False;
    }

    font_set->font = XLoadQueryFont(oc->core.om->core.display,
				    font_set->font_name);
    if (font_set->font == NULL)
	return False;
    
    XFreeFontInfo(NULL, font_set->info, 1);
    font_set->info = NULL;

    if (font_set->font->min_byte1 || font_set->font->max_byte1)
	font_set->is_xchar2b = True;
    else
	font_set->is_xchar2b = False;

    return True;
}

static int
get_fontscope(font_set, buffer, length, args, num_args)
FontSet font_set;
char *buffer;
int length;
XPointer *args;
int num_args;
{
    int i, j, k;
    char *p;

    int count = font_set->font_data->fontscope_count;

    if (count < 1) return length;

    if (font_set->is_xchar2b)
	length >>= 1;

    p = buffer;
    for (i = 0; i < length; i++) {
   	unsigned long code;
	FontScope fontscope = font_set->font_data->fontscope;
	if (font_set->is_xchar2b) {
	    if (font_set->side == XlcGL) {
		code = (((*p++) & 0x7f) << 8);
		code += (*p++) & 0x7f;
	    } else if (font_set->side == XlcGR) {
		code = (((*p++) | 0x80) << 8);
		code += (*p++) | 0x80;
	    } else {
		code = ((*p++) << 8);
		code += (*p++);
	    }
	} else {
	    if (font_set->side == XlcGL) {
		code = (*p++) & 0x7f;
	    } else if (font_set->side == XlcGR) {
		code = (*p++) | 0x80;
	    } else {
		code = *p++;
	    }
	}
	for (j = 0; j < count; j++, fontscope++) {
	    FontScopeRange range_list = fontscope->range_list;
	    int range_count = fontscope->range_count;
	    for (k = 0; k < range_count; k++, range_list++) {
		if (range_list->start <= code &&
		    code <= range_list->end) {
		    *((FontScope *)args[0]) = fontscope;
		    if (font_set->is_xchar2b)
			i <<= 1;
		    return i;
		}
	    }
	}
    }
    if (font_set->is_xchar2b)
	length <<= 1;
    return length;
}

static void
load_font_of_fontscope(oc, primary_font, font_scope)
XOC oc;
char *primary_font;
FontScope font_scope;
{
    char **name_list, **cur_name_list;
    char *pattern, *last, buf[BUFSIZ];
    char *base_name;
    int count, num_fields;
    int length;
    Bool append_charset;
    Bool first_time = True;

    name_list = _XParseBaseFontNameList(oc->core.base_name_list, &count);
    if (name_list == NULL)
	return;
    cur_name_list = name_list;

    count++; /* for primary_font */

    while (count-- > 0) {
	if (first_time) {
	    pattern = primary_font;
	    first_time = False;
	} else {
	    pattern = *cur_name_list++;
	}
	if (pattern == NULL || *pattern == '\0')
	    continue;

	append_charset = False;

	strcpy(buf, pattern);
	length = strlen(pattern);
	last = buf + length - 1;
	for (num_fields = 0, base_name = buf; *base_name != '\0';
	     base_name++)
	    if (*base_name == '-') num_fields++;
	if (strchr(pattern, '*') == NULL) {
	    if (num_fields == 12) {
		append_charset = True;
		*++last = '-';
		last++;
	    }
	}
	if (append_charset == False) {
	    if (num_fields == 13 || num_fields == 14) {
		append_charset = True;
		last = strrchr (buf, '-');
		if (num_fields == 14) {
		    *last = '\0';
		    last = strrchr (buf, '-');
		}
		last++;
	    } else if (*last == '*') {
		append_charset = True;
		if (length > 3 && *(last-3) == '-' && *(last-2) == '*'
		    && *(last-1) == '-') {
		    last -= 2;
		}
		*++last = '-';
		last++;
	    }
	}
	if (append_charset)
	    strcpy(last, font_scope->fontname);
	font_scope->font = XLoadQueryFont(oc->core.om->core.display,
					  buf);
	if (font_scope->font)
	    break;
    }

    if (name_list) XFreeStringList(name_list);

    /* If failed in loading, set load_failed flag and don't try
       to load the same font again. */
    if (font_scope->font == NULL)
	font_scope->load_failed = True;
    return;
}

static int
apply_fontscope(oc, font_scope, font_set,
		from, from_len, to, to_len, args, num_args)
XOC oc;
FontScope font_scope;
FontSet font_set;
char *from;
int from_len;
char *to;
int to_len;
XPointer *args;
int num_args;
{
    int i;
    FontScopeRange range_list;
    int range_count = font_scope->range_count;
    int length = 0;
    int cs_len;
    unsigned long code;

    if (font_scope->font == NULL && font_scope->load_failed == False) {
	load_font_of_fontscope(oc, font_set->font_name, font_scope);
    }
    *((XFontStruct **) args[0]) = font_scope->font;

    if (font_set->is_xchar2b)
	from_len >>= 1;
 
    while (from_len) {
	Bool found = False;
	code = 0;
	if (font_set->is_xchar2b) {
	    if (font_set->side == XlcGL) {
		code = (((*from++) & 0x7f) << 8);
		code += (*from++) & 0x7f;
	    } else if (font_set->side == XlcGR) {
		code = (((*from++) | 0x80) << 8);
		code += (*from++) | 0x80;
	    } else {
		code = ((*from++) << 8);
		code += (*from++);
	    }
	} else {
	    if (font_set->side == XlcGL) {
		code = (*from++) & 0x7f;
	    } else if (font_set->side == XlcGR) {
		code = (*from++) | 0x80;
	    } else {
		code = *from++;
	    }
	}
	from_len -= 1;
	range_list = font_scope->range_list;
	for (i = 0; i < range_count; i++, range_list++) {
	    if (range_list->start <= code &&
		code <= range_list->end) {
		code += (range_list->shift - range_list->start);
		if (font_set->is_xchar2b) {
		    *to++ = (char)((code & 0xff00) >> 8);
		    *to++ = (char)(code & 0x7f);
		    length += 2;
		} else {
		    *to++ = (char)code;
		    length += 1;
		}
		found = True;
		break;
	    }
	}
	if (found == False) break;
    }
    return length;
}

int
_XomConvert(oc, conv, from, from_left, to, to_left, args, num_args)
    XOC oc;
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XPointer cs, lc_args[1];
    XlcCharSet charset;
    int length, cs_left, cs_len;
    int ret = -1;
    FontSet font_set;
    char *buf, buf_local[BUFSIZ];
    FontScope font_scope;
    XPointer sc_args[1];
    XPointer from_save;
    int from_left_save;
    
    buf = (*to_left > BUFSIZ)? Xmalloc(*to_left) : buf_local;
    if (buf == NULL)
	return ret;
    cs = buf;
    cs_left = (*to_left > BUFSIZ)? *to_left : BUFSIZ;
    memset(buf, 0, cs_left);

    lc_args[0] = (XPointer) &charset;

    from_save = *from;
    from_left_save = *from_left;

    ret = _XlcConvert(conv, from, from_left, &cs, &cs_left, lc_args, 1);
    if (ret < 0)
	goto err;

    font_set = _XomGetFontSetFromCharSet(oc, charset);
    if (font_set == NULL) {
	ret = -1;
	goto err;
    }

    if (font_set->font == NULL && load_font(oc, font_set) == False) {
	ret = -1;
	goto err;
    }

    length = *to_left - cs_left;

    if (length == 0) {
	ret = -1;
	goto err;
    }

    sc_args[0] = (XPointer)&font_scope;
    cs_len = get_fontscope(font_set, buf, length, sc_args, 1);
    if (cs_len == 0) {
	/* apply fontscope from the beginning of buffer */
	char *buf2, buf2_local[BUFSIZ];
	XFontStruct *font;
	XPointer xfs_arg[1];
	int to_len;

	buf2 = (length > BUFSIZ)? Xmalloc(length) : buf2_local;
	if (buf2 == NULL) {
	    ret = -1;
	    goto err;
	}
	to_len = (length > BUFSIZ)? length : BUFSIZ;
	memset(buf2, 0, to_len);

	xfs_arg[0] = (XPointer)&font;
	cs_len = apply_fontscope(oc, font_scope, font_set,
				 buf, length, buf2, to_len,
				 xfs_arg, 1);
	if (font != NULL && cs_len != 0) {
	    memcpy((char *)*to, buf2, cs_len);
	    *((XFontStruct **) args[0]) = font;
	} else {
	    memcpy((char *)*to, buf, cs_len);
	    *((XFontStruct **) args[0]) = font_set->font;
	}
	if (buf2 != buf2_local) Xfree(buf2);
    } else {
      if (font_set->is_xchar2b && font_set->char_length == 1) {
	/* for Arabic only so far */
	int i;
	for (i = 0; i < length; i++) {
	  (*to)[i*2 + 1] = buf[i];
	  (*to)[i*2] = 0;	
	}
      } else {
	memcpy((char *)*to, buf, cs_len);
      }
      *((XFontStruct **) args[0]) = font_set->font;
    }
    *((Bool *) args[1]) = font_set->is_xchar2b;

    if (font_set->side != charset->side) {
	if (font_set->side == XlcGL)
	    shift_to_gl(*to, cs_len);
	else if (font_set->side == XlcGR)
	    shift_to_gr(*to, cs_len);
    }

    *to += cs_len;

    if (cs_len < length) {
	XPointer buf = *from;
	*from = from_save;
	*from += (buf - from_save) * cs_len / length;
	if (font_set->is_xchar2b && !(font_set->char_length == 1)) {
	  *from_left = from_left_save - cs_len/2;
	} else {
	  *from_left = from_left_save - from_left_save * cs_len / length;
	}
    }
    if (font_set->is_xchar2b && !(font_set->char_length == 1)) {
      cs_len >>= 1;
    }

    *to_left -= cs_len;

 err:
    if (buf != buf_local)
	Xfree(buf);

    return ret;
}

XlcConv
_XomInitConverter(oc, type)
    XOC oc;
    XOMTextType type;
{
    XOCGenericPart *gen = XOC_GENERIC(oc);
    XlcConv conv;
    char *conv_type;
    XLCd lcd;

    if (type == XOMWideChar) {
	conv = gen->wcs_to_cs;
	conv_type = XlcNWideChar;
    } else {
	conv = gen->mbs_to_cs;
	conv_type = XlcNMultiByte;
    }

    if (conv) {
	_XlcResetConverter(conv);
	return conv;
    }

    lcd = oc->core.om->core.lcd;

    conv = _XlcOpenConverter(lcd, conv_type, lcd, XlcNCharSet);
    if (conv == (XlcConv) NULL)
	return (XlcConv) NULL;

    if (type == XOMWideChar)
	gen->wcs_to_cs = conv;
    else
	gen->mbs_to_cs = conv;

    return conv;
}
