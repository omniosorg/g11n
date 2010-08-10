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
 * Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.
 */

#ifdef	SCCS
static char	SccsId[] = "$SunId$ SMI";
#endif

#include <stdio.h>
#include <locale.h>
#include <widec.h>
#include <wctype.h>

/* Define TIS620.2533 character classes */
#define         CTRL    0
#define         NON     1
#define         CONS    2
#define         LV      3
#define         FV1     4
#define         FV2     5
#define         FV3     6
#define         BV1     7
#define         BV2     8
#define         BD      9
#define         TONE    10
#define         AD1     11
#define         AD2     12
#define         AD3     13
#define         AV1     14
#define         AV2     15
#define         AV3     16

/* Define type of display level of TIS620.2533 character */
#define         NONDISP 0       /* unprintable character */
#define         BASE    1       /* base character */
#define         ABOVE   2       /* above character */
#define         BELOW   3       /* below character */
#define         TOP     4       /* top character */

/* Thai punctuation code */
#define		PAIYANNOI	0xcf
#define		BAHT		0xdf
#define		MAIYAMOK	0xe6
#define		FONGMAN		0xef
#define		ANGKHANKHU	0xfa
#define		KHOMUT		0xfb

/* Thai numeric code */
#define		THAIZERO	0xf0
#define		THAININE	0xf9

#define	SPACEWCSTRING	L" ";
#define	NULLWCSTRING	L"";
#define	FILTERWCHAR	L'~';


/* Define character type */
static int __TISChrType[256] = {

        /* ASCII */
        CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,
        CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,
        CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,
        CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,  CTRL,

        /* TIS 620-2533 */
        CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,
        CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,
        CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,
        CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,  CTRL,
         NON,  CONS,  CONS,  CONS,  CONS,  CONS,  CONS,  CONS,
        CONS,  CONS,  CONS,  CONS,  CONS,  CONS,  CONS,  CONS,
        CONS,  CONS,  CONS,  CONS,  CONS,  CONS,  CONS,  CONS,
        CONS,  CONS,  CONS,  CONS,  CONS,  CONS,  CONS,  CONS,
        CONS,  CONS,  CONS,  CONS,   FV3,  CONS,   FV3,  CONS,
        CONS,  CONS,  CONS,  CONS,  CONS,  CONS,  CONS,   NON,
         FV1,   AV2,   FV1,   FV1,   AV1,   AV3,   AV2,   AV3,
         BV1,   BV2,    BD,   NON,   NON,   NON,   NON,   NON,
          LV,    LV,    LV,    LV,    LV,   FV2,   NON,   AD2,
        TONE,  TONE,  TONE,  TONE,   AD1,   AD1,   AD3,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,   NON,
         NON,   NON,   NON,   NON,   NON,   NON,   NON,  CTRL

};

/* Define global variables */
static int wd_bind_strength[8][8] = {
	/* Cn */
/* Cn-1 */	-1, -1, -1, -1, -1, -1, -1, -1,
		-1,  7,  0,  0,  0, -1, -1, -1,
		-1,  0,  7,  0,  0, -1, -1, -1,
		-1,  0,  0,  7,  0, -1, -1, -1,
		-1,  0,  0,  0,  0, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1
};


static void _init(void)
{
}

int 
_wdchkind_(wc)
wchar_t wc;
{

    switch (wcsetno(wc))
    {
      case 0:
	return (isalpha(wc) || isdigit(wc) || wc == '_');

      case 1:
	switch (__TISChrType[(int) wc])
	{
	  case CONS:
	  case   LV:
	  case  FV1:
	  case  FV2:
	  case  FV3:
	  case  BV1:
	  case  BV2:
	  case   BD:
	  case TONE:
	  case  AD1:
	  case  AD2:
	  case  AD3:
	  case  AV1:
	  case  AV2:
	  case  AV3:
	    return 2;

	  case  NON:
	    if (wc >= THAIZERO && wc <= THAININE)
		return 3;

	    switch (wc)
	    {
	      case  PAIYANNOI:
	      case       BAHT:
	      case   MAIYAMOK:
	      case    FONGMAN:
	      case ANGKHANKHU:
	      case     KHOMUT:
		return 4;
	    }
	    break;

	  default:
	    return 5; /* NOT TIS620.2533 */
		      /* 0xDB, 0xA0, 0xDB-0xDE, 0xFC-0xFF, 0x80-0x9F */
	}
	break;

      case 2:
	return 6; /* Not reached */

      case 3:
	return 7; /* Not reached */
   }
   return 0; /* Not reached */
}

int
_wdbindf_(wc1, wc2, type)
wchar_t wc1, wc2;
int type;
{
    return (wd_bind_strength[_wdchkind_(wc1)][_wdchkind_(wc2)]);
}

wchar_t *_wddelim_(wc1, wc2, type)
wchar_t wc1, wc2;
int type;
{
    if (iswspace(wc1))
	return NULLWCSTRING;
    return SPACEWCSTRING;
}

wchar_t
_mcfilter_()
{
    return FILTERWCHAR;
}
