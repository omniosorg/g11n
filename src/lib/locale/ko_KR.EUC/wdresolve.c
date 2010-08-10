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
 *
 * Copyright (c) 1991, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 1991, Nihon Sun Microsystems K.K.
 */

#ident	" @(#)wdresolve.c	1.8 96/01/10 "

#include <stdlib.h>
#include <locale.h>
#include <wctype.h>
#include <widec.h>
#include <ko/xctype.h>

static int	initialized = 0;
static int	conservative_edit = 0;
static char	*multicolumn_filler;

static void
_init(void)
{
	register int i, n;
	register char *itol, *ieol, *punct;
	char env_name[64];

	/* differenciate text editor from text formatter (user's option) */
	if (getenv("MB_CONSERVATIVE_EDIT"))
		conservative_edit = 1;

	/* get filler character for multicolumn character at right margin */
	multicolumn_filler = getenv("MC_FILLER");

	initialized = 1;
}

int
_wdchkind_(wc)
wchar_t wc;
{

	if (!initialized)
		_init();
	switch (wcsetno(wc)) {
		case 1:
			/* Hangul */
			if (iskhangul(wc))
				return (2);
			/* Hanja */
			if (iskhanja(wc))
				return (3);
			/* Hiragana */
			if (iskhira(wc))
				return (4);
			/* Katakana */
			if (iskkata(wc))
				return (5);
			/* Greek */
			if (iskgreek(wc))
				return (6);
			/* Russian */
			if (iskrussian(wc))
				return (7);
			/* Latin */
			if (isklatin(wc))
				return (8);
			/* Alphanumeric */
			if (iskroman(wc) || iskromannum(wc))
				return (9);
			/* misc. */
			return(0);
			break;	/* NOT REACHED */
		case 2:
			return (11);
			break;	/* NOT REACHED */
		case 3:
			return (12);
			break;	/* NOT REACHED */
		case 0:
			return (isalpha(wc) || isdigit(wc) || wc == '_');
			break;	/* NOT REACHED */
	}
	return (0);
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

	/* Since Hangul is blank-delimited text language
	   return type 6 always */

	return (6);
}

wchar_t *
_wddelim_(wc1, wc2, type)
wchar_t wc1, wc2;
int type;
{
	static wchar_t delim[2] = {0};

	if (!initialized)
		_init();
	
	if (conservative_edit && type == 2) {
		delim[0] = ' ';
		delim[1] = 0;
		return (&delim[0]);
	}

	delim[0] = ' ';
	delim[1] = 0;
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
