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
#pragma ident "@(#)xpr_prolog_scan.c	1.2	00/10/05	SMI"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>


#include "xpr_prolog_scan.h"
#include "general_header.h"

#define STRTCOMN	"STARTCOMMON"
#define ENDCOMN		"ENDCOMMON"
#define ORIENTATION	"ORIENTATION"
#define PAGELEN		"PAGELENGTH"
#define LINELEN		"LINELENGTH"
#define NUMCOLS		"NUMCOLS"
#define EXHDNGFNT	"EXTRAHDNGFONT"
#define EXBODYFNT	"EXTRABODYFONT"
#define HDGSIZE		"HDNGFONTSIZE"
#define BDYSIZE		"BODYFONTSIZE"
#define DRWCOORDS	"DRAWABLECOORDS"
#define STRTTXT		"STARTTEXT"
#define YTEXTBOUND	"YTEXTBOUNDARY"
#define PRODPI		"PROLOGDPI"
#define MOVEX		"XDISPLACEMENT"
#define MOVEY		"YDISPLACEMENT"
#define PAGESTR		"PAGESTRING"
#define STRTPGE		"STARTPAGE"
#define ENDPGE		"ENDPAGE"
#define STRTCOL		"STARTCOLUMN"
#define ENDCOL		"ENDCOLUMN"
#define USRSTRPOS	"USERSTRINGPOS"
#define TMSTRPOS	"TIMESTRINGPOS"
#define PGSTRPOS	"PAGESTRINGPOS"
#define SBSTRPOS	"SUBJECTSTRINGPOS"
#define LINE		"LINE"
#define ARC		"ARC"
#define STRTFORCEDCOL	"STARTFORCEDCOLUMN"
#define ENDFORCEDCOL	"ENDFORCEDCOLUMN"
#define STRTFORCEDPGE	"STARTFORCEDPAGE"
#define ENDFORCEDPGE	"ENDFORCEDPAGE"

char *format_file=NULL;

int read_config_file(char *filename, prolog_data **pd);

static void err_missing_fields(int line_cnt) ;
static void err_non_unsigned_int(int line_cnt) ;
static void flat_scan(FILE *fp) ;
static FILE *read_drawables(FILE *fp, int line_cnt, draw_decor *, char *) ;
static FILE *read_props(FILE *fp, int line_cnt) ;
static int isdigitstr ( char *s) ;
static line_tag* get_line(void) ;
static arc_tag* get_arc(void) ;
static void check_tok_n_save(char *t, int param_pos, int line_cnt, char *desc) ;
static void arc_save(int param_pos, char *t) ;
static void line_save(int param_pos, char *t) ;
static void read_and_fill(char *format_file) ;
static void init_prolog_data(prolog_data *spd);


static prolog_data scanned_prolog_data;
static draw_decor dr_pg, dr_col, dr_forced_page, dr_forced_col;
static draw_decor *current_draw_decor=NULL;
static line_tag *temp_line_ptr=NULL, *start_line_ptr=NULL, \
	*current_line_ptr = NULL;
static arc_tag *temp_arc_ptr=NULL, *start_arc_ptr=NULL, \
	*current_arc_ptr=NULL;

/* external interface, filename = pointer fo filename path 		*/
/* pd = prolog_data struct pointer, which will be filled in with the 	*/
/* values 								*/

int read_config_file(char *filename, prolog_data **pd) {
	format_file=filename;
	init_prolog_data(&scanned_prolog_data);
	read_and_fill(filename);	
	*pd=&scanned_prolog_data;
	return 0;
}

static void err_non_unsigned_int(int line_cnt) {
	err_exit(catgets(cat_fd,ERR_SET, 54, "%s: confile file:%s line:%d; "
	"non-unsigned integer in  unsigned integer field.\n"), progname, format_file, line_cnt);
}

static void 
err_non_signednumeic(int line) {
	err_exit( catgets( cat_fd, ERR_SET, 58,
			"%s: config file:%s line: %d; "
			"expected value is signed/unsigned integer.\n"), 
			progname, format_file, line);
}
static void err_missing_fields(int line_cnt) {
	err_exit(catgets(cat_fd,ERR_SET, 55,"%s: confile file:%s line:%d; "
	"missing field(s).\n"), progname, format_file, line_cnt);
}

static int issigneddigitstr ( char *s) {
	uchar_t c;

	c=*s++;
	if(c=='-' || c=='+' || isdigit(c)) {
		while(c = *s++) {
			if (!isdigit(c))		
				return 0;
		}
	}
	return 1;
}
static int isdigitstr ( char *s) {
	uchar_t c;

	while(c = *s++) {
		if (!isdigit(c))		
			return 0;
	}
	return 1;
}
static line_tag* get_line(void) {
	line_tag *line_p;
	line_p=(line_tag *)malloc(sizeof(line_tag));
	if(line_p==NULL)
		err_exit(catgets(cat_fd,ERR_SET, 25, 
		"%s: Malloc failure; line %d, file "
		"%s\n"),progname, __LINE__, __FILE__ );
	return line_p;
}
static arc_tag* get_arc(void) {
	arc_tag* arc_p;
	arc_p=(arc_tag *)malloc(sizeof(arc_tag));
	if(arc_p==NULL)
		err_exit(catgets(cat_fd,ERR_SET, 25, 
		"%s: Malloc failure; line %d, file "
		"%s\n"),progname, __LINE__, __FILE__ );
	return arc_p;
}

static void line_save(int param_pos, char *t) {
	if(param_pos==1)
		current_line_ptr->x=atoi(t);
	else if(param_pos==2)
		current_line_ptr->y=atoi(t);
	else if(param_pos==3)
		current_line_ptr->x1=atoi(t);
	else if(param_pos==4)
		current_line_ptr->y1=atoi(t);
}

static void arc_save(int param_pos, char *t) {
	if(param_pos==1)
		current_arc_ptr->x=atoi(t);
	else if(param_pos==2)
		current_arc_ptr->y=atoi(t);
	else if(param_pos==3)
		current_arc_ptr->width=atoi(t);
	else if(param_pos==4)
		current_arc_ptr->height=atoi(t);
	else if(param_pos==5)
		current_arc_ptr->angle1=atoi(t);
	else if(param_pos==6)
		current_arc_ptr->angle2=atoi(t);
}
static void check_strng_int(char *t, int line_cnt, char *desc) {
	if(strcmp(desc,MOVEX)==0 || strcmp(desc,MOVEY)==0) {
		if (!issigneddigitstr(t)) {
			err_non_signednumeic(line_cnt);
		}
	} else if(!(strcmp(desc,EXHDNGFNT)==0 || strcmp(desc,EXBODYFNT)==0)) {
		if (!isdigitstr(t)) {
			err_non_unsigned_int(line_cnt);
		}
	}
}

static void check_tok_n_save(char *t, int param_pos, int line_cnt, char *desc) {
	if(t== NULL) {
		err_missing_fields(line_cnt);
	} else 
		check_strng_int(t, line_cnt, desc);
	errno=0;
	if(strcmp(desc,LINE)==0)
		line_save(param_pos, t);
	else if(strcmp(desc,ARC)==0)
		arc_save(param_pos, t);
	else if(strcmp(desc,ORIENTATION)==0)
		scanned_prolog_data.orientation=atoi(t);
	else if(strcmp(desc,PAGELEN)==0)
		scanned_prolog_data.page_len=atoi(t);
	else if(strcmp(desc,LINELEN)==0)
		scanned_prolog_data.line_len=atoi(t);
	else if(strcmp(desc,NUMCOLS)==0)
		scanned_prolog_data.num_cols=atoi(t);
	else if(strcmp(desc,HDGSIZE)==0) 
		scanned_prolog_data.hdng_font_size=strdup(t);
	else if(strcmp(desc,BDYSIZE)==0)
		scanned_prolog_data.body_font_size=strdup(t);
	else if(strcmp(desc,EXHDNGFNT)==0) 
		scanned_prolog_data.extra_hdng_font=strdup(t);
	else if(strcmp(desc,EXBODYFNT)==0)
		scanned_prolog_data.extra_body_font=strdup(t);
	else if(strcmp(desc,DRWCOORDS)==0)
		scanned_prolog_data.draw_coords=atoi(t);
	else if(strcmp(desc,PRODPI)==0)
		scanned_prolog_data.prolog_dpi=atoi(t);
	else if(strcmp(desc,MOVEX)==0)
		scanned_prolog_data.move_x=atoi(t);
	else if(strcmp(desc,MOVEY)==0)
		scanned_prolog_data.move_y=atoi(t);
	else if(strcmp(desc,PAGESTR)==0)
		scanned_prolog_data.page_str=atoi(t);
	else if((strcmp(desc,STRTTXT)==0) && param_pos==1 )
		scanned_prolog_data.text_x=atoi(t);
	else if(strcmp(desc,STRTTXT)==0 && param_pos==2 )
		scanned_prolog_data.text_y=atoi(t);
	else if(strcmp(desc,YTEXTBOUND)==0) 
		scanned_prolog_data.y_bound=atoi(t);
	else if((strcmp(desc,USRSTRPOS)==0) && param_pos==1 ) 
		current_draw_decor->usr_str_x=atoi(t);
	else if(strcmp(desc,USRSTRPOS)==0 && param_pos==2 )
		current_draw_decor->usr_str_y=atoi(t);
	else if(strcmp(desc,TMSTRPOS)==0 && param_pos==1 )
		current_draw_decor->tm_str_x=atoi(t);
	else if(strcmp(desc,TMSTRPOS)==0 && param_pos==2 )
		current_draw_decor->tm_str_y=atoi(t);
	else if(strcmp(desc,PGSTRPOS)==0 && param_pos==1 )
		current_draw_decor->pg_str_x=atoi(t);
	else if(strcmp(desc,PGSTRPOS)==0 && param_pos==2 )
		current_draw_decor->pg_str_y=atoi(t);
	else if(strcmp(desc,SBSTRPOS)==0 && param_pos==1 )
		current_draw_decor->sb_str_x=atoi(t);
	else if(strcmp(desc,SBSTRPOS)==0 && param_pos==2 )
		current_draw_decor->sb_str_y=atoi(t);
	if(errno) {
		err_non_unsigned_int(line_cnt) ;
	}
}

static FILE *read_drawables(FILE *fp, int line_cnt, draw_decor *dd, char *endcond) {
	char buf[MAXLINE];
	char *t;
	static int counter=0;

	start_line_ptr=current_line_ptr=temp_line_ptr=NULL;
	start_arc_ptr=current_arc_ptr=temp_arc_ptr=NULL;
	current_draw_decor=dd;

	while(fgets(buf, sizeof(buf), fp)) {
		counter++;
		line_cnt++;
		if (!ends_with_NL(buf)) {
			fprintf(stderr,catgets(cat_fd, ERR_SET, 32,
			"%s: file: %s, line:%d too long\n"),
			progname, format_file, line_cnt);
		}
		zapNL(buf);
		if((t=strtok(buf,CFFS_STR)) == NULL) {
			continue;
		} else if (is_comment_char(*t)) {
			continue;
		} else if (eq(t, endcond)) {
			if(counter==1 && !strcmp(endcond,ENDPGE)) {
				scanned_prolog_data.draw_page=NULL;
			}
			if(counter==1 && !strcmp(endcond,ENDCOL)) {
				scanned_prolog_data.draw_col=NULL;
			}
			if(counter==1 && !strcmp(endcond,ENDFORCEDCOL)) {
				scanned_prolog_data.draw_forced_col=NULL;
			}
			if(counter==1 && !strcmp(endcond,ENDFORCEDPGE)) {
				scanned_prolog_data.draw_forced_page=NULL;
			}
			return fp;
		} else if (eq(t, LINE)) {
			if(start_line_ptr==NULL) {
				start_line_ptr=get_line();
				dd->draw_line=start_line_ptr;
				current_line_ptr=start_line_ptr;
				current_line_ptr->next=NULL;
			} else {
				temp_line_ptr=get_line();
				temp_line_ptr->next=NULL;
				current_line_ptr->next=temp_line_ptr;
				current_line_ptr=temp_line_ptr;
			}
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, LINE);
			check_tok_n_save((t= (char *)next_tok()), 2, line_cnt, LINE);
			check_tok_n_save((t= (char *)next_tok()), 3, line_cnt, LINE);
			check_tok_n_save((t= (char *)next_tok()), 4, line_cnt, LINE);
		} else if (eq(t, ARC)) {
			if(start_arc_ptr==NULL) {
				start_arc_ptr=get_arc();
				dd->draw_arc=start_arc_ptr;
				current_arc_ptr=start_arc_ptr;
				current_arc_ptr->next=NULL;
			} else {
				temp_arc_ptr=get_arc();
				temp_arc_ptr->next=NULL;
				current_arc_ptr->next=temp_arc_ptr;
				current_arc_ptr=temp_arc_ptr;
			}
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, ARC);
			check_tok_n_save((t= (char *)next_tok()), 2, line_cnt, ARC);
			check_tok_n_save((t= (char *)next_tok()), 3, line_cnt, ARC);
			check_tok_n_save((t= (char *)next_tok()), 4, line_cnt, ARC);
			check_tok_n_save((t= (char *)next_tok()), 5, line_cnt, ARC);
			check_tok_n_save((t= (char *)next_tok()), 6, line_cnt, ARC);
		} else if (eq(t, USRSTRPOS)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			USRSTRPOS);
			check_tok_n_save((t= (char *)next_tok()), 2, line_cnt, \
			USRSTRPOS);
		} else if (eq(t, TMSTRPOS)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			TMSTRPOS);
			check_tok_n_save((t= (char *)next_tok()), 2, line_cnt, \
			TMSTRPOS);
		} else if (eq(t, PGSTRPOS)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			PGSTRPOS);
			check_tok_n_save((t= (char *)next_tok()), 2, line_cnt, \
			PGSTRPOS);
		} else if (eq(t, SBSTRPOS)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			SBSTRPOS);
			check_tok_n_save((t= (char *)next_tok()), 2, line_cnt, \
			SBSTRPOS);
		} else {
			err_exit(catgets(cat_fd, ERR_SET, 57, "%s: Unrecognized"
			" keyword %s in config. file %s\n"), 
			progname, t, format_file);
		}
	}
}

static FILE *read_props(FILE *fp, int line_cnt) {
	char buf[MAXLINE];
	char *t;

	while(fgets(buf, sizeof(buf), fp)) {
		line_cnt++;
		if (!ends_with_NL(buf)) {
			fprintf(stderr,catgets(cat_fd, ERR_SET, 32,
			"%s: file: %s, line:%d too long\n"),
			progname, format_file, line_cnt);
		}
		zapNL(buf);
		if((t=strtok(buf,CFFS_STR)) == NULL) {
			continue;
		} else if (is_comment_char(*t)) {
			continue;
		} else if (eq(t, ENDCOMN)) {
			return fp;
		} else if (eq(t, ORIENTATION)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			ORIENTATION);
		} else if (eq(t, PAGELEN)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			PAGELEN);
		} else if (eq(t, LINELEN)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			LINELEN);
		} else if (eq(t, NUMCOLS)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			NUMCOLS);
		} else if (eq(t, PRODPI)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			PRODPI);
		} else if (eq(t, YTEXTBOUND)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			YTEXTBOUND);
		} else if (eq(t, MOVEX)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			MOVEX);
		} else if (eq(t, MOVEY)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			MOVEY);
		} else if (eq(t, PAGESTR)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			PAGESTR);
		} else if (eq(t, STRTTXT)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			STRTTXT);
			check_tok_n_save((t= (char *)next_tok()), 2, line_cnt, \
			STRTTXT);
		} else if (eq(t, DRWCOORDS)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			DRWCOORDS);
		} else if (eq(t, HDGSIZE)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			HDGSIZE);
		} else if (eq(t, BDYSIZE)) {
			check_tok_n_save((t= (char *)next_tok()), 1, line_cnt, \
			BDYSIZE);
		} else if (eq(t, EXHDNGFNT)) {
			check_tok_n_save((t= (char *)next_string_tok()), 1, line_cnt, \
			EXHDNGFNT);
		} else if (eq(t, EXBODYFNT)) {
			check_tok_n_save((t= (char *)next_string_tok()), 1, line_cnt, \
			EXBODYFNT);
		} else {
			err_exit(catgets(cat_fd, ERR_SET, 57, "%s: Unrecognized"
			" keyword %s in config. file %s\n"), 
			progname, t, format_file);
		}
	}
}

static void flat_scan(FILE *fp) {
	char buf[MAXLINE];
	char *t;
	int line_cnt=0;

	rewind(fp);
	while(fgets(buf, sizeof(buf), fp)) {
		line_cnt++;
		if (!ends_with_NL(buf)) {
			fprintf(stderr,catgets(cat_fd, ERR_SET, 32,
			"%s: file: %s, line:%d too long\n"),
			progname, format_file, line_cnt);
		}
		zapNL(buf);
		if((t=strtok(buf,CFFS_STR)) == NULL) {
			continue;
		} else if (is_comment_char(*t)) {
			continue;
		} else if (eq(t, STRTCOMN)) {
			fp=read_props(fp, line_cnt);
			continue;
		} else if (eq(t, STRTFORCEDPGE)) {
			fp=read_drawables(fp, line_cnt, scanned_prolog_data.draw_forced_page, ENDFORCEDPGE);
			continue;
		} else if (eq(t, STRTFORCEDCOL)) {
			fp=read_drawables(fp, line_cnt, scanned_prolog_data.draw_forced_col, ENDFORCEDCOL);
			continue;
		} else if (eq(t, STRTPGE)) {
			fp=read_drawables(fp, line_cnt, scanned_prolog_data.draw_page, ENDPGE);
			continue;
		} else if (eq(t, STRTCOL)) {
			fp=read_drawables(fp, line_cnt, scanned_prolog_data.draw_col, ENDCOL);
			continue;
		} else {
			err_exit(catgets(cat_fd, ERR_SET, 57, "%s: Unrecognized"
			" keyword %s in config. file %s\n"), 
			progname, t, format_file);
		}
	}
}
#ifdef TESTFLAT
static void read_and_fill(char *format_file) {}
void main() {
#else
static void read_and_fill(char *format_file) {
#endif

	FILE *fp;
	if((fp=fopen(format_file,"r"))!=NULL) {
		flat_scan(fp);
	} else {
		err_exit(catgets(cat_fd, ERR_SET, 56,
		"%s: Unable to open Prologue file %s.\n"),
		progname, format_file);
	}
	/*
	scanned_prolog_data.draw_arc=start_arc_ptr;
	scanned_prolog_data.draw_line=start_line_ptr;
	*/
}
static void draw_decor_init(draw_decor *dd) {
	dd->usr_str_x=-1;
	dd->usr_str_y=-1;
	dd->tm_str_x=-1;
	dd->tm_str_y=-1;
	dd->pg_str_x=-1;
	dd->pg_str_y=-1;
	dd->sb_str_x=-1;
	dd->sb_str_y=-1;
}

static void init_prolog_data(prolog_data *spd) {
	memset(spd, 0, sizeof(prolog_data));
	spd->orientation=0;	
	spd->num_cols=1;	
	spd->hdng_font_size=strdup(DEFHDGSIZE);	
	spd->body_font_size=strdup(DEFBDYSIZE);	
	spd->extra_hdng_font=NULL;
	spd->extra_body_font=NULL;
	spd->prolog_dpi=PROLOGDPIDEFAULT;	
	spd->y_bound=DEFYBOUND;	
	spd->text_x=STRTXDEF;
	spd->text_y=STRTYDEF;
	spd->move_x=0;
	spd->move_y=0;
	spd->draw_coords=0;	
	spd->page_str=0;	

	spd->draw_page=&dr_pg;
	memset(spd->draw_page, 0, sizeof(draw_decor));
	draw_decor_init(spd->draw_page);

	spd->draw_col=&dr_col;
	memset(spd->draw_col, 0, sizeof(draw_decor));
	draw_decor_init(spd->draw_col);

	spd->draw_forced_col=&dr_forced_col;
	memset(spd->draw_forced_col, 0, sizeof(draw_decor));
	draw_decor_init(spd->draw_forced_col);

	spd->draw_forced_page=&dr_forced_page;
	memset(spd->draw_forced_page, 0, sizeof(draw_decor));
	draw_decor_init(spd->draw_forced_page);
}
