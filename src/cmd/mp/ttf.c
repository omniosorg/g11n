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
#pragma ident   "@(#)ttf.c	1.3 99/11/20 SMI"

#include <stdio.h>
#include <sys/types.h>
#include "ttf.h"

/* composite font flags */

#define ARGS_ARE_WORDS       0x001
#define ARGS_ARE_XY_VALUES   0x002
#define ROUND_XY_TO_GRID     0x004
#define WE_HAVE_A_SCALE      0x008
/* reserved                  0x010 */
#define MORE_COMPONENTS      0x020
#define WE_HAVE_AN_XY_SCALE  0x040
#define WE_HAVE_A_2X2        0x080
#define WE_HAVE_INSTR        0x100
#define USE_MY_METRICS       0x200

#define MAX_COMPOUND_LEVEL 	20

static ushort_t tt2ps_get16(FILE * );
static ulong_t tt2ps_get32(FILE * );
static void tt2ps_load_table(ttffont_t *);
static void tt2ps_load_fontinfo(ttffont_t *);
static BOOL tt2ps_load_font(int, ushort_t , float *, int , ttffont_t * , tt_GlyphData_t * );
static void tt2ps_moveto(int , int , tt_OutlineData_t *, int *);
static void tt2ps_lineto(int , int , tt_OutlineData_t *, int *);
static void tt2ps_curveto(fixp , fixp , int , int , tt_GlyphData_t * , tt_OutlineData_t *, int *);
void tt2ps_clear_outlinedata ( tt_OutlineData_t *);
static ushort_t tt2ps_font_index(int , ttffont_t * );
static void tt2ps_clear_glyphdata(tt_GlyphData_t *);
static void* get_zeroed_mem(int ) ;
static subglyph_t* get_subglyph(void) ;
static BOOL tt2ps_load_simple_glyph(ushort_t , float *, int , ttffont_t * , tt_GlyphData_t * , int , subglyph_t *, int) ;
static BOOL tt2ps_load_compound_glyph(ushort_t , float *, int , ttffont_t * , tt_GlyphData_t * , subglyph_t *, int) ;
static long get_glyphd_num_ctr_offset(ushort_t idx, ttffont_t * ttfont) ;

static int current_cmapd_seg_cnt=0;

/*
 * Open TrueType font
 * This routine support both TrueType and TrueType Collection
 * font format.
 * TTC segment number is specified by arg #3 - ttc_num
 */
int
tt2ps_open_ttfont(char *fontname, ttffont_t * ttfont, int ttc_num)
{
	int             sfnt_ver;
	ulong_t         ttc_offset;
	ulong_t         ttc_count;
	char            buf[5];

	if ((ttfont->fn = fopen(fontname, "rb")) == NULL) {
		return TT_NOTFOUND;
	}
	fseek(ttfont->fn, 0L, SEEK_SET);
	sfnt_ver = tt2ps_get32(ttfont->fn);
	if (sfnt_ver == 0x00010000) {
		ttc_offset = 0;	/* TrueType font */
	} else {
		fseek(ttfont->fn, 0L, SEEK_SET);
		fread(buf, 1, 4, ttfont->fn);
		buf[4] = 0;
		if (strncmp(buf, "ttcf", 4) != 0) {
			return TT_BADFONT;
		}
		fseek(ttfont->fn, 8L, SEEK_SET);
		ttc_count = tt2ps_get32(ttfont->fn);
		if (ttc_num > ttc_count || ttc_num < 0)
			ttc_num = 0;

		while (ttc_num--)
			tt2ps_get32(ttfont->fn);
		ttc_offset = tt2ps_get32(ttfont->fn);
	}

	ttfont->fontd = (tt_FontData_t *) calloc(sizeof(tt_FontData_t), 1);
	ttfont->cmapd = (tt_CmapData_t *) calloc(sizeof(tt_CmapData_t), 1);
	ttfont->ttc_offset = ttc_offset;

	tt2ps_load_table(ttfont);
	tt2ps_load_fontinfo(ttfont);
	if(tt2ps_load_cmap(ttfont) == FALSE){
		return TT_BADFORMAT;
	}

	return TT_SUCCESS;
}


/*
 * Load TrueType Cmap informations
 * This routine support only format 4 of Microsoft cmap (PlatformID = 3)
 */
BOOL
tt2ps_load_cmap(ttffont_t * ttfont)
{
	ushort_t          i, j, cnt, fmt;
	ulong_t          *off;
	ushort_t         *enc, *platform;
	FILE           *fn = ttfont->fn;
	tt_CmapData_t     *cmapd = ttfont->cmapd;
	tt_FontData_t     *fontd = ttfont->fontd;

	fseek(fn, fontd->cmap + 2, SEEK_SET);
	cnt = tt2ps_get16(fn);

	off = (ulong_t *) calloc(sizeof(ulong_t), cnt);
	platform = (ushort_t *) calloc(sizeof(ushort_t), cnt);
	enc = (ushort_t *) calloc(sizeof(ushort_t), cnt);
	for (i = 0; i < cnt; i++) {
		platform[i] = tt2ps_get16(fn);
		enc[i] = tt2ps_get16(fn);
		off[i] = tt2ps_get32(fn);
	}

	cmapd->endCount = (ushort_t *) NULL;	/* Initialize */

	for (i = 0; i < cnt; i++) {
		fseek(fn, fontd->cmap + off[i], SEEK_SET);
		fmt = tt2ps_get16(fn);

		if ( fmt != 4 ||  platform[i] != 3) { 
			continue;
		}
		cmapd->encoding = enc[i];
		tt2ps_get16(fn);/* length */
		tt2ps_get16(fn);/* version */
		cmapd->segCount = (tt2ps_get16(fn) / 2);
		tt2ps_get16(fn);/* searchRange */
		tt2ps_get16(fn);/* entrySelector */
		tt2ps_get16(fn);/* rangeShift */
		cmapd->endCount =
			(ushort_t *) calloc(sizeof(ushort_t), cmapd->segCount);
		for (j = 0; j < cmapd->segCount; j++) {
			cmapd->endCount[j] = tt2ps_get16(fn);
		}
		tt2ps_get16(fn);/* reservedPad */
		cmapd->startCount =
			(ushort_t *) calloc(sizeof(ushort_t), cmapd->segCount);
		for (j = 0; j < cmapd->segCount; j++) {
			cmapd->startCount[j] = tt2ps_get16(fn);
		}
		cmapd->idDelta =
			(ushort_t *) calloc(sizeof(ushort_t), cmapd->segCount);
		for (j = 0; j < cmapd->segCount; j++) {
			cmapd->idDelta[j] = tt2ps_get16(fn);
		}
		cmapd->idRangeOffset =
			(ushort_t *) calloc(sizeof(ushort_t), cmapd->segCount);
		for (j = 0; j < cmapd->segCount; j++) {
			cmapd->idRangeOffset[j] = tt2ps_get16(fn);
		}

		fgetpos(fn, &cmapd->gryphIdArray);

		break;
	}
	free(off);
	free(platform);
	free(enc);
	if (!(cmapd->endCount)) {
		return FALSE;
	}
	/* Get ymax ymin from head */
	fseek(fn, fontd->head + 18L, SEEK_SET);
	cmapd->unitsPerEm = (ushort_t) tt2ps_get16(fn);
	fseek(fn, fontd->head + 36L, SEEK_SET);
	cmapd->xmin = (short) tt2ps_get16(fn);
	cmapd->ymin = (short) tt2ps_get16(fn);
	cmapd->xmax = (short) tt2ps_get16(fn);
	cmapd->ymax = (short) tt2ps_get16(fn);
	tt2ps_get16(fn);	/* macStyle */
	tt2ps_get16(fn);	/* lowestRecPPEM */
	tt2ps_get16(fn);	/* fontDirectionHint */
	cmapd->iTLF = tt2ps_get16(fn);	/* indexToLocFormat */

	return TRUE;
}
static ushort_t
tt2ps_get16(FILE * fn)
{
	uchar_t           c1 ; 
	uchar_t           c2 ;
	c1 = getc(fn);
	c2 = getc(fn);
	return ((ushort_t) c1) * 256 + c2;
}

static ulong_t
tt2ps_get32(FILE * fn)
{
	ushort_t          s1 = tt2ps_get16(fn);
	ushort_t          s2 = tt2ps_get16(fn);
	return ((ulong_t) s1) * 65536L + s2;
}

/*
 * Load TrueType header table
 */
static void
tt2ps_load_table(ttffont_t * ttfont)
{
	char            buf[5];
	short           i, cnt;
	FILE           *fn = ttfont->fn;
	tt_FontData_t     *fontd = ttfont->fontd;
	ulong_t           offset = ttfont->ttc_offset;

	fseek(fn, 4L + offset, SEEK_SET);
	cnt = tt2ps_get16(fn);
	fseek(fn, 12L + offset, SEEK_SET);
	for (i = 0; i < cnt; i++) {
		fread(buf, 1, 4, fn);
		buf[4] = 0;
		if (!strcmp(buf, "cmap")) {
			tt2ps_get32(fn);
			fontd->cmap = tt2ps_get32(fn);
			tt2ps_get32(fn);
		} else if (!strcmp(buf, "loca")) {
			tt2ps_get32(fn);
			fontd->loca = tt2ps_get32(fn);
			tt2ps_get32(fn);	/* loca_size */
		} else if (!strcmp(buf, "glyf")) {
			tt2ps_get32(fn);
			fontd->glyf = tt2ps_get32(fn);
			tt2ps_get32(fn);
		} else if (!strcmp(buf, "head")) {
			tt2ps_get32(fn);
			fontd->head = tt2ps_get32(fn);
			tt2ps_get32(fn);
		} else if (!strcmp(buf, "maxp")) {
			tt2ps_get32(fn);
			fontd->maxp = tt2ps_get32(fn);
			tt2ps_get32(fn);
		} else if (!strcmp(buf, "hhea")) {
			tt2ps_get32(fn);
			fontd->hhea = tt2ps_get32(fn);
			tt2ps_get32(fn);
		} else if (!strcmp(buf, "hmtx")) {
			tt2ps_get32(fn);
			fontd->hmtx = tt2ps_get32(fn);
			tt2ps_get32(fn);
		} else {	/* Skip */
			tt2ps_get32(fn);
			tt2ps_get32(fn);
			tt2ps_get32(fn);
		}
	}
}

/*
 * Load TrueType font informations.
 * numGlyphs - # of glyphs in TrueType font
 * numbetOfHMetrics - # of HMetrics information
 * hmetrics - hmetrics data
 */
void
tt2ps_load_fontinfo(ttffont_t * ttfont)
{
	ushort_t          i;
	ushort_t          last_aw;
	FILE           *fn = ttfont->fn;
	tt_FontData_t     *fontd = ttfont->fontd;

	fseek(fn, fontd->maxp + 4L, SEEK_SET);
	fontd->numGlyphs = (ushort_t) tt2ps_get16(fn);
	fseek(fn, fontd->hhea + 34L, SEEK_SET);
	fontd->numberOfHMetrics = (ushort_t) tt2ps_get16(fn);
	fseek(fn, fontd->hmtx, SEEK_SET);
	fontd->hmetrics =
		(tt_longHorMetric_t *) calloc(sizeof(tt_longHorMetric_t), fontd->numGlyphs);
	for (i = 0; i < fontd->numberOfHMetrics; i++) {
		fontd->hmetrics[i].aw = (ushort_t) tt2ps_get16(fn);
		fontd->hmetrics[i].lsb = (short) tt2ps_get16(fn);
		last_aw = fontd->hmetrics[i].aw;
	}
	for (i = fontd->numberOfHMetrics; i < fontd->numGlyphs; i++) {
		fontd->hmetrics[i].aw = last_aw;
		fontd->hmetrics[i].lsb = (short) tt2ps_get16(fn);
	}
}

/*
 * Load simple glyph data
 */
static BOOL
tt2ps_load_simple_glyph(ushort_t idx, float *dx, int size,
	ttffont_t * ttfont, tt_GlyphData_t * glyphd, int composite, 
	subglyph_t *sg, int level) {
	long            off,k,l;
	ushort_t          j;
	short           x1, y1, x2, y2;
	short 	i;
	uchar_t           c, ct;
	float           scale;
	unsigned int xx, xy, yx, yy;
	FILE           *fn = ttfont->fn;
	tt_FontData_t     *fontd = ttfont->fontd;
	tt_CmapData_t     *cmapd = ttfont->cmapd;
	BOOL ret;


	if (composite) {
		if(cmapd->iTLF == 0) {
			fseek(fn, fontd->loca + (long) idx * 2L, SEEK_SET);
			off = (long)tt2ps_get16(fn);
			off *= 2;
		} else {
			fseek(fn, fontd->loca + (long) idx * 4L, SEEK_SET);
			off = tt2ps_get32(fn);
		}
		fseek(fn, fontd->glyf + off, SEEK_SET);
		glyphd->num_ctr = (short) tt2ps_get16(fn);
		if (glyphd->num_ctr<0) {
			ret=tt2ps_load_compound_glyph(idx, dx, size, ttfont, glyphd, (subglyph_t*)NULL, level+1 );
			return ret;
		}
	}
	x1 = tt2ps_get16(fn);	/* xMin */
	y1 = tt2ps_get16(fn);	/* yMin */
	x2 = tt2ps_get16(fn);	/* xMax */
	y2 = tt2ps_get16(fn);	/* yMax */
	scale = (float) cmapd->unitsPerEm / 1000.0;
	*dx = (float) fontd->hmetrics[idx].aw / cmapd->unitsPerEm * size;
	glyphd->epts_ctr = (ushort_t *) calloc(sizeof(ushort_t), glyphd->num_ctr);
	for (i = 0; i < glyphd->num_ctr; i++)
		glyphd->epts_ctr[i] = tt2ps_get16(fn);
	glyphd->num_pts = glyphd->epts_ctr[glyphd->num_ctr - 1] + 1;
	j = tt2ps_get16(fn);
	while (j--)
		getc(fn);	/* skip instruction */
	glyphd->flag = (uchar_t *) calloc(1, glyphd->num_pts);
	glyphd->xcoor = (fixp *) calloc(sizeof(fixp), glyphd->num_pts);
	glyphd->ycoor = (fixp *) calloc(sizeof(fixp), glyphd->num_pts);
	for (j = 0; j < glyphd->num_pts;) {
		glyphd->flag[j++] = c = getc(fn);
		if (c & 8) {
			ct = getc(fn);
			while (ct--)
				glyphd->flag[j++] = c;
		}
	}
	for (j = 0; j < glyphd->num_pts; j++) {
		if (glyphd->flag[j] & 2) {
			c = getc(fn);
			glyphd->xcoor[j] = (glyphd->flag[j] & 0x10) ? ((short) c) : (-1 * (short) c);
		} else if (glyphd->flag[j] & 0x10)
			glyphd->xcoor[j] = 0;
		else
			glyphd->xcoor[j] = (short) tt2ps_get16(fn);
		if(sg!=NULL&&sg->arg1&&j==0)
			glyphd->xcoor[j]+=(short)sg->arg1;
	}
	for (j = 0; j < glyphd->num_pts; j++) {
		if (glyphd->flag[j] & 4) {
			c = getc(fn);
			glyphd->ycoor[j] = (glyphd->flag[j] & 0x20) ? ((short) c) : (-1 * (short) c);
		} else if (glyphd->flag[j] & 0x20)
			glyphd->ycoor[j] = 0;
		else
			glyphd->ycoor[j] = (short) tt2ps_get16(fn);
		if(sg!=NULL&&sg->arg2&&j==0)
			glyphd->ycoor[j]+=(short)sg->arg2;
	}
	for (j = 1; j < glyphd->num_pts; j++) {
		glyphd->xcoor[j] += glyphd->xcoor[j - 1];
		glyphd->ycoor[j] += glyphd->ycoor[j - 1];
	}
	for (j = 0; j < glyphd->num_pts; j++) {
		glyphd->xcoor[j] = (fixp) (((long) (glyphd->xcoor[j])) / scale);
		glyphd->ycoor[j] = (fixp) (((long) (glyphd->ycoor[j])) / scale);
	}

	/* Deals with only a condition. This is not general at all */
	/* Additional work needed to take care of all scaling      */
	/* conditions						   */

	if(sg != NULL && ( sg->flags & WE_HAVE_AN_XY_SCALE ) && 
				sg->transform.xx==0x30000 &&
				sg->transform.yy==0x10000 ) {

		for (j = 0; j < glyphd->num_pts; j++) {
			 glyphd->xcoor[j]*=-1;
			 glyphd->xcoor[j]+=(short)sg->arg1;
		}
	}
	return TRUE;
}
static void*
get_zeroed_mem(int n) {
	void *p=(void*)malloc(n);
	if (p==NULL) {
		fprintf(stderr,"Not enough memory\n");	
		exit(-1);
	}
	memset(p,0,n);
	return p;
}
static subglyph_t*
get_subglyph(void) {
	return (subglyph_t *)get_zeroed_mem(sizeof(subglyph_t));
}

static BOOL
tt2ps_load_compound_glyph(ushort_t idx, float *dx, int size,
	ttffont_t * ttfont, tt_GlyphData_t * glyphd, subglyph_t *sg, int level) {
	short           x1, y1, x2, y2, i;
	signed long 	xx, xy, yx, yy;
	subglyph_t	*start=NULL, *temp=NULL, *current=NULL;
	FILE           *fn = ttfont->fn;
	tt_FontData_t     *fontd = ttfont->fontd;
	tt_CmapData_t     *cmapd = ttfont->cmapd;
	tt_GlyphData_t     *temp_gd=glyphd;
	int 		cnt_i=0;
	long            k,l;
	BOOL 		ret;

	if(level>MAX_COMPOUND_LEVEL) return TRUE;

	x1 = tt2ps_get16(fn);	/* xMin */
	y1 = tt2ps_get16(fn);	/* yMin */
	x2 = tt2ps_get16(fn);	/* xMax */
	y2 = tt2ps_get16(fn);	/* yMax */
	do {
		if (current==NULL) { 
			start=get_subglyph();
			start->next=NULL;
			current=start;
		} else {
			temp=get_subglyph();
			temp->next=NULL;
			current->next=temp;
			current=temp;
		}	
		current->flags = tt2ps_get16(fn);
		current->index = tt2ps_get16(fn);
		current->next = (subglyph_t *)NULL;

		k=2;
		if (current->flags & ARGS_ARE_WORDS ) {
			k += 2;
		}

		if (current->flags & WE_HAVE_A_SCALE ) {
			 k += 2;
		}

		if ( current->flags & WE_HAVE_AN_XY_SCALE ) {
			k += 4;
		}

		if ( current->flags & WE_HAVE_A_2X2 ) {
			k += 8;
		}
		
		if ( current->flags & ARGS_ARE_WORDS ) {
			k = (short) tt2ps_get16(fn);
			l = (short) tt2ps_get16(fn);
		} else {
			l = (short) tt2ps_get16(fn);
			k = ( char ) ( l >> 8);
			l = ( char ) ( l & 0xff );
		}

		current->arg1 = k;
		current->arg2 = l;

		xx = yy = 1 << 16;
		xy = yx = 0;

		if ( current->flags & WE_HAVE_A_SCALE ) {
			xx = (unsigned long)tt2ps_get16(fn) << 2;
			yy = xx;
			/*
			subglyph2->is_scaled = TRUE;
			*/
		} else if ( current->flags & WE_HAVE_AN_XY_SCALE ) {
			xx = (unsigned long)tt2ps_get16(fn) << 2;
			yy = (unsigned long)tt2ps_get16(fn) << 2;
			/*
			subglyph2->is_scaled = TRUE;
			*/
		} else if ( current->flags & WE_HAVE_A_2X2 ) {
			xx = (unsigned long)tt2ps_get16(fn) << 2;
			xy = (unsigned long)tt2ps_get16(fn) << 2;
			yx = (unsigned long)tt2ps_get16(fn) << 2;
			yy = (unsigned long)tt2ps_get16(fn) << 2;
			/*
			subglyph2->is_scaled = TRUE; 
			*/
		}	
	      current->transform.xx = xx;
	      current->transform.xy = xy;
	      current->transform.yx = yx;
	      current->transform.yy = yy;

	} while( current->flags & MORE_COMPONENTS );
		
	for(current=start;current;current=temp) {
		tt_GlyphData_t     *gd=NULL;
		cnt_i++;
		if(cnt_i>1) {
			gd=( tt_GlyphData_t *)get_zeroed_mem(sizeof( tt_GlyphData_t));	
			temp_gd->next=gd;
			gd->next=NULL;
		} else gd=glyphd;
		ret=tt2ps_load_simple_glyph(current->index, dx, size, ttfont, gd, cnt_i, current, level);
		if(ret==FALSE) return FALSE;
		temp=current->next;
		temp_gd=gd;
		free(current);
	}
	return ret;
}
static long
get_glyphd_num_ctr_offset(ushort_t idx, ttffont_t * ttfont) {
	FILE           	*fn = ttfont->fn;
	tt_CmapData_t   *cmapd = ttfont->cmapd;
	tt_FontData_t   *fontd = ttfont->fontd;
	long            off;

	if(cmapd->iTLF == 0) {
		fseek(fn, fontd->loca + (long) idx * 2L, SEEK_SET);
		off = (long)tt2ps_get16(fn);
		off *= 2;
	} else {
		fseek(fn, fontd->loca + (long) idx * 4L, SEEK_SET);
		off = tt2ps_get32(fn);
	}
	return(off);
}
/*
 * Load glyph data
 */
static BOOL
tt2ps_load_font(int k, ushort_t idx, float *dx, int size,
		ttffont_t * ttfont, tt_GlyphData_t * glyphd)
{
	long            off;
	ushort_t          j;
	uchar_t           c, ct;
	float           scale;
	FILE           *fn = ttfont->fn;
	tt_FontData_t     *fontd = ttfont->fontd;
	tt_CmapData_t   *cmapd = ttfont->cmapd;
	BOOL ret;

	off = get_glyphd_num_ctr_offset(idx, ttfont);
	if(k<cmapd->endCount[current_cmapd_seg_cnt]) {
		long next_off = get_glyphd_num_ctr_offset(idx+1, ttfont);

		/* Satisfying this condition means current character is
		 * space equivalent, width may be different.
		 */

		if(off == next_off) {
			scale = (float) cmapd->unitsPerEm / 1000.0;
			*dx = (float) fontd->hmetrics[idx].aw / cmapd->unitsPerEm * size;
			return(TRUE);
		}
	}	
	fseek(fn, fontd->glyf + off, SEEK_SET);
	glyphd->num_ctr = (short) tt2ps_get16(fn);
	if (glyphd->num_ctr < 0 )  {
		ret=tt2ps_load_compound_glyph(idx, dx, size, ttfont, glyphd, (subglyph_t*)NULL, 0);
	}  else {
		ret=tt2ps_load_simple_glyph(idx, dx, size, ttfont, glyphd, 0, (subglyph_t*)NULL,0);
		}


	return ret;
}

/*
 * Set PS Moveto data
 */
static void
tt2ps_moveto(int x, int y, tt_OutlineData_t *odata, int *point)
{
	odata->data_type[*point] = TT_MOVETO;
	odata->data[*point].moveto = (tt_MoveSegment_t *)calloc(sizeof(tt_MoveSegment_t), 1);
	odata->data[*point].moveto->x1 = x;
	odata->data[*point].moveto->y1 = y;
	(*point)++;
}

/*
 * Set PS Lineto data
 */
static void
tt2ps_lineto(int x, int y, tt_OutlineData_t *odata, int *point)
{
	odata->data_type[*point] = TT_LINETO;
	odata->data[*point].lineto = (tt_LineSegment_t *)calloc(sizeof(tt_LineSegment_t), 1);
	odata->data[*point].lineto->x1 = x;
	odata->data[*point].lineto->y1 = y;
	(*point)++;
}

/*
 * Set PS Curveto data
 * Convert TrueType curve data to PS curve data in this routine
 */
static void
tt2ps_curveto(fixp x, fixp y, int s, int t, tt_GlyphData_t * glyphd, tt_OutlineData_t *odata, int *point)
{
	int             N, i;
	double          sx[3], sy[3], cx[4], cy[4];

	N = t - s + 2;


	for (i = 0; i < N - 1; i++) {
		odata->data_type[*point] = TT_CURVETO;
		odata->data[*point].curveto = 
			(tt_CurveSegment_t *)calloc(sizeof(tt_CurveSegment_t), 1);

		sx[0] = i == 0 ? glyphd->xcoor[s - 1] : (glyphd->xcoor[i + s] + glyphd->xcoor[i + s - 1]) / 2;
		sy[0] = i == 0 ? glyphd->ycoor[s - 1] : (glyphd->ycoor[i + s] + glyphd->ycoor[i + s - 1]) / 2;
		sx[1] = glyphd->xcoor[s + i];
		sy[1] = glyphd->ycoor[s + i];
		sx[2] = i == N - 2 ? x : (glyphd->xcoor[s + i] + glyphd->xcoor[s + i + 1]) / 2;
		sy[2] = i == N - 2 ? y : (glyphd->ycoor[s + i] + glyphd->ycoor[s + i + 1]) / 2;
		cx[3] = sx[2];
		cy[3] = sy[2];
		cx[1] = (2 * sx[1] + sx[0]) / 3;
		cy[1] = (2 * sy[1] + sy[0]) / 3;
		cx[2] = (sx[2] + 2 * sx[1]) / 3;
		cy[2] = (sy[2] + 2 * sy[1]) / 3; 
		odata->data[*point].curveto->x1 = cx[1]; 
		odata->data[*point].curveto->y1 = cy[1]; 
		odata->data[*point].curveto->x2 = cx[2];
		odata->data[*point].curveto->y2 = cy[2];
		odata->data[*point].curveto->x3 = cx[3];
		odata->data[*point].curveto->y3 = cy[3];

		(*point)++;
	}
}

/*
 * Clear tt_OutlineData_t struct
 */
void
tt2ps_clear_outlinedata ( tt_OutlineData_t *odata)
{
	int	i;
	tt_OutlineData_t *temp;
	while(odata) {
		for(i = 0; i < odata->count; i++){
			switch (odata->data_type[i]) {
			case TT_MOVETO:
				free(odata->data[i].moveto);
				break;
			case TT_LINETO:
				free(odata->data[i].lineto);
				break;
			case TT_CURVETO:
				free(odata->data[i].curveto);
				break;
			}
		}
		free(odata->data_type);
		free(odata->data);
		temp=odata->next;
		free(odata);
		odata=temp;
	}
}

void
tt_ps_glyph_start(int k, int scale, FILE *fp)
{
	fprintf(fp, "/U%x {\nCT\n%.3f %.3f S\nN\n",
      	(unsigned int)k, 0.001 * scale, 0.001 * scale);
}

void
tt_ps_glyph_end(int k, FILE *fp)
{
	fprintf(fp, "U%x\n\n", (unsigned int)k);
}

void
tt_ps_glyph_print(tt_OutlineData_t *odata, FILE *fp)
{
	int	i;

	while(odata) {
		for(i = 0; i < odata->count; i++) {
			switch (odata->data_type[i]) {
			case TT_MOVETO:
				fprintf(fp, "%d %d M\n", 
					odata->data[i].moveto->x1, 
					odata->data[i].moveto->y1);
				break;
			case TT_LINETO:
				fprintf(fp, "%d %d L\n", 
					odata->data[i].lineto->x1, 
					odata->data[i].lineto->y1);
				break;
			case TT_CURVETO:
				fprintf(fp, "%.0f %.0f %.0f %.0f %.0f %.0f C\n",
					odata->data[i].curveto->x1,
					odata->data[i].curveto->y1,
					odata->data[i].curveto->x2,
					odata->data[i].curveto->y2,
					odata->data[i].curveto->x3,
					odata->data[i].curveto->y3);
				break;
			}
		}
		odata=odata->next;
	} 
	fprintf(fp, "CR } def\n"); 
}


/* 
 * Convert code data to Glyph index
 */
static ushort_t
tt2ps_font_index(int k, ttffont_t * ttfont)
{
	int             j;
	ushort_t          idx = 0xffff;
	ushort_t          ridx;
	short           delta;
	ushort_t          offset;
	FILE           *fn = ttfont->fn;
	tt_CmapData_t     *cmapd = ttfont->cmapd;
	for (j = 0; j < cmapd->segCount; j++) {
		if (k >= cmapd->startCount[j] && k <= cmapd->endCount[j]) {
			offset = cmapd->idRangeOffset[j];
			if (offset == 0) {
				delta = (short)cmapd->idDelta[j];
				idx = (ushort_t) (k + delta);
			} else {
				ridx = offset / 2 + (k - cmapd->startCount[j]) - cmapd->segCount + j;

				fseek(fn, cmapd->gryphIdArray + ridx * 2L, SEEK_SET);
				idx = tt2ps_get16(fn);
			}
			break;
		}
	}
	current_cmapd_seg_cnt = j;
	return idx;
}

/*
 * Clear tt_GlyphData_t struct
 */
static void
tt2ps_clear_glyphdata(tt_GlyphData_t * glyphd)
{
	free(glyphd->epts_ctr);
	free(glyphd->flag);
	free(glyphd->xcoor);
	free(glyphd->ycoor);
	free(glyphd);
}

/*
 * Convert TrueType outline data to PS outline data
 * Return Value
 *	 1 = Success
 *	-1 = Glyph Not Found
 *	 0 = Composite Font
 */
int
tt2ps_get_psdata(int k, int scale, ttffont_t * ttfont, tt_OutlineData_t * odata)
{
	ushort_t          cidx,cnt_i;
	short           i;
	int             j, start_offpt, end_offpt, fst, datapoint = 0;
	float           dx;
	tt_OutlineData_t *start=NULL, *temp=NULL;
	tt_GlyphData_t     *glyphd=( tt_GlyphData_t *)get_zeroed_mem(sizeof( tt_GlyphData_t));
	tt_GlyphData_t     *tgp;

	cidx = tt2ps_font_index(k, ttfont);
	if (cidx == 0xffff) {
		tt2ps_clear_glyphdata(glyphd);
		return -1;
	}
	if (tt2ps_load_font(k, cidx, &dx, scale, ttfont, glyphd) == FALSE) {
		/*
		 * Font error 
		 */
		tt2ps_clear_glyphdata(glyphd);
		return 0;
	}

	tgp=glyphd;
	cnt_i=0;
	odata->next=NULL;
	while(glyphd) {
		cnt_i++;
		datapoint = 0;
		for (i = 0, j = 0; i < glyphd->num_ctr; i++) {
			datapoint++;
			start_offpt = 0;
			for (j++; j <= glyphd->epts_ctr[i]; j++) {
				if (!(glyphd->flag[j] & 1)) {	/* Off curve */
					if (!start_offpt) {
						start_offpt = end_offpt = j;
					} else
						end_offpt++;
				} else {/* On Curve */
					if (start_offpt) {
						datapoint+=(end_offpt - start_offpt+2);
					} else {
						datapoint++;
					}
				}
			}
			if (start_offpt) {
				datapoint+=(end_offpt - start_offpt+2);
			} else {
				datapoint++;
			}
		}
		if (cnt_i!=1){
			temp = (tt_OutlineData_t *)calloc(sizeof(tt_OutlineData_t), 1);
			temp->next=NULL;
			odata->next=temp;
			odata=temp;
		} else if (cnt_i==1) start=odata;
		odata->count = datapoint;

		odata->data_type = (int *)calloc(sizeof(int), odata->count);
		odata->data = (tt_OutlineSegment_t *)calloc(sizeof(tt_OutlineSegment_t),
				odata->count);

		datapoint = 0;
		for (i = 0, j = 0; i < glyphd->num_ctr; i++) {
			fst = j;
			tt2ps_moveto(glyphd->xcoor[j], glyphd->ycoor[j], 
				odata, &datapoint);
			start_offpt = 0;

			for (j++; j <= glyphd->epts_ctr[i]; j++) {
				if (!(glyphd->flag[j] & 1)) {	/* Off curve */
					if (!start_offpt) {
						start_offpt = end_offpt = j;
					} else
						end_offpt++;
				} else {/* On Curve */
					if (start_offpt) {
						tt2ps_curveto(glyphd->xcoor[j], 
							glyphd->ycoor[j], start_offpt, 
							end_offpt, glyphd,
							odata, &datapoint);
						start_offpt = 0;
					} else {
						tt2ps_lineto(glyphd->xcoor[j], 
							glyphd->ycoor[j],
							odata, &datapoint);
					}
				}
			}

			if (start_offpt) {
				tt2ps_curveto(glyphd->xcoor[fst], glyphd->ycoor[fst], 
					start_offpt, end_offpt, glyphd,
					odata, &datapoint);
			} else {
				tt2ps_lineto(glyphd->xcoor[fst], glyphd->ycoor[fst],
					odata, &datapoint);
			}
		}

		odata->xsize = dx;
		tgp = glyphd->next;
		tt2ps_clear_glyphdata(glyphd);
		glyphd=tgp;
	}
	odata=start;
	return 1;
}
