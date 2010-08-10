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
 * Copyright (c) 1991, 2010, Oracle and/or its affiliates. All rights reserved.
 */

#ident	"$Id: wdresolve.c,v 1.1 1998/10/20 15:34:48 jkang Exp $"

#include <stdlib.h>
#include <wctype.h>
#include <widec.h>
#include <locale.h>
#include <zh/xctype.h>

#define is_pinyin(c)	((c) > 127 ? _iswctype(c, _E20) : 0)
#define is_chuyin(c)    ((c) > 127 ? _iswctype(c, _E21) : 0)
#define is_hiragana(c)    ((c) > 127 ? _iswctype(c, _E11) : 0)
#define is_katakana(c)    ((c) > 127 ? _iswctype(c, _E12) : 0)
#define is_greek(c)    ((c) > 127 ? _iswctype(c, _E13) : 0)
#define is_russian(c)    ((c) > 127 ? _iswctype(c, _E15) : 0)
#define is_other_special(c)    ((c) > 127 ? \
			 _iswctype(c, _E16 | _E18 | _E19) : 0)

static int wd_bind_strength[][14] = {
	3, 5, 5, 5, 5, 5, 5, 5, 5, 4, 5, -1, -1, -1,
	5, 6, 3, 5, 3, 3, 3, 3, 3, 3, 5, -1, -1, -1,
	5, 3, 4, 3, 3, 3, 3, 3, 3, 5, 3, -1, -1, -1,
	5, 6, 3, 6, 3, 3, 3, 3, 3, 5, 3, -1, -1, -1,
	5, 3, 3, 3, 6, 3, 3, 3, 3, 5, 3, -1, -1, -1,
	5, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, -1, -1, -1,
	5, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, -1, -1, -1,
	5, 3, 3, 3, 3, 3, 3, 6, 3, 5, 3, -1, -1, -1,
	5, 3, 3, 3, 3, 3, 3, 3, 6, 5, 3, -1, -1, -1,
	5, 3, 5, 5, 5, 5, 5, 5, 5, 3, 5, -1, -1, -1,
	5, 6, 3, 3, 3, 3, 3, 3, 3, 5, 6, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0
};

static int	initialized = 0;
static int	conservative_edit = 0;
static char	*multicolumn_filler;

static void
_init(void)
{
	register int i, n;
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
			/* Chinese ideograms */
			if ( isideogram(wc) )
                                return (2);

			/* pinyin symbols */
			if ( is_pinyin(wc) )
                                return (3);

			/* zhuyin symbols */
			if (is_chuyin(wc) )
				return(10);

			/* wide alphanumeric & _ */
			if ( isenglish(wc) || isnumber(wc) || wc == 0xa3df )
				return(4);

                        /* Japanese hiragana */
			if ( is_hiragana(wc) )
				return(5);

			/* Japanese katakana */
			if ( is_katakana(wc) )
				return(6);

			/* Greek alphabet */
			if ( is_greek(wc) )
				return(7);

			/* Russian alphabet */
			if ( is_russian(wc) )
				return(8);

			/* other special symbols in GB-80 */
			if ( is_other_special(wc) )
				return(9);

			break;	/* NOT REACHED */
		case 2:
			return (12);
			break;	/* NOT REACHED */
		case 3:
			return (13);
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
	int	i, j;

	if (!initialized)
		_init();

	i = _wdchkind_(wc1);
	j = _wdchkind_(wc2);

	return( wd_bind_strength[i][j] );
}

wchar_t *
_wddelim_(wc1, wc2, type)
wchar_t wc1, wc2;
int type;
{
	static wchar_t delim[2] = {0};
	int	i, j;

	if (!initialized)
		_init();

	if (conservative_edit && type == 2) {
		delim[0] = ' ';
		delim[1] = 0;
		return (&delim[0]);
	}

	i = _wdchkind_(wc1);
	j = _wdchkind_(wc2);

	if ( (i==1 && j==1) || (i==3 && j==3) || (i==10 && j==10)
			|| (i==4 && j==4) || (i==7 && j==7) || (i==8 && j==8) )
		delim[0] = 0;
	else {
		delim[0] = ' ';
		delim[1] = 0;
	}

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
