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
/* $XConsortium: XlcPublic.h,v 1.3 94/12/12 20:34:32 kaleb Exp $ */
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

#ifndef _XLCPUBLIC_H_
#define _XLCPUBLIC_H_

#include "Xlcint.h"

#define XlcNCharSize "charSize"
#define XlcNCodeset "codeset"
#define XlcNControlSequence "controlSequence"
#define XlcNDefaultString "defaultString"
#define XlcNEncodingName "encodingName"
#define XlcNLanguage "language"
#define XlcNMbCurMax "mbCurMax"
#define XlcNName "name"
#define XlcNSetSize "setSize"
#define XlcNSide "side"
#define XlcNStateDependentEncoding "stateDependentEncoding"
#define XlcNTerritory "territory"

typedef enum {
    XlcUnknown, XlcC0, XlcGL, XlcC1, XlcGR, XlcGLGR, XlcOther
} XlcSide;

typedef struct _XlcCharSetRec *XlcCharSet;

typedef char* (*XlcGetCSValuesProc)(
#if NeedFunctionPrototypes
    XlcCharSet		/* charset */,
    XlcArgList		/* args */,
    int			/* num_args */
#endif
);

typedef struct _XlcCharSetRec {
    char *name;			/* character set name */
    XrmQuark xrm_name;
    char *encoding_name;	/* XLFD encoding name */
    XrmQuark xrm_encoding_name;
    XlcSide side;		/* GL, GR or others */
    int char_size;		/* number of bytes per character */
    int set_size;		/* graphic character sets */
    char *ct_sequence;		/* control sequence of CT */
    XlcGetCSValuesProc get_values;
} XlcCharSetRec;

/*
 * conversion methods
 */

typedef struct _XlcConvRec *XlcConv;

typedef XlcConv (*XlcOpenConverterProc)(
#if NeedFunctionPrototypes
    XLCd		/* from_lcd */,
    char*		/* from_type */,
    XLCd		/* to_lcd */,
    char*		/* to_type */
#endif
);

typedef void (*XlcCloseConverterProc)(
#if NeedFunctionPrototypes
    XlcConv		/* conv */
#endif
);

typedef int (*XlcConvertProc)(
#if NeedFunctionPrototypes
    XlcConv		/* conv */,
    XPointer*		/* from */,
    int*		/* from_left */,
    XPointer*		/* to */,
    int*		/* to_left */,
    XPointer*		/* args */,
    int			/* num_args */
#endif
);

typedef void (*XlcResetConverterProc)(
#if NeedFunctionPrototypes
    XlcConv		/* conv */
#endif
);

typedef struct _XlcConvMethodsRec{
    XlcCloseConverterProc close;
    XlcConvertProc convert;
    XlcResetConverterProc reset;
} XlcConvMethodsRec, *XlcConvMethods;

/*
 * conversion data
 */

#define XlcNMultiByte "multiByte"
#define XlcNWideChar "wideChar"
#define XlcNCompoundText "compoundText"
#define XlcNString "string"
#define XlcNCharSet "charSet"
#define XlcNCTCharSet "CTcharSet"
#define XlcNChar "char"

typedef struct _XlcConvRec {
    XlcConvMethods methods;
    XPointer state;
} XlcConvRec;


_XFUNCPROTOBEGIN

extern Bool _XInitPublicOM(
#if NeedFunctionPrototypes
    XLCd		/* lcd */
#endif
);

extern Bool _XInitPublicIM(
#if NeedFunctionPrototypes
    XLCd		/* lcd */
#endif
);

extern char *_XGetLCValues(
#if NeedVarargsPrototypes
    XLCd		/* lcd */,
    ...
#endif
);

extern XlcCharSet _XlcGetCharSet(
#if NeedFunctionPrototypes
    char*		/* name */
#endif
);

extern Bool _XlcAddCharSet(
#if NeedFunctionPrototypes
    XlcCharSet		/* charset */
#endif
);

extern char *_XlcGetCSValues(
#if NeedVarargsPrototypes
    XlcCharSet		/* charset */,
    ...
#endif
);

extern XlcConv _XlcOpenConverter(
#if NeedFunctionPrototypes
    XLCd		/* from_lcd */,
    char*		/* from_type */,
    XLCd		/* to_lcd */,
    char*		/* to_type */
#endif
);

extern void _XlcCloseConverter(
#if NeedFunctionPrototypes
    XlcConv		/* conv */
#endif
);

extern int _XlcConvert(
#if NeedFunctionPrototypes
    XlcConv		/* conv */,
    XPointer*		/* from */,
    int*		/* from_left */,
    XPointer*		/* to */,
    int*		/* to_left */,
    XPointer*		/* args */,
    int			/* num_args */
#endif
);

extern void _XlcResetConverter(
#if NeedFunctionPrototypes
    XlcConv		/* conv */
#endif
);

extern Bool _XlcSetConverter(
#if NeedFunctionPrototypes
    XLCd			/* from_lcd */,
    char*			/* from_type */,
    XLCd			/* to_lcd */,
    char*			/* to_type */,
    XlcOpenConverterProc	/* open_converter */
#endif
);

extern void _XlcGetResource(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    char*		/* category */,
    char*		/* class */,
    char***		/* value */,
    int*		/* count */
#endif
);

extern char *_XlcFileName(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    char*		/* category */
#endif
);

extern char *_XlcLocaleDirName(
#if NeedFunctionPrototypes
    char*		/* lc_name */
#endif
);
extern int _Xwcslen(
#if NeedFunctionPrototypes
    wchar_t*		/* wstr */
#endif
);

extern wchar_t *_Xwcscpy(
#if NeedFunctionPrototypes
    wchar_t*		/* wstr1 */,
    wchar_t*		/* wstr2 */
#endif
);

extern int _XlcCompareISOLatin1(
#if NeedFunctionPrototypes
    char*		/* str1 */,
    char*		/* str2 */
#endif
);

extern int _XlcNCompareISOLatin1(
#if NeedFunctionPrototypes
    char*		/* str1 */,
    char*		/* str2 */,
    int			/* len */
#endif
);

_XFUNCPROTOEND

#endif  /* _XLCPUBLIC_H_ */
