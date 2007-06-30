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

/* $XConsortium: lcSjis.c,v 1.14 95/06/26 20:41:49 kaleb Exp $ */
/****************************************************************

        Copyright 1992, 1993 by FUJITSU LIMITED
        Copyright 1993 by Fujitsu Open Systems Solutions, Inc.
	Copyright 1994 by Sony Corporation

Permission to use, copy, modify, distribute and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of FUJITSU LIMITED,
Fujitsu Open Systems Solutions, Inc. and Sony Corporation  not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
FUJITSU LIMITED, Fujitsu Open Systems Solutions, Inc. and
Sony Corporation make no representations about the suitability of
this software for any purpose.  It is provided "as is" without
express or implied warranty.

FUJITSU LIMITED, FUJITSU OPEN SYSTEMS SOLUTIONS, INC. AND SONY
CORPORATION DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL FUJITSU OPEN SYSTEMS SOLUTIONS, INC., FUJITSU LIMITED
AND SONY CORPORATION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
OR PERFORMANCE OF THIS SOFTWARE.

    Authors: Jeffrey Bloomfield		(jeffb@ossi.com)
	     Shigeru Yamada		(yamada@ossi.com)
             Yoshiyuki Segawa		(segawa@ossi.com)
    Modifier:Makoto Wakamatsu   Sony Corporation
				makoto@sm.sony.co.jp

*****************************************************************/

#include "Xlibint.h"
#include "XlcGeneric.h"
#include <stdio.h>
#include <widec.h>
#include <limits.h>

#include <ctype.h>
#ifdef WIN32
#define isascii __isascii
#endif

#define CS0	codesets[0]		/* Codeset 0 - 7-bit ASCII	*/
#define CS1	codesets[1]		/* Codeset 1 - Kanji		*/
#define CS2	codesets[2]		/* Codeset 2 - Half-Kana	*/
#define CS3	codesets[3]		/* Codeset 3 - User defined	*/

#define ascii		(codeset->cs_num == 0)
#define kanji		(codeset->cs_num == 1)
#define kana		(codeset->cs_num == 2)
#define userdef		(codeset->cs_num == 3)

#define	ASCII_CODESET	0
#define KANJI_CODESET	1
#define KANA_CODESET	2
#define USERDEF_CODESET	3
#define MAX_CODESETS	4

#define GR	0x80	/* begins right-side (non-ascii) region */
#define GL	0x7f    /* ends left-side (ascii) region        */

#define isleftside(c)	(((c) & GR) ? 0 : 1)
#define isrightside(c)	(!isleftside(c))

typedef unsigned char   Uchar;
typedef unsigned long	Ulong;
typedef unsigned int	Uint;

/* Acceptable range for 2nd byte of SJIS multibyte char */
#define VALID_MULTIBYTE(c) \
			((0x40<=((Uchar)c) && ((Uchar)c)<=0x7e) \
			|| (0x80<=((Uchar)c) && ((Uchar)c)<=0xfc))

#ifndef iskanji
#define iskanji(c)	((0x81<=((Uchar)c) && ((Uchar)c)<=0x9f) \
			|| (0xe0<=((Uchar)c) && ((Uchar)c)<=0xef))
#endif /* !iskanji */

#ifndef iskana
#define iskana(c)	(0xa1<=((Uchar)c) && ((Uchar)c)<=0xdf)
#endif /* !iskana */

#define	isuserdef(c)	(0xf0<=((Uchar)c) && ((Uchar)c)<=0xfc)
#define	isuserdef1(c)	(0xf0<=((Uchar)c) && ((Uchar)c)<=0xf4)
#define	isuserdef2(c)	(0xf5<=((Uchar)c) && ((Uchar)c)<=0xf9)
#define	issjnecibm(c)	(0xed<=((Uchar)c) && ((Uchar)c)<=0xef)

#define BIT8OFF(c)	((c) & GL)
#define BIT8ON(c)	((c) | GR)


static void jis_to_sjis();
static void sjis_to_jis();
static void udc1_to_jis();
static void udc2_to_jis();
void nec_to_ibm(Uchar *, Uchar *);
extern unsigned short ibmdctojis212(char *);
extern unsigned short ibmexttosj(Uchar, Uchar);
extern unsigned short jis212tosj(Uchar, Uchar);

/*
 * Notes:
 * 1.  16-bit widechar format is limited to 14 data bits.  Since the 2nd byte
 *     of SJIS multibyte chars are in the ranges of 0x40 - 7E and 0x80 - 0xFC,
 *     SJIS cannot map directly into 16 bit widechar format within the confines
 *     of a single codeset.  Therefore, for SJIS widechar conversion, SJIS Kanji
 *     is mapped into the JIS codeset.  (The algorithms used in jis_to_sjis()
 *     and sjis_to_jis() are from Ken Lunde (lunde@mv.us.adobe.com) and are in
 *     the public domain.)
 * 2.  Defining FORCE_INDIRECT_CONVERTER (see _XlcEucLoader())
 *     forces indirect (charset) conversions (e.g. wcstocs()<->cstombs()).
 * 3.  Using direct converters (e.g. mbstowcs()) decreases conversion
 *     times by 20-40% (depends on specific converter used).
 */


static int
sjis_mbstowcs(conv, from, from_left, to, to_left, args, num_args)
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
sjis_wcstombs(conv, from, from_left, to, to_left, args, num_args)
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

/*
 * sjis<->jis conversion for widechar kanji (See Note at top of file)
 */
static void
sjis_to_jis(p1, p2)
    Uchar *p1;
    Uchar *p2;
{
  register Uchar c1 = *p1;
  register Uchar c2 = *p2;
  register Uchar adjust = c2 < 0x9f;
  register Uchar rowOffset = c1 < 0xa0 ? 0x70 : 0xb0;
  register Uchar cellOffset = adjust ? (0x1f + (c2 > 0x7f)) : 0x7e;

  *p1 = ((c1 - rowOffset) << 1) - adjust;
  *p2 -= cellOffset;
}

static void
udc1_to_jis(p1, p2)
    Uchar *p1;
    Uchar *p2;
{
  register Uchar c1 = *p1;
  register Uchar c2 = *p2;
  register Uchar adjust = c2 < 0x9f;
  register Uchar rowOffset = 0xb0;
  register Uchar cellOffset = adjust ? (0x1f + (c2 > 0x7f)) : 0x7e;

  *p1 = ((c1 - rowOffset) << 1) - adjust - 0xa;
  *p2 -= cellOffset;

}

static void
udc2_to_jis(p1, p2)
    Uchar *p1;
    Uchar *p2;
{
  register Uchar c1 = *p1;
  register Uchar c2 = *p2;
  register Uchar adjust = c2 < 0x9f;
  register Uchar rowOffset = 0xb0;
  register Uchar cellOffset = adjust ? (0x1f + (c2 > 0x7f)) : 0x7e;

  *p1 = ((c1 - rowOffset) << 1) - adjust - 0x14;
  *p2 -= cellOffset;

}


static void
jis_to_sjis(p1, p2)
    Uchar *p1;
    Uchar *p2;
{
  register Uchar c1 = *p1;
  register Uchar c2 = *p2;
  register Uchar rowOffset = c1 < 0x5f ? 0x70 : c1 < 0x75 ? 0xb0 : 0xb5;
  register Uchar cellOffset = c1 % 2 ? 0x1f + (c2 > 0x5f) : 0x7e;

  *p1 = ((Uchar)(c1 + 1) >> 1) + rowOffset;
  *p2 = c2 + cellOffset;
}

static int
sjis_mbtocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = (XLCd)conv->state;
    XlcCharSet charset = NULL;
    int char_size = 0;
    int unconv_num = 0;
    register char *src = *from, *dst = *to;
    CodeSet *codesets = XLC_GENERIC(lcd, codeset_list);
    int codeset_num = XLC_GENERIC(lcd, codeset_num);

    if(*from_left == 0)
	return -1;

    /* 
     *  Remap NEC Selected IBM Defined Character to IBM Defined
     *  Character.
     */
    if(issjnecibm(*src)){
	nec_to_ibm((Uchar *)src, (Uchar *)(src+1));
    }

    if (iskanji(*src)) {
	if (KANJI_CODESET >= codeset_num)
	    return -1;
	charset = *CS1->charset_list;
	char_size = charset->char_size;

	if (*from_left >= char_size && *to_left >= char_size) {
	    *dst++ = *src++;
	    *dst++ = *src++; 
	    if (!VALID_MULTIBYTE((Uchar) *(src-1))) /* check 2nd byte */
		unconv_num++;
	    sjis_to_jis((Uchar *)(dst-2), (Uchar *)(dst-1));
	} else
	    return -1;
    } else if ((Uchar)*src == 0xfa && (Uchar)*(src+1) == 0x54) {
	/* 
	 * SJIS 0xfa54 (IBM Extended Character) is mapped to 
	 * 0x224c in JISX0208 
	 */
	if (KANJI_CODESET >= codeset_num)
	    return -1;
	charset = *CS1->charset_list;
	char_size = charset->char_size;

	if (*from_left >= char_size && *to_left >= char_size) {
	    *dst++ = 0x22; src++;
	    *dst++ = 0x4c; src++;
	} else
		return -1;
     } else if ((Uchar)*src == 0xfa && (Uchar)*(src+1) == 0x5b) {
	/* 
	 * SJIS 0xfa5b (IBM Extended Character) is mapped to 
	 * 0x268 in JISX0208 
	 */
	if (KANJI_CODESET >= codeset_num)
	    return -1;
	charset = *CS1->charset_list;
	char_size = charset->char_size;

	if (*from_left >= char_size && *to_left >= char_size) {
	    *dst++ = 0x22; src++;
	    *dst++ = 0x68; src++;
	} else
		return -1;
    } else if ((Uchar)*src >= 0xfa) { 
	/* 
	 * IBM Extended Characters are mapped to JISX0212
	 */
	if (KANJI_CODESET >= codeset_num)
	    return -1;
	charset = *CS3->charset_list;
	char_size = charset->char_size;

	if (*from_left >= char_size && *to_left >= char_size) {
		unsigned short	tmp_dst;
		tmp_dst = ibmdctojis212(src);
		src++; src++;
		if(tmp_dst == 0xffff){
    			*from_left -= char_size;
			*from = src;
			return -1;
		}

		*dst++  = ((tmp_dst & 0xff00) >> 8);
		*dst++  = (tmp_dst & 0xff);
	} else
		return -1;
    } else if (isuserdef1(*src)) {
	if (USERDEF_CODESET >= codeset_num)
	    return -1;
	charset = *CS1->charset_list;
	char_size = charset->char_size;
	
	if (*from_left >= char_size && *to_left >= char_size) {
	    *dst++ = *src++;
	    *dst++ = *src++; 
	    if (!VALID_MULTIBYTE((Uchar) *(src-1))) /* check 2nd byte */
		unconv_num++;
	    udc1_to_jis((Uchar *)(dst-2), (Uchar *)(dst-1));
	} else
	    return -1;
    } else if (isuserdef2(*src)) {
	if (USERDEF_CODESET >= codeset_num)
	    return -1;
	charset = *CS3->charset_list;
	char_size = charset->char_size;
	
	if (*from_left >= char_size && *to_left >= char_size) {
	    *dst++ = *src++;
	    *dst++ = *src++; 
	    if (!VALID_MULTIBYTE((Uchar) *(src-1))) /* check 2nd byte */
		unconv_num++;
	    udc2_to_jis((Uchar *)(dst-2), (Uchar *)(dst-1));
	} else
	    return -1;
    } else if (isascii(*src)) {
	if (ASCII_CODESET >= codeset_num)
	    return -1;
	charset = *CS0->charset_list;
	char_size = charset->char_size;

	if (*from_left >= char_size && *to_left >= char_size)
	    *dst++ = *src++;
	else
	    return -1;
    }
    else if (iskana(*src)) {
	if (KANA_CODESET >= codeset_num)
	    return  -1;
	charset = *CS2->charset_list;
	char_size = charset->char_size;

	if (*from_left >= char_size && *to_left >= char_size)
	    *dst++ = *src++;
	else
	   return -1;
    }
    else { 	/* unknown */
	(*from)++;
	(*from_left)--;
	return -1;
    }

    *from_left -= char_size;
    *to_left -= char_size;

    *to = dst;
    *from = src;

    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;

    return unconv_num;
}


static int
sjis_mbstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    char *tmp_from, *tmp_to;
    int tmp_from_left, tmp_to_left;
    XlcCharSet charset, tmp_charset;
    XPointer tmp_args[1];
    int unconv_num = 0, ret;



/* Determine the charset of the segment and convert one characater: */

    tmp_args[0] = (XPointer) &charset; /* charset from sjis_mbtocs() */
    while
      ((ret = sjis_mbtocs(conv, from, from_left, to, to_left, tmp_args, 1)) > 0)
	unconv_num += ret;
    if ( ret < 0 )
	return ret;

    tmp_from = *from;
    tmp_from_left = *from_left;
    tmp_to_left = *to_left;
    tmp_to = *to;

/* Convert remainder of the segment: */

    tmp_args[0] = (XPointer) &tmp_charset;
    while( (ret = sjis_mbtocs(conv, &tmp_from, &tmp_from_left, &tmp_to,
      &tmp_to_left, tmp_args, 1)) >= 0 ) {

	if (ret > 0) {
	    unconv_num += ret;
	    continue;
	}

	if (tmp_charset != charset)  /* quit on end of segment */
	    break;

	*from = tmp_from;
	*from_left = tmp_from_left;
	*to = tmp_to;
	*to_left = tmp_to_left;
    } 

    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;
    
    return unconv_num;
}

static int
sjis_wcstocs(conv, from, from_left, to, to_left, args, num_args)
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

    ret = sjis_mbstocs(conv, &from_mb, &mb_len, to, to_left, args, num_args);

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

static CodeSet
GetCodeSetFromCharSet(lcd, charset)
    XLCd lcd;
    XlcCharSet charset;
{
    register CodeSet *codeset = XLC_GENERIC(lcd, codeset_list);
    register XlcCharSet *charset_list;
    register int codeset_num, num_charsets;

    codeset_num = XLC_GENERIC(lcd, codeset_num);

    for ( ; codeset_num-- > 0; codeset++) {
	num_charsets = (*codeset)->num_charsets;
	charset_list = (*codeset)->charset_list;

	for ( ; num_charsets-- > 0; charset_list++)
	    if (*charset_list == charset)
		return *codeset;
    }

    return (CodeSet) NULL;
}


static int
sjis_cstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    char **from;
    int *from_left;
    char **to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = (XLCd) conv->state;
    register char *csptr = *from;
    register char *bufptr = *to;
    int csstr_len = *from_left;
    register int buf_len = *to_left;
    int length;
    CodeSet codeset;
    int cvt_length = 0;

    if (num_args < 1)
	return -1;
    
    if (!(codeset = GetCodeSetFromCharSet(lcd, (XlcCharSet) args[0])))
	return -1;

    csstr_len /= codeset->length;
    buf_len /= codeset->length;
    if (csstr_len < buf_len)
	buf_len = csstr_len;
    
    cvt_length += buf_len * codeset->length;

    if (bufptr) {
	while (buf_len--) {
	    length = codeset->length;
	    while (length--)
		*bufptr++ = codeset->length == 1 && codeset->side == XlcGR ?
		  BIT8ON(*csptr++) : BIT8OFF(*csptr++);

	    if (codeset->length == 2) {
		char *ct_name = codeset->charset_list[0]->name;
	        if(strncmp(ct_name, "JISX0212", 8)){ /* Not 212 (JISX0208) */
		    jis_to_sjis((Uchar *)(bufptr-2), (Uchar *)(bufptr-1));
		} else {
		    unsigned short dest;
		    dest = jis212tosj((Uchar) *(bufptr-2), 
				(Uchar) *(bufptr-1));
		    *(bufptr-2) = (dest >> 8) & 0xff;
		    *(bufptr-1) = dest & 0xff;
		}
	    }
	}
    }

    *from_left -= csptr - *from;
    *from = csptr;

    if (bufptr)
	*to += cvt_length;
    *to_left -= cvt_length;


    return 0;
}

static int
sjis_cstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = (XLCd)conv->state;
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

    if (sjis_cstombs(conv, &csptr, &csstr_len, &to_mb_data, &to_mb_left,
		     args, num_args) == -1)
	return -1;

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


/*
 *    Stripped down Direct CT converters for SJIS
 *
 */

#define BADCHAR(min_ch, c)  (BIT8OFF(c) < (char)min_ch && BIT8OFF(c) != 0x0 && \
			     BIT8OFF(c) != '\t' && BIT8OFF(c) != '\n' && \
			     BIT8OFF(c) != 0x1b)

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

typedef struct _StateRec {
    CTData GL_charset;
    CTData GR_charset;
    CTData charset;
} StateRec, *State;

#define CT_STD  0
#define CT_NSTD 1
#define CT_DIR  2
#define CT_EXT0 3
#define CT_EXT1 4
#define CT_EXT2 5
#define CT_VER  6

static CTData ctd_endp;
static CTData *ctdptr;

static CTDataRec ctdata[] =
{
  { XlcGL,      1, "ISO8859-1:GL",	   "\033(B"   ,  3, 0, 0, CT_STD  },
  { XlcGR,      1, "ISO8859-1:GR",	   "\033-A"   ,  3, 0, 0, CT_STD  },
  { XlcGL,      1, "JISX0201.1976-0:GL",   "\033(J"   ,  3, 0, 0, CT_STD  },
  { XlcGR,      1, "JISX0201.1976-0:GR",   "\033)I"   ,  3, 0, 0, CT_STD  },
  { XlcGL,      2, "JISX0208.1983-0:GL",   "\033$(B"  ,  4, 0, 0, CT_STD  },
  { XlcGR,      2, "JISX0208.1983-0:GR",   "\033$)B"  ,  4, 0, 0, CT_STD  },
  { XlcGL,      2, "JISX0212.1990-0:GL",   "\033$(D"  ,  4, 0, 0, CT_STD  },
  { XlcGR,      2, "JISX0212.1990-0:GR",   "\033$)D"  ,  4, 0, 0, CT_STD  },
  { XlcUnknown, 0, "Ignore-Ext-Status?",   "\033#"    ,  2, 0, 0, CT_VER  },
  { XlcUnknown, 0, "NonStd-?-OctetChar",   "\033%/0"  ,  4, 0, 0, CT_NSTD },
  { XlcUnknown, 1, "NonStd-1-OctetChar",   "\033%/1"  ,  4, 0, 0, CT_NSTD },
  { XlcUnknown, 2, "SUNUDCJA.1997-0",      "\033%/2"  ,  4, 0, 0, CT_NSTD },
/*{ XlcUnknown, 2, "NonStd-2-OctetChar",   "\033%/2"  ,  4, 0, 0, CT_NSTD },*/
  { XlcUnknown, 3, "NonStd-3-OctetChar",   "\033%/3"  ,  4, 0, 0, CT_NSTD },
  { XlcUnknown, 4, "NonStd-4-OctetChar",   "\033%/4"  ,  4, 0, 0, CT_NSTD },
  { XlcUnknown, 0, "Extension-2",	   "\033%/"   ,  3, 0, 0, CT_EXT2 },
  { XlcUnknown, 0, "Extension-0",	   "\033"     ,  1, 0, 0, CT_EXT0 },
  { XlcUnknown, 0, "Begin-L-to-R-Text",	   "\2331]"   ,  3, 0, 0, CT_DIR  },
  { XlcUnknown, 0, "Begin-R-to-L-Text",	   "\2332]"   ,  3, 0, 0, CT_DIR  },
  { XlcUnknown, 0, "End-Of-String",	   "\233]"    ,  2, 0, 0, CT_DIR  },
  { XlcUnknown, 0, "Extension-1",	   "\233"     ,  1, 0, 0, CT_EXT1 },
};

#define Ascii     0
#define Kanji     1
#define Kana      2
#define Userdef   3
#define HojoKanji 4

#define MAX_NAME_LEN		18
#define MAX_CT_ENCODING_LEN	4

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
    XlcCTInfoRec ct_info[20];
    int num = 20;

    ctdptr = (CTData*)Xmalloc(sizeof(CTData) * num);

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


static int
sjis_mbstocts(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    register int ct_len = *to_left;
    int cs_num;
    int clen;
    int unconv_num = 0;
    int num_conv;
    int tmp_num_conv;
    XPointer inbufptr = *from;
    register char *ctptr = *to;
    XPointer ct_base = ctptr;
    int	i;
    char	*EXT_M, *EXT_L;
    int		extlen = 0;

    StateRec ct_state;
    CTData charset;
    XLCd lcd = (XLCd) conv->state;
    int codeset_num = XLC_GENERIC(lcd, codeset_num);

    memset(ctptr, 0, ct_len);


/* Initial State: */

#ifdef sun
    ct_state.GL_charset = NULL;
#else
    ct_state.GL_charset = ctdptr[Ascii];
#endif
    ct_state.GR_charset = NULL;

    if (*from_left > *to_left)
        *from_left = *to_left;

    /* 
     * cs_num and charset should be initialized before loop, otherwise
     * charset will be null if invalid character is first inputed.
     */
    cs_num = Ascii;
    charset = ctdptr[Ascii];
    for (;*from_left > 0; (*from_left) -= charset->length) {
	
	tmp_num_conv = (int)(ctptr - ct_base);
	if(((*to_left)-tmp_num_conv) < MAX_NAME_LEN+MAX_CT_ENCODING_LEN+3){
		goto end;
	}

	/* 
	 *  Remap NEC Selected IBM Defined Character to IBM Defined
	 *  Character.
	 */
	if(issjnecibm(*inbufptr)){
		nec_to_ibm((Uchar *)inbufptr, (Uchar *)(inbufptr+1));
	}

	if (iskanji(*inbufptr)) {
	    if (KANJI_CODESET >= codeset_num) {
		unconv_num++;
		(*from_left)--;
		continue;
	    }
	    cs_num = Kanji;
	    charset = ctdptr[Kanji];
	    if (!VALID_MULTIBYTE((Uchar) *(inbufptr+1)))
		unconv_num++;
	}
	else if (((Uchar)*inbufptr == 0xfa) && ((Uchar)*(inbufptr+1) == 0x54)) {
	    cs_num = Kanji;
	    charset = ctdptr[Kanji];
	    *inbufptr = 0x81;
	    *(inbufptr+1) = 0xca;
	}
	else if (((Uchar)*inbufptr == 0xfa) && ((Uchar)*(inbufptr+1) == 0x5b)) {
	    cs_num = Kanji;
	    charset = ctdptr[Kanji];
	    *inbufptr = 0x81;
	    *(inbufptr+1) = 0xe6;
	}
	else if (isuserdef(*inbufptr)) {
	    unsigned short	tmp_dst;
	    if (USERDEF_CODESET >= codeset_num) {
		unconv_num++;
		(*from_left)--;
		continue;
	    }
	    if ((Uchar)*inbufptr >= 0xfa){  /* IBM Extended Character */
	    	tmp_dst = ibmdctojis212(inbufptr);
	    	if (tmp_dst == 0xffff) { /* Invalid Character */
		     unconv_num++;
		     (*from_left)--;
	    	} else if ( tmp_dst < 0x7520  ) { /* Mapped to JISX0212 */
	    	    cs_num = HojoKanji;
	    	    charset = ctdptr[HojoKanji];
		    *inbufptr = ((tmp_dst & 0xff00) >> 8);
		    *(inbufptr+1) = (tmp_dst & 0xff);
		    *inbufptr = BIT8ON(*inbufptr);
		    *(inbufptr+1) = BIT8ON(*(inbufptr+1));
		} else { /* Hojo Kanji Mapped in Private Area */
	    	    cs_num = Userdef;
	    	    charset = ctdptr[Userdef];
		    *inbufptr = ((tmp_dst & 0xff00) >> 8) - 0x3e;
		    *(inbufptr+1) = (tmp_dst & 0xff);
		}
	    } else { /* User Defined Character */
	    	cs_num = Userdef;
	    	charset = ctdptr[Userdef];
		sjis_to_jis((Uchar *)inbufptr, (Uchar *)(inbufptr+1));
		*inbufptr -= 0x5e;
	    }
	    /*
	    if (!VALID_MULTIBYTE((Uchar) *(inbufptr+1)))
	        unconv_num++;
	   */
	}
	else if (isascii(*inbufptr)) {
	    if (ASCII_CODESET >= codeset_num) {
		unconv_num++;
		(*from_left)--;
		continue;
	    }
	    cs_num = Ascii;
	    charset = ctdptr[Ascii];
	}
	else if (iskana(*inbufptr)) {
	    if (KANA_CODESET >= codeset_num) {
		unconv_num++;
		(*from_left)--;
		continue;
	    }
	    cs_num = Kana;
	    charset = ctdptr[Kana];
	}
	else { 		 /* unknown */
	    unconv_num++;
	    (*from_left)--;
	    continue;
	}

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

	    if (cs_num == Userdef && ctptr){

		EXT_M = ctptr; ctptr++;
		EXT_L = ctptr; ctptr++;
		strcpy(ctptr, ctdptr[cs_num]->name);
		ctptr += strlen(ctdptr[cs_num]->name);
		*ctptr = (char)0x2; /* STX */
		ctptr++;
		extlen = strlen(ctdptr[cs_num]->name) + 1;
	    }
	}

	clen = charset->length;
	do {
	    *ctptr++ =*inbufptr++;
	    if(cs_num == Userdef)
	    	extlen++;
	} while (--clen); 

	if(cs_num == Userdef) {
		*EXT_L = (char)(extlen % 128 + 128);
		*EXT_M = (char)(extlen / 128 + 128);
	}

	if (charset->length >= 2 && (cs_num == Kanji || cs_num == Kana)) {
	    sjis_to_jis((Uchar *)(ctptr-2), (Uchar *)(ctptr-1));
	    if (BADCHAR(charset->min_ch, *(ctptr-2)) ||
		BADCHAR(charset->min_ch, *(ctptr-1))) {
		unconv_num++;
		continue;
	    }
#ifdef sun
	    if (charset->side == XlcGR) {
		*(ctptr-2) = BIT8ON(*(ctptr-2));
		*(ctptr-1) = BIT8ON(*(ctptr-1));
	    } else if (charset->side == XlcGL) {
		*(ctptr-2) = BIT8OFF(*(ctptr-2));
		*(ctptr-1) = BIT8OFF(*(ctptr-1));
	    }
#endif
	}
#ifndef sun
	else
	    if (BADCHAR(charset->min_ch, *(ctptr-1))) {
		unconv_num++;
		continue;
	    }
#endif

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
    }

end:

    *to = (XPointer)ctptr;
    *from = (XPointer)inbufptr;

    if ((num_conv = (int)(ctptr - ct_base)) > 0)
	(*to_left) -= num_conv;

    return unconv_num;

}
#undef BADCHAR

static int
sjis_wcstocts(conv, from, from_left, to, to_left, args, num_args)
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

    ret = sjis_mbstocts(conv, &from_mb, &mb_len, to, to_left,
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

#define UDCJA_SEG	"SUNUDCJA.1997-0"

XPointer sunudcja_conv(
	XPointer inbufptr, 
	XPointer outbufptr, 
	int	 *to_left,
	int	  ct_seglen)
{
	int	seg_len;
	int	i;

	seg_len = strlen(UDCJA_SEG);

	if(*(inbufptr + seg_len) != 0x2)	/* not terminated by STX */
		return outbufptr;

	if(strncmp(inbufptr, UDCJA_SEG, seg_len - 1))
		return outbufptr;	/* Not a SunUDCJA Segment */

	inbufptr += (seg_len+1);	/* Skip Segment Name */
	ct_seglen -= (seg_len+1);

	for(i = 0; i < ct_seglen; i += 2){
	    if(*inbufptr >= 0x21 && *inbufptr <= 0x34) { /* UDC */
		*inbufptr += 0x5e;
		jis_to_sjis((Uchar *)inbufptr, (Uchar *)(inbufptr+1));
		*outbufptr++ = *inbufptr++; (*to_left)--;
		*outbufptr++ = *inbufptr++; (*to_left)--;
	    } else if (*inbufptr >= 0x35 && *inbufptr <= 0x36) { /* IBM DC */
		unsigned short	dest;

		dest = ibmexttosj((Uchar) *inbufptr, (Uchar) *(inbufptr+1));
		*outbufptr++ = (dest >> 8) & 0xff; (*to_left)--; inbufptr++;
		*outbufptr++ = dest & 0xff; (*to_left)--; inbufptr++;
	    } else {
		*inbufptr++;
		*inbufptr++;
	    }
	}
	return outbufptr;
}

#define SKIP_I(str)	while (*(str) >= 0x20 && *(str) <=  0x2f) (str)++;
#define SKIP_P(str)	while (*(str) >= 0x30 && *(str) <=  0x3f) (str)++;

static int
sjis_ctstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    register XPointer inbufptr =  *from;
    register XPointer outbufptr =  *to;
    XPointer inbuf_base;
    XPointer outbuf_base = outbufptr;
    register int clen, length;
    int unconv_num = 0;
    unsigned int ct_seglen = 0;
    Uchar ct_type;
    CTData ctdp = ctdata;	/* default */
    int	i;
    char *ct_name;
    int GR_Left_FLAG = 0;
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

    for (length = ctdata[Ascii].length; *from_left > 0 ; (*from_left) -= length)
    {
	ct_type = CT_STD;
	if (*inbufptr == '\033' || *inbufptr == (char)'\233') {

	    for (ctdp = ctdata; ctdp <= ctd_endp ; ctdp++) {

		if(!strncmp(inbufptr, ctdp->ct_encoding, ctdp->ct_encoding_len))
		{
		    inbufptr += ctdp->ct_encoding_len;
		    (*from_left) -= ctdp->ct_encoding_len;
		    if( ctdp->length ) {
			length = ctdp->length;
			if (*from_left < length) {
			    *to = (XPointer)outbufptr;
			    *to_left -= outbufptr - outbuf_base;
			    return( unconv_num + *from_left );
			}
		    }
		    ct_type = ctdp->ct_type;
		    ct_name = ctdp->name;
		    break;
		}
	    }
	    if (ctdp > ctd_endp)  	/* failed to match CT sequence */
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
		    return( unconv_num + *from_left );
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
	    (*from_left) -= ct_seglen;
	    continue;
	  case CT_EXT1:
	    inbuf_base = inbufptr;
	    SKIP_P(inbufptr);
	    SKIP_I(inbufptr);
	    inbufptr++;
	    ct_seglen = (unsigned)(inbufptr - inbuf_base);
	    (*from_left) -= ct_seglen;
	    continue;
	  case CT_DIR:
	    continue;
	  case CT_VER:
	    inbufptr += 2;
	    (*from_left) -= 2;
	    continue;
	}

	if (ctdp->side == XlcGL || isrightside (*inbufptr)) {
	    clen = length;
	} else if (ct_type == CT_NSTD) {
	    clen = 1;
	    save_outbuf = False;
	} else {
	    clen = 1;
	    GR_Left_FLAG = 1; /* ASCII */
	    *from_left += length - clen;
	}
	do {
	    Uchar mask = (length == 2) ? GL : 0xff;
	    if (*to_left <= 0) {
		*to = (XPointer)outbufptr;
		unconv_num = *from_left;
		return unconv_num;
	    }
	    if (save_outbuf == True) {
		*outbufptr++ = *inbufptr++ & mask;
	    	(*to_left)--;
	    }
	} while (--clen);

	if (save_outbuf == True && length >= 2 && GR_Left_FLAG == 0) {
	    if(strncmp(ct_name, "JISX0212", 8)){ /* Not 212 (JISX0208) */
		jis_to_sjis((Uchar *)(outbufptr-2), (Uchar *)(outbufptr-1));
	    } else {
		unsigned short dest;
		dest = 
		    jis212tosj((Uchar) *(outbufptr-2), (Uchar) *(outbufptr-1));
		*(outbufptr-2) = (dest >> 8) & 0xff;
		*(outbufptr-1) = dest & 0xff;
	    }
	}

	GR_Left_FLAG = 0;
	save_outbuf = True;
    }

    *to = (XPointer)outbufptr;

#ifndef sun
    if ((num_conv = (int)(outbufptr - outbuf_base)) > 0)
	(*to_left) -= num_conv;
#endif

    return unconv_num;
}


static int
sjis_ctstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = (XLCd)conv->state;
    wchar_t *bufptr;
    register int buf_len = *to_left;
    char *mb_data, *to_mb_data;
    int mb_len, to_mb_left;
    int wc_len;
    char *csptr;
    int csstr_len;

    csptr = *((char **) from);
    csstr_len = *from_left;

    mb_data = (char *)Xmalloc((*to_left) * MB_LEN_MAX);
    if (mb_data == NULL)
        return -1;

    mb_len = (*to_left) * MB_LEN_MAX;
    memset(mb_data, 0, mb_len);

    to_mb_data = mb_data;
    to_mb_left = mb_len;

    bufptr = *((wchar_t **) to);

    if (sjis_ctstombs(conv, &csptr, &csstr_len, &to_mb_data, &to_mb_left,
		      args, num_args) == -1) {
        Xfree(mb_data);
	return -1;
    }

    wc_len = mbstowcs(NULL, mb_data, mb_len - to_mb_left);
    if (wc_len > buf_len) {
	/* overflow */
	*to_left = 0;
        Xfree(mb_data);
	return 1;
    }

    if (wc_len != mbstowcs(bufptr, mb_data, mb_len - to_mb_left)) {
        Xfree(mb_data);
	return -1;
    }

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

static void
close_converter(conv)
    XlcConv conv;
{
	Xfree((char *) conv);
}


static XlcConv
create_conv(lcd, methods)
    XLCd lcd;
    XlcConvMethods methods;
{
    XlcConv conv;

    conv = (XlcConv) Xmalloc(sizeof(XlcConvRec));
    if (conv == NULL)
	return (XlcConv) NULL;
    
    conv->methods = methods;
    conv->state = (XPointer) lcd;
    return conv;
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


enum { MBSTOCS, WCSTOCS, MBTOCS, CSTOMBS, CSTOWCS, MBSTOWCS, WCSTOMBS,
       WCSTOCTS, MBSTOCTS, CTSTOMBS, CTSTOWCS, STRTOMBS};

static XlcConvMethodsRec conv_methods[] = {
    {close_converter, sjis_mbstocs,  NULL },
    {close_converter, sjis_wcstocs,  NULL },
    {close_converter, sjis_mbtocs,   NULL },
    {close_converter, sjis_cstombs,  NULL },
    {close_converter, sjis_cstowcs,  NULL },
    {close_converter, sjis_mbstowcs, NULL },
    {close_converter, sjis_wcstombs, NULL },
    {close_converter, sjis_wcstocts, NULL },
    {close_converter, sjis_mbstocts, NULL },
    {close_converter, sjis_ctstombs, NULL },
    {close_converter, sjis_ctstowcs, NULL },
    {close_converter, strtombs,      NULL },
};


static XlcConv
open_mbstocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[MBSTOCS]);
}

static XlcConv
open_wcstocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[WCSTOCS]);
}

static XlcConv
open_mbtocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[MBTOCS]);
}

static XlcConv
open_cstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[CSTOMBS]);
}

static XlcConv
open_cstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[CSTOWCS]);
}

static XlcConv
open_mbstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[MBSTOWCS]);
}

static XlcConv
open_wcstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[WCSTOMBS]);
}

static XlcConv
open_wcstocts(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[WCSTOCTS]);
}

static XlcConv
open_mbstocts(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[MBSTOCTS]);
}

static XlcConv
open_ctstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[CTSTOMBS]);
}

static XlcConv
open_ctstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[CTSTOWCS]);
}

static XlcConv
open_strtombs(from_lcd, from_type, to_lcd, to_type)
XLCd from_lcd;
char *from_type;
XLCd to_lcd;
char *to_type;
{
    return create_conv(from_lcd, &conv_methods[STRTOMBS]);
}

XLCd
#ifdef DYNAMIC_LOAD
_XlcGenericLoader(name)
#else
_XlcSjisLoader(name)
#endif
    char *name;
{
    XLCd lcd;
    extern void _XlcAddUtf8Converters(XLCd);

    lcd = _XlcCreateLC(name, _XlcGenericMethods);
    if (lcd == NULL)
	return lcd;

    if ((_XlcNCompareISOLatin1(XLC_PUBLIC_PART(lcd)->codeset, "pck", 3)) &&
	(_XlcNCompareISOLatin1(XLC_PUBLIC_PART(lcd)->codeset, "sjis", 4))) {
	_XlcDestroyLC(lcd);
	return (XLCd) NULL;
    }

    initCTptr(lcd);

    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCharSet, open_wcstocs);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNWideChar, open_cstowcs);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCharSet, open_mbstocs);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNMultiByte, open_cstombs);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNChar, open_mbtocs);

#ifndef FORCE_INDIRECT_CONVERTER
    _XlcSetConverter(lcd, XlcNCompoundText, lcd, XlcNMultiByte, open_ctstombs);
    _XlcSetConverter(lcd, XlcNCompoundText, lcd, XlcNWideChar, open_ctstowcs);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCompoundText, open_mbstocts);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNWideChar, open_mbstowcs);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCompoundText, open_wcstocts);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNMultiByte, open_wcstombs);
#endif

    _XlcAddUtf8Converters(lcd);

    return lcd;
}

/*
 * sjtoibmext[]:
 * Directly convert Shift-JIS encoded 2bytes codes
 * to the EUC encoded 2bytes codes.
 * This table is used for "IBM Extended Character Set" only.
 *
 * Principal:
 * 1: Subtract 0xfa40.
 * 2: Index the array by using the value above as index number.
 * 3: Value "0xffff" means the index number is invalid or, there's
 *    no EUC value that corresponds to.
 *
 * Note: The SJIS code points, 0xfa54 and 0xfa5b are mapped to JIS208
 *       hence they are NOT mapped with tables below.
 *       (They are mapped to 0xa2cc and 0xa2e8 in JIS208 respectively.)
 */
static unsigned short sjtoibmext[] = {
0xf3f3, 0xf3f4, 0xf3f5, 0xf3f6, 0xf3f7, 0xf3f8, 0xf3f9, 0xf3fa, /* 0xfa47 */
0xf3fb, 0xf3fc, 0xf3fd, 0xf3fe, 0xf4a1, 0xf4a2, 0xf4a3, 0xf4a4,
0xf4a5, 0xf4a6, 0xf4a7, 0xf4a8, 0xffff, 0xa2c3, 0xf4a9, 0xf4aa,
0xf4ab, 0xf4ac, 0xf4ad, 0xffff, 0xd4e3, 0xdcdf, 0xe4e9, 0xe3f8,
0xd9a1, 0xb1bb, 0xf4ae, 0xc2ad, 0xc3fc, 0xe4d0, 0xc2bf, 0xbcf4,
0xb0a9, 0xb0c8, 0xf4af, 0xb0d2, 0xb0d4, 0xb0e3, 0xb0ee, 0xb1a7,
0xb1a3, 0xb1ac, 0xb1a9, 0xb1be, 0xb1df, 0xb1d8, 0xb1c8, 0xb1d7,
0xb1e3, 0xb1f4, 0xb1e1, 0xb2a3, 0xf4b0, 0xb2bb, 0xb2e6, 0xffff,
0xb2ed, 0xb2f5, 0xb2fc, 0xf4b1, 0xb3b5, 0xb3d8, 0xb3db, 0xb3e5,
0xb3ee, 0xb3fb, 0xf4b2, 0xf4b3, 0xb4c0, 0xb4c7, 0xb4d0, 0xb4de,
0xf4b4, 0xb5aa, 0xf4b5, 0xb5af, 0xb5c4, 0xb5e8, 0xf4b6, 0xb7c2,
0xb7e4, 0xb7e8, 0xb7e7, 0xf4b7, 0xf4b8, 0xf4b9, 0xb8ce, 0xb8e1,
0xb8f5, 0xb8f7, 0xb8f8, 0xb8fc, 0xb9af, 0xb9b7, 0xbabe, 0xbadb,
0xcdaa, 0xbae1, 0xf4ba, 0xbaeb, 0xbbb3, 0xbbb8, 0xf4bb, 0xbbca,
0xf4bc, 0xf4bd, 0xbbd0, 0xbbde, 0xbbf4, 0xbbf5, 0xbbf9, 0xbce4,
0xbced, 0xbcfe, 0xf4be, 0xbdc2, 0xbde7, 0xf4bf, 0xbdf0, 0xbeb0,
0xbeac, 0xf4c0, 0xbeb3, 0xbebd, 0xbecd, 0xbec9, 0xbee4, 0xbfa8,
0xbfc9, 0xc0c4, 0xc0e4, 0xc0f4, 0xc1a6, 0xf4c1, 0xc1f5, 0xc1fc,
0xf4c2, 0xc1f8, 0xc2ab, 0xc2a1, 0xc2a5, 0xf4c3, 0xc2b8, 0xc2ba,
0xf4c4, 0xc2c4, 0xc2d2, 0xc2d7, 0xc2db, 0xc2de, 0xc2ed, 0xc2f0,
0xf4c5, 0xc3a1, 0xc3b5, 0xc3c9, 0xc3b9, 0xf4c6, 0xc3d8, 0xc3fe,
0xf4c7, 0xc4cc, 0xf4c8, 0xc4d9, 0xc4ea, 0xc4fd, 0xf4c9, 0xc5a7,
0xc5b5, 0xc5b6, 0xf4ca, 0xc5d5, 0xc6b8, 0xc6d7, 0xc6e0, 0xc6ea,
0xc6e3, 0xc7a1, 0xc7ab, 0xc7c7, 0xc7c3, 0xffff, 0xffff, 0xffff,
0xc7cb, 0xc7cf, 0xc7d9, 0xf4cb, 0xf4cc, 0xc7e6, 0xc7ee, 0xc7fc, /* 0xfb47 */
0xc7eb, 0xc7f0, 0xc8b1, 0xc8e5, 0xc8f8, 0xc9a6, 0xc9ab, 0xc9ad,
0xf4cd, 0xc9ca, 0xc9d3, 0xc9e9, 0xc9e3, 0xc9fc, 0xc9f4, 0xc9f5,
0xf4ce, 0xcab3, 0xcabd, 0xcaef, 0xcaf1, 0xcbae, 0xf4cf, 0xcbca,
0xcbe6, 0xcbea, 0xcbf0, 0xcbf4, 0xcbee, 0xcca5, 0xcbf9, 0xccab,
0xccae, 0xccad, 0xccb2, 0xccc2, 0xccd0, 0xccd9, 0xf4d0, 0xcdbb,
0xf4d1, 0xcebb, 0xf4d2, 0xceba, 0xcec3, 0xf4d3, 0xcef2, 0xb3dd,
0xcfd5, 0xcfe2, 0xcfe9, 0xcfed, 0xf4d4, 0xf4d5, 0xf4d6, 0xffff,
0xf4d7, 0xd0e5, 0xf4d8, 0xd0e9, 0xd1e8, 0xf4d9, 0xf4da, 0xd1ec,
0xd2bb, 0xf4db, 0xd3e1, 0xd3e8, 0xd4a7, 0xf4dc, 0xf4dd, 0xd4d4,
0xd4f2, 0xd5ae, 0xf4de, 0xd7de, 0xf4df, 0xd8a2, 0xd8b7, 0xd8c1,
0xd8d1, 0xd8f4, 0xd9c6, 0xd9c8, 0xd9d1, 0xf4e0, 0xf4e1, 0xf4e2,
0xf4e3, 0xf4e4, 0xdcd3, 0xddc8, 0xddd4, 0xddea, 0xddfa, 0xdea4,
0xdeb0, 0xf4e5, 0xdeb5, 0xdecb, 0xf4e6, 0xdfb9, 0xf4e7, 0xdfc3,
0xf4e8, 0xf4e9, 0xe0d9, 0xf4ea, 0xf4eb, 0xe1e2, 0xf4ec, 0xf4ed,
0xf4ee, 0xe2c7, 0xe3a8, 0xe3a6, 0xe3a9, 0xe3af, 0xe3b0, 0xe3aa,
0xe3ab, 0xe3bc, 0xe3c1, 0xe3bf, 0xe3d5, 0xe3d8, 0xe3d6, 0xe3df,
0xe3e3, 0xe3e1, 0xe3d4, 0xe3e9, 0xe4a6, 0xe3f1, 0xe3f2, 0xe4cb,
0xe4c1, 0xe4c3, 0xe4be, 0xf4ef, 0xe4c0, 0xe4c7, 0xe4bf, 0xe4e0,
0xe4de, 0xe4d1, 0xf4f0, 0xe4dc, 0xe4d2, 0xe4db, 0xe4d4, 0xe4fa,
0xe4ef, 0xe5b3, 0xe5bf, 0xe5c9, 0xe5d0, 0xe5e2, 0xe5ea, 0xe5eb,
0xf4f1, 0xf4f2, 0xf4f3, 0xe6e8, 0xe6ef, 0xe7ac, 0xf4f4, 0xe7ae,
0xf4f5, 0xe7b1, 0xf4f6, 0xe7b2, 0xe8b1, 0xe8b6, 0xf4f7, 0xf4f8,
0xe8dd, 0xf4f9, 0xf4fa, 0xe9d1, 0xf4fb, 0xffff, 0xffff, 0xffff,
0xe9ed, 0xeacd, 0xf4fc, 0xeadb, 0xeae6, 0xeaea, 0xeba5, 0xebfb, /* 0xfc47 */
0xebfa, 0xf4fd, 0xecd6, 0xf4fe, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

unsigned short
ibmdctojis212(char *src)
{
	unsigned short	s;
	unsigned short	dest;
	int		count;

	/* Check invalid character */
	if ((Uchar)*src < 0xfa || (Uchar)*src > 0xfc)
		return 0xffff;
	if ((Uchar)*(src + 1) < 0x40 || (Uchar)*(src + 1) > 0xfc)
		return 0xffff;

	count = (Uchar)*src - 0xfa; /* 'raw' number */
	s = ( (Uchar)*src << 8); src++;
	s += (Uchar)*src;

	s = s -  0xfa40 - (count * 0x40);
	dest = sjtoibmext[s];

	if(dest != 0xffff)
		dest -= 0x8080;

	return dest;
}

#define SJSPACE	0x8140

/*
 * lookuptbl()
 * Return the index number if its index-ed number
 * is the same as dest value.
 */
static unsigned short
lookuptbl(unsigned short dest)
{
	unsigned short tmp;
	int i;
	int sz = (sizeof (sjtoibmext) / sizeof (sjtoibmext[0]));

	for (i = 0; i < sz; i++) {
		tmp = (sjtoibmext[i] & 0x7f7f);
		if (tmp == dest)
			return ((i + 0xfa40 + ((i / 0xc0) * 0x40)));
	}
	return (SJSPACE);
}

unsigned short 
ibmexttosj(Uchar c1, Uchar c2)
{
	unsigned short	dest;

	dest = ((c1 + 0x3e) << 8);
	dest += c2;
	dest = lookuptbl(dest);

	return dest;
}

unsigned short
jis212tosj(Uchar c1, Uchar c2)
{
	unsigned short	dest;

	if (c1 >= 0x75) {
		Uchar c3 = c1;
		Uchar c4 = c2;
		jis_to_sjis(&c3, &c4);
		c3 += 5;
		dest = (c3 << 8) | c4;
	} else {
		dest = ((c1) << 8);
		dest += c2;
		dest = lookuptbl(dest);
	}
	return dest;
}

/*
 * Remap NEC/IBM codes to IBM codes
 * if p1/p2 == 0xffff, that means the source
 * code point is illegal in the current spec.
 */

void
nec_to_ibm(p1, p2)
Uchar	*p1;
Uchar	*p2;
{
	Uint	dest;

	dest = ((*p1 << 8) + *p2);

	if ((0xed40 <= dest) && (dest <= 0xed62)) { 
		dest += 0xd1c; 
	} else if ((0xed63 <= dest) && (dest <= 0xed7e)) { 
		dest += 0xd1d; 
	} else if ((0xed80 <= dest) && (dest <= 0xede0)) { 
		dest += 0xd1c; 
	} else if ((0xede1 <= dest) && (dest <= 0xedfc)) { 
		dest += 0xd5f; 
	} else if ((0xee40 <= dest) && (dest <= 0xee62)) { 
		dest += 0xd1c; 
	} else if ((0xee63 <= dest) && (dest <= 0xee7e)) { 
		dest += 0xd1d; 
	} else if ((0xee80 <= dest) && (dest <= 0xeee0)) { 
		dest += 0xd1c; 
	} else if ((0xeee1 <= dest) && (dest <= 0xeeec)) { 
		dest += 0xd5f; 
	} else if ((0xeeef <= dest) && (dest <= 0xeef8)) { 
		dest += 0xb51; 
	} else if ((0xeef9 <= dest) && (dest <= 0xeefc)) { 
		dest += 0xb5b; 
	} else { 
		dest = SJSPACE; 
	}

	*p2 = (dest & 0xff);
	*p1 = ((dest >> 8) & 0xff);
}
