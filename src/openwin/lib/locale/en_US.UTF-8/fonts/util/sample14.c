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
 * Copyright (c) 1998-2001 by Sun Microsystems, Inc.
 * All rights reserved.
 */
#pragma ident "@(#)sample14.c	1.16 01/08/15 SMI"


#include <stdio.h>
#include <locale.h>
#include <wchar.h>

const char *header = "STARTFONT 2.1\n\
COMMENT Copyright (c) 1998-2001 by Sun Microsystems, Inc.\n\
COMMENT All rights reserved.\n\
COMMENT\n\
COMMENT \"\045\045W\045\045 \045\045E\045\045 SMI\"\n\
COMMENT \n\
FONT -sun-supplement-medium-r-normal--14-140-75-75-p-140-unicode-fontspecific\n\
SIZE 14 72 72\n\
FONTBOUNDINGBOX 14 14 0 -3\n\
STARTPROPERTIES 19\n\
FONTNAME_REGISTRY \"\"\n\
FOUNDRY \"sun\"\n\
FAMILY_NAME \"supplement\"\n\
WEIGHT_NAME \"medium\"\n\
SLANT \"R\"\n\
SETWIDTH_NAME \"normal\"\n\
ADD_STYLE_NAME \"\"\n\
PIXEL_SIZE 14\n\
POINT_SIZE 140\n\
RESOLUTION_X 75\n\
RESOLUTION_Y 75\n\
SPACING \"P\"\n\
AVERAGE_WIDTH 140\n\
CHARSET_REGISTRY \"unicode\"\n\
CHARSET_ENCODING \"fontspecific\"\n\
DEFAULT_CHAR 0\n\
FONT_DESCENT 3\n\
FONT_ASCENT 13\n\
COPYRIGHT \"Copyright (c) 1998-2001 by Sun Microsystems, Inc.\"\n\
ENDPROPERTIES\n\
CHARS 24\n";

const char *notdef = "STARTCHAR notdefined\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -3\n\
BITMAP\n\
0000\n\
4800\n\
9200\n\
2400\n\
4800\n\
9200\n\
2400\n\
4800\n\
9200\n\
2400\n\
4800\n\
0000\n\
0000\n\
ENDCHAR\n";

const char *replacement = "STARTCHAR replcmnt_fffd\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -3\n\
BITMAP\n\
0000\n\
1000\n\
3800\n\
4400\n\
5400\n\
f600\n\
ee00\n\
6c00\n\
7c00\n\
2800\n\
1000\n\
0000\n\
0000\n\
ENDCHAR\n";

const char *noglyph_half = "STARTCHAR no_half\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -3\n\
BITMAP\n\
0000\n\
fe00\n\
c600\n\
aa00\n\
aa00\n\
9200\n\
9200\n\
aa00\n\
aa00\n\
c600\n\
fe00\n\
0000\n\
0000\n\
ENDCHAR\n";

const char *noglyph_full = "STARTCHAR no_full\n\
ENCODING %-d\n\
SWIDTH 1000 0\n\
DWIDTH 14 0\n\
BBX 14 13 0 -3\n\
BITMAP\n\
0000\n\
7ff8\n\
7038\n\
4848\n\
4488\n\
4308\n\
4308\n\
4488\n\
5868\n\
6018\n\
7ff8\n\
0000\n\
0000\n\
ENDCHAR\n";

const char *euro = "STARTCHAR euro_20ac\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -3\n\
BITMAP\n\
0000\n\
0000\n\
1c00\n\
2200\n\
4000\n\
fc00\n\
4000\n\
f800\n\
4000\n\
2200\n\
1c00\n\
0000\n\
0000\n\
ENDCHAR\n";

const char *Yumlaut = "STARTCHAR Yumlaut_0178\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -3\n\
BITMAP\n\
2800\n\
2800\n\
0000\n\
4400\n\
4400\n\
2800\n\
2800\n\
1000\n\
1000\n\
1000\n\
1000\n\
0000\n\
0000\n\
ENDCHAR\n";

const char *oe = "STARTCHAR oe_0153\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -3\n\
BITMAP\n\
0000\n\
0000\n\
0000\n\
0000\n\
6c00\n\
9200\n\
9200\n\
9e00\n\
9000\n\
9200\n\
6c00\n\
0000\n\
0000\n\
ENDCHAR\n";

const char *OE = "STARTCHAR OE_0152\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -3\n\
BITMAP\n\
0000\n\
7e00\n\
9000\n\
9000\n\
9000\n\
9e00\n\
9000\n\
9000\n\
9000\n\
9000\n\
7e00\n\
0000\n\
0000\n\
ENDCHAR\n";

/* The following 16 glyphs are for Latin-3/Esperanto support. */
const char *Ccircumflex = "STARTCHAR Ccircumflex_0108\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -1\n\
BITMAP\n\
1800\n\
2400\n\
0000\n\
1c00\n\
2400\n\
4000\n\
4000\n\
4000\n\
4000\n\
4000\n\
2000\n\
1c00\n\
0000\n\
ENDCHAR\n";

const char *ccircumflex = "STARTCHAR ccircumflex_0109\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -2\n\
BITMAP\n\
0000\n\
1800\n\
2400\n\
0000\n\
1c00\n\
2000\n\
4000\n\
4000\n\
4000\n\
2000\n\
1c00\n\
0000\n\
0000\n\
ENDCHAR\n";

const char *Cdotabove = "STARTCHAR Cdotabove_010a\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -1\n\
BITMAP\n\
1000\n\
1000\n\
0000\n\
1c00\n\
2400\n\
4000\n\
4000\n\
4000\n\
4000\n\
4000\n\
2000\n\
1c00\n\
0000\n\
ENDCHAR\n";

const char *cdotabove = "STARTCHAR cdotabove_010b\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -2\n\
BITMAP\n\
0000\n\
1000\n\
1000\n\
0000\n\
1c00\n\
2000\n\
4000\n\
4000\n\
4000\n\
2000\n\
1c00\n\
0000\n\
0000\n\
ENDCHAR\n";

const char *Gcircumflex = "STARTCHAR Gcircumflex_011c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -1\n\
BITMAP\n\
1800\n\
2400\n\
0000\n\
1c00\n\
2400\n\
4000\n\
4000\n\
4000\n\
4400\n\
4400\n\
2400\n\
1c00\n\
0000\n\
ENDCHAR\n";

const char *gcircumflex = "STARTCHAR gcircumflex_011d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -2\n\
BITMAP\n\
0000\n\
1800\n\
2400\n\
0000\n\
3c00\n\
4400\n\
4400\n\
4400\n\
4c00\n\
3400\n\
0400\n\
0400\n\
3800\n\
ENDCHAR\n";

const char *Gdotabove = "STARTCHAR Gdotabove_0120\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -1\n\
BITMAP\n\
1000\n\
1000\n\
0000\n\
1c00\n\
2400\n\
4000\n\
4000\n\
4000\n\
4400\n\
4400\n\
2400\n\
1c00\n\
0000\n\
ENDCHAR\n";

const char *gdotabove = "STARTCHAR gdotabove_0121\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -2\n\
BITMAP\n\
0000\n\
1000\n\
1000\n\
0000\n\
3c00\n\
4400\n\
4400\n\
4400\n\
4c00\n\
3400\n\
0400\n\
0400\n\
3800\n\
ENDCHAR\n";

const char *Hcircumflex = "STARTCHAR Hcircumflex_0124\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -1\n\
BITMAP\n\
1800\n\
2400\n\
0000\n\
4400\n\
4400\n\
4400\n\
4400\n\
7c00\n\
4400\n\
4400\n\
4400\n\
4400\n\
0000\n\
ENDCHAR\n";

const char *hcircumflex = "STARTCHAR hcircumflex_0125\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -1\n\
BITMAP\n\
1800\n\
2400\n\
0000\n\
4000\n\
4000\n\
5800\n\
6400\n\
4400\n\
4400\n\
4400\n\
4400\n\
4400\n\
0000\n\
ENDCHAR\n";

const char *Jcircumflex = "STARTCHAR Jcircumflex_0134\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -1\n\
BITMAP\n\
1800\n\
2400\n\
0000\n\
7c00\n\
1000\n\
1000\n\
1000\n\
1000\n\
1000\n\
1000\n\
1000\n\
e000\n\
0000\n\
ENDCHAR\n";

const char *jcircumflex = "STARTCHAR jcircumflex_0135\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -2\n\
BITMAP\n\
0000\n\
1800\n\
2400\n\
0000\n\
7800\n\
0800\n\
0800\n\
0800\n\
0800\n\
0800\n\
0800\n\
0800\n\
7000\n\
ENDCHAR\n";

const char *Scircumflex = "STARTCHAR Scircumflex_015c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -1\n\
BITMAP\n\
1800\n\
2400\n\
0000\n\
3c00\n\
4000\n\
4000\n\
6000\n\
3800\n\
0c00\n\
0400\n\
0400\n\
7800\n\
0000\n\
ENDCHAR\n";

const char *scircumflex = "STARTCHAR scircumflex_015d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -2\n\
BITMAP\n\
0000\n\
1800\n\
2400\n\
0000\n\
3c00\n\
4000\n\
4000\n\
3800\n\
0400\n\
0400\n\
7800\n\
0000\n\
0000\n\
ENDCHAR\n";

const char *Ubreve = "STARTCHAR Ubreve_016c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -1\n\
BITMAP\n\
4400\n\
3800\n\
0000\n\
4400\n\
4400\n\
4400\n\
4400\n\
4400\n\
4400\n\
4400\n\
4400\n\
3800\n\
0000\n\
ENDCHAR\n";

const char *ubreve = "STARTCHAR ubreve_016d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 7 0\n\
BBX 7 13 0 -2\n\
BITMAP\n\
0000\n\
4400\n\
3800\n\
0000\n\
4400\n\
4400\n\
4400\n\
4400\n\
4400\n\
4c00\n\
3400\n\
0000\n\
0000\n\
ENDCHAR\n";
/* The above 16 glyphs are for Latin-3/Esperanto support. */

const char *footer = "ENDFONT\n";

main()
{
	printf(header);

	printf(notdef, 0);
	printf(noglyph_half, 1);
	printf(noglyph_full, 2);
	printf(euro, 3);
	printf(replacement, 4);
	printf(OE, 5);
	printf(oe, 6);
	printf(Yumlaut, 7);
	printf(Ccircumflex, 8);
	printf(ccircumflex, 9);
	printf(Cdotabove, 10);
	printf(cdotabove, 11);
	printf(Gcircumflex, 12);
	printf(gcircumflex, 13);
	printf(Gdotabove, 14);
	printf(gdotabove, 15);
	printf(Hcircumflex, 16);
	printf(hcircumflex, 17);
	printf(Jcircumflex, 18);
	printf(jcircumflex, 19);
	printf(Scircumflex, 20);
	printf(scircumflex, 21);
	printf(Ubreve, 22);
	printf(ubreve, 23);

	printf(footer);

	exit(0);
}
