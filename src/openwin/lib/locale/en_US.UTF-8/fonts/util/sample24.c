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
#pragma ident "@(#)sample24.c	1.13 01/08/15 SMI"


#include <stdio.h>
#include <locale.h>
#include <wchar.h>

const char *header = "STARTFONT 2.1\n\
COMMENT Copyright (c) 1998-2001 by Sun Microsystems, Inc.\n\
COMMENT All rights reserved.\n\
COMMENT\n\
COMMENT \"\045\045W\045\045 \045\045E\045\045 SMI\"\n\
COMMENT \n\
FONT -sun-supplement-medium-r-normal--24-240-75-75-p-240-unicode-fontspecific\n\
SIZE 24 72 72\n\
FONTBOUNDINGBOX 24 24 0 -4\n\
STARTPROPERTIES 19\n\
FONTNAME_REGISTRY \"\"\n\
FOUNDRY \"sun\"\n\
FAMILY_NAME \"supplement\"\n\
WEIGHT_NAME \"medium\"\n\
SLANT \"R\"\n\
SETWIDTH_NAME \"normal\"\n\
ADD_STYLE_NAME \"\"\n\
PIXEL_SIZE 24\n\
POINT_SIZE 240\n\
RESOLUTION_X 75\n\
RESOLUTION_Y 75\n\
SPACING \"P\"\n\
AVERAGE_WIDTH 240\n\
CHARSET_REGISTRY \"unicode\"\n\
CHARSET_ENCODING \"fontspecific\"\n\
DEFAULT_CHAR 0\n\
FONT_DESCENT 4\n\
FONT_ASCENT 22\n\
COPYRIGHT \"Copyright (c) 1998-2001 by Sun Microsystems, Inc.\"\n\
ENDPROPERTIES\n\
CHARS 24\n";

const char *notdef = "STARTCHAR notdefined\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
924000\n\
249000\n\
492000\n\
924000\n\
249000\n\
492000\n\
924000\n\
249000\n\
492000\n\
924000\n\
249000\n\
492000\n\
924000\n\
249000\n\
492000\n\
924000\n\
249000\n\
492000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *replacement = "STARTCHAR replcmnt_fffd\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
060000\n\
0f0000\n\
1f8000\n\
1f8000\n\
30c000\n\
666000\n\
6f6000\n\
ff7000\n\
fe7000\n\
fcf000\n\
79e000\n\
79e000\n\
3fc000\n\
198000\n\
198000\n\
0f0000\n\
060000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *noglyph_half = "STARTCHAR no_half\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
fff000\n\
c03000\n\
a05000\n\
a09000\n\
911000\n\
911000\n\
8a1000\n\
841000\n\
841000\n\
8a1000\n\
911000\n\
911000\n\
a09000\n\
a05000\n\
c05000\n\
c03000\n\
fff000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *noglyph_full = "STARTCHAR no_full\n\
ENCODING %-d\n\
SWIDTH 1000 0\n\
DWIDTH 24 0\n\
BBX 24 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
3ffffc\n\
30000c\n\
2c0034\n\
220044\n\
210084\n\
20c304\n\
202404\n\
201804\n\
201804\n\
202404\n\
20c304\n\
210084\n\
220044\n\
240024\n\
280014\n\
30000c\n\
3ffffc\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *euro = "STARTCHAR euro_20ac\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
000000\n\
078000\n\
1c6000\n\
302000\n\
201000\n\
600000\n\
ffc000\n\
ff8000\n\
400000\n\
400000\n\
ff8000\n\
ff0000\n\
400000\n\
601000\n\
303000\n\
1fe000\n\
0fc000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *Yumlaut = "STARTCHAR Yumlaut_0178\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
198000\n\
198000\n\
000000\n\
e03000\n\
606000\n\
30c000\n\
108000\n\
198000\n\
090000\n\
0f0000\n\
060000\n\
060000\n\
060000\n\
060000\n\
060000\n\
060000\n\
060000\n\
060000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *oe = "STARTCHAR oe_0153\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
31c000\n\
5b6000\n\
ce1000\n\
8c1000\n\
841000\n\
841000\n\
87f000\n\
840000\n\
840000\n\
840000\n\
c61000\n\
4f3000\n\
71e000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *OE = "STARTCHAR OE_0152\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
3bf000\n\
7e0000\n\
c60000\n\
860000\n\
860000\n\
860000\n\
860000\n\
87e000\n\
860000\n\
860000\n\
860000\n\
860000\n\
860000\n\
860000\n\
c60000\n\
6e0000\n\
3bf000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

/* The following 16 glyphs are for Latin-3/Esperanto support. */
const char *Ccircumflex = "STARTCHAR Ccircumflex_0108\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
070000\n\
0d8000\n\
18c000\n\
000000\n\
07e000\n\
182000\n\
300000\n\
300000\n\
600000\n\
600000\n\
600000\n\
600000\n\
600000\n\
700000\n\
300000\n\
3c2000\n\
1fe000\n\
07c000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *ccircumflex = "STARTCHAR ccircumflex_0109\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
038000\n\
06c000\n\
0c6000\n\
000000\n\
07e000\n\
1c2000\n\
180000\n\
300000\n\
300000\n\
300000\n\
300000\n\
300000\n\
180000\n\
1c2000\n\
07e000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *Cdotabove = "STARTCHAR Cdotabove_010a\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
030000\n\
030000\n\
000000\n\
000000\n\
07e000\n\
182000\n\
300000\n\
300000\n\
600000\n\
600000\n\
600000\n\
600000\n\
600000\n\
700000\n\
300000\n\
3c2000\n\
1fe000\n\
07c000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *cdotabove = "STARTCHAR cdotabove_010b\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
030000\n\
030000\n\
000000\n\
000000\n\
07e000\n\
1c2000\n\
180000\n\
300000\n\
300000\n\
300000\n\
300000\n\
300000\n\
180000\n\
1c2000\n\
07e000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *Gcircumflex = "STARTCHAR Gcircumflex_011c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
070000\n\
0d8000\n\
18c000\n\
000000\n\
07e000\n\
182000\n\
300000\n\
300000\n\
600000\n\
600000\n\
600000\n\
606000\n\
606000\n\
706000\n\
306000\n\
3c6000\n\
1fe000\n\
07c000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *gcircumflex = "STARTCHAR gcircumflex_011d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
0e0000\n\
1b0000\n\
318000\n\
000000\n\
1ec000\n\
31c000\n\
20c000\n\
60c000\n\
60c000\n\
60c000\n\
60c000\n\
60c000\n\
71c000\n\
3fc000\n\
1cc000\n\
00c000\n\
008000\n\
218000\n\
3f0000\n\
ENDCHAR\n";

const char *Gdotabove = "STARTCHAR Gdotabove_0120\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
030000\n\
030000\n\
000000\n\
000000\n\
07e000\n\
182000\n\
300000\n\
300000\n\
600000\n\
600000\n\
600000\n\
606000\n\
606000\n\
706000\n\
306000\n\
3c6000\n\
1fe000\n\
07c000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *gdotabove = "STARTCHAR gdotabove_0121\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
0c0000\n\
0c0000\n\
000000\n\
000000\n\
1ec000\n\
31c000\n\
20c000\n\
60c000\n\
60c000\n\
60c000\n\
60c000\n\
60c000\n\
71c000\n\
3fc000\n\
1cc000\n\
00c000\n\
008000\n\
218000\n\
3f0000\n\
ENDCHAR\n";

const char *Hcircumflex = "STARTCHAR Hcircumflex_0124\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
070000\n\
0d8000\n\
18c000\n\
000000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
3fe000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *hcircumflex = "STARTCHAR hcircumflex_0125\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
070000\n\
0d8000\n\
18c000\n\
000000\n\
300000\n\
300000\n\
300000\n\
33c000\n\
37e000\n\
3c6000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *Jcircumflex = "STARTCHAR Jcircumflex_0134\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
070000\n\
0d8000\n\
18c000\n\
000000\n\
1f8000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
230000\n\
3e0000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *jcircumflex = "STARTCHAR jcircumflex_0135\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
038000\n\
06c000\n\
0c6000\n\
000000\n\
1f8000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
018000\n\
230000\n\
3e0000\n\
ENDCHAR\n";

const char *Scircumflex = "STARTCHAR Scircumflex_015c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
070000\n\
0d8000\n\
18c000\n\
000000\n\
0fc000\n\
184000\n\
300000\n\
300000\n\
380000\n\
1c0000\n\
0f0000\n\
03c000\n\
00e000\n\
006000\n\
006000\n\
006000\n\
30c000\n\
3f8000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *scircumflex = "STARTCHAR scircumflex_015d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
070000\n\
0d8000\n\
18c000\n\
000000\n\
0fc000\n\
304000\n\
300000\n\
380000\n\
3e0000\n\
0fc000\n\
01e000\n\
006000\n\
006000\n\
20c000\n\
3f8000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *Ubreve = "STARTCHAR Ubreve_016c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
070000\n\
0d8000\n\
18c000\n\
000000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
306000\n\
18c000\n\
0f8000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *ubreve = "STARTCHAR ubreve_016d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 12 0\n\
BBX 12 24 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
0e0000\n\
1b0000\n\
318000\n\
000000\n\
60c000\n\
60c000\n\
60c000\n\
60c000\n\
60c000\n\
60c000\n\
60c000\n\
60c000\n\
63c000\n\
7ec000\n\
3cc000\n\
000000\n\
000000\n\
000000\n\
000000\n\
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
