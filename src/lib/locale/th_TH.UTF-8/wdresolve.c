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
 * Copyright (c) 1991-1997, 2001 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#ident	"@(#)wdresolve.c 1.4	01/10/31 SMI"

#include <stdlib.h>
#include <locale.h>
#include <wctype.h>

static int	initialized = 0;
static int	conservative_edit = 0;
static char	*multicolumn_filler;

static void
_init(void)
{
    int i, n;
    char *itol, *ieol, *punct;
    char env_name[64];
    
    /* differenciate text editor from text formatter (user's option) */
    if (getenv("MB_CONSERVATIVE_EDIT"))
	conservative_edit = 1;
    
    /* get filler character for multicolumn character at right margin */
    multicolumn_filler = getenv("MC_FILLER");
    
    initialized = 1;
}

/*
 * The _wdchkind_ routine at below needs to be more elaborately classify wc.
 */
int
_wdchkind_(wchar_t wc)
{
    if (!initialized)
	_init();
    
    if (iswalpha(wc) || iswdigit(wc) || wc == L'_')
	return(0);
    
    if (iswpunct(wc))
	return(1);
    
    /* Hangul */
    if ((wc >= 0x0000ac00 && wc <= 0x0000d7a3) ||
	(wc >= 0x00003131 && wc <= 0x0000318e) ||
	(wc >= 0x00001100 && wc <= 0x00001159) ||
	(wc >= 0x0000115f && wc <= 0x000011a2) ||
	(wc >= 0x000011ab && wc <= 0x000011f9) ||
	(wc >= 0x0000ffa0 && wc <= 0x0000ffdc))
	return(2);
    
    /* Han */
    if ((wc >= 0x00002e80 && wc <= 0x00002e99) ||  /* CJK Radicals Supplement */
	(wc >= 0x00002e9b && wc <= 0x00002ef3) ||  /* CJK Radicals Supplement */
	(wc >= 0x00002f00 && wc <= 0x00002fd5) ||  /* Kangxi Radicals */
	(wc >= 0x00003105 && wc <= 0x0000312c) ||  /* Bopomofo */
    	(wc >= 0x00003400 && wc <= 0x00004db5) ||  /* CJK Ideograph Ext A */
    	(wc >= 0x00004e00 && wc <= 0x00009fa5) ||  /* CJK Unified Han */
    	(wc >= 0x0000f900 && wc <= 0x0000fa2d) ||  /* CJK Compat. Ideograph */
    	(wc >= 0x00020000 && wc <= 0x0002a6d6) ||  /* CJK Ideograph Ext B */
    	(wc >= 0x0002f800 && wc <= 0x0002fa1d))    /* CJK Compat. Ideograph */
	return(3);
    
    /* All other characters */
    return (4);
}

int
_wdbindf_(wchar_t wc1, wchar_t wc2, int type)
{
    wchar_t *special;
    int hit;
    
    if (!initialized)
	_init();
    if (conservative_edit && type == 2)
	return (6);
    
    /* Since Hangul is blank-delimited text language
       return type 6 always */
    
    return (6);
}

wchar_t *
_wddelim_(wchar_t wc1, wchar_t wc2, int type)
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
