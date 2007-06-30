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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <stdlib.h>
#include "print_preprocess.h"

#define SPARC_BOM_TEST	uchar_arr[0]==0x00 && uchar_arr[1]==0x00 \
			&& uchar_arr[2] == 0xfe && uchar_arr[3]==0xff
#define X86_BOM_TEST	uchar_arr[3]==0x00 && uchar_arr[2]== 0x00 && \
			uchar_arr[1]==0xfe && uchar_arr[0]==0xff
#define PR_IN		print_info_st[line_pos]


extern ttffont_t *ttf_fonts;
extern pcffont_t *pcf_fonts;
extern pcffont_t *CurrentFont;
extern tp1font_t *tp1_fonts;

print_info	*print_info_st=NULL;
static int	printstyle=0;

extern int 	pcf_ret, ttf_ret, tp1_ret;
extern int check_dict(int, float, int);
extern int pres_pcfbm(ucs4_t *, pcffont_t *, pcf_bm_t **, pcf_charmet_t *, 
pcf_SCcharmet_t *, int);
extern float ret_dx( int , int);

static ucs4val_u	pres_form_arr[MAXLINE];
static int tabstops = TAB_STOPS;
static	pcf_SCcharmet_t *scCmetrics;
static	pcf_charmet_t *Cmetrics;
static	tt_OutlineData_t *odata;

void process_ctl_line(unsigned char *, int, int, int);
void process_wcs_line( wchar_t *, int, int);
static void      find_font_mech(ucs4_t, int);
static double forward_one_tab(void);
static void print_info_fill(int ) ;

/* this function will change in case of support for commandline
 * printtype entry in later versions.
 */

int gen_incremental_pos(ucs4_t c, double *x_diff, double *y_diff ) {

    if (is_backspace(c)) {
	*x_diff = -SPACINGwidth;
	return(1);
    }

    if (is_formfeed(c)) {
	*x_diff = START_NEW_PAGE;
	*y_diff = START_NEW_PAGE;
	return(1);
    }

    if (is_tab(c)) {
    	*x_diff = forward_one_tab();
	return(1);
    }
    
    if (is_newline(c)) {
	*x_diff = 0.0;
	*y_diff = 10.0;
	return(1);
    }

    if (c == SPACINGchar) {
	*x_diff = SPACINGwidth;
	return(1);
    }

    return(0);
}
void 
process_wcs_line(wchar_t *wc, int wclen, int ptype) {
	int i;

	printstyle = ptype;
	for(i=0; i<wclen && wc[i] != 0x00; i++) {
		pres_form_arr[i].ucs4val = wc[i];
	}
	pres_form_arr[i].ucs4val = 0x0a;
	print_info_fill(i+1);
}
		
void 
process_ctl_line( unsigned char *uchar_arr, int osize, int lsize, int ptype) {
	static int pres_nos=0;
	ucs4_t st;
	unsigned int i=0,j=0;

	printstyle = ptype;
	if ( lsize != 1 && lsize != 2 && lsize != 4 ) {
		err_exit(catgets( cat_fd, ERR_SET , 33,
			"%s: incorrect layout size\n"),progname, lsize);
	}
		
#if defined(sparc)
	if (lsize == 1) {
		for(i=0; i<osize; i++) {
			pres_form_arr[j].ucs4_equi.c0 = 0x00;
			pres_form_arr[j].ucs4_equi.c1 = 0x00;
			pres_form_arr[j].ucs4_equi.c2 = 0x00;
			pres_form_arr[j].ucs4_equi.c3 = uchar_arr[i];
			j++;
		}
	} else if ( lsize == 2 ) {
		for(i=0; i<osize; i=i+2) {
			pres_form_arr[j].ucs4_equi.c0 = 0x00;
			pres_form_arr[j].ucs4_equi.c1 = 0x00;
			pres_form_arr[j].ucs4_equi.c2 = uchar_arr[i];
			pres_form_arr[j].ucs4_equi.c3 = uchar_arr[i+1];
			j++;
		}
	} else if ( lsize == 4 ) {
		for(i=0; i<osize; i=i+4) {
			pres_form_arr[j].ucs4_equi.c0 = uchar_arr[i];
			pres_form_arr[j].ucs4_equi.c1 = uchar_arr[i+1];
			pres_form_arr[j].ucs4_equi.c2 = uchar_arr[i+2];
			pres_form_arr[j].ucs4_equi.c3 = uchar_arr[i+3];
			j++;
		}
	}	
#else
	if (lsize == 1) {
		for(i=0; i<osize; i++) {
			pres_form_arr[j].ucs4_equi.c3 = 0x00;
			pres_form_arr[j].ucs4_equi.c2 = 0x00;
			pres_form_arr[j].ucs4_equi.c1 = 0x00;
			pres_form_arr[j].ucs4_equi.c0 = uchar_arr[i];
			j++;
		}
	} else if ( lsize == 2 ) {
		for(i=0; i<osize; i=i+2) {
			pres_form_arr[j].ucs4_equi.c3 = 0x00;
			pres_form_arr[j].ucs4_equi.c2 = 0x00;
			pres_form_arr[j].ucs4_equi.c1 = uchar_arr[i];
			pres_form_arr[j].ucs4_equi.c0 = uchar_arr[i+1];
			j++;
		}
	} else if ( lsize == 4 ) {
		for(i=0; i<osize; i=i+4) {
			pres_form_arr[j].ucs4_equi.c3 = uchar_arr[i];
			pres_form_arr[j].ucs4_equi.c2 = uchar_arr[i+1];
			pres_form_arr[j].ucs4_equi.c1 = uchar_arr[i+2];
			pres_form_arr[j].ucs4_equi.c0 = uchar_arr[i+3];
			j++;
		}
	}	
#endif
			
	/* Appending a '\n' at the end of input as to make */
	/* positioning correct when new line is input */

	pres_form_arr[j].ucs4_equi.c1= 0x00;
	pres_form_arr[j].ucs4_equi.c2= 0x00;

#if defined(sparc)
	pres_form_arr[j].ucs4_equi.c0= 0x00;
	pres_form_arr[j].ucs4_equi.c3= 0x0a;
#else 
	pres_form_arr[j].ucs4_equi.c3= 0x00;
	pres_form_arr[j].ucs4_equi.c0= 0x0a;
#endif

	pres_nos=j+1;	/* for '/n' is added above to input line */
	print_info_fill(pres_nos);
}

static void
print_info_fill(int pres_nos) {
	int i=0;
	print_info_st = (print_info *)realloc(print_info_st,sizeof(print_info)*pres_nos);
	memset(print_info_st, 0, sizeof(print_info)*pres_nos);
	if (print_info_st== NULL ) {
		malloc_err_disp_exit(__LINE__, __FILE__);
	}
	for(i=0; i<pres_nos; i++) {
		/*
		fprintf(stderr, "pres_form_arr[%d].ucs4val -- 0x%08x\n", i, pres_form_arr[i].ucs4val);
		*/
		find_font_mech(pres_form_arr[i].ucs4val, i);
	}

	i--;	

	if ( i >= 0 && print_info_st[i].val != 0x0a ) {
		print_info_st[i].line_pos = i;
		print_info_st[i].val = 0x0a;
	}
}	

static double
forward_one_tab(void)
{
    double ts_point;

    ts_point = tabstops * SPACINGwidth;
    return(ts_point);

    /*
    Cx = (((int) ((Cx + ts_point) / ts_point)) * ts_point) + left_margin;

    if (exceeds_right_margin(Cx))
	start_newline();
   */

    /*fprintf(stdout,"%f %f M\n", Cx, Cy);*/
}

	/* This code basically forces the width of each character
	 * it prints out to be an intergral multiple of 
	 * spacingwidth ., thus paching up the underlying font 
	 * metrix problem of non-uniform character width.
	 */

int pcf_incremental_pos(int index, double *x_diff, double *y_diff ) {
  	int j; 
	ucs4_t c=print_info_st[index].val;
	int pstle = print_info_st[index].print_style;

	if ( abs(wcwidth(c)) > 0) {
		*x_diff= abs(wcwidth(c))*SPACINGwidth;
	} else {
		*x_diff=SPACINGwidth;
	}

	if((j=check_dict(c, *x_diff, pstle))<0) {
		return(1);
	} 

	return(0);
}

int tt_incremental_pos(int index , double *incr_x, double *incr_y) {
	
	int j=0;
	int pstle = print_info_st[index].print_style;

	/* changed for ctlprinting */
	if((*incr_x=print_info_st[index].font_u.ttf_st.ttodata->xsize) < 0.0 ){
		*incr_x = print_info_st[index].scale_fact;
	}
	if((j=check_dict(print_info_st[index].val, *incr_x, pstle))<0) {
		return(1);
	} else {
		
		*incr_x = ret_dx( pstle, j);
		return(0);
	}
}

static void
find_font_mech( ucs4_t val, int line_pos ) {
	int f_style_select = DEFAULT_STYLE;	
	int f_ndx = -1;
	int rval, encoding, ret;
	static int save_pt_sz=0;
	pcf_bm_t *pcfbm;


	pcf_ret = ttf_ret = tp1_ret = -1;

	f_ndx = presform_fontndx(val);
	/*
	fprintf(stderr, "For val = 0x%08x , print style pcf =  %d \n\t\t\t ttf = %d \n\t\t\t tp1 = %d\n", val, pcf_ret, ttf_ret, tp1_ret); 
	*/
	if(tp1_ret >= 0) {
		PR_IN.line_pos = line_pos;
		PR_IN.val = val;
		PR_IN.disp_var = 1;
		PR_IN.font_style = TP1_STYLE;
		PR_IN.print_style = printstyle;;
		PR_IN.scale_fact = current_pt_sz;
		PR_IN.font_u.tp1_st.tp1_font = &tp1_fonts[f_ndx];
		PR_IN.font_u.tp1_st.target_char = (ucs4_t*)malloc(sizeof(ucs4_t));

		if (PR_IN.font_u.tp1_st.target_char !=NULL) {
			*(PR_IN.font_u.tp1_st.target_char) = \
					tp1_fonts[f_ndx].cuf(val);
		} else {
			malloc_err_disp_exit(__LINE__, __FILE__);
		}
	} else if ( pcf_ret >= 0 ) {
		if(save_pt_sz!=current_pt_sz) {
			init_putPS();
			save_pt_sz = current_pt_sz;
		}

		PR_IN.line_pos=line_pos;
		PR_IN.val=val;
		PR_IN.disp_var = 1;
		PR_IN.font_style=PCF_STYLE;
		PR_IN.print_style = printstyle;;

		if(is_motion_char(val)) {
			PR_IN.scale_fact = -1;
			return;
		}
		scaling_factors(&(pcf_fonts[f_ndx]), current_pt_sz*PTSZ_SCALE, \
				DEF_XRES, DEF_YRES);
		Cmetrics = (pcf_charmet_t *)malloc(\
			(unsigned int)sizeof(pcf_charmet_t ));
		if (Cmetrics == NULL ) {
			malloc_err_disp_exit(__LINE__, __FILE__);
		}
		scCmetrics = ( pcf_SCcharmet_t *)malloc(\
			(unsigned int)sizeof(pcf_SCcharmet_t));
		if (scCmetrics == NULL ) {
			malloc_err_disp_exit(__LINE__, __FILE__);
		}
		CurrentFont = &pcf_fonts[f_ndx];
		if ((rval = pres_pcfbm( &val, &pcf_fonts[f_ndx], &pcfbm,
			Cmetrics, scCmetrics, 0)) < 0) {

			if (rval == XU_IGNORE) {
				PR_IN.scale_fact = -1;
			}
		} else {
#ifdef SDEBUG
	fprintf(stderr, "%d is *********--> Cmetrics->height for C%x\n", Cmetrics->height, val);
#endif
			PR_IN.scale_fact = current_pt_sz;
			PR_IN.font_u.pcf_st.scCmet= scCmetrics;
			PR_IN.font_u.pcf_st.Cmet= Cmetrics;
			PR_IN.font_u.pcf_st.pcfbm = pcfbm;
		}
	} else if ( ttf_ret >= 0 ) {
		encoding = ttf_fonts[f_ndx].cmapd->encoding ;
		PR_IN.line_pos=line_pos;
		PR_IN.val=val;
		PR_IN.disp_var = 1;
		PR_IN.font_style=TTF_STYLE;
		PR_IN.print_style = printstyle;;
		PR_IN.scale_fact = current_pt_sz ;

		/* symbol encoding (encoding 0), does not need to be 
		 * converted 
		 */

		if(encoding)	
			val = (&ttf_fonts[f_ndx])->cuf(val);

		if( encoding == 1 && ( val == 0x20 || val == 0xa0 )) {
			PR_IN.scale_fact= -1;
			return;
		}

		odata = (tt_OutlineData_t *) malloc( (unsigned int)sizeof(tt_OutlineData_t));
		if(odata == NULL)
			malloc_err_disp_exit(__LINE__, __FILE__);

		ret = tt2ps_get_psdata(val, (int)current_pt_sz, \
				&ttf_fonts[f_ndx], odata);

		if (ret <= 0) {
			PR_IN.scale_fact= -1;
			free(odata);
			return;
		}

		PR_IN.font_u.ttf_st.ttfont =  &ttf_fonts[f_ndx];
		PR_IN.font_u.ttf_st.ttodata = odata;
	}	
}
