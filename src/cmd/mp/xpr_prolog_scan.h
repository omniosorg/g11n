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
#ifndef _XPR_PROLOG_SCAN
#define _XPR_PROLOG_SCAN

#pragma ident "@(#)xpr_prolog_scan.h	1.3	00/10/09	SMI"

#define DEFHDGSIZE	"120"
#define DEFBDYSIZE	"90"
#define STRTXDEF	135
#define STRTYDEF	280
#define DEFYBOUND	3005
#define PROLOGDPIDEFAULT	300

typedef struct line_struct  {
	int x, y, x1, y1;
	struct line_struct *next;
} line_tag;

typedef struct arc_struct {
	int x, y, angle1, angle2;
	uint_t width, height;
	struct arc_struct *next;
} arc_tag;

typedef struct draw_struct {
	int usr_str_x, usr_str_y;
	int tm_str_x, tm_str_y;
	int pg_str_x, pg_str_y;
	int sb_str_x, sb_str_y;
	arc_tag *draw_arc;
	line_tag *draw_line;
} draw_decor;

typedef struct prolog_scan_data {
	int orientation;	
	int page_len;	
	int line_len;	
	int num_cols;
	char *extra_hdng_font;
	char *extra_body_font;
	char *hdng_font_size;	
	char *body_font_size;
	int prolog_dpi;
	int y_bound;
	int move_x, move_y;
	int text_x, text_y;
	int page_str;
	int draw_coords;
	draw_decor *draw_page;
	draw_decor *draw_col;
	draw_decor *draw_forced_col;
	draw_decor *draw_forced_page;
} prolog_data;
extern int read_config_file(char *filename, prolog_data **pd);

#endif /* _XPR_PROLOG_SCAN */
