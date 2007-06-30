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
#pragma ident "@(#)sample12.c	1.16 01/08/15 SMI"

#include <stdio.h>
#include <locale.h>
#include <wchar.h>

const char *header = "STARTFONT 2.1\n\
COMMENT Copyright (c) 1998-2001 by Sun Microsystems, Inc.\n\
COMMENT All rights reserved.\n\
COMMENT\n\
COMMENT \"\045\045W\045\045 \045\045E\045\045 SMI\"\n\
COMMENT \n\
FONT -sun-supplement-medium-r-normal--12-120-75-75-p-120-unicode-fontspecific\n\
SIZE 12 72 72\n\
FONTBOUNDINGBOX 12 12 0 -2\n\
STARTPROPERTIES 19\n\
FONTNAME_REGISTRY \"\"\n\
FOUNDRY \"sun\"\n\
FAMILY_NAME \"supplement\"\n\
WEIGHT_NAME \"medium\"\n\
SLANT \"R\"\n\
SETWIDTH_NAME \"normal\"\n\
ADD_STYLE_NAME \"\"\n\
PIXEL_SIZE 12\n\
POINT_SIZE 120\n\
RESOLUTION_X 75\n\
RESOLUTION_Y 75\n\
SPACING \"P\"\n\
AVERAGE_WIDTH 120\n\
CHARSET_REGISTRY \"unicode\"\n\
CHARSET_ENCODING \"fontspecific\"\n\
DEFAULT_CHAR 0\n\
FONT_DESCENT 2\n\
FONT_ASCENT 12\n\
COPYRIGHT \"Copyright (c) 1998-2001 by Sun Microsystems, Inc.\"\n\
ENDPROPERTIES\n\
CHARS 24\n";

const char *notdef = "STARTCHAR notdefined\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
00\n\
4c\n\
90\n\
24\n\
48\n\
90\n\
24\n\
48\n\
90\n\
24\n\
00\n\
ENDCHAR\n";

const char *replacement = "STARTCHAR replcmnt_fffd\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
00\n\
10\n\
38\n\
44\n\
54\n\
f6\n\
6c\n\
7c\n\
28\n\
10\n\
00\n\
ENDCHAR\n";

const char *noglyph_half = "STARTCHAR no_half\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
00\n\
fc\n\
cc\n\
cc\n\
b4\n\
b4\n\
b4\n\
cc\n\
cc\n\
fc\n\
00\n\
ENDCHAR\n";

const char *noglyph_full = "STARTCHAR no_full\n\
ENCODING %-d\n\
SWIDTH 1000 0\n\
DWIDTH 12 0\n\
BBX 12 11 0 -2\n\
BITMAP\n\
0000\n\
7fe0\n\
6060\n\
50a0\n\
4920\n\
4620\n\
4920\n\
50a0\n\
6060\n\
7fe0\n\
0000\n\
ENDCHAR\n";

const char *euro = "STARTCHAR euro_20ac\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
00\n\
18\n\
24\n\
40\n\
f8\n\
40\n\
f0\n\
40\n\
44\n\
38\n\
00\n\
ENDCHAR\n";

const char *Yumlaut = "STARTCHAR Yumlaut_0178\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
00\n\
28\n\
28\n\
00\n\
44\n\
44\n\
38\n\
10\n\
10\n\
10\n\
00\n\
ENDCHAR\n";

const char *oe = "STARTCHAR oe_0153\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
00\n\
00\n\
00\n\
4c\n\
b2\n\
92\n\
9e\n\
90\n\
92\n\
6c\n\
00\n\
ENDCHAR\n";

const char *OE = "STARTCHAR OE_0152\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
00\n\
6e\n\
90\n\
90\n\
90\n\
9e\n\
90\n\
90\n\
90\n\
6e\n\
00\n\
ENDCHAR\n";

/* The following 16 glyphs are for Latin-3/Esperanto support. */
const char *Ccircumflex = "STARTCHAR Ccircumflex_0108\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -1\n\
BITMAP\n\
10\n\
28\n\
00\n\
38\n\
48\n\
80\n\
80\n\
80\n\
40\n\
38\n\
00\n\
ENDCHAR\n";

const char *ccircumflex = "STARTCHAR ccircumflex_0109\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
10\n\
28\n\
00\n\
38\n\
40\n\
40\n\
40\n\
40\n\
38\n\
00\n\
00\n\
ENDCHAR\n";

const char *Cdotabove = "STARTCHAR Cdotabove_010a\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -1\n\
BITMAP\n\
10\n\
10\n\
00\n\
38\n\
48\n\
80\n\
80\n\
80\n\
40\n\
38\n\
00\n\
ENDCHAR\n";

const char *cdotabove = "STARTCHAR cdotabove_010b\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
10\n\
10\n\
00\n\
38\n\
40\n\
40\n\
40\n\
40\n\
38\n\
00\n\
00\n\
ENDCHAR\n";

const char *Gcircumflex = "STARTCHAR Gcircumflex_011c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -1\n\
BITMAP\n\
10\n\
28\n\
00\n\
38\n\
48\n\
80\n\
80\n\
88\n\
48\n\
38\n\
00\n\
ENDCHAR\n";

const char *gcircumflex = "STARTCHAR gcircumflex_011d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
10\n\
28\n\
00\n\
38\n\
48\n\
48\n\
48\n\
58\n\
28\n\
08\n\
70\n\
ENDCHAR\n";

const char *Gdotabove = "STARTCHAR Gdotabove_0120\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -1\n\
BITMAP\n\
10\n\
10\n\
00\n\
38\n\
48\n\
80\n\
80\n\
88\n\
48\n\
38\n\
00\n\
ENDCHAR\n";

const char *gdotabove = "STARTCHAR gdotabove_0121\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
10\n\
10\n\
00\n\
38\n\
48\n\
48\n\
48\n\
58\n\
28\n\
08\n\
70\n\
ENDCHAR\n";

const char *Hcircumflex = "STARTCHAR Hcircumflex_0124\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -1\n\
BITMAP\n\
10\n\
28\n\
00\n\
48\n\
48\n\
48\n\
78\n\
48\n\
48\n\
48\n\
00\n\
ENDCHAR\n";

const char *hcircumflex = "STARTCHAR hcircumflex_0125\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -1\n\
BITMAP\n\
10\n\
28\n\
00\n\
40\n\
40\n\
50\n\
68\n\
48\n\
48\n\
48\n\
00\n\
ENDCHAR\n";

const char *Jcircumflex = "STARTCHAR Jcircumflex_0134\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
10\n\
28\n\
00\n\
78\n\
10\n\
10\n\
10\n\
10\n\
10\n\
10\n\
e0\n\
ENDCHAR\n";

const char *jcircumflex = "STARTCHAR jcircumflex_0135\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
00\n\
10\n\
28\n\
00\n\
70\n\
10\n\
10\n\
10\n\
10\n\
10\n\
e0\n\
ENDCHAR\n";

const char *Scircumflex = "STARTCHAR Scircumflex_015c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -1\n\
BITMAP\n\
10\n\
28\n\
00\n\
38\n\
40\n\
40\n\
30\n\
08\n\
08\n\
70\n\
00\n\
ENDCHAR\n";

const char *scircumflex = "STARTCHAR scircumflex_015d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
10\n\
28\n\
00\n\
38\n\
40\n\
60\n\
18\n\
08\n\
70\n\
00\n\
00\n\
ENDCHAR\n";

const char *Ubreve = "STARTCHAR Ubreve_016c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -1\n\
BITMAP\n\
48\n\
30\n\
00\n\
48\n\
48\n\
48\n\
48\n\
48\n\
48\n\
30\n\
00\n\
ENDCHAR\n";

const char *ubreve = "STARTCHAR ubreve_016d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 6 0\n\
BBX 6 11 0 -2\n\
BITMAP\n\
48\n\
30\n\
00\n\
48\n\
48\n\
48\n\
48\n\
58\n\
28\n\
00\n\
00\n\
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
