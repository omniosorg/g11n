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
#pragma ident "@(#)sample20.c	1.13 01/08/15 SMI"


#include <stdio.h>
#include <locale.h>
#include <wchar.h>

const char *header = "STARTFONT 2.1\n\
COMMENT Copyright (c) 1998-2001 by Sun Microsystems, Inc.\n\
COMMENT All rights reserved.\n\
COMMENT\n\
COMMENT \"\045\045W\045\045 \045\045E\045\045 SMI\"\n\
COMMENT \n\
FONT -sun-supplement-medium-r-normal--20-200-75-75-p-200-unicode-fontspecific\n\
SIZE 20 72 72\n\
FONTBOUNDINGBOX 20 20 0 -4\n\
STARTPROPERTIES 19\n\
FONTNAME_REGISTRY \"\"\n\
FOUNDRY \"sun\"\n\
FAMILY_NAME \"supplement\"\n\
WEIGHT_NAME \"medium\"\n\
SLANT \"R\"\n\
SETWIDTH_NAME \"normal\"\n\
ADD_STYLE_NAME \"\"\n\
PIXEL_SIZE 20\n\
POINT_SIZE 200\n\
RESOLUTION_X 75\n\
RESOLUTION_Y 75\n\
SPACING \"P\"\n\
AVERAGE_WIDTH 200\n\
CHARSET_REGISTRY \"unicode\"\n\
CHARSET_ENCODING \"fontspecific\"\n\
DEFAULT_CHAR 0\n\
FONT_DESCENT 4\n\
FONT_ASCENT 20\n\
COPYRIGHT \"Copyright (c) 1998-2001 by Sun Microsystems, Inc.\"\n\
ENDPROPERTIES\n\
CHARS 24\n";

const char *notdef = "STARTCHAR notdefined\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
924000\n\
248000\n\
490000\n\
924000\n\
248000\n\
490000\n\
924000\n\
248000\n\
490000\n\
924000\n\
248000\n\
490000\n\
924000\n\
248000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *replacement = "STARTCHAR replcmnt_fffd\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
0c0000\n\
1e0000\n\
1e0000\n\
210000\n\
4d8000\n\
5e8000\n\
fec000\n\
fdc000\n\
738000\n\
738000\n\
3f0000\n\
120000\n\
1e0000\n\
0c0000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *noglyph_half = "STARTCHAR no_half\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
ffc000\n\
c0c000\n\
c14000\n\
a24000\n\
a24000\n\
944000\n\
884000\n\
884000\n\
944000\n\
a24000\n\
a24000\n\
c14000\n\
c0c000\n\
ffc000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *noglyph_full = "STARTCHAR no_full\n\
ENCODING %-d\n\
SWIDTH 1000 0\n\
DWIDTH 20 0\n\
BBX 20 20 0 -4\n\
BITMAP\n\
000000\n\
3fffc0\n\
3801c0\n\
280240\n\
260440\n\
210840\n\
209040\n\
206040\n\
206040\n\
209040\n\
210840\n\
220440\n\
240240\n\
280140\n\
3000c0\n\
3fffc0\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *euro = "STARTCHAR euro_20ac\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
1f0000\n\
318000\n\
204000\n\
400000\n\
400000\n\
ff8000\n\
400000\n\
400000\n\
ff0000\n\
400000\n\
604000\n\
318000\n\
1f0000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *Yumlaut = "STARTCHAR Yumlaut_0178\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
0a0000\n\
0a0000\n\
404000\n\
208000\n\
318000\n\
110000\n\
0a0000\n\
0a0000\n\
040000\n\
040000\n\
040000\n\
040000\n\
040000\n\
040000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *oe = "STARTCHAR oe_0153\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
000000\n\
778000\n\
dc8000\n\
884000\n\
884000\n\
8fc000\n\
880000\n\
880000\n\
880000\n\
dcc000\n\
738000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *OE = "STARTCHAR OE_0152\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
77c000\n\
dc0000\n\
8c0000\n\
8c0000\n\
8c0000\n\
8c0000\n\
8f8000\n\
8c0000\n\
8c0000\n\
8c0000\n\
8c0000\n\
dc0000\n\
77c000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

/* The following 16 glyphs are to support Latin-3/Esperanto. */
const char *Ccircumflex = "STARTCHAR Ccircumflex_0108\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
060000\n\
090000\n\
198000\n\
000000\n\
0f8000\n\
108000\n\
200000\n\
400000\n\
400000\n\
400000\n\
400000\n\
400000\n\
400000\n\
200000\n\
108000\n\
0f8000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *ccircumflex = "STARTCHAR ccircumflex_0109\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
060000\n\
090000\n\
198000\n\
000000\n\
0f8000\n\
108000\n\
200000\n\
200000\n\
200000\n\
200000\n\
200000\n\
108000\n\
0f8000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *Cdotabove = "STARTCHAR Cdotabove_010a\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
060000\n\
060000\n\
000000\n\
000000\n\
0f8000\n\
108000\n\
200000\n\
400000\n\
400000\n\
400000\n\
400000\n\
400000\n\
400000\n\
200000\n\
108000\n\
0f8000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *cdotabove = "STARTCHAR cdotabove_010b\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
060000\n\
060000\n\
000000\n\
000000\n\
0f8000\n\
108000\n\
200000\n\
200000\n\
200000\n\
200000\n\
200000\n\
108000\n\
0f8000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *Gcircumflex = "STARTCHAR Gcircumflex_011c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
060000\n\
090000\n\
198000\n\
000000\n\
0f8000\n\
108000\n\
200000\n\
400000\n\
400000\n\
400000\n\
400000\n\
408000\n\
408000\n\
208000\n\
108000\n\
0f8000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *gcircumflex = "STARTCHAR gcircumflex_011d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
0c0000\n\
120000\n\
330000\n\
000000\n\
1d0000\n\
230000\n\
410000\n\
410000\n\
410000\n\
410000\n\
410000\n\
230000\n\
3d0000\n\
010000\n\
220000\n\
3e0000\n\
000000\n\
ENDCHAR\n";

const char *Gdotabove = "STARTCHAR Gdotabove_0120\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
060000\n\
060000\n\
000000\n\
000000\n\
0f8000\n\
108000\n\
200000\n\
400000\n\
400000\n\
400000\n\
400000\n\
408000\n\
408000\n\
208000\n\
108000\n\
0f8000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *gdotabove = "STARTCHAR gdotabove_0121\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
0c0000\n\
0c0000\n\
000000\n\
000000\n\
1d0000\n\
230000\n\
410000\n\
410000\n\
410000\n\
410000\n\
410000\n\
230000\n\
3d0000\n\
010000\n\
220000\n\
3e0000\n\
000000\n\
ENDCHAR\n";

const char *Hcircumflex = "STARTCHAR Hcircumflex_0124\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
040000\n\
0a0000\n\
1b0000\n\
000000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
3f8000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *hcircumflex = "STARTCHAR hcircumflex_0125\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
040000\n\
0a0000\n\
1b0000\n\
000000\n\
200000\n\
200000\n\
200000\n\
270000\n\
388000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *Jcircumflex = "STARTCHAR Jcircumflex_0134\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
0c0000\n\
120000\n\
330000\n\
000000\n\
3e0000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
260000\n\
3c0000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *jcircumflex = "STARTCHAR jcircumflex_0135\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
060000\n\
090000\n\
198000\n\
000000\n\
3e0000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
020000\n\
220000\n\
3c0000\n\
ENDCHAR\n";

const char *Scircumflex = "STARTCHAR Scircumflex_015c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
0c0000\n\
120000\n\
330000\n\
000000\n\
1f0000\n\
210000\n\
200000\n\
200000\n\
100000\n\
0c0000\n\
030000\n\
018000\n\
008000\n\
008000\n\
210000\n\
3e0000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *scircumflex = "STARTCHAR scircumflex_015d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
0c0000\n\
120000\n\
330000\n\
000000\n\
1f0000\n\
200000\n\
200000\n\
300000\n\
0e0000\n\
030000\n\
010000\n\
010000\n\
3e0000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *Ubreve = "STARTCHAR Ubreve_016c\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
040000\n\
0a0000\n\
1b0000\n\
000000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
110000\n\
0f0000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";

const char *ubreve = "STARTCHAR ubreve_016d\n\
ENCODING %-d\n\
SWIDTH 500 0\n\
DWIDTH 10 0\n\
BBX 10 20 0 -4\n\
BITMAP\n\
000000\n\
000000\n\
000000\n\
040000\n\
0a0000\n\
1b0000\n\
000000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
208000\n\
238000\n\
1c8000\n\
000000\n\
000000\n\
000000\n\
000000\n\
ENDCHAR\n";
/* The above 16 glyphs are to support Latin-3/Esperanto. */

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
