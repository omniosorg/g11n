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
 * Copyright (c) 1991,1997, by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma ident	"@(#)wdresolve.c	1.11 98/03/23 SMI"

#include <stdlib.h>
#include <wctype.h>
#include <locale.h>
#include <string.h>

#include <wdresolve.h>
static int	initialized = 0;
static int	conservative_edit = 0;
static char	*multicolumn_filler;
static wchar_t ctype_ieol[128], ctype_itol[128], ctype_punct[5];
static wchar_t special_kana;

#define	iswjisx0201r(wc)	iswctype(wc, wctype("jisx0201r"))
#define	iswjisx0208(wc)		iswctype(wc, wctype("jisx0208"))
#define	iswjisx0212(wc)		iswctype(wc, wctype("jisx0212"))
#define	iswudc(wc)		iswctype(wc, wctype("udc"))
#define	iswvdc(wc)		iswctype(wc, wctype("vdc"))
#define	iswjspecial(wc)		iswctype(wc, wctype("jspecial"))
#define	iswjalnum(wc)		(iswctype(wc, wctype("jalpha")) || \
				 iswctype(wc, wctype("jdigit")))
#define	iswjhira(wc)		iswctype(wc, wctype("jhira"))
#define	iswjkata(wc)		iswctype(wc, wctype("jkata"))
#define	iswjgreek(wc)		iswctype(wc, wctype("jgreek"))
#define	iswjrussian(wc)		iswctype(wc, wctype("jrussian"))
#define	iswjline(wc)		iswctype(wc, wctype("line"))

static void
_init(void)
{
	register int i, n;
	register char *itol, *ieol, *punct;
	char env_name[64];

	/* convert character classification boundaries to wchar_t */
	(void)mbtowc(&special_kana, (char *)x0208_special_kana, MB_CUR_MAX);

	/* differenciate text editor from text formatter (user's option) */
	if (getenv("MB_CONSERVATIVE_EDIT"))
		conservative_edit = 1;

	/* get filler character for multicolumn character at right margin */
	multicolumn_filler = getenv("MC_FILLER");
	(void)strcpy(env_name, "CTYPE_ITOL.");
	(void)strcat(env_name, setlocale(LC_CTYPE, NULL));

	/* get characters inhibited at top of a line */
	if (!(itol = getenv(env_name)))
		itol = (char *)itol_dflt;
	i = 0;
	while (i < 127 && *itol) {
		if ((n = mbtowc(&ctype_itol[i++], itol, MB_CUR_MAX)) <= 0)
			break;	/* silently */
		itol += n;
	}
	ctype_itol[i] = 0;
	(void)strcpy(env_name, "CTYPE_IEOL.");
	(void)strcat(env_name, setlocale(LC_CTYPE, NULL));

	/* get characters inhibited at end of a line */
	if (!(ieol = getenv(env_name)))
		ieol = (char *)ieol_dflt;
	i = 0;
	while (i < 127 && *ieol) {
		if ((n = mbtowc(&ctype_ieol[i++], ieol, MB_CUR_MAX)) <= 0)
			break;	/* silently */
		ieol += n;
	}
	ctype_ieol[i] = 0;
	punct = (char *)punct_dflt;
	i = 0;
	while (i < 7 && *punct) {
		if ((n = mbtowc(&ctype_punct[i++], punct, MB_CUR_MAX)) <= 0)
			break;	/* silently */
		punct += n;
	}
	ctype_punct[i] = 0;
	initialized = 1;
}

int
_wdchkind_(wc)
wchar_t wc;
{

	if (!initialized)
		_init();
	if (iswascii(wc)) {
		return (iswalpha(wc) || iswdigit(wc) || wc == L'_');
	} else if (iswvdc(wc)) {
		return (9);
	} else if (iswjisx0208(wc)) {
		/* special katakana */
		if (wc == special_kana)
			return (4);
		/* miscellaneous symbols */
		if (iswjspecial(wc))
			return (0);
		/* alphanumeric */
		if (iswjalnum(wc))
			return (2);
		/* hiragana */
		if (iswjhira(wc))
			return (3);
		/* katakana */
		if (iswjkata(wc))
			return (4);
		/* Greek */
		if (iswjgreek(wc))
			return (6);
		/* Russian */
		if (iswjrussian(wc))
			return (0);
		/* other non-Kanji */
		if (iswjline(wc))
			return (7);
		/* Kanji */
		return (5);
	} else if (iswjisx0201r(wc)) {
		return (8);
	} else if (iswjisx0212(wc)) {
		return (9);
	} else if (iswudc(wc)) {
		return (10);
	}
	return (0); 	/* Only C1 cntrl */
}

int
_wdbindf_(wc1, wc2, type)
wchar_t wc1, wc2;
int type;
{
	register wchar_t *special;
	int hit;

	if (!initialized)
		_init();
	if (conservative_edit && type ==2)
		return (6);
	hit = 0;
	special = ctype_itol;
	while (*special)
		if (*special++ == wc2)
			hit = 1;
	special = ctype_ieol;
	while (*special)
		if (*special++ == wc1)
			hit = 1;
	if ((iswascii(wc1) && iswascii(wc2)) || hit)
		return (6);
	special = ctype_punct;
	while  (*special)
		if (*special++ == wc1)
			return (2);
	return (4);
}

wchar_t *
_wddelim_(wc1, wc2, type)
wchar_t wc1, wc2;
int type;
{
	static wchar_t delim[2] = {0};
	int inhibit;
	register wchar_t *special;

	if (!initialized)
		_init();
	inhibit = 0;
	if (conservative_edit && type == 2) {
		delim[0] = L' ';
		delim[1] = 0;
		return (&delim[0]);
	}
	special = ctype_itol;
	while (*special)
		if (*special++ == wc1)
			inhibit = 1;
	special = ctype_ieol;
	while (*special)
		if (*special++ == wc2)
			inhibit = 1;
	if (!inhibit && (wc1 != L'\n') && (iswascii(wc1) ||
	   iswascii(wc2) || iswjisx0201r(wc1) || iswjisx0201r(wc2))) {
		delim[0] = L' ';
		delim[1] = 0;
	} else delim[0] = 0;
	return (&delim[0]);
}

wchar_t
_mcfiller_()
{
	wchar_t fillerchar;

	if (!initialized)
		_init();
	if (!multicolumn_filler) {
		fillerchar = '~';
	} else {
		if (mbtowc(&fillerchar, multicolumn_filler, MB_CUR_MAX) <= 0)
			fillerchar = '~';
		if (!iswprint(fillerchar))
			fillerchar = '~';
	}
	return (fillerchar);
}
