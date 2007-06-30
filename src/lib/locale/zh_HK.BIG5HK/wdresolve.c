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
 * Copyright (c) 1991, Sun Microsystems, Inc.
 * Copyright (c) 1991, Nihon Sun Microsystems K.K.
 */

#ident	"@(#)wdresolve.c	1.6 94/01/31 SMI"


#include <stdlib.h>
#include <widec.h>
#include <wctype.h>
#include <zh_TW/xctype.h>
#include <locale.h>

static int wd_bind_strength[][10] = {
	2, 3, 3, 3, 3, 3, 3, 3, 3, -1,
	3, 6, 2, 2, 6, 3, 2, 2, 5, -1,
	1, 3, 3, 3, 5, 2, 2, 2, 2, -1,
	1, 2, 3, 6, 2, 2, 2, 2, 2, -1,
	5, 5, 5, 6, 5, 5, 3, 2, 2, -1,
	2, 2, 2, 2, 2, 6, 2, 3, 2, -1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, -1,
	3, 3, 3, 3, 3, 3, 3, 3, 3, -1,
	2, 5, 3, 3, 6, 2, 2, 2, 5, -1,
       -1,-1,-1,-1,-1,-1,-1,-1,-1, -1,
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

	/* Chinese ideograms */
	if ( ishantzu(wc) || ishradical(wc) )
                return (2);

	/* zhuyin symbols */
	if ( ishchuyin(wc) || ishphontone(wc) )
                return (3);

	if (ishparen(wc) || ishpunct(wc) )
		return(4);

	/* Greek alphabet */
	if ( ishgreek(wc) )
		return(5);

	if ( ishunit(wc) || ishline(wc) || ishsci(wc) || ishgen(wc) )
		return(6);
			
	if ( ishspace(wc) )
		return(7);

	if ( ishdigit(wc) || ishupper(wc) || ishlower(wc) )
		return(8);

	if (wc > 0L && wc < 0x7f)
		return (isalpha(wc) || isdigit(wc) || wc == '_');

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
