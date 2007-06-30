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

/* $XConsortium: lcGenConv.c,v 1.7 95/02/22 22:03:01 kaleb Exp $ */
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
#include "XlcGeneric.h"
#include <stdio.h>
#include <widec.h>

#if !defined(X_NOT_STDC_ENV) && !defined(macII) && !defined(sun)
#define STDCVT
#endif

typedef struct _StateRec {
    XLCd lcd;
    XlcCharSet charset;
    XlcCharSet GL_charset;
    XlcCharSet GR_charset;
} StateRec, *State;

static void
init_state(conv)
    XlcConv conv;
{
    register State state = (State) conv->state;
    register XLCdGenericPart *gen = XLC_GENERIC_PART(state->lcd);
    register CodeSet codeset;

    codeset = gen->initial_state_GL;
    if (codeset && codeset->charset_list)
	state->GL_charset = *codeset->charset_list;
    codeset = gen->initial_state_GR;
    if (codeset && codeset->charset_list)
	state->GR_charset = *codeset->charset_list;

    if (state->GL_charset == NULL)
      if (gen->codeset_list != NULL)
	if (codeset == *gen->codeset_list)
	    state->GL_charset = *codeset->charset_list;
}

static int
compare(src, encoding, length)
    register char *src;
    register char *encoding;
    register int length;
{
    char *start = src;

    while (length-- > 0) {
	if (*src++ != *encoding++)
	    return 0;
	if (*encoding == '\0')
	    return src - start;
    }

    return 0;
}

static int
mbtocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    XLCd lcd = state->lcd;
    register char *src, *dst;
    unsigned char *mb_parse_table;
    ParseInfo *parse_list, parse_info;
    XlcCharSet charset;
    int length, number, encoding_len = 0;
    register i;

    src = *((char **) from);
    dst = *((char **) to);

    if (mb_parse_table = XLC_GENERIC(lcd, mb_parse_table)) {
	number = mb_parse_table[(unsigned char) *src];
	if (number > 0) {
	    parse_list = XLC_GENERIC(lcd, mb_parse_list) + number - 1;
	    for ( ; parse_info = *parse_list; parse_list++) {
		encoding_len = compare(src, parse_info->encoding, *from_left);
		if (encoding_len > 0) {
		    switch (parse_info->type) {
		      case E_SS:	
			    charset = *parse_info->codeset->charset_list;
			    if (num_args == 2 &&
				(XlcCharSet)args[1] != charset)
				/* check if charset is changed now */
				return -1;
			    src += encoding_len;
			    goto found;
			case E_LSL:
			case E_LSR:
			    src += encoding_len;
			    charset = *parse_info->codeset->charset_list;
			    if (parse_info->type == E_LSL)
			    	state->GL_charset = charset;
			    else
			    	state->GR_charset = charset;
			    length = 0;
			    goto end;
			case E_GL:
			    charset = state->GL_charset;
			    goto found;
			case E_GR:
			    charset = state->GR_charset;
			    goto found;
		    }
		}
	    }
	}
    }

    if ((*src & 0x80) && state->GR_charset)
	charset = state->GR_charset;
    else
	charset = state->GL_charset;

found:
    if (charset == NULL ||
	(num_args == 2 && (XlcCharSet) args[1] != charset))
	return -1;

    length = charset->char_size;
    if (length > *from_left - encoding_len)
	return -1;

    if (dst) {
	if (length > *to_left)
	    return -1;
	if (charset->side == XlcGL) {
	    for (i = 0; i < length; i++)
		*dst++ = *src++ & 0x7f;
	} else if (charset->side == XlcGR) {
	    for (i = 0; i < length; i++)
		*dst++ = *src++ | 0x80;
	} else {
	    for (i = 0; i < length; i++)
		*dst++ = *src++;
	}
	*to = (XPointer) dst;
	*to_left -= length;
    }
end:
    *from = (XPointer) src;
    *from_left -= encoding_len + length;
    state->charset = charset;
    if (num_args == 1)
	*((XlcCharSet *) args[0]) = charset;

    return 0;
}

static int
mbstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XlcCharSet charset = NULL;
    XPointer tmp_args[2], save_from = *from;
    int ret, unconv_num = 0, tmp_num = 1;

    tmp_args[0] = (XPointer) &charset;

    while (*from_left > 0 && *to_left > 0) {
	ret = mbtocs(conv, from, from_left, to, to_left, tmp_args, tmp_num);
	if (ret < 0)
	    break;
	unconv_num += ret;
	if (tmp_num == 1 && charset) {
	    tmp_args[1] = (XPointer) charset;
	    tmp_num = 2;
	}
    }

    if (save_from == *from)
	return -1;

    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;
    
    return unconv_num;
}

static int
wcstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XlcCharSet charset = NULL;
    XPointer tmp_args[2], save_from = *from;
    int ret, unconv_num = 0, tmp_num = 1;
    char	mb_buf[10];
    char	*from_mb;
    int		mb_len = 0;
    int		mb_left = 0;
    wchar_t	*wcptr;

    if (from == NULL || *from == NULL)
	return 0;

    tmp_args[0] = (XPointer) &charset;

    wcptr = *((wchar_t **)from);

    while (*from_left > 0 && *to_left > 0) {
	mb_len = wctomb(mb_buf, *wcptr);
	if(mb_len == -1)
		return -1;
	mb_buf[mb_len] = '\0';
	mb_left = mb_len;
	from_mb = mb_buf;
	ret = mbtocs(conv, &from_mb, &mb_left, to, to_left, tmp_args, tmp_num);
	if (ret < 0)
	    break;
	unconv_num += ret;
	wcptr++;
	(*from_left)--; 
	if (tmp_num == 1 && charset) {
	    tmp_args[1] = (XPointer) charset;
	    tmp_num = 2;
	}
    }

    *from = (XPointer)wcptr;

    if (save_from == *from)
	return -1;

    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;
    
    return unconv_num;
}

static CodeSet
_XlcGetCodeSetFromCharSet(state, charset)
    State state;
    XlcCharSet charset;
{
    XLCd lcd = state->lcd;
    register CodeSet *codeset = XLC_GENERIC(lcd, codeset_list);
    register XlcCharSet *charset_list;
    register codeset_num, num_charsets;

    codeset_num = XLC_GENERIC(lcd, codeset_num);

    for ( ; codeset_num-- > 0; codeset++) {
	num_charsets = (*codeset)->num_charsets;
	charset_list = (*codeset)->charset_list;

	for ( ; num_charsets-- > 0; charset_list++)
	    if (*charset_list == charset)
		return *codeset;
    }

    /* fallback */
    if (charset->side == XlcGL) {
        charset = state->GL_charset;
    } else {
        charset = state->GR_charset;
    }
    codeset_num = XLC_GENERIC(lcd, codeset_num);
    codeset = XLC_GENERIC(lcd, codeset_list);
    for ( ; codeset_num-- > 0; codeset++) {
        num_charsets = (*codeset)->num_charsets;
        charset_list = (*codeset)->charset_list;
        for ( ; num_charsets-- > 0; charset_list++)
            if (*charset_list == charset)
                return *codeset;
    }

}

static int
cstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    register char *csptr;
    register char *bufptr;
    int csstr_len;
    register buf_len;
    int num, encoding_len = 0;
    CodeSet codeset;
    XlcCharSet charset;
    EncodingType type;
    int cvt_length;

    csptr = *((char **) from);
    bufptr = *((char **) to);
    csstr_len = *from_left;
    buf_len = *to_left;

    if (num_args < 1)
	return -1;
    
    charset = (XlcCharSet) args[0];

    codeset = _XlcGetCodeSetFromCharSet(state, charset);
    if (codeset == NULL)
	return -1;

    cvt_length = 0;
    if (codeset->parse_info) {
	switch (type = codeset->parse_info->type) {
	    case E_SS:
		encoding_len = strlen(codeset->parse_info->encoding);
		break;
	    case E_LSL:
	    case E_LSR:
		if (type == E_LSL) {
		    if (charset == state->GL_charset)
			break;
		} else {
		    if (charset == state->GR_charset)
			break;
		}
		encoding_len = strlen(codeset->parse_info->encoding);
		if (encoding_len > buf_len)
		    return -1;
		cvt_length += encoding_len;
		if (bufptr) {
		    strcpy(bufptr, codeset->parse_info->encoding);
		    bufptr += encoding_len;
		}
		buf_len -= encoding_len;
		encoding_len = 0;
		if (type == E_LSL)
		    state->GL_charset = charset;
		else
		    state->GR_charset = charset;
		break;
	}
    }

    csstr_len /= codeset->length;
    buf_len /= codeset->length + encoding_len;
#ifndef sun
    if (csstr_len < buf_len)
	buf_len = csstr_len;
    
    cvt_length += buf_len * (encoding_len + codeset->length);
#endif
    if (bufptr) {
#ifdef sun
	while (buf_len-- && csstr_len--) {
	    cvt_length += encoding_len + codeset->length;
#else
	while (buf_len--) {
#endif
	    if (encoding_len) {
		strcpy(bufptr, codeset->parse_info->encoding);
		bufptr += encoding_len;
	    }
	    num = codeset->length;
	    if (codeset->side == XlcGL) {
		while (num--)
		    *bufptr++ = *csptr++ & 0x7f;
	    } else if (codeset->side == XlcGR) {
		while (num--)
		    *bufptr++ = *csptr++ | 0x80;
	    } else {
		while (num--)
		    *bufptr++ = *csptr++;
	    }
    	}
#ifdef sun
    } else {
	cvt_length += csstr_len * (encoding_len + codeset->length);
	csptr += csstr_len * (codeset->length);
#endif
    }

    *from_left -= csptr - *((char **) from);
    *from = (XPointer) csptr;

    if (bufptr)
	*to = (XPointer) bufptr;
    *to_left -= cvt_length;

    return 0;
}

static int
cstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    XLCd lcd = state->lcd;
    wchar_t *bufptr;
    register int buf_len = *to_left;
    char mb_data[BUFSIZ], *to_mb_data;
    int mb_len, to_mb_left;
    int wc_len;
    char *csptr;
    int csstr_len;

    csptr = *((char **) from);
    csstr_len = *from_left;

    mb_len = BUFSIZ;
    memset(mb_data, 0, mb_len);

    to_mb_data = mb_data;
    to_mb_left = mb_len;

    bufptr = *((wchar_t **) to);

    if (cstombs(conv, &csptr, &csstr_len, &to_mb_data, &to_mb_left,
		args, num_args) == -1)
	return -1;

    mb_data[mb_len - to_mb_left] = (char)0;
    wc_len = mbstowcs(NULL, mb_data, mb_len - to_mb_left);
    if (wc_len > buf_len) {
	/* overflow */
	*to_left = 0;
	return -1;
    }

    if (wc_len != mbstowcs(bufptr, mb_data, mb_len - to_mb_left))
	return -1;

    *from = csptr;
    *from_left = csstr_len;
    *to_left -= wc_len;
    if (bufptr != NULL) {
	bufptr += wc_len;
	*to = (XPointer)bufptr;
    }
    return 0;
}


static void
close_converter(conv)
    XlcConv conv;
{
    if (conv->state) {
	Xfree((char *) conv->state);
    }
    if (conv->methods) {
	XFree((char*) conv->methods);
    }

    Xfree((char *) conv);
}

static XlcConv
create_conv(lcd, methods)
    XLCd lcd;
    XlcConvMethods methods;
{
    XlcConv conv;
    State state;

    conv = (XlcConv) Xmalloc(sizeof(XlcConvRec));
    if (conv == NULL)
	return (XlcConv) NULL;
    
    conv->methods = (XlcConvMethods) Xmalloc(sizeof(XlcConvMethodsRec));
    if (conv->methods == NULL)
	goto err;
    *conv->methods = *methods;
    if (XLC_PUBLIC(lcd, is_state_depend))
	conv->methods->reset = init_state;

    conv->state = (XPointer) Xmalloc(sizeof(StateRec));
    if (conv->state == NULL)
	goto err;
    bzero((char *) conv->state, sizeof(StateRec));
    
    state = (State) conv->state;
    state->lcd = lcd;
    init_state(conv);
    
    return conv;

err:
    close_converter(conv);

    return (XlcConv) NULL;
}

static XlcConvMethodsRec mbstocs_methods = {
    close_converter,
    mbstocs,
    NULL
} ;

static XlcConv
open_mbstocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &mbstocs_methods);
}

static XlcConvMethodsRec wcstocs_methods = {
    close_converter,
    wcstocs,
    NULL
} ;

static XlcConv
open_wcstocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &wcstocs_methods);
}

static XlcConvMethodsRec mbtocs_methods = {
    close_converter,
    mbtocs,
    NULL
} ;

static XlcConv
open_mbtocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &mbtocs_methods);
}

static XlcConvMethodsRec cstombs_methods = {
    close_converter,
    cstombs,
    NULL
} ;

static XlcConv
open_cstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &cstombs_methods);
}

static XlcConvMethodsRec cstowcs_methods = {
    close_converter,
    cstowcs,
    NULL
} ;

static XlcConv
open_cstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &cstowcs_methods);
}


static int
strtombs(conv, from, from_left, to, to_left, args, num_args)
XlcConv conv;
XPointer *from;
int *from_left;
XPointer *to;
int *to_left;
XPointer *args;
int num_args;
{
    register char *strptr;
    register char *mbsptr;
    int length = *from_left;
    strptr = *((char **)from);
    mbsptr = *((char **)to);

    memcpy(mbsptr, strptr, length);
    *from += length;
    *from_left = 0;
    *to += length;
    *to_left -= length;
    return 0;
}

static XlcConvMethodsRec strtombs_methods = {
    close_converter,
    strtombs,
    NULL
};

static XlcConv
open_strtombs(from_lcd, from_type, to_lcd, to_type)
XLCd from_lcd;
char *from_type;
XLCd to_lcd;
char *to_type;
{
    return create_conv(from_lcd, &strtombs_methods);
}


#ifdef STDCVT
static int
stdc_mbstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    char *src = *((char **) from);
    wchar_t *dst = *((wchar_t **) to);
    int src_left = *from_left;
    int dst_left = *to_left;
    int length;

    while (src_left > 0 && dst_left > 0) {
	length = mbtowc(dst, src, src_left);
	if (length < 0)
	    break;

	src += length;
	src_left -= length;
	if (dst)
	    dst++;
	dst_left--;

	if (length == 0) {
	    src++;
	    src_left--;
	    break;
	}
    }

    if (*from_left == src_left)
	return -1;

    *from = (XPointer) src;
    if (dst)
	*to = (XPointer) dst;
    *from_left = src_left;
    *to_left = dst_left;

    return 0;
}

static int
stdc_wcstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    wchar_t *src = *((wchar_t **) from);
    char *dst = *((char **) to);
    int src_left = *from_left;
    int dst_left = *to_left;
    int length;

    while (src_left > 0 && dst_left > 0) {
	length = wctomb(dst, *src);		/* XXX */
	if (length < 0 || dst_left < length)
	    break;

	src++;
	src_left--;
	dst += length;
	dst_left -= length;

	if (length == 0) {
	    dst++;
	    dst_left--;
	    break;
	}
    }

    if (*from_left == src_left)
	return -1;

    *from = (XPointer) src;
    *to = (XPointer) dst;
    *from_left = src_left;
    *to_left = dst_left;

    return 0;
}

static int
stdc_wcstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    wchar_t wch, *src = *((wchar_t **) from);
    XlcCharSet charset = NULL;
    XPointer tmp_args[2], tmp_from, save_from = *from;
    char tmp[32];
    int length, ret, src_left = *from_left;
    int unconv_num = 0, tmp_num = 1;

    tmp_args[0] = (XPointer) &charset;

    while (src_left > 0 && *to_left > 0) {
	if (wch = *src) {
	    length = wctomb(tmp, wch);
	} else {
	    length = 1;
	    *tmp = '\0';
	}
		
	if (length < 0)
	    break;

	tmp_from = (XPointer) tmp;
	ret = mbtocs(conv, &tmp_from, &length, to, to_left, tmp_args, tmp_num);
	if (ret < 0)
	    break;
	unconv_num += ret;
	if (tmp_num == 1 && charset) {
	    tmp_args[1] = (XPointer) charset;
	    tmp_num = 2;
	}

	src++;
	src_left--;
    }

    if (save_from == (XPointer) src)
	return -1;

    *from = (XPointer) src;
    *from_left = src_left;

    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;
    
    return unconv_num;
}

#define DefineLocalBuf		char local_buf[BUFSIZ]
#define AllocLocalBuf(length)	(length > BUFSIZ ? (char*) Xmalloc(length) : local_buf)
#define FreeLocalBuf(ptr)	if (ptr != local_buf) Xfree(ptr)

static int
stdc_cstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = ((State) conv->state)->lcd;
    DefineLocalBuf;
    XPointer buf, save_buf;
    int length, left, ret;
    
    left = length = *to_left * XLC_PUBLIC(lcd, mb_cur_max);
    buf = save_buf = (XPointer) AllocLocalBuf(length);
    if (buf == NULL)
	return -1;

    ret = cstombs(conv, from, from_left, &buf, &left, args, num_args);
    if (ret < 0)
	goto err;
    
    buf = save_buf;
    length -= left;
    if (stdc_mbstowcs(conv, &buf, &length, to, to_left, args, num_args) < 0)
	ret = -1;

err:
    FreeLocalBuf(save_buf);

    return ret;
}

static XlcConvMethodsRec stdc_mbstowcs_methods = {
    close_converter,
    stdc_mbstowcs,
    NULL
} ;

static XlcConv
open_stdc_mbstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &stdc_mbstowcs_methods);
}

static XlcConvMethodsRec stdc_wcstombs_methods = {
    close_converter,
    stdc_wcstombs,
    NULL
} ;

static XlcConv
open_stdc_wcstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &stdc_wcstombs_methods);
}

static XlcConvMethodsRec stdc_wcstocs_methods = {
    close_converter,
    stdc_wcstocs,
    NULL
} ;

static XlcConv
open_stdc_wcstocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &stdc_wcstocs_methods);
}

static XlcConvMethodsRec stdc_cstowcs_methods = {
    close_converter,
    stdc_cstowcs,
    NULL
} ;

static XlcConv
open_stdc_cstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &stdc_cstowcs_methods);
}
#endif /* STDCVT */


#ifndef FORCE_INDIRECT_CONVERTER

typedef unsigned char   Uchar;

typedef struct _CTDataRec {
    int side;
    int length;
    char *name;
    char *ct_encoding;
    int ct_encoding_len;
    int set_size;
    Uchar min_ch;
    Uchar ct_type;
} CTDataRec, *CTData;

typedef struct _CT_StateRec {
    CTData GL_charset;
    CTData GR_charset;
    CTData charset;
} CT_StateRec;

#define CT_STD  0
#define CT_NSTD 1
#define CT_DIR  2
#define CT_EXT0 3
#define CT_EXT1 4
#define CT_EXT2 5
#define CT_VER  6

static CTData ctdata;
static CTData ctd_endp;
static CTData *ctdptr;

#define MAX_CTINFO	30
#define MAX_CTDATA_EXT	12

static CTDataRec ctdata_ext[] =
{
  { XlcUnknown, 0, "Ignore-Ext-Status?",   "\033#"    ,  2, 0, 0, CT_VER  },
  { XlcUnknown, 0, "NonStd-?-OctetChar",   "\033%/0"  ,  4, 0, 0, CT_NSTD },
  { XlcUnknown, 1, "NonStd-1-OctetChar",   "\033%/1"  ,  4, 0, 0, CT_NSTD },
  { XlcUnknown, 2, "SUNUDCJA.1997-0",      "\033%/2"  ,  4, 0, 0, CT_NSTD },
  { XlcUnknown, 3, "NonStd-3-OctetChar",   "\033%/3"  ,  4, 0, 0, CT_NSTD },
  { XlcUnknown, 4, "NonStd-4-OctetChar",   "\033%/4"  ,  4, 0, 0, CT_NSTD },
  { XlcUnknown, 0, "Extension-2",          "\033%/"   ,  3, 0, 0, CT_EXT2 },
  { XlcUnknown, 0, "Extension-0",          "\033"     ,  1, 0, 0, CT_EXT0 },
  { XlcUnknown, 0, "Begin-L-to-R-Text",    "\2331]"   ,  3, 0, 0, CT_DIR  },
  { XlcUnknown, 0, "Begin-R-to-L-Text",    "\2332]"   ,  3, 0, 0, CT_DIR  },
  { XlcUnknown, 0, "End-Of-String",        "\233]"    ,  2, 0, 0, CT_DIR  },
  { XlcUnknown, 0, "Extension-1",          "\233"     ,  1, 0, 0, CT_EXT1 },
};

#define Ascii     0
#define Kanji     1
#define Kana      2
#define HojoKanji 3
#define Userdef   4

#define MAX_NAME_LEN            18
#define MAX_CT_ENCODING_LEN     4

#ifdef SS2
#undef SS2
#define SS2     0x8e    /* Single-shift char: CS2 */
#endif
#ifdef SS3
#undef SS3
#define SS3     0x8f    /* Single-shift char: CS3 */
#endif

#define GR      0x80    /* begins right-side (non-ascii) region */
#define GL      0x7f    /* ends left-side (ascii) region        */

#define isleftside(c)   (((c) & GR) ? 0 : 1)
#define isrightside(c)  (!isleftside(c))

#define BIT8OFF(c)      ((c) & GL)
#define BIT8ON(c)       ((c) | GR)

#define SKIP_I(str)     while (*(str) >= 0x20 && *(str) <=  0x2f) (str)++;
#define SKIP_P(str)     while (*(str) >= 0x30 && *(str) <=  0x3f) (str)++;

/*
 * initCTptr(): Set ctptr[] to point at ctdata[], indexed by codeset_num.
 */
static void
initCTptr(lcd)
    XLCd lcd;
{
    int num_codesets = XLC_GENERIC(lcd, codeset_num);
    int num_charsets;
    int i, j;
    CodeSet *codesets = XLC_GENERIC(lcd, codeset_list);
    CodeSet codeset;
    XlcCharSet charset;
    CTData ctdp;
    XlcCTInfoRec ct_info[MAX_CTINFO];
    int num = MAX_CTINFO;
    int _XlcGetCTInfo(XlcCTInfoRec *, int *);

    _XlcGetCTInfo(ct_info, &num);

    ctdata = (CTData)Xmalloc(sizeof(CTDataRec) * MAX_CTINFO);
    ctdptr = (CTData*)Xmalloc(sizeof(CTData) * MAX_CTINFO);
    
    for (i = 0; i < num; i++) {
    	ctdata[i].side = ct_info[i].charset->side;
	ctdata[i].length = ct_info[i].charset->char_size;
	ctdata[i].name = ct_info[i].charset->name;
	ctdata[i].ct_encoding = ct_info[i].encoding;
	ctdata[i].ct_encoding_len = ct_info[i].encoding_len;
	ctdata[i].set_size = 0;
	ctdata[i].min_ch = 0;
	ctdata[i].ct_type = CT_STD;
    }

    for(j = 0; i < MAX_CTINFO && j < MAX_CTDATA_EXT; i++, j++){
    	ctdata[i].side = ctdata_ext[j].side;
	ctdata[i].length = ctdata_ext[j].length;
	ctdata[i].name = ctdata_ext[j].name;
	ctdata[i].ct_encoding = ctdata_ext[j].ct_encoding;
	ctdata[i].ct_encoding_len = ctdata_ext[j].ct_encoding_len;
	ctdata[i].set_size = ctdata_ext[j].set_size;
	ctdata[i].min_ch = ctdata_ext[j].min_ch;
	ctdata[i].ct_type = ctdata_ext[j].ct_type;
	num++;
    }

    ctdp = ctdata;
    ctd_endp = &ctdata[num-1];
    ctdptr[Ascii] = &ctdata[0];		/* failsafe */

    for (i = 0; i < num_codesets; i++) {

	codeset = codesets[i];
	num_charsets = codeset->num_charsets;

	for (j = 0; j < num_charsets; j++) {

	    charset = codeset->charset_list[j];

	    for (ctdp = ctdata; ctdp <= ctd_endp; ctdp++)

		if (! strcmp(ctdp->name, charset->name)) {

		    ctdptr[codeset->cs_num] = ctdp;

		    ctdptr[codeset->cs_num]->set_size =
		      charset->set_size;

		    ctdptr[codeset->cs_num]->min_ch =
		      charset->set_size == 94 &&
		      (ctdptr[codeset->cs_num]->length > 1 ||
		      ctdptr[codeset->cs_num]->side == XlcGR) ? 0x21 : 0x20;

		    break;
		}
	}
    }
}

#define UDCJA_SEG	"SUNUDCJA.1997-0"

XPointer sunudcja_conv(
        XPointer inbufptr,
        XPointer outbufptr,
        int      *to_left,
        int       ct_seglen)
{
        int     seg_len;
        int     i;

	seg_len = strlen(UDCJA_SEG);

        if(*(inbufptr + seg_len) != 0x2)        /* not terminated by STX */
                return outbufptr;

        if(strncmp(inbufptr, UDCJA_SEG, seg_len - 1))
                return outbufptr;       /* Not a SunUDCJA Segment */

	inbufptr += (seg_len+1);        /* Skip Segment Name */
        ct_seglen -= (seg_len+1);

	for(i = 0; i < ct_seglen; i += 2){
            if(*inbufptr >= 0x21 && *inbufptr <= 0x2a) { 
		/* UDC mapped in 208 */
                *outbufptr = *inbufptr + 0xd4;	/* byte 1 */
		outbufptr++; inbufptr++; (*to_left)--;
                *outbufptr = BIT8ON(*inbufptr);	/* byte 2 */
		outbufptr++; inbufptr++; (*to_left)--;
            } else if(*inbufptr >= 0x2b && *inbufptr <= 0x34) { 
		/* UDC mapped in 212 */
		*outbufptr = SS3;
		outbufptr++; (*to_left)--;
                *outbufptr = *inbufptr + 0xca;	/* byte 1 */
		outbufptr++; inbufptr++; (*to_left)--;
                *outbufptr = BIT8ON(*inbufptr);	/* byte 2 */
		outbufptr++; inbufptr++; (*to_left)--;
	    } else if(*inbufptr >= 0x35 && *inbufptr <= 0x36) {
		/* IBM DC mapped in 212 */
		*outbufptr = SS3;
		outbufptr++; (*to_left)--;
                *outbufptr = *inbufptr + 0xbe;	/* byte 1 */
		outbufptr++; inbufptr++; (*to_left)--;
                *outbufptr = BIT8ON(*inbufptr);	/* byte 2 */
		outbufptr++; inbufptr++; (*to_left)--;
	    } else {
		*inbufptr++;
		*inbufptr++;
	    }
	}
	return outbufptr;
}

gen_mbstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    char *inbufptr;
    wchar_t *outbufptr;
    int wc_len;

    inbufptr = *((char **)from);
    outbufptr = *((wchar_t **)to);

    wc_len = mbstowcs(NULL, inbufptr, *from_left);
    if (*to_left < wc_len) return -1;

    if ((int)mbstowcs(outbufptr, inbufptr, *from_left) != wc_len) return -1;

    inbufptr += *from_left;
    *from = inbufptr;
    *from_left = 0;
    outbufptr += wc_len;
    *to = (XPointer)outbufptr;
    *to_left -= wc_len;

    return 0;
}


static int
gen_wcstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    wchar_t *inbufptr;
    char *outbufptr;
    int mb_len;

    inbufptr = *((wchar_t **)from);
    outbufptr = *((char **)to);

    mb_len = wcstombs(NULL, inbufptr, *from_left);
    if (*to_left < mb_len) return -1;

    if ((int)wcstombs(outbufptr, inbufptr, mb_len) != mb_len) return -1;

    inbufptr += *from_left;
    *from = (XPointer)inbufptr;
    *from_left = 0;
    outbufptr += mb_len;
    *to = outbufptr;
    *to_left -= mb_len;

    return 0;
}
static int
ctstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    register XPointer inbufptr = *from;
    register XPointer outbufptr = *to;
    XPointer inbuf_base;
    XPointer outbuf_base = outbufptr;
    register clen, length;
    int unconv_num = 0;
    int num_conv;
    unsigned int ct_seglen = 0;
    Uchar ct_type = 0;
    CTData ctdp = ctdptr[Ascii];	/* default */
    int i;

#ifdef sun
    Bool save_outbuf = True;
    /* If outbufptr is NULL, doen't save output, but just counts
       a length to hold the output */
    if (outbufptr == NULL) save_outbuf = False;
#endif
#ifndef sun
    if (*from_left > *to_left)
	*from_left = *to_left;
#endif


    for (length = ctdp->length; *from_left > 0; (*from_left) -= length)
    {
	ct_type = CT_STD;
	if (*inbufptr == '\033' || *inbufptr == (char)'\233') {

	    for (ctdp = ctdata; ctdp <= ctd_endp ; ctdp++) {

		if(!strncmp(inbufptr, ctdp->ct_encoding, ctdp->ct_encoding_len))
		{
		    inbufptr += ctdp->ct_encoding_len;
		    (*from_left) -= ctdp->ct_encoding_len;
		    if (ctdp->length) {
			length = ctdp->length;
			if( *from_left < length ) {
			    *to = (XPointer)outbufptr;
			    *to_left -= outbufptr - outbuf_base;
			    return( unconv_num + *from_left );
			}
		    }
		    ct_type = ctdp->ct_type;
		    break;
		}
	    }
	    if (ctdp > ctd_endp) 	/* failed to match CT sequence */
		unconv_num++;
	}

/* The following code insures that non-standard encodings, direction, extension,
 * and version strings are ignored; subject to change in future.
 */
	switch (ct_type) {
	case CT_STD:
	    break;
	case CT_EXT2:
	    inbufptr++;
	    (*from_left)--;
	    continue;
	case CT_NSTD:
	    ct_seglen = (BIT8OFF(*inbufptr) << 7) + BIT8OFF(*(inbufptr+1)) + 2;
	    	if(*from_left < ct_seglen){
	    	*to = (XPointer)outbufptr;
	    	*to_left -= outbufptr - outbuf_base;
	    	return (unconv_num + *from_left);
	    }
	    outbufptr = 
		sunudcja_conv(inbufptr+2, outbufptr, to_left, ct_seglen - 2);
	    inbufptr += ct_seglen;
	    (*from_left) -= (ct_seglen - 2);
	    continue;
	case CT_EXT0:
	    inbuf_base = inbufptr;
	    SKIP_I(inbufptr);
	    inbufptr++;
	    ct_seglen = (unsigned)(inbufptr - inbuf_base);
	    *(from_left) -= ct_seglen;
	    continue;
	case CT_EXT1:
	    inbuf_base = inbufptr;
	    SKIP_P(inbufptr);
	    SKIP_I(inbufptr);
	    inbufptr++;
	    ct_seglen = (unsigned)(inbufptr - inbuf_base);
	    *(from_left) -= ct_seglen;
	    continue;
	case CT_DIR:
	    continue;
	case CT_VER:
	    inbufptr += 2;
	    *(from_left) -= 2;
	    continue;
	}

	clen = length;
	do {
	    if(ct_type == CT_NSTD)
		break;

	    if (*to_left <= 0) {
		*to = (XPointer)outbufptr;
		unconv_num = *from_left;
		return unconv_num;
	    }
	    if (ctdp->length == clen) {
		if (strstr(ctdp->name, "JISX0208")) {
		  if(ctdp->side == XlcGL){
		    *inbufptr = BIT8ON(*inbufptr);
		    *(inbufptr+1) = BIT8ON(*(inbufptr+1));
		  } else if(ctdp->side == XlcGR && isleftside(*inbufptr)){
		    clen = ctdptr[Ascii]->length;
		    length = clen;
		  }
		} else if (ctdp == ctdptr[Kana] && isrightside(*inbufptr)) {
		    if (save_outbuf == True) {
			/* *outbufptr++ = ctdp->sshift; */
			*outbufptr++ = SS2; /* 0x8e */
		    }
		    (*to_left)--;
		    *inbufptr = BIT8ON(*inbufptr);
		} else if (ctdp == ctdptr[HojoKanji]){
		  if(ctdp->side == XlcGR && isleftside(*inbufptr)){
		    clen = ctdptr[Ascii]->length;
		    length = clen;
		  } else {
		    if (save_outbuf == True) {
			/* *outbufptr++ = ctdp->sshift; */
			*outbufptr++ = SS3; /* 0x8f */
		    }
		    (*to_left)--;
		    *inbufptr = BIT8ON(*inbufptr);
		    *(inbufptr+1) = BIT8ON(*(inbufptr+1));
		  }
		}
	    }
	    if (save_outbuf == True) {
		*outbufptr++ = *inbufptr;
	    }
	    (*to_left)--;
	    inbufptr++;
	} while (--clen);
    }

    *to = outbufptr;
    *from = inbufptr;
#ifndef sun
    if ((num_conv = (int)(outbufptr - outbuf_base)) > 0)
	(*to_left) -= num_conv;
#endif

    return unconv_num;

}

static int
ctstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = ((State) conv->state)->lcd;
    wchar_t *bufptr;
    register int buf_len = *to_left;
    char *mb_data, *to_mb_data;
    int mb_len, to_mb_left;
    int wc_len;
    char *csptr;
    int csstr_len;

    csptr = *((char **) from);
    csstr_len = *from_left;

    mb_data = (char *)Xmalloc((*to_left)*4);
    if (mb_data == NULL)
	return -1;

    mb_len = (*to_left)*4;
    memset(mb_data, 0, mb_len);

    to_mb_data = mb_data;
    to_mb_left = mb_len;

    bufptr = *((wchar_t **) to);

    if (ctstombs(conv, &csptr, &csstr_len, &to_mb_data, &to_mb_left,
		      args, num_args) == -1)
	return -1;

    wc_len = mbstowcs(NULL, mb_data, mb_len - to_mb_left);
    if (wc_len > buf_len) {
	/* overflow */
	*to_left = 0;
	return 1;
    }

    if (wc_len != mbstowcs(bufptr, mb_data, mb_len - to_mb_left))
	return -1;

    *from = csptr;
    *from_left = csstr_len;
    *to_left -= wc_len;
    if (bufptr != NULL) {
	bufptr += wc_len;
	*to = (XPointer)bufptr;
    }
    Xfree(mb_data);
    return 0;
}

static int
mbstocts(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    register ct_len = *to_left;
    int cs_num;
    int clen, length;
    int unconv_num = 0;
    int num_conv;
    XPointer inbufptr = *from;
    register char *ctptr = *to;
    XPointer ct_base = ctptr;
    XLCd lcd = ((State) conv->state)->lcd;
    int codeset_num = XLC_GENERIC(lcd, codeset_num);
    CT_StateRec ct_state;
    CTData charset;
    int tmp_num_conv;
    char        *EXT_M, *EXT_L;
    int         extlen = 0;
    int		i;


/* Initial State: */
    ct_state.GL_charset = ctdptr[0]; /* Codeset 0 */
    ct_state.GR_charset = NULL;

    if (*from_left > *to_left)
        *from_left = *to_left;

    for (;*from_left > 0; (*from_left) -= length) {

        tmp_num_conv = (int)(ctptr - ct_base);
        if(((*to_left)-tmp_num_conv) < MAX_NAME_LEN+MAX_CT_ENCODING_LEN+3){
		goto end;
        }

	if (isleftside(*inbufptr)) {		/* 7-bit (CS0) */
	    if (Ascii >= codeset_num) {
	      	unconv_num++;
		(*from_left)--;
		continue;
	    }
	    cs_num = Ascii;
	    charset = ctdptr[Ascii];
	}
	else if ((Uchar)*inbufptr == SS2) {	/* JISX0201 Kana */
	    if (Kana >= codeset_num) {
	      	unconv_num++;
		(*from_left)--;
		continue;
	    }
	    cs_num = Kana;
	    charset = ctdptr[Kana];
	    inbufptr++;
	    (*from_left)--;
	}
	else if ((Uchar)*inbufptr == SS3) {	/* JISX0212 */
	    if (HojoKanji >= codeset_num) {
	      	unconv_num++;
		(*from_left)--;
		continue;
	    }
	    if((Uchar)*(inbufptr+1) >= 0xf5) { /* UDC mapped in 212 */
		cs_num = Userdef;
		charset = ctdptr[Userdef];
		*(inbufptr+1) -= 0xca;
	    } else {
		cs_num = HojoKanji;
		charset = ctdptr[HojoKanji];
	    }
	    inbufptr++;
	    (*from_left)--;
	}
	else {
	    if (Kanji >= codeset_num) {		/* JISX0208 Kanji */
	      	unconv_num++;
		(*from_left)--;
		continue;
	    }
	    if((Uchar)*inbufptr >= 0xf5) { /* UDC mapped in 208 */
		cs_num = Userdef;
		charset = ctdptr[Userdef];
		*(inbufptr) -= 0xd4;
	    } else {
		cs_num = Kanji;
		charset = ctdptr[Kanji];
	    }
	}

	length = charset->length;

#ifndef sun
	/* To avoid data loss, allow control characters as part of ascii
	   01/30/96 tajima */
	if (BADCHAR(charset->min_ch, *inbufptr))
            continue;
#endif

	if ( (charset->side == XlcGR && charset != ct_state.GR_charset) ||
	     (charset->side == XlcGL && charset != ct_state.GL_charset) ||
             (charset->side == XlcUnknown && charset != ct_state.charset)) {

	    ct_len -= ctdptr[cs_num]->ct_encoding_len;
	    if (ct_len < 0) {
		unconv_num++;
		break;
	    }
	
	    if (ctptr) {
		strcpy(ctptr, ctdptr[cs_num]->ct_encoding);
		ctptr += ctdptr[cs_num]->ct_encoding_len;
	    }

            if (cs_num == Userdef && ctptr){  /* UDC/IBM DC in ext. seg */
                EXT_M = ctptr; ctptr++;
                EXT_L = ctptr; ctptr++;
                strcpy(ctptr, ctdptr[cs_num]->name);
                ctptr += strlen(ctdptr[cs_num]->name);
                *ctptr = (char)0x2; /* STX */
                ctptr++;
                extlen = strlen(ctdptr[cs_num]->name) + 1;
            }
	}

#ifdef sun
	if (charset->side == XlcGR) {
	    ct_state.GR_charset = charset;
	    ct_state.GL_charset = NULL;
	    ct_state.charset = NULL;
	} else if (charset->side == XlcGL) {
	    ct_state.GL_charset = charset;
	    ct_state.GR_charset = NULL;
	    ct_state.charset = NULL;
	} else if (charset->side == XlcUnknown) {
	    ct_state.GL_charset = NULL;
	    ct_state.GR_charset = NULL;
	    ct_state.charset = charset;
	}
#else
	if (charset->side == XlcGR)
	    ct_state.GR_charset = charset;
	else if (charset->side == XlcGL)
	    ct_state.GL_charset = charset;
#endif

	clen = length;

	do {
	    *ctptr++ = charset == ct_state.GR_charset ?
		BIT8ON(*inbufptr++) : BIT8OFF(*inbufptr++);
	    if(cs_num == Userdef)
		extlen++;
	} while (--clen); 

        if(cs_num == Userdef) {
                *EXT_L = (char)(extlen % 128 + 128);
                *EXT_M = (char)(extlen / 128 + 128);
        }
    }

end:

    *to = (XPointer)ctptr;
    *from = (XPointer)inbufptr;

    if ((num_conv = (int)(ctptr - ct_base)) > 0)
	(*to_left) -= num_conv;

    return unconv_num;

}

static int
wcstocts(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    wchar_t *wcptr;
    int wcstr_len;
    int mb_len;
    char *mb_sub = (char*)NULL;
    char *from_mb;
    wchar_t *wc_sub = (wchar_t*)NULL;
    int ret = -1;
    int mb_conv_len, wc_conv_len;
    register int i;

    if (from == NULL || *from == NULL)
	return 0;

    wcptr = *((wchar_t **)from);
    wcstr_len = *from_left;

    wc_sub = (wchar_t*)Xmalloc(sizeof(wchar_t) * (wcstr_len + 1));
    if (wc_sub == NULL)
	goto err;

    /* Some client passes the wide character string which contains a NULL
       character inside the string. This prevents wcstring(3I) functions
       from parsing the string to the end. For now, I replace it with
       a SPACE as a harmless workaround. */
    for (i = 0; i < wcstr_len; i++) {
	if (wcptr[i] == (wchar_t)0) wcptr[i] = (wchar_t)' ';
    }
    wsncpy(wc_sub, wcptr, wcstr_len);
    wc_sub[wcstr_len] = (wchar_t)0;

    mb_len = wcstombs(NULL, wc_sub, wcstr_len);
    if (mb_len == -1)
	goto err;

    mb_sub = (char*)Xmalloc(mb_len + 1);
    if (mb_sub == NULL)
	goto err;
    memset(mb_sub, 0, mb_len + 1);

    from_mb = mb_sub;

    if (mb_len != wcstombs(mb_sub, wcptr, mb_len))
	goto err;

    mb_conv_len = mb_len;

    ret = mbstocts(conv, &from_mb, &mb_len, to, to_left,
			args, num_args);

    mb_conv_len -= mb_len;
    mb_sub[mb_conv_len] = (char)0;
    wc_conv_len = mbstowcs(NULL, mb_sub, mb_conv_len);
    
    wcptr += wc_conv_len;
    *from_left -= wc_conv_len;
    *from = (XPointer)wcptr;
err:
    if (mb_sub) Xfree((char *)mb_sub);
    if (wc_sub) Xfree((char *)wc_sub);
    return ret;
}

static XlcConvMethodsRec ctstombs_methods = {
    close_converter,
    ctstombs,
    NULL
} ;

static XlcConv
open_ctstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &ctstombs_methods);
}

static XlcConvMethodsRec ctstowcs_methods = {
    close_converter,
    ctstowcs,
    NULL
} ;

static XlcConv
open_ctstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &ctstowcs_methods);
}

static XlcConvMethodsRec mbstocts_methods = {
    close_converter,
    mbstocts,
    NULL
} ;

static XlcConv
open_mbstocts(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &mbstocts_methods);
}

static XlcConvMethodsRec mbstowcs_methods = {
    close_converter,
    gen_mbstowcs,
    NULL
} ;

static XlcConv
open_mbstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &mbstowcs_methods);
}

static XlcConvMethodsRec wcstocts_methods = {
    close_converter,
    wcstocts,
    NULL
} ;

static XlcConv
open_wcstocts(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &wcstocts_methods);
}

static XlcConvMethodsRec wcstombs_methods = {
    close_converter,
    gen_wcstombs,
    NULL
} ;

static XlcConv
open_wcstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &wcstombs_methods);
}

#endif /* FORCE_INDIRECT_CONVERTER */

XLCd
_XlcGenericLoader(name)
    char *name;
{
    XLCd lcd;
    XLCdGenericPart *gen;
    extern void _XlcAddUtf8Converters(XLCd);

    lcd = _XlcCreateLC(name, _XlcGenericMethods);
    if (lcd == NULL)
	return lcd;

    initCTptr(lcd);

    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNChar, open_mbtocs);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCharSet, open_mbstocs);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNMultiByte, open_cstombs);
    _XlcSetConverter(lcd, XlcNString, lcd, XlcNMultiByte, open_strtombs);
#ifdef STDCVT
    gen = XLC_GENERIC_PART(lcd);

    if (gen->use_stdc_env == True) {
	_XlcSetConverter(lcd,XlcNMultiByte,lcd,XlcNWideChar,open_stdc_mbstowcs);
	_XlcSetConverter(lcd,XlcNWideChar,lcd,XlcNMultiByte,open_stdc_wcstombs);
    } else {
        _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNWideChar, open_mbstowcs);
	_XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNMultiByte, open_wcstombs);
    }
    if (gen->force_convert_to_mb == True) {
	_XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCharSet,open_stdc_wcstocs);
	_XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNWideChar,open_stdc_cstowcs);
    } else {
#endif
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCharSet, open_wcstocs);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNWideChar, open_cstowcs);
#ifdef STDCVT
    }
#endif

#ifndef FORCE_INDIRECT_CONVERTER
    _XlcSetConverter(lcd, XlcNCompoundText, lcd, XlcNWideChar, open_ctstowcs);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCompoundText, open_wcstocts);
    _XlcSetConverter(lcd, XlcNCompoundText, lcd, XlcNMultiByte, open_ctstombs);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCompoundText, open_mbstocts);
#endif

     _XlcAddUtf8Converters(lcd);

    return lcd;
}
