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
/* $XConsortium: XlcPubI.h,v 1.4 95/02/22 22:02:55 kaleb Exp $ */
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

#ifndef _XLCPUBLICI_H_
#define _XLCPUBLICI_H_

#include "XlcPublic.h"

#define XLC_PUBLIC(lcd, x)	(((XLCdPublic) lcd->core)->pub.x)
#define XLC_PUBLIC_PART(lcd)	(&(((XLCdPublic) lcd->core)->pub))
#define XLC_PUBLIC_METHODS(lcd)	(&(((XLCdPublicMethods) lcd->methods)->pub))

/*
 * XLCd public methods
 */

typedef struct _XLCdPublicMethodsRec *XLCdPublicMethods;

typedef XLCd (*XlcPubCreateProc)(
#if NeedFunctionPrototypes
    char*		/* name */,
    XLCdMethods		/* methods */
#endif
);

typedef Bool (*XlcPubInitializeProc)(
#if NeedFunctionPrototypes
    XLCd		/* lcd */
#endif
);

typedef void (*XlcPubDestroyProc)(
#if NeedFunctionPrototypes
    XLCd		/* lcd */
#endif
);

typedef char* (*XlcPubGetValuesProc)(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    XlcArgList		/* args */,
    int			/* num_args */
#endif
);

typedef void (*XlcPubGetResourceProc)(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    char*		/* category */,
    char*		/* class */,
    char***		/* value */,
    int*		/* count */
#endif
);

typedef struct _XLCdPublicMethodsPart {
    XLCdPublicMethods superclass;
    XlcPubCreateProc create;
    XlcPubInitializeProc initialize;
    XlcPubDestroyProc destroy;
    XlcPubGetValuesProc get_values;
    XlcPubGetResourceProc get_resource;
} XLCdPublicMethodsPart;

typedef struct _XLCdPublicMethodsRec {
    XLCdMethodsRec core;
    XLCdPublicMethodsPart pub;
} XLCdPublicMethodsRec;

/*
 * XLCd public data
 */

typedef struct _XLCdPublicPart {
    char *siname;			/* for _XlcMapOSLocaleName() */
    char *language;			/* language part of locale name */
    char *territory;			/* territory part of locale name */
    char *codeset;			/* codeset part of locale name */
    char *encoding_name;		/* encoding name */
    int mb_cur_max;			/* ANSI C MB_CUR_MAX */
    Bool is_state_depend;		/* state-depend encoding */
    char *default_string;		/* for XDefaultString() */
    XPointer xlocale_db;
} XLCdPublicPart;

typedef struct _XLCdPublicRec {
    XLCdCoreRec core;	
    XLCdPublicPart pub;
} XLCdPublicRec, *XLCdPublic;

extern XLCdMethods _XlcPublicMethods;

_XFUNCPROTOBEGIN

extern XLCd _XlcCreateLC(
#if NeedFunctionPrototypes
    char*		/* name */,
    XLCdMethods		/* methods */
#endif
);

extern void _XlcDestroyLC(
#if NeedFunctionPrototypes
    XLCd		/* lcd */
#endif
);

extern Bool _XlcParseCharSet(
#if NeedFunctionPrototypes
    XlcCharSet		/* charset */
#endif
);

extern XlcCharSet _XlcCreateDefaultCharSet(
#if NeedFunctionPrototypes
    char*		/* name */,
    char*		/* control_sequence */
#endif
);

extern XlcCharSet _XlcAddCT(
#if NeedFunctionPrototypes
    char*		/* name */,
    char*		/* control_sequence */
#endif
);

extern XrmMethods _XrmDefaultInitParseInfo(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    XPointer*		/* state */
#endif
);

extern int _XmbTextPropertyToTextList(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    Display*		/* dpy */,
    XTextProperty*	/* text_prop */,
    char***		/* list_ret */,
    int*		/* count_ret */
#endif
);

extern int _XwcTextPropertyToTextList(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    Display*		/* dpy */,
    XTextProperty*	/* text_prop */,
    wchar_t***		/* list_ret */,
    int*		/* count_ret */
#endif
);

extern int _XmbTextListToTextProperty(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    Display*		/* dpy */,
    char**		/* list */,
    int			/* count */,
    XICCEncodingStyle	/* style */,
    XTextProperty*	/* text_prop */
#endif
);

extern int _XwcTextListToTextProperty(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    Display*		/* dpy */,
    wchar_t**		/* list */,
    int			/* count */,
    XICCEncodingStyle	/* style */,
    XTextProperty*	/* text_prop */
#endif
);

extern void _XwcFreeStringList(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    wchar_t**		/* list */
#endif
);

extern int _XlcResolveLocaleName(
#if NeedFunctionPrototypes
    const char*		/* lc_name */,
    char*		/* full_name */,
    char*		/* language */,
    char*		/* territory */,
    char*		/* codeset */
#endif
);

extern int _XlcResolveDBName(
#if NeedFunctionPrototypes
    const char*		/* lc_name */,
    char*		/* file_name */
#endif
);

extern int _XlcResolveI18NPath(
#if NeedFunctionPrototypes
    char*		/* path_name */
#endif
);

extern XPointer _XlcCreateLocaleDataBase(
#if NeedFunctionPrototypes
    XLCd		/* lcd */
#endif
);

extern void _XlcDestroyLocaleDataBase(
#if NeedFunctionPrototypes
    XLCd		/* lcd */
#endif
);

extern void _XlcGetLocaleDataBase(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    char*		/* category */,
    char*		/* name */,
    char***		/* value */,
    int*		/* count */
#endif
);

_XFUNCPROTOEND

#endif  /* _XLCPUBLICI_H_ */
