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
#pragma ident "@(#)xpr_client.c	1.10	00/10/19	SMI"

#include <X11/Xlib.h>
#include "X11/extensions/Print.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "mp.h"
#include "extern.h"
#include "general_header.h"
#include "xpr_prolog_scan.h"
#include "xpr_client.h"

extern char *progname;

#define	DECOR decorations

typedef struct decor {
	char *mesgtype;
	char *username;
	char *timestr;
	unsigned char *subject;
	int sub_size;
} decor_tag;

static decor_tag decorations;
static  prolog_data *pd;
static  float ppr;
static int start_page=0;

void set_xpr_headings(char *, unsigned char *, int);

static void xpr_set_fonts(void);
static void xpr_output(char *, int);
static void save_proc(Display *, XPContext, uchar_t *, uint_t, XPointer);
static void finish_proc(Display *, XPContext, XPGetDocStatus, XPointer);
static void xpr_loop(Display *, int, int);
static void detail_switch(int, int, int, XEvent);
static int mem_chk(void *);
static void xpr_consumer();
static XFontSet *xpr_setfonts(char *, char *, XFontSetExtents *);
static XFontSet *xpr_setbodyfonts(char *, char *, XFontSetExtents *);
static XFontSet *xpr_setheadingfonts(char *, char *, XFontSetExtents *);
static int xpr_printfile(char *, int);
static void new_forced_draw(draw_decor *) ;
static void new_page_draw(draw_decor *) ;
static void new_col_draw(draw_decor *) ;
static void new_draw(draw_decor *) ;
static int xpr_printmatter(char *s, int);
static int xpr_print_username(int, int);
static int xpr_print_pagestring(int, int);
static int xpr_print_subjstring(int, int);
static int xpr_print_timestring(int, int);
static Window xpr_setupprintwindow(XRectangle *);
static ushort_t get_target_printer_resolution();
static void convert_to_printer_dpi(prolog_data *);
static void scale_xy(prolog_data *, float , float );
static void convert_data(draw_decor *, float, float);
static void shift_pd(int, int);
static void shift_drawable_coords(draw_decor *, int, int);
static int check_xprserver_in_server_list(void);
static int check_xprserver_in_printer_name(char *);
static int check_printer_in_display(char *, char *);
static char *check_n_add_disp_num(char *);

static  Display *xprdpy;
static  int xprevtbase, xprerrbase;
static	GC xprgc;
static	XRectangle xprrect;
static	XPContext xprcontext;
static  ushort_t xpr_print_res=0;
static	XFontSet *xprfs, *xprhfs;
static	XFontSetExtents *xprfse, *xprhfse;
static	Window xprwin;
static	int xprpcnt;
static	XPPrinterList xprplist;
static	char *xprdefbodyfontspec;
static	char *xprdefheadingfontspec;
static	int xpr_log_pg_cnt = 0;
static  int xpr_height = 0;
static	int xpr_width = 0;
static	int startx = 0;
static  int xprline_cnt = 0;
static  int xprdone = 0;
static  int page_no = 0;
static	char *xprbodyfontspec = NULL;
static	char *xprheadingfontspec = NULL;
static	char *xprserver;

static int mem_chk(void *ptr) {
	if (ptr)
		return (1);
	else
		return (0);
}

void set_xpr_headings(char *name, unsigned char *def, int len) {
	int mem_size = len +1;
	if (strcmp(name, "MailFor") == 0) {
		DECOR.mesgtype = (char *)realloc(DECOR.mesgtype, mem_size);
		if (mem_chk(DECOR.mesgtype))
			strcpy(DECOR.mesgtype, (char *)def);
		else
			malloc_err_disp_exit(__LINE__, __FILE__);
	}
	if (strcmp(name, "User") == 0) {
		DECOR.username = (char *)realloc(DECOR.username, mem_size);
		if (mem_chk(DECOR.username))
			strcpy(DECOR.username, (char *)def);
		else
			malloc_err_disp_exit(__LINE__, __FILE__);
	}
	if (strcmp(name, "TimeNow") == 0) {
		DECOR.timestr = (char *)realloc(DECOR.timestr, mem_size);
		if (mem_chk(DECOR.timestr)){
			strcpy(DECOR.timestr, (char *)def);
			strlen(DECOR.timestr)>=26?DECOR.timestr[26]='\0':0;
		}else
			malloc_err_disp_exit(__LINE__, __FILE__);
	}
	if (strcmp(name, "Subject") == 0) {
		DECOR.subject = (unsigned char *)realloc(DECOR.subject, mem_size);
		if (mem_chk(DECOR.subject)) {
			bcopy(def,  DECOR.subject, len);
			DECOR.sub_size=len;
			DECOR.subject[len]='\0';
		}
		else
			malloc_err_disp_exit(__LINE__, __FILE__);
	}
}
void xpr_end() {
	if(xprdpy && xprfs && xprhfs && xprcontext) {
		XpEndJob(xprdpy); 
		XFreeFontSet(xprdpy,*xprfs);
		XFreeFontSet(xprdpy,*xprhfs);
		XpDestroyContext(xprdpy, xprcontext);
		XCloseDisplay(xprdpy);
	}
	return;
}

void xpr_print(uchar_t *s, int size, int stat) {
	if (stat == FEND) {
		if(start_page && xprdpy && xprfs && xprhfs && xprcontext) {
			XpEndPage(xprdpy);
		}
		start_page=0;
		xpr_log_pg_cnt=0;
		xprline_cnt=0;
		startx=0;
		page_no=0;
		return;
	}
	xpr_output((char *)s, size);
}

/*
	Get the options and do the setup
*/

static void shift_pd(int x_s, int y_s) {
	pd->text_x+=x_s;
	pd->text_y+=y_s;
	pd->y_bound+=y_s;
	if(pd->draw_forced_page)
		shift_drawable_coords(pd->draw_forced_page, x_s, y_s);
	if(pd->draw_forced_col)
		shift_drawable_coords(pd->draw_forced_col, x_s, y_s);
	if(pd->draw_page)
		shift_drawable_coords(pd->draw_page, x_s, y_s);
	if(pd->draw_col)
		shift_drawable_coords(pd->draw_col, x_s, y_s);
}
static void shift_drawable_coords(draw_decor *dd,int x_s, int y_s) {
	arc_tag *ap=dd->draw_arc;
	line_tag *lp=dd->draw_line;

	for (; ap != NULL; ap = ap->next) {
		ap->x+=x_s;
		ap->y+=y_s;
	}

	for (; lp != NULL; lp = lp->next) {
		lp->x+=x_s;
		lp->y+=y_s;
		lp->x1+=x_s;
		lp->y1+=y_s;
	}

	if(dd->usr_str_x != -1 && dd->usr_str_y != -1) {
		dd->usr_str_x +=x_s;
		dd->usr_str_y +=y_s;
	}

	if(dd->tm_str_x != -1 && dd->tm_str_y != -1) {
		dd->tm_str_x +=x_s;
		dd->tm_str_y +=y_s;
	}

	if(dd->pg_str_x != -1 && dd->pg_str_y != -1) {
		dd->pg_str_x +=x_s;
		dd->pg_str_y +=y_s;
	}
	if(dd->sb_str_x != -1 && dd->sb_str_y != -1) {
		dd->sb_str_x +=x_s;
		dd->sb_str_y +=y_s;
	}
}

static char *check_n_add_disp_num(char *dis_name) {

	char *modified, *t;

	t = strchr(dis_name,':');
	if(t)
		return(dis_name);

	modified = (char *) malloc(strlen(dis_name)+strlen(DEF_DISPLAY)+1);
	sprintf(modified,"%s%s",dis_name,DEF_DISPLAY);
	return(modified);
}

static int check_printer_in_display(char *dis_name, char *printer) {
	Display *local_disp;

	dis_name = check_n_add_disp_num(dis_name);

	if (!(local_disp = XOpenDisplay(dis_name))) {
		return 0;
	}

	if (!XpQueryExtension(local_disp, &xprevtbase, &xprerrbase)) {
		XCloseDisplay(local_disp);
		err_exit(catgets(cat_fd, ERR_SET, 48, "%s: X print extention not supported\n"), progname);
	}

	if ((xprplist = XpGetPrinterList(local_disp, printer, &xprpcnt)) && \
		xprpcnt > 0) {
		xprcontext = XpCreateContext(local_disp, printer);
		xprserver = dis_name;
		xprdpy = local_disp;
	} else {
		return 0;
	}

	XpFreePrinterList(xprplist);
	return 1;
}


static int check_xprserver_in_printer_name(char *tp){
	char *printer;
	char *display;
	int ret = 0;
	char *pname=strdup(tp);

	if (( printer = strtok(pname, "@")) != NULL ) {
		char *display = strtok(NULL, "@");
		target_printer = strdup(printer);
		if (display && check_printer_in_display (display, printer)) {
				xprserver = display;
				return 1;
		}
	} else {
		err_exit(catgets(cat_fd, ERR_SET, 45, "%s: Invalid printer "
		"name for Print Server\n"), progname);
	}
	free(pname);
	return 0;
}

static int check_xprserver_in_server_list(void){

	const char *es = (const char *)getenv("XPSERVERLIST");
	char *ds, *display;

	if ( es == NULL )
		return 0;
	else
		ds = strdup(es);

	for(display = strtok(ds, " "); display ; display = strtok(NULL, " ")) {	
		if (check_printer_in_display(display, target_printer)) {
			xprserver = display;	
			return 1;
		}
	}
	free(ds);
	return 0;
}	

void xpr_init() {
	XPHinterProc pHintProc = NULL;
	char *pHintString = NULL;
	char *attrPool;
	const char *server_list=NULL;
	char buf[500];
	int bz;

	read_config_file(proname, &pd);
	bz = atoi(pd->body_font_size);
	if(pd->move_x || pd->move_y)
		shift_pd(pd->move_x, pd->move_y);
	/* change the page / line lenght for the main routines */

	if(pd->page_len>0)
		plen=bz*pd->page_len/(set_pt_sz*10);
	if(pd->line_len>0)
		llen=bz*pd->line_len/((set_pt_sz)*10);

	if(!plen || !llen || 
		(paper_size==US && set_pt_sz > MAX_US_PORTRAIT_PT_SZ) || 
		(paper_size==A4 && set_pt_sz > MAX_A4_PORTRAIT_PT_SZ) ||
		(paper_size==US && set_pt_sz > MAX_US_LANDSCAPE_PT_SZ) || 
		(paper_size==A4 && set_pt_sz > MAX_A4_LANDSCAPE_PT_SZ)) {

		plen=pd->page_len;
		llen=pd->line_len;
		set_pt_sz=0.1*bz;
	}

	xpr_set_fonts();

	if (!check_xprserver_in_printer_name(target_printer)) {
		if (!check_xprserver_in_server_list()) {
			xprserver = (char *)getenv("XPDISPLAY");	
			if (!((xprserver && check_printer_in_display(xprserver, target_printer)) ||  check_printer_in_display(DEF_DISPLAY, target_printer)))
				err_exit(catgets(cat_fd, ERR_SET, 44, "%s: Cannot open X Print Server Display\n"), progname);
		}
	}

	if (pd->orientation) {
		XpSetAttributes(xprdpy, xprcontext, XPDocAttr, \
			(char *)"*content-orientation: landscape", XPAttrMerge);
	}

	XpSetContext(xprdpy, xprcontext);
	XpSelectInput(xprdpy, xprcontext, XPPrintMask);

	xprwin = xpr_setupprintwindow(&xprrect);
	xpr_print_res=get_target_printer_resolution();
	convert_to_printer_dpi(pd);

	if ( paper_size== A4 ) { 
		float x_scale, y_scale;
		if(pd->orientation) {
			x_scale=11.693/11.0;
			y_scale=8.268/8.5;
		} else {
			y_scale=11.693/11.0;
			x_scale=8.268/8.5;
		}
		llen = x_scale*(float)llen;
		plen = y_scale*(float)plen;
		scale_xy(pd, x_scale, y_scale);
	}

}

static void xpr_output(char *s, int size) {
	pid_t ppid, pid;
	static int first_time = 1;
	if (first_time) {
			/* XPGetData for getting */
			/* the data back, XPSpool for direct spooling to */
			/* target printer */
		XpStartJob(xprdpy, print_spool?XPSpool:XPGetData); 

		xpr_set_fonts();
		xprfs = (XFontSet *)xpr_setbodyfonts(xprbodyfontspec, \
		xprdefbodyfontspec, xprfse);
		xprhfs = (XFontSet *)xpr_setheadingfonts(xprheadingfontspec, \
		xprdefheadingfontspec, xprhfse);
		if(!print_spool)
			xpr_consumer();
		first_time = 0;
	}
	xpr_printfile(s, size);
}

static void xpr_set_fonts() {
	char *c;

	xprdefbodyfontspec = (char *)malloc(strlen(DEFAULT_BODY_FONTSPEC)+100);
	if(!xprdefbodyfontspec)
		malloc_err_disp_exit(__LINE__, __FILE__);
	xprdefheadingfontspec = (char *)malloc(strlen(DEFAULT_HEADING_FONTSPEC)+100);
	if(!xprdefheadingfontspec)
			malloc_err_disp_exit(__LINE__, __FILE__);
	xprbodyfontspec=pd->extra_body_font;
	xprheadingfontspec=pd->extra_hdng_font;
	
	/* replace body/heading font sizes "%d" strings with scanned data */

	if(xprheadingfontspec)
		while ((c = replace_str(xprheadingfontspec, "%d", \
		    pd->hdng_font_size)) != NULL);

	if(xprbodyfontspec) {
		if(set_pt_sz)
			sprintf(pd->body_font_size,"%d",set_pt_sz*10);
		while ((c = replace_str(xprbodyfontspec, "%d", \
		    pd->body_font_size)) != NULL);
	}

	strcpy(xprdefheadingfontspec, DEFAULT_HEADING_FONTSPEC);
	while ((c = replace_str(xprdefheadingfontspec, "%d", \
	    pd->hdng_font_size)) != NULL);

	strcpy(xprdefbodyfontspec, DEFAULT_BODY_FONTSPEC);
	if(set_pt_sz)
		sprintf(pd->body_font_size,"%d",set_pt_sz*10);
	while ((c = replace_str(xprdefbodyfontspec, "%d", \
	    pd->body_font_size)) != NULL);

}

static void save_proc(Display *xprdpy, XPContext xprcontext, uchar_t *data, \
uint_t len, XPointer ip) {
	FILE *fp = (FILE *)ip;
	fwrite(data, len, 1, fp); /* output the data */
}


static void finish_proc(Display *xprdpy, XPContext pcontent, XPGetDocStatus \
status, XPointer ip) {
	FILE *fp = (FILE *)ip;
	XPPrintEvent event;

	if (status != XPGetDocFinished) {
		fprintf(stderr, "finishProc called; status = %s.\n",
			(status == XPGetDocFinished)?"Finished"
			:(status == XPGetDocError)?"Error":\
			"Second Consumer Error");
	}
	fclose(fp);
	xprdone = (status == XPGetDocFinished)?1:-1;
}

static void xpr_loop(Display *ds, int evtbse, int consumer) {

	while (!xprdone) {
		XEvent evt;
		XPPrintEvent xprevt;
		int detail;
		int xprcancel;

		XNextEvent(ds, &evt);
		xprevt = *((XPPrintEvent *)(&evt));
		detail = xprevt.detail;
		xprcancel = xprevt.cancel;
		if (evt.type >= evtbse) {
			switch (evt.type-evtbse) {

			case XPAttributeNotify:
				break;

			case XPPrintNotify:
				detail_switch(detail, xprcancel, consumer, evt);

			default:
				;
			}
		}

	}
}

static void detail_switch(int detail, int cancel, int consumer, XEvent evt) {

	switch (detail) {
		case XPStartJobNotify:
			break;

		case XPEndJobNotify:
			if (consumer) {
				if (!xprdone) {
					fprintf(stderr, catgets(cat_fd, ERR_SET,
					47,"%s: Printing not finished "
					"properly\n"),progname);
				}
				if (xprdone == 1 && cancel) {
					fprintf(stderr, catgets(cat_fd, ERR_SET,
					59,"%s: Print cancelled\n"), progname);
					xprdone = -1;
				}
			} else {
				xprdone = 1;
			}
			break;

		case XPStartDocNotify:
			break;

		case XPEndDocNotify:
			break;

		case XPStartPageNotify:
			break;

		case XPEndPageNotify:
			break;

		default:
			break;
		}
}

static void xpr_consumer() {
	int pid;
	/* reopen stdout */
	FILE *fpprt = fdopen(dup_stdout_des, "w");
	if (!fpprt)
		err_exit(catgets(cat_fd, ERR_SET, 49, "%s: Failed reopening"
		" output file\n"),progname);
	if ((pid = fork()) != 0) {
		Display *consdisplay;
		if (!(consdisplay = XOpenDisplay(xprserver)))
			err_exit(catgets(cat_fd, ERR_SET, 44,
				"%s: Cannot open X Print Server Display\n"),
				progname);
		XpSetContext(consdisplay, xprcontext);
		XpSelectInput(consdisplay, xprcontext, \
		XPPrintMask|XPAttributeMask);
		if (0 == XpGetDocumentData(consdisplay, xprcontext, \
		save_proc, finish_proc, (XPointer)fpprt))
			err_exit(catgets(cat_fd, ERR_SET, 50, "%s: Unable to "
			"register output data save & finish routines\n"),
			progname);
		xpr_loop(consdisplay, xprevtbase, 1);
		XCloseDisplay(consdisplay);
		exit((xprdone > 0)?0:1);
	} else {
	}
}

static XFontSet *xpr_setfonts(char *fontspec, char *deffontspec, \
XFontSetExtents *fse) {
	XFontSet *lfs;
	char **missing;
	int misscnt;
	char *defstring;

	if ((lfs = (XFontSet *)malloc(sizeof (XFontSet))) == NULL) 
			malloc_err_disp_exit(__LINE__, __FILE__);

	if (!fontspec) {
		fontspec = deffontspec;
	} else {
		if((fontspec=(char *)realloc(fontspec, strlen(fontspec) + strlen(deffontspec)+200))==NULL)
			malloc_err_disp_exit(__LINE__, __FILE__);
		/* append a comma after the font read from config. file */
		memmove(fontspec+strlen(fontspec),",",1);
		memmove(fontspec+strlen(fontspec), deffontspec, strlen(deffontspec));
	}
	*lfs = XCreateFontSet(xprdpy, fontspec, &missing, &misscnt, &defstring);
	if (!lfs) {
		err_exit(catgets(cat_fd, ERR_SET, 51, "%s: Fail creating fontset from fontspec %s\n"), progname,fontspec);
	} else if (!(fse = XExtentsOfFontSet(*lfs))) {
		fprintf(stderr, catgets(cat_fd, WARN_SET, 5, "%s: Failed to get"
			" extents of font set\n"), progname);
	}
	return (lfs);
}

static XFontSet *xpr_setbodyfonts(char *fontspec, char *deffontspec, \
XFontSetExtents *fse) {
	return ((XFontSet *)xpr_setfonts(fontspec, deffontspec, fse));
}

static XFontSet *xpr_setheadingfonts(char *fontspec, char *deffontspec, \
XFontSetExtents *fse) {
	return ((XFontSet *)xpr_setfonts(fontspec, deffontspec, fse));
}

static int xpr_printfile(char *str, int size) {

	if (xprline_cnt == 0 && str == NULL) {
		err_exit(catgets(cat_fd, ERR_SET, 2, "%s: empty input file %s, "
		"nothing printed\n"), progname, curfname);
	}
	xpr_printmatter(str, size);

	xprline_cnt++;
	return (1);
}

static XRectangle ink, logical;

static int xpr_print_subjstring(int sb_x, int sb_y) {

	int height = 0, offset = sb_y;
	unsigned char hbuf[MAXLINE];
	static int subj_no = 0;

	bcopy(decorations.subject, hbuf, decorations.sub_size);
	/*
	XmbTextExtents(*xprhfs, hbuf, decorations.sub_size, &ink, &logical);
	*/
	ctl_draw_string(xprdpy, xprwin, *xprhfs, xprgc, \
			sb_x, sb_y, hbuf, decorations.sub_size );
	/*
	offset += 2*(height+2);
	*/
	XFlush(xprdpy);
	return (offset);
}

static int xpr_print_pagestring(int pg_x, int pg_y) {

	static int i=0;
	int height = 0, offset = pg_y;
	char hbuf[MAXLINE];
	int pg_ext=0;

	logical.height=0;
	if(pd->page_str) {
		strcpy(hbuf,"Page");
		pg_ext=XmbTextExtents(*xprhfs, hbuf, strlen(hbuf), &ink, &logical);
		XmbDrawString(xprdpy, xprwin, *xprhfs, xprgc, pg_x, pg_y, hbuf, strlen(hbuf));
	}
	sprintf(hbuf, "%d", ++page_no);
	/*
	XmbTextExtents(*xprhfs, hbuf, strlen(hbuf), &ink, &logical);
	*/
	XmbDrawString(xprdpy, xprwin, *xprhfs, xprgc, pg_x+(pg_ext/3), pg_y+logical.height, hbuf, strlen(hbuf));
	offset += 2*(height+2);
	XFlush(xprdpy);
	return (offset);
}

static int xpr_print_timestring(int tm_x, int tm_y) {

	int height = 0, offset = tm_y;
	char hbuf[MAXLINE];

	sprintf(hbuf, decorations.timestr);
	XmbTextExtents(*xprhfs, hbuf, strlen(hbuf), &ink, &logical);
	/*
	if (xprhfse) {
		height = xprhfse->max_logical_extent.height;
	} else {
		height = logical.height+2;
	}
	*/
	XmbDrawString(xprdpy, xprwin, *xprhfs, xprgc, tm_x, tm_y, hbuf, \
	strlen(hbuf));
	offset += 2*(height+2);
	XFlush(xprdpy);
	return (offset);
}

static int xpr_print_username(int x, int y) {

	int height, offset = y;
	char hbuf[MAXLINE];
	sprintf(hbuf, decorations.mesgtype);
	sprintf(hbuf+strlen(hbuf), decorations.username);
	XmbTextExtents(*xprhfs, hbuf, strlen(hbuf), &ink, &logical);
	if (xprhfse) {
		height = xprhfse->max_logical_extent.height;
	} else {
		height = logical.height+2;
	}
	XmbDrawString(xprdpy, xprwin, *xprhfs, xprgc, x, y, hbuf, strlen(hbuf));
	offset += 2*(height+2);
	XFlush(xprdpy);
	return (offset);
}

static void convert_data(draw_decor *dp, float x_scale, float y_scale) {
	arc_tag *da; 
	line_tag *dl=NULL;

	dp->usr_str_x*=x_scale;
	dp->usr_str_y*=y_scale;
	dp->tm_str_x*=x_scale;
	dp->tm_str_y*=y_scale;
	dp->pg_str_x*=x_scale;
	dp->pg_str_y*=y_scale;
	dp->sb_str_x*=x_scale;
	dp->sb_str_y*=y_scale;

	da=dp->draw_arc;
	for(;da;da=da->next) {
		da->x*=x_scale;
		da->y*=y_scale;
		da->width*=x_scale;
		da->height*=y_scale;
	}

	dl=dp->draw_line;
	for(;dl;dl=dl->next) {
		dl->x*=x_scale;
		dl->y*=y_scale;
		dl->x1*=x_scale;
		dl->y1*=y_scale;
	}
}
static void scale_xy(prolog_data *p, float x_scale, float y_scale) {
	if(x_scale!=1.0 || y_scale!=1.0) {
		p->text_x*=x_scale;
		p->text_y*=y_scale;
		p->y_bound*=y_scale;
		if(p->draw_forced_page)
			convert_data(p->draw_forced_page, x_scale, y_scale);
		if(p->draw_forced_col)
			convert_data(p->draw_forced_col, x_scale, y_scale);
		if(p->draw_page)
			convert_data(p->draw_page, x_scale, y_scale);
		if(p->draw_col)
			convert_data(p->draw_col, x_scale, y_scale);
	}
}
static void convert_to_printer_dpi(prolog_data *p) {
	ppr=(float)xpr_print_res/(float)p->prolog_dpi;
	if(ppr!=1.0)
		scale_xy(p, ppr, ppr);
}

static void new_forced_draw(draw_decor *dd) {
	new_draw(dd);
}
static void new_page_draw(draw_decor *dd) {
	new_draw(dd);
}

static void new_col_draw(draw_decor *dd) {
	new_draw(dd);
}
static void new_draw(draw_decor *dd) {
	arc_tag *ap=dd->draw_arc;
	line_tag *lp=dd->draw_line;
	for (; ap != NULL; ap = ap->next)
		XDrawArc(xprdpy, xprwin, xprgc, ap->x + startx, ap->y, \
	 	    ap->width, ap->height, ap->angle1*64, ap->angle2*64);

	for (; lp != NULL; lp = lp->next)
		XDrawLine(xprdpy, xprwin, xprgc, lp->x + startx, lp->y, \
		    lp->x1 + startx, lp->y1);

	if(dd->usr_str_x != -1 && dd->usr_str_y != -1)
		xpr_print_username(dd->usr_str_x + startx, dd->usr_str_y);
	if(dd->tm_str_x != -1 && dd->tm_str_y != -1)
		xpr_print_timestring(dd->tm_str_x + startx, dd->tm_str_y);
	if(dd->pg_str_x != -1 && dd->pg_str_y != -1)
		xpr_print_pagestring(dd->pg_str_x + startx, dd->pg_str_y);
	if(dd->sb_str_x != -1 && dd->sb_str_y != -1)
		xpr_print_subjstring(dd->sb_str_x + startx, dd->sb_str_y);

	/* remedy for Xerror in thai locale when no decoration */
	if(nodecor)
		XmbDrawString(xprdpy, xprwin, *xprhfs, xprgc, 0, 0, " ", 1);
}
static int blank_line(int x, int y) {
	XmbTextExtents(*xprfs, " ", 1, &ink, &logical);
	return(logical.height);
}

static int xpr_printmatter(char *str, int size) {
	static int xoffset;
	static int offset = 0;
	int maxheight = 0;
	prolog_data tpd;
	tpd = *pd;

	if (xprline_cnt%plen == 0) {
		XGCValues values;
		Screen *pscreen;
		if (!xpr_log_pg_cnt) {
			start_page=1;
			XpStartPage(xprdpy, xprwin);
			if(tpd.draw_page && !nodecor) {
				new_page_draw(tpd.draw_page);		
			}
			if(tpd.draw_forced_page) {
				new_forced_draw(tpd.draw_forced_page);		
			}
		}

		if(tpd.draw_forced_col) {
			new_forced_draw(tpd.draw_forced_col);		
		}

		if(tpd.draw_col && !nodecor) {
			new_col_draw(tpd.draw_col);
		}
		xprline_cnt=0;
		offset = pd->text_y+4.5*set_pt_sz;
		xoffset = startx + pd->text_x;
	}

	if (xprfse) {
		maxheight = xprfse->max_logical_extent.height;
	}
	if(!size || str=="") {
		maxheight=blank_line(xoffset, offset);
		}
	else
		maxheight=ctl_draw_string(xprdpy, xprwin, *xprfs, xprgc, \
			xoffset, offset, str, size);
	offset += maxheight;
	if ((xprline_cnt%plen == plen-1) || ((offset/*+maxheight*/)>pd->y_bound)) {
		if (++xpr_log_pg_cnt == pd->num_cols) {
			startx = 0;
			start_page=0;
			XpEndPage(xprdpy);
			xpr_log_pg_cnt = 0;
		} else {
			startx += xpr_width/pd->num_cols;
		}
		xprline_cnt=plen-1;
		offset = pd->text_y;
	}
	return (0);
}

static Window xpr_setupprintwindow(XRectangle *rect) {
	Window pwin;
	unsigned short width, height;
	Screen *pscreen;
	int xprborderwidth = 0;

	pscreen = XpGetScreenOfContext(xprdpy, xprcontext);

	XpGetPageDimensions(xprdpy, xprcontext, &width, &height, rect);
	/*
	xpr_width = width;
	xpr_height = height;
	*/
	xpr_width = rect->width;
	xpr_height = rect->height;
	/*
	fprintf(stderr, "Pix width of page = %d\nPix height of page = \
	%d\n rect->x = %d\n rect->y = %d\n rect->width = %d\n \
	rect->height = %d\n", width, height, rect->x, rect->y, rect->width, \
	rect->height);
	*/
	pwin = XCreateSimpleWindow(xprdpy, RootWindowOfScreen(pscreen),
			rect->x, rect->y, rect->width, rect->height,
			xprborderwidth,
			xprborderwidth?
			BlackPixelOfScreen(pscreen):
			WhitePixelOfScreen(pscreen),
			WhitePixelOfScreen(pscreen));
	xprgc = DefaultGCOfScreen(pscreen);
	return (pwin);
}

static ushort_t get_target_printer_resolution() {
	char *attrPool;
	char *cp;
	char flg=0;
	attrPool = XpGetAttributes( xprdpy, xprcontext, XPPrinterAttr);
	while((cp=strtok(attrPool,":\n"))!=NULL) {
		if(!strcmp("*printer-resolutions-supported", cp)) {
			int at;
			if((cp=strtok(NULL,":"))!=NULL) {
				at=atoi(cp);
				if(at==0) {
					err_exit(catgets(cat_fd, ERR_SET, 46, \
						"%s: Invalid Printer Resolution\n"), progname );
				}
			}			
			return((ushort_t)at);
		}
		attrPool=NULL;
	}
	return(0);
}
