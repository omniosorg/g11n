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
 * Copyright (c) 2000 by Sun Microsystems, Inc.
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
#pragma ident "@(#)xpr_print_line.c	1.2	00/10/05	SMI"

#include <wchar.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include "xpr_print_line.h"
#include "general_header.h"

#define CTL_MAX_BUF_SIZE  1024	/* we are only passing ~80 chars per line */
#define MAX_CHAR_LENGTH         10
#define MAX_PLANES         32
#define TWO_BYTE_FONT(f)        (( (f)->min_byte1 != 0 || (f)->max_byte1 != 0))
#define IS2BYTE_FONT(n)		(n==2)
#define True 1
#define False 0

typedef char Boolean;

extern int shapecharsetsize;
static MultiFontTable *XmMultiFont = NULL;

static void InitMultiFontTable() {
    int i = 0;

    if (XmMultiFont == NULL) {

	XmMultiFont = (MultiFontTable*)XtMalloc(sizeof(MultiFontTable));
	for (i = 0; i < MAX_PLANES; i++) {
	    XmMultiFont->MultiFontRec[i].fid = 0;
	    XmMultiFont->MultiFontRec[i].fname = NULL;
	}
	XmMultiFont->num_entries = 0;
    }
}

/*
 * Add an entry into the MultiFont table
*/
static int
mtable_addentry(mfontRec mfRec)
{
   int i=0;
   if ((mfRec.fname == NULL) || (strcmp(mfRec.fname, "") == 0))
        return 0;

   else if (XmMultiFont->num_entries == (MAX_PLANES - 1)) /* Exceed no of planes */
        return 0;

   else {
       for (i = 0; i < XmMultiFont->num_entries; i++)
	   if (XmMultiFont->MultiFontRec[i].fid == mfRec.fid) return 1;
       XmMultiFont->MultiFontRec[i].fid = mfRec.fid;
       XmMultiFont->MultiFontRec[i].fname = mfRec.fname;
       XmMultiFont->num_entries++;
   }
   return 1;
}

int
parseShape(char *modifier)
{
#define MAX_BUF_LEN 200
   char		str_buf[MAX_BUF_LEN];
   int		i = 0, result = True;
   char		*tmpStr = NULL;
   Boolean	bufhasequal = True;
   mfontRec	mfRec;

   InitMultiFontTable();

   if (modifier)
	tmpStr = (char *)strdup(modifier);
   else
	return 0;

   i = 0;
   while ((tmpStr) && (*tmpStr != '\0')) {

	if (isspace(*tmpStr))
		tmpStr++;

	else if (*tmpStr == '"')
		tmpStr++;

	else if ((*tmpStr == '=') || (*tmpStr == ';')) {

	    if (*tmpStr == '=')
		bufhasequal = True;
	    else
		bufhasequal = False;

	    str_buf[i] = '\0';

	    if (bufhasequal == True) { /* Store string as encoding */
		mfRec.fname = (char *)strdup(str_buf);
		bufhasequal = False;
	    }
	    else { /* Store string as fontid */
		sscanf(str_buf, "%X", &mfRec.fid);
		result = mtable_addentry(mfRec);
            }

	    i = 0;
	    str_buf[i] = '\0';
	    tmpStr++;
        }

        else {
                str_buf[i] = *tmpStr;
                i++;
		tmpStr++;
        }
   }
   return result;
}
static int ISUTF8_MFONT() {
   if (XmMultiFont != NULL) 
   	if (XmMultiFont->num_entries > 1) return 1;
   return 0;
}


static int
is_a_mb_ctl_char( unsigned char *text, int pos)
{
   int		i = 0;
   unsigned int	tst_id = 0;
   if (text == NULL)
	tst_id = 0;
   else
	tst_id = (0xFFFF & text[pos]) | (0xFFFF & text[pos+1]);
	/*
       tst_id = ((((unsigned int)text[pos] << 8) & 0xFF) | (text[pos+1] & 0xFF));
       */
   if (tst_id <= 0)
	return 0;

   for (i = 0; i < XmMultiFont->num_entries; i++) {
       if (tst_id == XmMultiFont->MultiFontRec[i].fid)
	    return 1;
	}
   return 0;
} 

static XFontStruct*
look_for_font(XFontSet fontset, unsigned int tst_id)
{
   int		i, j, found = 0, num_fonts = 0;
   char		**fname_list, *substr = NULL;
   char s[20];
   XFontStruct	**fstruct_list;
   for (i = 0; ((i < XmMultiFont->num_entries) && (found == 0)); i++) {

        if (tst_id == XmMultiFont->MultiFontRec[i].fid) {

	   num_fonts = XFontsOfFontSet(fontset, &fstruct_list, &fname_list);

	   if (num_fonts <= 0)
		fprintf(stderr,catgets(cat_fd, ERR_SET, 52, "%s: No fonts in "
		"fontset\n"), progname);

	   else {
	      for (j = 0; j < num_fonts; j++) {
		 substr = (char *)strstr(fname_list[j], XmMultiFont->MultiFontRec[i].fname);
		 if (substr != NULL) {
		    substr = NULL;
		    return (fstruct_list[j]);
		 }
	      }
	   }
        }
   }

   if (found == 0)
	return((XFontStruct*)NULL);
}
/*
 * Uses the first two bytes to obtain a font id, then returns appropriate
 * font.
*/
static XFontStruct*
get_mb_ctl_mfont(XFontSet fontset, unsigned char *text, int pos)
{
   unsigned int tst_id = 0;
   
   if (text == NULL)
	tst_id = 0;
   else
	tst_id = (0xFFFF & text[pos]) | (0xFFFF & text[pos+1]);
   if (tst_id <= 0)
	return((XFontStruct*)NULL);
   else
	return (look_for_font(fontset, tst_id));
}
int ctl_draw_string(Display *display, Drawable d, XFontSet fontset, GC gc,
	int x, int y, unsigned char *text, int bytes_text) {

	int ret_extent;
	int max_height=0;
	XRectangle   oir,olr;

	if (bytes_text <= 0)
		return 0;
	if (ISUTF8_MFONT()) { /* UTF8 */
    		int ct;
		int tmp_xtent = 0;

		for (ct = 0; ct < bytes_text; ct++) {
			if (is_a_mb_ctl_char(text, ct)) {
				XFontStruct	*tmp_fstr = NULL;
				int dir = 0, fnt_ascent = 0, fnt_descent = 0;
				XCharStruct   overall_return;

				tmp_fstr = get_mb_ctl_mfont(fontset, text, ct);

				ct += 2; 
				if (tmp_fstr) {

					XGCValues	values;
					Font		orig_gc_font;
					int           dir = 0, fnt_ascent = 0, fnt_descent = 0;
					XCharStruct   overall_return;

					XGetGCValues(display, gc, GCFont, &values);
					orig_gc_font = values.font;
					XSetFont(display, gc, tmp_fstr->fid);
					if (TWO_BYTE_FONT(tmp_fstr)) {
		    				XDrawString16(display, d, gc, x, y, (XChar2b*)(text + ct), 1);
		    				x += XTextWidth16(tmp_fstr, (XChar2b*)(text + ct), 1);
						XTextExtents16(tmp_fstr, (XChar2b*)(text + ct), 1, &dir,  &fnt_ascent, &fnt_descent, &overall_return);
					} else {
		  				XDrawString(display, d, gc, x, y, (char*)(text + ct + 1), 1);
		  				x += XTextWidth(tmp_fstr, (char *)(text + ct + 1), 1);
						XTextExtents(tmp_fstr, (char*)(text + ct), 1, &dir,  &fnt_ascent, &fnt_descent, &overall_return);
					}
					values.font = orig_gc_font;
					XChangeGC(display, gc, GCFont, &values);
				if(max_height< (fnt_ascent+fnt_descent))
					max_height=fnt_ascent+fnt_descent;
				}
				ct += 2;
			} else { /* Not a CTL character */
				XRectangle ink_ret, logic_ret;	
				wchar_t    byte1, byte2, byte3, byte4, to_draw_wc;
				byte4 = 0x00FFFFFF | (text[ct] << 24);
				byte3 = 0xFF00FFFF | (text[ct+1] << 16);
				byte2 = 0xFFFF00FF | (text[ct+2] << 8);
				byte1 = 0xFFFFFF00 | text[ct+3];
				to_draw_wc = byte4 & byte3 & byte2 & byte1;

				XwcDrawString(display, d, fontset, gc, x, y, &to_draw_wc, 1);
				x += XwcTextExtents(fontset,&to_draw_wc,1,&ink_ret,&logic_ret);
				if (logic_ret.height>max_height) max_height=logic_ret.height;
				ct += 4;
			}
			ct -=1; /* Negate the effect of for loop increment */
		}
	} else if (IS2BYTE_FONT(shapecharsetsize)) {
		int	 fonts_in_fs;
		char	 **font_name_list;
		XFontStruct **font_struct_list;
		int	 dir, font_ascent, font_descent;
		XCharStruct overall_return;

		fonts_in_fs = XFontsOfFontSet(fontset, &font_struct_list, &font_name_list);

		if (fonts_in_fs <= 0)
			fprintf(stderr,catgets(cat_fd, ERR_SET, 52, "%s: No fonts in "
			"fontset\n"), progname);
     		else {  /* Note: we are not handling num_fonts_in_fontset > 1 */

			/* save the font in GC and restore it after drawing if needed */
			Font		orig_gc_font;
			XGCValues	values;
			unsigned long	valuemask;
			XFontStruct	*temp_font_struct;
			Boolean		is_font_already_set;

			valuemask = GCFont; /* Get the old GC font id */
			XGetGCValues(display, gc, valuemask, &values);
			orig_gc_font = values.font;
			XSetFont(display, gc, (*font_struct_list)->fid); /* Set new font id */
			XDrawString16(display, d, gc, x, y, (XChar2b*) text, bytes_text / 2); 
			XmbTextExtents(fontset,(char *)text,bytes_text / 2,&oir,&olr);
			if (olr.height>max_height) max_height=olr.height;
			/* restore the font id in the gc if the gc had a proper font earlier */
			temp_font_struct = XQueryFont(display, orig_gc_font);
			is_font_already_set = !(temp_font_struct == NULL);
			if (is_font_already_set) { /* Set the new font id */

				values.font = orig_gc_font;
				valuemask = GCFont;
				XChangeGC(display, gc, valuemask, &values);
			}
			else ;/* forget about restoring it */
			}
	} else /* 1 byte font */ {
		int i;	
		XmbDrawString(display, d, fontset, gc, x, y, (char *)text,bytes_text);
		XmbTextExtents(fontset,(char *)text,bytes_text,&oir,&olr);
		if (olr.height>max_height) max_height=olr.height;
	}

  	return max_height;
}
