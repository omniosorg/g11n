/* $XConsortium: lcPublic.c,v 1.5 94/02/08 11:41:49 kaleb Exp $ */
/*
 * Copyright (c) 1996, 2001 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma	ident	"@(#)lcPublicUnicode.c	1.4 01/07/16 SMI"

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

#include <stdio.h>
#include <X11/Xlibint.h>
#include <X11/XlcPubI.h>

#include "XlcDBUnicode.h"

/* Overwrite "initialize" and "destroy" method in PublicMethods */
static Bool initialize();
static void destroy();

static XLCdPublicMethodsRec publicMethods = {
    {
	NULL,
	_XlcDefaultMapModifiers,
	NULL,
	NULL,
	_XrmDefaultInitParseInfo,
	_XmbTextPropertyToTextList,
	_XwcTextPropertyToTextList,
	_XmbTextListToTextProperty,
	_XwcTextListToTextProperty,
	_XwcFreeStringList,
	NULL,
	NULL,
	NULL
    }, 
    {
	NULL,
	NULL,
	initialize,
	destroy,
	NULL,
	_XlcGetLocaleDataBase
    }
};


void
_XlcInitPublicMethodsUnicode()
{
    publicMethods.core.close =
	((XLCdPublicMethods)_XlcPublicMethods)->core.close;
    publicMethods.core.default_string =
	((XLCdPublicMethods)_XlcPublicMethods)->core.default_string;
    publicMethods.pub.create =
	((XLCdPublicMethods)_XlcPublicMethods)->pub.create;
    publicMethods.pub.get_values =
	((XLCdPublicMethods)_XlcPublicMethods)->pub.get_values;
    _XlcPublicMethods = (XLCdMethods)&publicMethods;
}


#ifdef sun
extern Bool _XlcInitCTInfo(char*);
#else
extern Bool _XlcInitCTInfo();
#endif

static Bool
initialize(XLCd lcd)
{
    XLCdMethods lcdmethods = lcd->methods;
    XLCdMethods core = &publicMethods.core;
    XLCdPublicMethodsPart *methods = XLC_PUBLIC_METHODS(lcd);
    XLCdPublicMethodsPart *pub_methods = &publicMethods.pub;
    XLCdPublicPart *pub = XLC_PUBLIC_PART(lcd);
    char lang[128], terr[128], code[128], *str;
    char *name;
    char **values;
    int num;
#if !defined(X_NOT_STDC_ENV) && !defined(X_LOCALE)
    char siname[256];
    char *_XlcMapOSLocaleName();
#endif

#ifdef sun
    _XlcInitCTInfo(lcd->core->name);
#else
    _XlcInitCTInfo();
#endif

    /* initialize_core */
    if (lcdmethods->close == NULL)
	lcdmethods->close = core->close;

    if (lcdmethods->map_modifiers == NULL)
	lcdmethods->map_modifiers = core->map_modifiers;

    if (lcdmethods->open_om == NULL)
	_XInitPublicOM(lcd);

    if (lcdmethods->open_im == NULL)
	_XInitPublicIM(lcd);

    if (lcdmethods->init_parse_info == NULL)
	lcdmethods->init_parse_info = core->init_parse_info;

    if (lcdmethods->mb_text_prop_to_list == NULL)
	lcdmethods->mb_text_prop_to_list = core->mb_text_prop_to_list;

    if (lcdmethods->wc_text_prop_to_list == NULL)
	lcdmethods->wc_text_prop_to_list = core->wc_text_prop_to_list;

    if (lcdmethods->mb_text_list_to_prop == NULL)
	lcdmethods->mb_text_list_to_prop = core->mb_text_list_to_prop;

    if (lcdmethods->wc_text_list_to_prop == NULL)
	lcdmethods->wc_text_list_to_prop = core->wc_text_list_to_prop;

    if (lcdmethods->wc_free_string_list == NULL)
	lcdmethods->wc_free_string_list = core->wc_free_string_list;

    if (lcdmethods->default_string == NULL)
	lcdmethods->default_string = core->default_string;

    name = lcd->core->name;
#if !defined(X_NOT_STDC_ENV) && !defined(X_LOCALE)
    name = _XlcMapOSLocaleName(name, siname);
#endif
	
    if (_XlcResolveLocaleName(name, NULL, lang, terr, code) == 0)
	return False;

    str = (char*) Xmalloc(strlen(name) + strlen(lang) + strlen(terr) +
			  strlen(code) + 4);
    if (str == NULL)
	return False;

    strcpy(str, name);
    pub->siname = str;
    str += strlen(str) + 1;

    strcpy(str, lang);
    pub->language = str;
    str += strlen(str) + 1;

    strcpy(str, terr);
    pub->territory = str;
    str += strlen(str) + 1;

    strcpy(str, code);
    pub->codeset = str;

    if (pub->default_string == NULL)
	pub->default_string = "";

    if (methods->get_values == NULL)
	methods->get_values = pub_methods->get_values;

    if (methods->get_resource == NULL)
	methods->get_resource = pub_methods->get_resource;

    /* load_public */
    if(_XlcCreateLocaleDataBaseUnicode(lcd) == NULL)
#ifdef sun
    {
	XLCdPublicMethodsPart *methods = XLC_PUBLIC_METHODS(lcd);
	/* In this case, localeDB was not found, so get_resource()
	   method should do nothing */
	methods->get_resource = NULL;
    }
#else
	return False;
#endif

    _XlcGetResource(lcd, "XLC_XLOCALE", "mb_cur_max", &values, &num);
    if (num > 0) {
	pub->mb_cur_max = atoi(values[0]);
	if (pub->mb_cur_max < 1)
	    pub->mb_cur_max = 1;
    } else
	pub->mb_cur_max = 1;

    _XlcGetResource(lcd, "XLC_XLOCALE", "state_dependent", &values, &num);
    if (num > 0 && !_XlcCompareISOLatin1(values[0], "True"))
	pub->is_state_depend = True;
    else
	pub->is_state_depend = False;

    _XlcGetResource(lcd, "XLC_XLOCALE", "encoding_name", &values, &num);
    str = (num > 0) ? values[0] : "STRING";
    pub->encoding_name = (char*) Xmalloc(strlen(str) + 1);
    if (pub->encoding_name == NULL)
	return False;
    strcpy(pub->encoding_name, str);

    return True;
}

static void
destroy(XLCd lcd)
{
    XLCdPublicPart *pub = XLC_PUBLIC_PART(lcd);

    _XlcDestroyLocaleDataBaseUnicode(lcd);

    if (pub->siname)
	Xfree(pub->siname);
    if (pub->encoding_name)
	Xfree(pub->encoding_name);

    /* destroy_core */
    if (lcd->core) {
	if (lcd->core->name)
            Xfree(lcd->core->name);
	Xfree(lcd->core);
    }

    if (lcd->methods)
	Xfree(lcd->methods);

    Xfree(lcd);
}
