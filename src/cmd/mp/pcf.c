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
 * Copyright (c) 1999 by Sun Microsystems, Inc.
 * All rights reserved.
 */
/*
 * Copyright (C) 1994 X Consortium
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNEC-
 * TION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the X Consortium shall not
 * be used in advertising or otherwise to promote the sale, use or other deal-
 * ings in this Software without prior written authorization from the X Consor-
 * tium.
 *
 * X Window System is a trademark of X Consortium, Inc.
 */
#pragma ident   "@(#)pcf.c	1.3 00/02/02 SMI"
#include <stdio.h>
#include <X11/Xmd.h>
#include <assert.h>
#include <errno.h>
#include "general_header.h"
#include "print_preprocess.h"
#include "pcf_private.h"
#include "pcf.h"
#define pcfGetINT8(file, format) (position++, FontFileGetc(file))
#define FileDes(f)  ((int) (f)->private)

extern print_info	*print_info_st;

static void dump_Fmetrics(pcffont_t *);
static code_int getcode(CompressedFile *);
static int BufFileClose(BufFilePtr,int);
static int get_font_property(FontPtr, char *, ulong_t *);
static BufFilePtr BufFileCreate ( char *, int (*)(), int (*)(),int (*)() );
static int BufCompressedClose(BufFilePtr, int);
static int BufCompressedSkip(BufFilePtr, int);
static int BufCompressedFill(BufFilePtr);
static BufFilePtr BufFilePushCompressed(BufFilePtr);
static FontFilePtr FontFileOpen(char *);
static void FontFileClose (FontFilePtr);
static int pcfGetLSB32(FontFilePtr);
static PCFTablePtr pcfReadTOC(FontFilePtr,int*);
static Bool pcfGetProperties(FontInfoPtr,FontFilePtr, PCFTablePtr, int);
static void pcfGetCompressedMetric(FontFilePtr,CARD32, xCharInfo*);
static Bool pcfSeekToType(FontFilePtr, PCFTablePtr, int, CARD32, CARD32*, CARD32*);
static int pcfGetINT16(FontFilePtr, CARD32);
static int pcfGetINT32(FontFilePtr, CARD32);
static Bool pcfGetAccel(FontInfoPtr, FontFilePtr, PCFTablePtr, int, CARD32);
static void pcfGetMetric(FontFilePtr, CARD32, xCharInfo *);
static int pcfReadFont(FontPtr, FontFilePtr, int, int, int, int);
static char *NameForAtom(Atom);
static BufFilePtr BufFileOpenRead(int);
static int BufFileRawFill(BufFilePtr);
static int BufFileRawSkip(BufFilePtr, int);
static int BufFileRawClose (BufFilePtr, int);
static int bitmapGetGlyphs(FontPtr, unsigned long, unsigned char *, FontEncoding , unsigned long * , CharInfoPtr *);
static int bitmapGetMetrics(FontPtr, unsigned long, unsigned char *, FontEncoding, unsigned long *, xCharInfo **);
static void pcfUnloadFont(FontPtr);
static int handle_cuferr(int , ucs4_t *, int *);
static int handle_illegalchar(ucs4_t *, int *);
static int handle_nonidentchar(ucs4_t *, int *);
static int handle_nobitmap(ucs4_t *, pcffont_t *, pcf_charmet_t *, pcf_bm_t **) ;
static int handle_nongraphchar(ucs4_t *, int *);
static pcf_bm_t * xpcf_getcbm(int , pcffont_t *, pcf_charmet_t *);
static void BitOrderInvert(unsigned char *, int);
static Bool pcfHasType ( PCFTablePtr, int, CARD32);
static void TwoByteSwap(unsigned char *, int);
static void FourByteSwap(unsigned char *, int);
static int RepadBitmap(char *, char *, unsigned int, unsigned int, int, int);
static Atom MakeAtom(char *, unsigned, int );
static Bool ResizeReverseMap ();
static NameEqual (char *, char *,int );
static Hash(char *, int);
static ResizeHashTable();
static void put_PSbitmap(ucs4_t , pcf_bm_t *, pcf_charmet_t *, pcf_SCcharmet_t *);
void init_putPS(void);
unsigned long * Xrealloc(unsigned long   *, int);
unsigned long * Xalloc(int);
int BufFileRead (BufFilePtr, char *, int);
void pcf_postscript(ucs4_t c, pcf_bm_t *, pcf_charmet_t *, pcf_SCcharmet_t *);
int pres_pcfbm(ucs4_t *, pcffont_t *, pcf_bm_t **, pcf_charmet_t *, pcf_SCcharmet_t *, int);

double SPACINGwidth = -1;
ucs4_t SPACINGchar = (ucs4_t) 0x20;
ucs4_t REFERENCEchar = (ucs4_t) 0x20;

extern pcffont_t *pcf_fonts;
extern pcffont_t	*CurrentFont;
static int position;
static CharInfoRec nonExistantChar;
static AtomListPtr  *hashTable;
static int          hashMask;
static int          hashSize, hashUsed;
static int          hashMask;
static int          rehash;

/*
static ucs4_t	pcf_bmap[4][UCS4_MAXVAL/sizeof(ucs4_t)];
static int dictcnt = 0;
*/

static unsigned char _reverse_byte[0x100] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

static void
put_PSbitmap(ucs4_t code, pcf_bm_t *bitmap, 
	pcf_charmet_t *Cmetrics, pcf_SCcharmet_t *Smetrics)
{
    int j;
    int k;

    if (bitmap == NULL)
	return;

    fprintf(stdout,"/C%x { GR %.2f %.2f S %d %d T [%d 0 0 -%d 0 %d]\n\t{<", 
    	    code,
    	    Smetrics->widthBits, Smetrics->height,
    	    Cmetrics->widthBits, Cmetrics->height,
    	    Cmetrics->widthBits, Cmetrics->height,
    	    Cmetrics->ascent);

    k = Cmetrics->widthBytes * Cmetrics->height;

#ifdef SDEBUG
	fprintf(stderr, "%d is -------Cmetrics->widthBytes for :C%x \n", Cmetrics->widthBytes, code);
	fprintf(stderr, "%d is -------Cmetrics->height for :C%x \n", Cmetrics->height, code);
	fprintf(stderr, "%d is ------ no. of glyph elems for:C%x \n", k, code);
#endif
    for (j = 0; j < k; j++)
	fprintf(stdout,"%.2x", bitmap[j]);

    fprintf(stdout,">} IG } def\n"); 
    fprintf(stdout,"C%x\n", code);

    /*
    dictcnt++;
    */
}

pcf_SCcharmet_t *
get_SCmetrics(ucs4_t val)
{
	int ndx;
	ucs4_t v;
	pcf_bm_t *bm;
	pcf_charmet_t cm;
	static pcf_SCcharmet_t sm;

	/* map ucs4 val to a 'pcf_fonts' array index */

	v = val;

	if ((ndx = presform_fontndx(val)) == XU_UCS4UNDEF) {
		return NULL;
	} else if ((pcf_ret <= -1 ) || ( pres_pcfbm(&v, &pcf_fonts[ndx], &bm, &cm, &sm, 1) < 0)) {
		return NULL;
	} else if (v != val) {	/* return NULL if val got mapped to */
				/* another character */
		return NULL; 
	} else {
		return &sm;
	}
}

void
init_putPS(void)
{
    pcf_SCcharmet_t *sm;

    if ((sm = get_SCmetrics(REFERENCEchar)) \
		|| (sm = get_SCmetrics(SPACINGchar))) {
		SPACINGwidth = sm->origin_xoff;
	} else SPACINGwidth = (float)SPACE_WIDTH_PER_PTSZ * current_pt_sz;
}
void
pcf_postscript(ucs4_t c, pcf_bm_t *pcfbm, 
    	pcf_charmet_t *Cmetrics, pcf_SCcharmet_t *scCmetrics)
{


    put_PSbitmap(c, pcfbm, Cmetrics, scCmetrics);

}
int
load_pcf_font(pcffont_t *font)
{
    FontRec fr;
    FontInfoPtr fi;
    FontFilePtr ff;
    ulong_t value;

    if ((ff = FontFileOpen(font->file)) == NULL)
        return -1;


    if (pcfReadFont(&fr, ff, MSBFirst, MSBFirst, 1, 1) != Successful)
    	return -1;
    fi = &(fr.info);

    font->Fmetrics.ascent = fi->fontAscent;
    font->Fmetrics.descent = fi->fontDescent;
    font->Fmetrics.linespace = fi->fontAscent + fi->fontDescent;
    font->Fmetrics.firstchar = (fi->firstRow * 256) + fi->firstCol;
    font->Fmetrics.lastchar = (fi->lastRow * 256) + fi->lastCol;
    font->Fmetrics.lastCol = fi->lastCol;
    font->Fmetrics.firstCol = fi->firstCol;
    
    font->bitmaps = ((BitmapFontPtr)(fr.fontPrivate))->encoding;

    if (get_font_property(&fr, "POINT_SIZE", &value) == -1)
    	return -1;
    font->Fmetrics.ptsz = (int) value;

    if (get_font_property(&fr, "RESOLUTION_X", &value) == -1)
    	return -1;
    font->Fmetrics.Xres = (int) value;

    if (get_font_property(&fr, "RESOLUTION_Y", &value) == -1)
    	return -1;
    font->Fmetrics.Yres = (int) value;

#ifdef SDEBUG
    fprintf(stderr,"Fmetrics: ascent:%d, descent:%d, linespace:%d, "
	"ptsz:%d, Xres:%d, Yres:%d\n", font->Fmetrics.ascent, font->Fmetrics.descent,
	(&(font->Fmetrics))->linespace, (&(font->Fmetrics))->ptsz, (&(font->Fmetrics))->Xres, (&(font->Fmetrics))->Yres);
#endif

    debug(dump_Fmetrics(&(font->Fmetrics)));

    FontFileClose(ff);
    return 0;
}

void
scaling_factors(pcffont_t *font, double ptsz, int Xres, int Yres)
{
    font->Yscale = ((ptsz * (double)Yres) 
	/ ((double)font->Fmetrics.ptsz * (double)font->Fmetrics.Xres));

    font->Xscale = ((((double)Xres * (double)font->Fmetrics.Yres)
	/ ((double)Yres * (double)font->Fmetrics.Xres))
	    * (font->Yscale));
#if 0
    if (xsflag == OPTARG_SET)
    	font->Xscale = xs_argval;
    else if (xsflag == OPTARG_ADD)
    	font->Xscale += xs_argval;
    else if (xsflag == OPTARG_SUB) 	/* for clarity, we subract a positive */
    	font->Xscale -= xs_argval; 	/* rather than add a negative. */

    if (ysflag == OPTARG_SET)
    	font->Yscale = ys_argval;
    else if (ysflag == OPTARG_ADD)
    	font->Yscale += ys_argval;
    else if (ysflag == OPTARG_SUB)
    	font->Yscale -= ys_argval;
#endif
#ifdef SDEBUG
	fprintf(stderr,"%f -- font->Yscale\n %f -- font->Xscale\n", font->Yscale, font->Xscale);
#endif
}

void
scale_Fmetrics(pcffont_t *font) {
	font->scFmetrics.ascent = font->Fmetrics.ascent * font->Yscale;
	font->scFmetrics.descent = font->Fmetrics.descent * font->Yscale;
	font->scFmetrics.linespace = font->Fmetrics.linespace * font->Yscale;

#ifdef SDEBUG
fprintf(stderr,"%f font->scFmetrics.ascent\n \
%f font->scFmetrics.descent\n\
%f font->scFmetrics.linespace\n", font->scFmetrics.ascent, font->scFmetrics.descent, font->scFmetrics.linespace);
#endif

}

void
scale_Cmetrics(pcf_charmet_t *Cm, pcf_SCcharmet_t *Sm)
{
	Sm->width = Cm->width * CurrentFont->Xscale;
	Sm->height = Cm->height * CurrentFont->Yscale;
	Sm->widthBits = Cm->widthBits * CurrentFont->Xscale;
	Sm->ascent = Cm->ascent * CurrentFont->Yscale;
	Sm->descent = Cm->descent * CurrentFont->Yscale;
	Sm->origin_xoff = Cm->origin_xoff * CurrentFont->Xscale;

#ifdef SDEBUG
	fprintf(stderr, "%f is CurrentFont->Xscale\n", CurrentFont->Xscale);
	fprintf(stderr, "%f is CurrentFont->Yscale\n", CurrentFont->Yscale);
	fprintf(stderr, "%d is Cm->widthBits\n", Cm->widthBits);
	fprintf(stderr, "%d is Cm->height\n", Cm->height);
	fprintf(stderr, "%f is Sm->widthBits\n", Sm->widthBits);
	fprintf(stderr, "%f is Sm->height\n", Sm->height);
#endif

	/*
	debug(dump_scCmetrics(Sm));
	*/
}
int
pres_pcfbm(ucs4_t *val, pcffont_t *font, pcf_bm_t **bitmap,
    	pcf_charmet_t *Cmetrics, pcf_SCcharmet_t *scCmetrics, int dontcache)
{
	int fv;
	ucs4_t v;
#if SDEBUG
	fprintf(stderr, "0x%08x -- ucs4val \n", *val);
#endif

	v = *val;
	if (is_motion_char(v)) {
		*bitmap = NULL;
		return XU_MOTION_CHAR;
	}

	if (non_graphic_char(v)) {
		int ndx;
		if ((fv = handle_nongraphchar(&v, &ndx)) == XU_IGNORE) {
			return XU_IGNORE;
		} else {
			font = &pcf_fonts[ndx];
			CurrentFont = font;
		}
	}

	if ((fv = font->cuf(v)) < 0) {
		int ndx;
		if ((fv = handle_cuferr(fv, &v, &ndx)) == XU_IGNORE) {
			return XU_IGNORE;
		} else {
			font = &pcf_fonts[ndx];
			CurrentFont = font;
		}
	}

	if ((*bitmap = (pcf_bm_t *)xpcf_getcbm(fv, font, Cmetrics)) == NULL) {
		if (handle_nobitmap(&v, font, Cmetrics, bitmap) == XU_IGNORE)
			return XU_IGNORE;
	}
#if SDEBUG
	fprintf(stderr, "0x%02x -- **bitmap \n", (unsigned char)**bitmap);
#endif
	*val = v;
	scale_Cmetrics(Cmetrics, scCmetrics);
	return(1);
}

#ifdef DEBUG

static void
dump_Cmetrics(pcf_charmet_t *cm)
{
    dprintf2("cmetrics: ascent:%d, descent:%d, ", cm->ascent, cm->descent);

    dprintf3("width:%d, height:%d, widthBytes:%d, ", 
    	cm->width, cm->height, cm->widthBytes);

    dprintf3("LSB:%d,RSB: %d, xoff:%d\n ", 
	cm->LSBearing, cm->RSBearing, cm->origin_xoff);
}
static void
dump_Fmetrics(pcffont_t *fm)
{
    dprintf6("Fmetrics: ascent:%d, descent:%d, linespace:%d, "
	"ptsz:%d, Xres:%d, Yres:%d\n", fm->ascent, fm->descent, 
    	fm->linespace, fm->ptsz, fm->Xres, fm->Yres);
}
#endif

unsigned long *
Xrealloc (n,m)
    unsigned long   *n;
{
    if (!n)
	return (unsigned long *) malloc (m);
    else
	return (unsigned long *) realloc ((char *) n, m);
}
unsigned long *
Xalloc (m)
{
    return (unsigned long *) malloc (m);
}

/*
 *	Invert byte order within each 16-bits of an array.
 */
void
TwoByteSwap(buf, nbytes)
    unsigned char *buf;
    int nbytes;
{
    register unsigned char c;

    for (; nbytes > 0; nbytes -= 2, buf += 2)
    {
	c = buf[0];
	buf[0] = buf[1];
	buf[1] = c;
    }
}
/*
 *	Invert byte order within each 32-bits of an array.
 */
static void
FourByteSwap(buf, nbytes)
    unsigned char *buf;
    int nbytes;
{
    register unsigned char c;

    for (; nbytes > 0; nbytes -= 4, buf += 4) 
    {
	c = buf[0];
	buf[0] = buf[3];
	buf[3] = c;
	c = buf[1];
	buf[1] = buf[2];
	buf[2] = c;
    }
}
static int
handle_nongraphchar(ucs4_t *val, int *ndx)
{
	return XU_IGNORE;
}

static Bool
pcfHasType (tables, ntables, type)
    PCFTablePtr tables;
    int         ntables;
    CARD32      type;
{
    int         i;

    for (i = 0; i < ntables; i++)
	if (tables[i].type == type)
	    return TRUE;
    return FALSE;
}
/*
 *	Invert bit order within each BYTE of an array.
 */
static void
BitOrderInvert(buf, nbytes)
    unsigned char *buf;
    int nbytes;
{
    register unsigned char *rev = _reverse_byte;

    for (; --nbytes >= 0; buf++)
	*buf = rev[*buf];
}

static pcf_bm_t *
xpcf_getcbm(int code, pcffont_t *font, pcf_charmet_t *Cmetrics)
{
    int j;
    CharInfoPtr ci;

    assert(font->loaded);
    /*
     * For the default no glyph character to appear in
     * output stream
     */
     onemoretime:
    if (code < font->Fmetrics.firstchar || code > font->Fmetrics.lastchar)
	return NULL;

    j = (code - (((code - font->Fmetrics.firstchar) / 256)
    	* (font->Fmetrics.firstCol + (255 - font->Fmetrics.lastCol))))
    	- font->Fmetrics.firstchar;
#ifdef SDEBUG
	fprintf(stderr, "%d is -- codewidth of C%x\n", wcwidth(code), code);
	fprintf(stderr, "%d is -- j value      C%x\n", wcwidth(code), code);
#endif
    assert(j >= 0);
    if (font->bitmaps[j] == NULL)
	{
			/*
			 * code added to replace the codepoints with
			 * no-glyph code
			 */

			if (wcwidth(code) == -1)
				{
				code = 0;
				goto onemoretime;
				}
        return NULL;
	}

    ci = font->bitmaps[j];
    Cmetrics->width = GLYPHWIDTHPIXELS(ci);
    Cmetrics->height = GLYPHHEIGHTPIXELS(ci);
    Cmetrics->widthBytes = GLYPHWIDTHBYTES(ci);
    Cmetrics->widthBits = GLYPHWIDTHBYTES(ci) * NBPB;
    Cmetrics->ascent = ci->metrics.ascent;
    Cmetrics->descent = ci->metrics.descent;
    Cmetrics->LSBearing = ci->metrics.leftSideBearing;
    Cmetrics->RSBearing = ci->metrics.rightSideBearing;
    Cmetrics->origin_xoff = ci->metrics.characterWidth;

#ifdef SDEBUG
	fprintf(stderr, "%d is -- Cmetrics->widthBytes of C%x\n", Cmetrics->widthBytes, code);
	fprintf(stderr, "%d is -- Cmetrics->height value  of C%x\n", Cmetrics->height, code);
#endif

#if 0
    if (cwflag) {
    	if (cwflag == OPTARG_ADD)
    	    Cmetrics->origin_xoff += cw_argval;
    	else if (cwflag == OPTARG_SUB)
    	    Cmetrics->origin_xoff -= cw_argval;
    	else if (cwflag == OPTARG_SET)
    	    Cmetrics->origin_xoff = cw_argval;
    }
#endif

    debug(dump_Cmetrics(Cmetrics));

    return (pcf_bm_t*) ci->bits;
}

/*
 *	Repad a bitmap
 */

static int
RepadBitmap (pSrc, pDst, srcPad, dstPad, width, height)
    char	*pSrc, *pDst;
    unsigned	srcPad, dstPad;
    int		width, height;
{
    int	    srcWidthBytes,dstWidthBytes;
    int	    row,col;
    char    *pTmpSrc,*pTmpDst;

    switch (srcPad) {
    case 1:	
	srcWidthBytes = (width+7)>>3;
	break;
    case 2:
	srcWidthBytes = ((width+15)>>4)<<1;
	break;
    case 4:	
	srcWidthBytes = ((width+31)>>5)<<2;
	break;
    case 8:	
	srcWidthBytes = ((width+63)>>6)<<3; 
	break;
    default:
	return 0;
    }
    switch (dstPad) {
    case 1:	
	dstWidthBytes = (width+7)>>3;
	break;
    case 2:
	dstWidthBytes = ((width+15)>>4)<<1;
	break;
    case 4:	
	dstWidthBytes = ((width+31)>>5)<<2;
	break;
    case 8:	
	dstWidthBytes = ((width+63)>>6)<<3; 
	break;
    default:
	return 0;
    }

    width = srcWidthBytes;
    if (width > dstWidthBytes)
	width = dstWidthBytes;
    pTmpSrc= pSrc;
    pTmpDst= pDst;
    for (row = 0; row < height; row++)
    {
	for (col = 0; col < width; col++)
	    *pTmpDst++ = *pTmpSrc++;
	while (col < dstWidthBytes)
 	{
	    *pTmpDst++ = '\0';
	    col++;
	}
	pTmpSrc += srcWidthBytes - width;
    }
    return dstWidthBytes * height;
}
static int
handle_cuferr(int err, ucs4_t *v, int *ndx)
{

	if (err == CUF_ILCH) {
		return handle_illegalchar(v, ndx);
	} else if (err == CUF_NICH) {
		return handle_nonidentchar(v, ndx);
	} else {
		err_exit( catgets(cat_fd, ERR_SET, 35, "%s: internal error: "
			"unable to get pcf glyph for 0x%08x\n"
			"file name:%s , line number:%d\n"), progname,
			*v, __FILE__, __LINE__ );
	}
}

static int
handle_illegalchar(ucs4_t *val, int *ndx)
{
	return XU_IGNORE;
}

static int
handle_nonidentchar(ucs4_t *val, int *ndx)
{
	return XU_IGNORE;
}

static int
handle_nobitmap(ucs4_t *val, pcffont_t *font, pcf_charmet_t *Cm, pcf_bm_t **bitmap) {
	int fv;
	/* Becasue NOBITMAPREPL is hard coded for FALLBACK_FONT */
	if(strstr(font->file, FALLBACK_FONT)==NULL)
		return XU_IGNORE;
	*val = NOBITMAPREPL;	
	if ((fv = font->cuf(*val)) < 0) return XU_IGNORE;
	if ((*bitmap = xpcf_getcbm(fv, font, Cm)) == NULL) return XU_IGNORE;
	return(1);
}


static int
BufFileRawFill (f)
    BufFilePtr	f;
{
    int	left;

    left = read (FileDes(f), f->buffer, BUFFILESIZE);
    if (left <= 0) {
	f->left = 0;
	return BUFFILEEOF;
    }
    f->left = left - 1;
    f->bufp = f->buffer + 1;
    return f->buffer[0];
}

static int
BufFileRawSkip (f, count)
    BufFilePtr	f;
    int		count;
{
    int	    curoff;
    int	    fileoff;
    int	    todo;

    curoff = f->bufp - f->buffer;
    fileoff = curoff + f->left;
    if (curoff + count <= fileoff) {
	f->bufp += count;
	f->left -= count;
    } else {
	todo = count - (fileoff - curoff);
	if (lseek (FileDes(f), todo, 1) == -1) {
	    if (errno != ESPIPE)
		return BUFFILEEOF;
	    while (todo) {
		curoff = BUFFILESIZE;
		if (curoff > todo)
		    curoff = todo;
		fileoff = read (FileDes(f), f->buffer, curoff);
		if (fileoff <= 0)
		    return BUFFILEEOF;
		todo -= fileoff;
	    }
	}
	f->left = 0;
    }
    return count;
}

static int
BufFileRawClose (f, doClose)
    BufFilePtr	f;
{
    if (doClose)
	close (FileDes (f));
    return 1;
}

static
NameEqual (a, b, l)
    char    *a, *b;
{
    while (l--)
	if (*a++ != *b++)
	    return FALSE;
    return TRUE;
}
static BufFilePtr
BufFileOpenRead(fd)
	 int fd;
{
return BufFileCreate ((char *) fd, BufFileRawFill, BufFileRawSkip, BufFileRawClose);
}

static AtomListPtr  *reverseMap;
static int          reverseMapSize;
static Atom         lastAtom;

static Bool ResizeReverseMap ()
{
    if (reverseMapSize == 0)
	reverseMapSize = 1000;
    else
	reverseMapSize *= 2;
    reverseMap = (AtomListPtr *) Xrealloc ((ulong_t *)reverseMap, reverseMapSize * sizeof (AtomListPtr));
    if (!reverseMap)
	return FALSE;
    else 
	return TRUE;
}

static char *
NameForAtom(atom)
    Atom atom;
{
    if (atom != None && atom <= lastAtom)
	return reverseMap[atom]->name;
    return 0;
}
/*****************************************************************
 * TAG( getcode )
 *
 * Read one code from the standard input.  If BUFFILEEOF, return -1.
 * Inputs:
 * 	stdin
 * Outputs:
 * 	code or -1 is returned.
 */

static char_type rmask[9] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

static code_int
getcode(file)
    CompressedFile  *file;
{
    register code_int code;
    register int r_off, bits;
    register char_type *bp = file->buf;
    register BufFilePtr	raw;

    if ( file->clear_flg > 0 || file->offset >= file->size ||
	file->free_ent > file->maxcode )
    {
	/*
	 * If the next entry will be too big for the current code
	 * size, then we must increase the size.  This implies reading
	 * a new buffer full, too.
	 */
	if ( file->free_ent > file->maxcode ) {
	    file->n_bits++;
	    if ( file->n_bits == file->maxbits )
		file->maxcode = file->maxmaxcode;	/* won't get any bigger now */
	    else
		file->maxcode = MAXCODE(file->n_bits);
	}
	if ( file->clear_flg > 0) {
    	    file->maxcode = MAXCODE (file->n_bits = INIT_BITS);
	    file->clear_flg = 0;
	}
	bits = file->n_bits;
	raw = file->file;
	while (bits > 0 && (code = BufFileGet (raw)) != BUFFILEEOF)
	{
	    *bp++ = code;
	    --bits;
	}
	bp = file->buf;
	if (bits == file->n_bits)
	    return -1;			/* end of file */
	file->size = file->n_bits - bits;
	file->offset = 0;
	/* Round size down to integral number of codes */
	file->size = (file->size << 3) - (file->n_bits - 1);
    }
    r_off = file->offset;
    bits = file->n_bits;
    /*
     * Get to the first byte.
     */
    bp += (r_off >> 3);
    r_off &= 7;
    /* Get first part (low order bits) */
#ifdef NO_UCHAR
    code = ((*bp++ >> r_off) & rmask[8 - r_off]) & 0xff;
#else
    code = (*bp++ >> r_off);
#endif /* NO_UCHAR */
    bits -= (8 - r_off);
    r_off = 8 - r_off;		/* now, offset into code word */
    /* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
    if ( bits >= 8 ) {
#ifdef NO_UCHAR
	code |= (*bp++ & 0xff) << r_off;
#else
	code |= *bp++ << r_off;
#endif /* NO_UCHAR */
	r_off += 8;
	bits -= 8;
    }
    /* high order bits. */
    if (rmask[bits])	
    	code |= (*bp & rmask[bits]) << r_off;
    file->offset += file->n_bits;

    return code;
}
int
BufFileClose (f, doClose)
    BufFilePtr  f;
{
	(void) (*f->close) (f, doClose);
	xfree (f);
}


static int
get_font_property(FontPtr Fp, char *name, ulong_t *value)
{
    int j;
    int n;
    char *prop_name;

    n = Fp->info.nprops;

    for (j = 0; j < n; j++) {
    	if (!(prop_name = NameForAtom(Fp->info.props[j].name)))
    	    continue;
    	if (eq(name, prop_name)) {
	    *value = (ulong_t) Fp->info.props[j].value;
    	    return 0;
    	}
    }

    return -1;
}


static BufFilePtr
BufFileCreate (private, io, skip, close)
    char    *private;
    int	    (*io)();
    int	    (*skip)();
    int	    (*close)();
{
    BufFilePtr	f;

    f = (BufFilePtr) xalloc (sizeof *f);
    if (!f)
	return 0;
    f->private = private;
    f->bufp = f->buffer;
    f->left = 0;
    f->io = io;
    f->skip = skip;
    f->close = close;
    return f;
}

int hsize_table[] = {
	5003,       /* 12 bits - 80% occupancy */
	9001,       /* 13 bits - 91% occupancy */
	18013,      /* 14 bits - 91% occupancy */
	35023,      /* 15 bits - 94% occupancy */
	69001       /* 16 bits - 95% occupancy */
};

static int  BufCompressedFill(), BufCompressedSkip(), BufCompressedClose();

static int
BufCompressedClose (f, doClose)
    BufFilePtr	f;
{
    CompressedFile  *file;
    BufFilePtr	    raw;

    file = (CompressedFile *) f->private;
    raw = file->file;
    xfree (file);
    BufFileClose (raw, doClose);
    return 1;
}

static int
BufCompressedSkip (f, bytes)
    BufFilePtr	f;
    int		bytes;
{
    int		    c;
    while (bytes--) { 
	if (BufFileGet(f) == BUFFILEEOF)
		return BUFFILEEOF;
    }
    return 0;
}

static int
BufCompressedFill (f)
    BufFilePtr	    f;
{
    CompressedFile  *file;
    register char_type *stackp, *de_stack;
    register char_type finchar;
    register code_int code, oldcode, incode;
    BufChar	    *buf, *bufend;

    file = (CompressedFile *) f->private;

    buf = f->buffer;
    bufend = buf + BUFFILESIZE;
    stackp = file->stackp;
    de_stack = file->de_stack;
    finchar = file->finchar;
    oldcode = file->oldcode;
    while (buf < bufend) {
	while (stackp > de_stack && buf < bufend)
	    *buf++ = *--stackp;

	if (buf == bufend)
	    break;

	if (oldcode == -1)
	    break;

	code = getcode (file);
	if (code == -1)
	    break;
    
    	if ( (code == CLEAR) && file->block_compress ) {
	    for ( code = 255; code >= 0; code-- )
	    	file->tab_prefix[code] = 0;
	    file->clear_flg = 1;
	    file->free_ent = FIRST - 1;
	    if ( (code = getcode (file)) == -1 )	/* O, untimely death! */
	    	break;
    	}
    	incode = code;
    	/*
     	 * Special case for KwKwK string.
     	 */
    	if ( code >= file->free_ent ) {
	    *stackp++ = finchar;
	    code = oldcode;
    	}
    
    	/*
     	 * Generate output characters in reverse order
     	 */
    	while ( code >= 256 )
    	{
	    *stackp++ = file->tab_suffix[code];
	    code = file->tab_prefix[code];
    	}
	finchar = file->tab_suffix[code];
	*stackp++ = finchar;
    
    	/*
     	 * Generate the new entry.
     	 */
    	if ( (code=file->free_ent) < file->maxmaxcode ) {
	    file->tab_prefix[code] = (unsigned short)oldcode;
	    file->tab_suffix[code] = finchar;
	    file->free_ent = code+1;
    	} 
	/*
	 * Remember previous code.
	 */
	oldcode = incode;
    }
    file->oldcode = oldcode;
    file->stackp = stackp;
    file->finchar = finchar;
    if (buf == f->buffer) {
	f->left = 0;
	return BUFFILEEOF;
    }
    f->bufp = f->buffer + 1;
    f->left = (buf - f->buffer) - 1;
    return f->buffer[0];
}

BufFilePtr
BufFilePushCompressed (f)
    BufFilePtr	f;
{
    int		    code;
    int		    maxbits;
    int		    hsize;
    CompressedFile  *file;
    int		    extra;

    if ((BufFileGet(f) != (magic_header[0] & 0xFF)) ||
	(BufFileGet(f) != (magic_header[1] & 0xFF)))
    {
	return 0;
    }
    code = BufFileGet (f);
    maxbits = code & BIT_MASK;
    if (maxbits > BITS || maxbits < 12)
	return 0;
    hsize = hsize_table[maxbits - 12];
    extra = (1 << maxbits) * sizeof (char_type) +
	    hsize * sizeof (unsigned short);
    file = (CompressedFile *) xalloc (sizeof (CompressedFile) + extra);
    if (!file)
	return 0;
    file->file = f;
    file->maxbits = maxbits;
    file->block_compress = code & BLOCK_MASK;
    file->maxmaxcode = 1 << file->maxbits;
    file->tab_suffix = (char_type *) &file[1];
    file->tab_prefix = (unsigned short *) (file->tab_suffix + file->maxmaxcode);
    /*
     * As above, initialize the first 256 entries in the table.
     */
    file->maxcode = MAXCODE(file->n_bits = INIT_BITS);
    for ( code = 255; code >= 0; code-- ) {
	file->tab_prefix[code] = 0;
	file->tab_suffix[code] = (char_type) code;
    }
    file->free_ent = ((file->block_compress) ? FIRST : 256 );
    file->clear_flg = 0;
    file->offset = 0;
    file->size = 0;
    file->stackp = file->de_stack;
    file->finchar = file->oldcode = getcode (file);
    if (file->oldcode != -1)
	*file->stackp++ = file->finchar;
    return BufFileCreate ((char *) file,
			  BufCompressedFill,
			  BufCompressedSkip,
			  BufCompressedClose);
}


static FontFilePtr
FontFileOpen (char *name) {
    int		fd;
    int		len;
    BufFilePtr	raw, cooked;

    fd = open (name, 0);
    if (fd < 0)
	return 0;
    raw = BufFileOpenRead (fd);
    if (!raw)
    {
	close (fd);
	return 0;
    }
    len = strlen (name);
    if (len > 2 && !strcmp (name + len - 2, ".Z")) {
	cooked = BufFilePushCompressed (raw);
	if (!cooked) {
	    BufFileClose (raw, TRUE);
	    return 0;
	}
	raw = cooked;
    }
    return (FontFilePtr) raw;
}

static void FontFileClose (FontFilePtr f)
{
    BufFileClose ((BufFilePtr) f, TRUE);
}

static int
pcfGetLSB32(file)
    FontFilePtr file;
{
    int         c;

    c = FontFileGetc(file);
    c |= FontFileGetc(file) << 8;
    c |= FontFileGetc(file) << 16;
    c |= FontFileGetc(file) << 24;
    position += 4;
    return c;
}

static      PCFTablePtr
pcfReadTOC(file, countp)
    FontFilePtr file;
    int        *countp;
{
    CARD32      version;
    PCFTablePtr tables;
    int         count;
    int         i;

    position = 0;
    version = pcfGetLSB32(file);
    if (version != PCF_FILE_VERSION)
	return (PCFTablePtr) NULL;
    count = pcfGetLSB32(file);
    tables = (PCFTablePtr) xalloc(count * sizeof(PCFTableRec));
    if (!tables)
	return (PCFTablePtr) NULL;
    for (i = 0; i < count; i++) {
	tables[i].type = pcfGetLSB32(file);
	tables[i].format = pcfGetLSB32(file);
	tables[i].size = pcfGetLSB32(file);
	tables[i].offset = pcfGetLSB32(file);
    }
    *countp = count;
    return tables;
}


static
ResizeHashTable ()
{
    int		newHashSize;
    int		newHashMask;
    AtomListPtr	*newHashTable;
    int		i;
    int		h;
    int		newRehash;
    int		r;

    if (hashSize == 0)
	newHashSize = 1024;
    else
	newHashSize = hashSize * 2;
    newHashTable = (AtomListPtr *) xalloc (newHashSize * sizeof (AtomListPtr));
    if (!newHashTable)
	return FALSE;
    bzero ((char *) newHashTable, newHashSize * sizeof (AtomListPtr));
    newHashMask = newHashSize - 1;
    newRehash = (newHashMask - 2);
    for (i = 0; i < hashSize; i++)
    {
	if (hashTable[i])
	{
	    h = (hashTable[i]->hash) & newHashMask;
	    if (newHashTable[h])
	    {
		r = hashTable[i]->hash % newRehash | 1;
		do {
		    h += r;
		    if (h >= newHashSize)
			h -= newHashSize;
		} while (newHashTable[h]);
	    }
	    newHashTable[h] = hashTable[i];
	}
    }
    xfree (hashTable);
    hashTable = newHashTable;
    hashSize = newHashSize;
    hashMask = newHashMask;
    rehash = newRehash;
    return TRUE;
}

static
Hash(string, len)
    char    *string;
{
    int	h;

    h = 0;
    while (len--)
	h = (h << 3) ^ *string++;
    if (h < 0)
	return -h;
    return h;
}
static Atom 
MakeAtom(string, len, makeit)
    char *string;
    unsigned len;
    int makeit;
{
    AtomListPtr	a;
    int		hash;
    int		h;
    int		r;

    hash = Hash (string, len);
    if (hashTable)
    {
    	h = hash & hashMask;
	if (hashTable[h])
	{
	    if (hashTable[h]->hash == hash && hashTable[h]->len == len &&
	    	NameEqual (hashTable[h]->name, string, len))
	    {
	    	return hashTable[h]->atom;
	    }
	    r = (hash % rehash) | 1;
	    for (;;)
	    {
		h += r;
		if (h >= hashSize)
		    h -= hashSize;
		if (!hashTable[h])
		    break;
		if (hashTable[h]->hash == hash && hashTable[h]->len == len &&
		    NameEqual (hashTable[h]->name, string, len))
		{
		    return hashTable[h]->atom;
		}
	    }
    	}
    }
    if (!makeit)
	return None;
    a = (AtomListPtr) xalloc (sizeof (AtomListRec) + len + 1);
    a->name = (char *) (a + 1);
    a->len = len;
    strncpy (a->name, string, len);
    a->name[len] = '\0';
    a->atom = ++lastAtom;
    a->hash = hash;
    if (hashUsed >= hashSize / 2)
    {
	ResizeHashTable ();
	h = hash & hashMask;
	if (hashTable[h])
	{
	    r = (hash % rehash) | 1;
	    do {
		h += r;
		if (h >= hashSize)
		    h -= hashSize;
	    } while (hashTable[h]);
	}
    }
    hashTable[h] = a;
    hashUsed++;
    if (reverseMapSize <= a->atom)
	ResizeReverseMap();
    reverseMap[a->atom] = a;
    return a->atom;
}

BufFileRead (f, b, n)
    BufFilePtr	f;
    char	*b;
    int		n;
{
    int	    c, cnt;
    cnt = n;
    while (cnt--) {
	c = BufFileGet (f);
	if (c == BUFFILEEOF)
	    break;
	*b++ = c;
    }
    return n - cnt - 1;
}
static Bool
pcfGetProperties(pFontInfo, file, tables, ntables)
    FontInfoPtr pFontInfo;
    FontFilePtr file;
    PCFTablePtr tables;
    int         ntables;
{
    FontPropPtr props = 0;
    int         nprops;
    char       *isStringProp = 0;
    CARD32      format;
    int         i;
    /* changed by suresh 07/12/99 from int to CARD 32*/
    CARD32         size;
    int         string_size;
    char       *strings;

    /* font properties */

    if (!pcfSeekToType(file, tables, ntables, PCF_PROPERTIES, &format, &size))
	goto Bail;
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	goto Bail;
    nprops = pcfGetINT32(file, format);
    props = (FontPropPtr) xalloc(nprops * sizeof(FontPropRec));
    if (!props)
	goto Bail;
    isStringProp = (char *) xalloc(nprops * sizeof(char));
    if (!isStringProp)
	goto Bail;
    for (i = 0; i < nprops; i++) {
	props[i].name = pcfGetINT32(file, format);
	isStringProp[i] = pcfGetINT8(file, format);
	props[i].value = pcfGetINT32(file, format);
    }
    /* pad the property array */
    /*
     * clever here - nprops is the same as the number of odd-units read, as
     * only isStringProp are odd length
     */
    if (nprops & 3)
    {
	i = 4 - (nprops & 3);
	FontFileSkip(file, i);
	position += i;
    }
    string_size = pcfGetINT32(file, format);
    strings = (char *) xalloc(string_size);
    if (!strings) {
	goto Bail;
    }
    FontFileRead(file, strings, string_size);
    position += string_size;
    for (i = 0; i < nprops; i++) {
	props[i].name = MakeAtom(strings + props[i].name,
				 strlen(strings + props[i].name), TRUE);
	if (isStringProp[i]) {
	    props[i].value = MakeAtom(strings + props[i].value,
				      strlen(strings + props[i].value), TRUE);
	}
    }
    xfree(strings);
    pFontInfo->isStringProp = isStringProp;
    pFontInfo->props = props;
    pFontInfo->nprops = nprops;
    return TRUE;
Bail:
    xfree(isStringProp);
    xfree(props);
    return FALSE;
}

static void
pcfGetCompressedMetric(file, format, metric)
    FontFilePtr file;
    CARD32      format;
    xCharInfo  *metric;
{
    metric->leftSideBearing = pcfGetINT8(file, format) - 0x80;
    metric->rightSideBearing = pcfGetINT8(file, format) - 0x80;
    metric->characterWidth = pcfGetINT8(file, format) - 0x80;
    metric->ascent = pcfGetINT8(file, format) - 0x80;
    metric->descent = pcfGetINT8(file, format) - 0x80;
    metric->attributes = 0;
}

static Bool
pcfSeekToType(file, tables, ntables, type, formatp, sizep)
    FontFilePtr file;
    PCFTablePtr tables;
    int         ntables;
    CARD32      type;
    CARD32     *formatp;
    CARD32     *sizep;
{
    int         i;

    for (i = 0; i < ntables; i++)
	if (tables[i].type == type) {
	    if (position > tables[i].offset)
		return FALSE;
	    if (!FontFileSkip(file, tables[i].offset - position))
		return FALSE;
	    position = tables[i].offset;
	    *sizep = tables[i].size;
	    *formatp = tables[i].format;
	    return TRUE;
	}
    return FALSE;
}

static int
pcfGetINT32(file, format)
    FontFilePtr file;
    CARD32      format;
{
    int         c;

    if (PCF_BYTE_ORDER(format) == MSBFirst) {
	c = FontFileGetc(file) << 24;
	c |= FontFileGetc(file) << 16;
	c |= FontFileGetc(file) << 8;
	c |= FontFileGetc(file);
    } else {
	c = FontFileGetc(file);
	c |= FontFileGetc(file) << 8;
	c |= FontFileGetc(file) << 16;
	c |= FontFileGetc(file) << 24;
    }
    position += 4;
    return c;
}
static int
pcfGetINT16(file, format)
    FontFilePtr file;
    CARD32      format;
{
    int         c;

    if (PCF_BYTE_ORDER(format) == MSBFirst) {
	c = FontFileGetc(file) << 8;
	c |= FontFileGetc(file);
    } else {
	c = FontFileGetc(file);
	c |= FontFileGetc(file) << 8;
    }
    position += 2;
    return c;
}

/*
 * pcfReadAccel
 *
 * Fill in the accelerator information from the font file; used
 * to read both BDF_ACCELERATORS and old style ACCELERATORS
 */

static Bool
pcfGetAccel(pFontInfo, file, tables, ntables, type)
    FontInfoPtr pFontInfo;
    FontFilePtr file;
    PCFTablePtr	tables;
    int		ntables;
    CARD32	type;
{
    CARD32      format;
    /* changed by suresh 07/12/99 from int to CARD 32*/
    CARD32		size;

    if (!pcfSeekToType(file, tables, ntables, type, &format, &size))
	goto Bail;
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT) &&
	!PCF_FORMAT_MATCH(format, PCF_ACCEL_W_INKBOUNDS)) 
    {
	goto Bail;
    }
    pFontInfo->noOverlap = pcfGetINT8(file, format);
    pFontInfo->constantMetrics = pcfGetINT8(file, format);
    pFontInfo->terminalFont = pcfGetINT8(file, format);
    pFontInfo->constantWidth = pcfGetINT8(file, format);
    pFontInfo->inkInside = pcfGetINT8(file, format);
    pFontInfo->inkMetrics = pcfGetINT8(file, format);
    pFontInfo->drawDirection = pcfGetINT8(file, format);
    pFontInfo->anamorphic = FALSE;
     /* natural alignment */ pcfGetINT8(file, format);
    pFontInfo->fontAscent = pcfGetINT32(file, format);
    pFontInfo->fontDescent = pcfGetINT32(file, format);
    pFontInfo->maxOverlap = pcfGetINT32(file, format);
    pcfGetMetric(file, format, &pFontInfo->minbounds);
    pcfGetMetric(file, format, &pFontInfo->maxbounds);
    if (PCF_FORMAT_MATCH(format, PCF_ACCEL_W_INKBOUNDS)) {
	pcfGetMetric(file, format, &pFontInfo->ink_minbounds);
	pcfGetMetric(file, format, &pFontInfo->ink_maxbounds);
    } else {
	pFontInfo->ink_minbounds = pFontInfo->minbounds;
	pFontInfo->ink_maxbounds = pFontInfo->maxbounds;
    }
    return TRUE;
Bail:
    return FALSE;
}

static void
pcfGetMetric(file, format, metric)
    FontFilePtr file;
    CARD32      format;
    xCharInfo  *metric;
{
    metric->leftSideBearing = pcfGetINT16(file, format);
    metric->rightSideBearing = pcfGetINT16(file, format);
    metric->characterWidth = pcfGetINT16(file, format);
    metric->ascent = pcfGetINT16(file, format);
    metric->descent = pcfGetINT16(file, format);
    metric->attributes = pcfGetINT16(file, format);
}

int
bitmapGetGlyphs(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    CharInfoPtr *glyphs;	/* RETURN */
{
    BitmapFontPtr  bitmapFont;
    unsigned int firstCol;
    register unsigned int numCols;
    unsigned int firstRow;
    unsigned int numRows;
    CharInfoPtr *glyphsBase;
    register unsigned int c;
    register CharInfoPtr pci;
    unsigned int r;
    CharInfoPtr *encoding;
    CharInfoPtr pDefault;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    encoding = bitmapFont->encoding;
    pDefault = bitmapFont->pDefault;
    firstCol = pFont->info.firstCol;
    numCols = pFont->info.lastCol - firstCol + 1;
    glyphsBase = glyphs;
    switch (charEncoding) {

    case Linear8Bit:
    case TwoD8Bit:
	if (pFont->info.firstRow > 0)
	    break;
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = encoding[c];
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols && (pci = encoding[c]))
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;
    case Linear16Bit:
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = encoding[c];
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols && (pci = encoding[c]))
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;

    case TwoD16Bit:
	firstRow = pFont->info.firstRow;
	numRows = pFont->info.lastRow - firstRow + 1;
	while (count--) {
	    r = (*chars++) - firstRow;
	    c = (*chars++) - firstCol;
	    if (r < numRows && c < numCols &&
		    (pci = encoding[r * numCols + c]))
		*glyphs++ = pci;
	    else if (pDefault)
		*glyphs++ = pDefault;
	}
	break;
    }
    *glyphCount = glyphs - glyphsBase;
    return Successful;
}


int
bitmapGetMetrics(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    xCharInfo **glyphs;		/* RETURN */
{
    int         ret;
    xCharInfo  *ink_metrics;
    CharInfoPtr metrics;
    BitmapFontPtr  bitmapFont;
    CharInfoPtr	oldDefault;
    int         i;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    oldDefault = bitmapFont->pDefault;
    bitmapFont->pDefault = &nonExistantChar;
    ret = bitmapGetGlyphs(pFont, count, chars, charEncoding, glyphCount, (CharInfoPtr *) glyphs);
    if (ret == Successful) {
	if (bitmapFont->ink_metrics) {
	    metrics = bitmapFont->metrics;
	    ink_metrics = bitmapFont->ink_metrics;
	    for (i = 0; i < *glyphCount; i++) {
		if (glyphs[i] != (xCharInfo *) & nonExistantChar)
		    glyphs[i] = ink_metrics + (((CharInfoPtr) glyphs[i]) - metrics);
	    }
	}
    }
    bitmapFont->pDefault = oldDefault;
    return ret;
}

void
pcfUnloadFont(pFont)
    FontPtr     pFont;
{
    BitmapFontPtr  bitmapFont;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    xfree(bitmapFont->ink_metrics);
    xfree(bitmapFont->encoding);
    xfree(bitmapFont->bitmaps);
    xfree(bitmapFont->metrics);
    xfree(pFont->info.isStringProp);
    xfree(pFont->info.props);
    xfree(bitmapFont);
    xfree(pFont);
}
static int
pcfReadFont(pFont, file, bit, byte, glyph, scan)
    FontPtr     pFont;
    FontFilePtr file;
    int         bit,
                byte,
                glyph,
                scan;
{
    CARD32      format;
    CARD32      size;
    BitmapFontPtr  bitmapFont = 0;
    int         i;
    PCFTablePtr tables = 0;
    int         ntables;
    int         nmetrics;
    int         nbitmaps;
    int         sizebitmaps;
    int         nink_metrics;
    CharInfoPtr metrics = 0;
    xCharInfo  *ink_metrics = 0;
    char       *bitmaps = 0;
    CharInfoPtr *encoding = 0;
    int         nencoding;
    int         encodingOffset;
    CARD32      bitmapSizes[GLYPHPADOPTIONS];
    CARD32     *offsets = 0;
    Bool	hasBDFAccelerators;

    pFont->info.props = 0;
    if (!(tables = pcfReadTOC(file, &ntables)))
	goto Bail;

    /* properties */

    if (!pcfGetProperties(&pFont->info, file, tables, ntables))
	goto Bail;

    /* Use the old accelerators if no BDF accelerators are in the file */

    hasBDFAccelerators = pcfHasType (tables, ntables, PCF_BDF_ACCELERATORS);
    if (!hasBDFAccelerators)
	if (!pcfGetAccel (&pFont->info, file, tables, ntables, PCF_ACCELERATORS))
	    goto Bail;

    /* metrics */

    if (!pcfSeekToType(file, tables, ntables, PCF_METRICS, &format, &size)) {
	goto Bail;
    }
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT) &&
	    !PCF_FORMAT_MATCH(format, PCF_COMPRESSED_METRICS)) {
	goto Bail;
    }
    if (PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	nmetrics = pcfGetINT32(file, format);
    else
	nmetrics = pcfGetINT16(file, format);
    metrics = (CharInfoPtr) xalloc(nmetrics * sizeof(CharInfoRec));
    if (!metrics) {
	goto Bail;
    }
    for (i = 0; i < nmetrics; i++)
	if (PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	    pcfGetMetric(file, format, &(metrics + i)->metrics);
	else
	    pcfGetCompressedMetric(file, format, &(metrics + i)->metrics);

    /* bitmaps */

    if (!pcfSeekToType(file, tables, ntables, PCF_BITMAPS, &format, &size))
	goto Bail;
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	goto Bail;

    nbitmaps = pcfGetINT32(file, format);
    if (nbitmaps != nmetrics)
	goto Bail;

    offsets = (CARD32 *) xalloc(nbitmaps * sizeof(CARD32));
    if (!offsets)
	goto Bail;

    for (i = 0; i < nbitmaps; i++)
	offsets[i] = pcfGetINT32(file, format);

    for (i = 0; i < GLYPHPADOPTIONS; i++)
	bitmapSizes[i] = pcfGetINT32(file, format);
    sizebitmaps = bitmapSizes[PCF_GLYPH_PAD_INDEX(format)];
    bitmaps = (char *) xalloc(sizebitmaps);
    if (!bitmaps)
	goto Bail;
    FontFileRead(file, bitmaps, sizebitmaps);
    position += sizebitmaps;

    if (PCF_BIT_ORDER(format) != bit)
	BitOrderInvert((unsigned char*)bitmaps, sizebitmaps);
    if ((PCF_BYTE_ORDER(format) == PCF_BIT_ORDER(format)) != (bit == byte)) {
	switch (bit == byte ? PCF_SCAN_UNIT(format) : scan) {
	case 1:
	    break;
	case 2:
	    TwoByteSwap((unsigned char*)bitmaps, sizebitmaps);
	    break;
	case 4:
	    FourByteSwap((unsigned char*)bitmaps, sizebitmaps);
	    break;
	}
    }
    if (PCF_GLYPH_PAD(format) != glyph) {
	char       *padbitmaps;
	int         sizepadbitmaps;
	int         old,
	            new;
	xCharInfo  *metric;

	sizepadbitmaps = bitmapSizes[PCF_SIZE_TO_INDEX(glyph)];
	padbitmaps = (char *) xalloc(sizepadbitmaps);
	if (!padbitmaps) {
	    goto Bail;
	}
	new = 0;
	for (i = 0; i < nbitmaps; i++) {
	    old = offsets[i];
	    metric = &metrics[i].metrics;
	    offsets[i] = new;
	    new += RepadBitmap(bitmaps + old, padbitmaps + new,
			       PCF_GLYPH_PAD(format), glyph,
			  metric->rightSideBearing - metric->leftSideBearing,
			       metric->ascent + metric->descent);
	}
	xfree(bitmaps);
	bitmaps = padbitmaps;
    }
    for (i = 0; i < nbitmaps; i++)
	metrics[i].bits = bitmaps + offsets[i];

    xfree(offsets);

    /* ink metrics ? */

    ink_metrics = NULL;
    if (pcfSeekToType(file, tables, ntables, PCF_INK_METRICS, &format, &size)) {
	format = pcfGetLSB32(file);
	if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT) &&
		!PCF_FORMAT_MATCH(format, PCF_COMPRESSED_METRICS)) {
	    goto Bail;
	}
	if (PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	    nink_metrics = pcfGetINT32(file, format);
	else
	    nink_metrics = pcfGetINT16(file, format);
	if (nink_metrics != nmetrics)
	    goto Bail;
	ink_metrics = (xCharInfo *) xalloc(nink_metrics * sizeof(xCharInfo));
	if (!ink_metrics)
	    goto Bail;
	for (i = 0; i < nink_metrics; i++)
	    if (PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
		pcfGetMetric(file, format, ink_metrics + i);
	    else
		pcfGetCompressedMetric(file, format, ink_metrics + i);
    }

    /* encoding */

    if (!pcfSeekToType(file, tables, ntables, PCF_BDF_ENCODINGS, &format, &size))
	goto Bail;
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	goto Bail;

    pFont->info.firstCol = pcfGetINT16(file, format);
    pFont->info.lastCol = pcfGetINT16(file, format);
    pFont->info.firstRow = pcfGetINT16(file, format);
    pFont->info.lastRow = pcfGetINT16(file, format);
    pFont->info.defaultCh = pcfGetINT16(file, format);

    nencoding = (pFont->info.lastCol - pFont->info.firstCol + 1) *
	(pFont->info.lastRow - pFont->info.firstRow + 1);

    encoding = (CharInfoPtr *) xalloc(nencoding * sizeof(CharInfoPtr));
    if (!encoding)
	goto Bail;

    pFont->info.allExist = TRUE;
    for (i = 0; i < nencoding; i++) {
	encodingOffset = pcfGetINT16(file, format);
	if (encodingOffset == 0xFFFF) {
	    pFont->info.allExist = FALSE;
	    encoding[i] = 0;
	} else
	    encoding[i] = metrics + encodingOffset;
    }

    /* BDF style accelerators (i.e. bounds based on encoded glyphs) */

    if (hasBDFAccelerators)
	if (!pcfGetAccel (&pFont->info, file, tables, ntables, PCF_BDF_ACCELERATORS))
	    goto Bail;

    bitmapFont = (BitmapFontPtr) xalloc(sizeof *bitmapFont);
    if (!bitmapFont)
	goto Bail;

    bitmapFont->version_num = PCF_FILE_VERSION;
    bitmapFont->num_chars = nmetrics;
    bitmapFont->num_tables = ntables;
    bitmapFont->metrics = metrics;
    bitmapFont->ink_metrics = ink_metrics;
    bitmapFont->bitmaps = bitmaps;
    bitmapFont->encoding = encoding;
    bitmapFont->pDefault = (CharInfoPtr) 0;
    if (pFont->info.defaultCh != (unsigned short) NO_SUCH_CHAR) {
	int         r,
	            c,
	            cols;

	r = pFont->info.defaultCh >> 8;
	c = pFont->info.defaultCh & 0xFF;
	if (pFont->info.firstRow <= r && r <= pFont->info.lastRow &&
		pFont->info.firstCol <= c && c <= pFont->info.lastCol) {
	    cols = pFont->info.lastCol - pFont->info.firstCol + 1;
	    r = r - pFont->info.firstRow;
	    c = c - pFont->info.firstCol;
	    bitmapFont->pDefault = encoding[r * cols + c];
	}
    }
    bitmapFont->bitmapExtra = (BitmapExtraPtr) 0;
    pFont->fontPrivate = (pointer) bitmapFont;
    pFont->get_glyphs = bitmapGetGlyphs;
    pFont->get_metrics = bitmapGetMetrics;
    pFont->unload_font = pcfUnloadFont;
    pFont->bit = bit;
    pFont->byte = byte;
    pFont->glyph = glyph;
    pFont->scan = scan;
    xfree(tables);
    return Successful;
Bail:
    xfree(ink_metrics);
    xfree(encoding);
    xfree(bitmaps);
    xfree(offsets);
    xfree(metrics);
    xfree(pFont->info.props);
    pFont->info.props = 0;
    xfree(bitmapFont);
    xfree(tables);
    return AllocError;
}
