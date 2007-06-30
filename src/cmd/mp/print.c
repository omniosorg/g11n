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
/*  @(#)print.c 3.11 99/11/09
 *
 *  Copyright (c) Steve Holden and Rich Burridge.
 *                All rights reserved.
 *
 *  Permission is given to distribute these sources, as long as the
 *  copyright messages are not removed, and no monies are exchanged.
 *
 *  No responsibility is taken for any errors inherent either
 *  to the comments or the code of this program, but if reported
 *  to me then an attempt will be made to fix them.
 */
/*
 * Copyright (c) 2000 by Sun Microsystems, Inc.
 * All rights reserved.
 */


#include "mp.h"
#include "extern.h"
#include "general_header.h"
#include "print_preprocess.h"
#include "configuration.h"
#include "ctl_processing.h"
#include <locale.h>

#define STRCHK(s)	(s+1) && *(s+1) && (s+2) && *(s+2) && (s+3) && *(s+3) 
#define DIGITCHK(s, first_pass) 	isdigit(*(s+1)) && isdigit(*(s+2)) && isdigit(*(s+3)) && (first_pass?(*(s-1) != '\\'):1) 

static void multi_print(int);
static void new_page_routines() ;
static void new_col_routines() ;
static void wc_processing(unsigned char *, int);
static void print_presentation_forms();
static void print_prolog_ps(unsigned char *);
static void multiprint_nextline(void) ;
static void set_disp_vars(transform_arr *);
static void check_n_put_newpage() ;



static int new_page, printstyle=0, hdngsize=14;

enum pform_t conf_presform_width(void) ;
extern bool multi_print_var;
extern double SPACINGwidth;
extern print_info	*print_info_st;
extern int process_ctl_line( uchar_t *, int, int, int);
extern void pcf_tt_put_postscript_prologue(void);
extern Bool set_current_locale(void) ;
extern void process_wcs_line( wchar_t *, int, int);
extern bool ctl_check(void);
extern transform_arr *ctl_processing(unsigned char * );
extern void free_transform_arr(transform_arr *);

void
boldshow(hdr, str)      /* Display a header all in bold. */
char *hdr, *str ;
{
  useline() ;
  if(!multi_print_var) {
  	FPUTS("BoldFont ", stdout) ;
  	startline() ;
	expand((unsigned char *) hdr, BOLDFONT, FALSE) ;
	expand((unsigned char *) str, BOLDFONT, FALSE) ;
  	endline() ;
  } else {
	char *cstr=(char *)malloc(2*llen+1);
	sprintf(cstr,"%s%s",hdr,str);
	strlen(hdr)+strlen(str)>=llen?cstr[llen]='\0':0;
  	expand((unsigned char *) cstr, BOLDFONT, TRUE ) ;
	free(cstr);
	/*
  	expand((unsigned char *) hdr, BOLDFONT, TRUE) ;
	useline();
  	expand((unsigned char *) str, BDITFONT, TRUE) ;
	*/
  }
}


void
endcol()
{
  FPUTS("sf ", stdout) ;
  linect = 1 ;
  ++colct ;
  if (landscape == TRUE)
    PRINTF("(%d) %d endcol\n", (2*pageno)+colct-2, colct) ;
  else
    PRINTF("(%d) %d endcol\n", pageno, colct) ;
  if (numcols > 1)
    set_defs() ;         /* Needed for correct heading in multiple columns. */
    /*
  if (multi_print_var)
  */
  	new_col_routines();
  if (landscape == TRUE && colct != numcols) return ;
  pageno++ ;
  new_page = 1;
}


void
endfile()
{
  linect = 0 ;
}


void
endline()
{
  if (!multi_print_var)
  	PRINTF(") showline\n") ;
}


void
endpage()
{
  linect = 0 ;
  PRINTF("(%1d) endpage\n", tpn) ;
}


void
write_output(char *line)
{
  FPUTS(line, stdout) ;
}

static void  wc_processing(unsigned char *s, int pstyle){
	wchar_t wclinebuf[MAXLEN];
	int mbout;
	
	if(set_current_locale()) {
		if((mbout = mbstowcs(wclinebuf, s, strlen((const char *)s) + 1))==(size_t)-1) {
			err_exit(catgets(cat_fd, ERR_SET,41,
				"%s: failed converting to wide "
				"character\n"), progname);
		}

		process_wcs_line( wclinebuf, mbout, pstyle);
		
	} else {
		err_exit(catgets(cat_fd, ERR_SET, 38, "%s: Not able to \
		set locale\n"), progname);
	}
}
static void multiprint_nextline(void) {
	static float ft=0.0;

	check_n_put_newpage();
	if(ft<=0.0 && printstyle!=HDNGFONT) { 
		if (landscape) 
			ft=((float)LANDSCAPE_ADD_SPACING*current_pt_sz)/(float)LANDSCAPE_PT_SZ;
		else 
			ft=((float)NORMAL_ADD_SPACING*current_pt_sz)/(float)NORMAL_PT_SZ;
	}	
	fprintf(stdout, "\n%.1f %d MP\n",ft, linect);
}
	
static void set_disp_vars(transform_arr * t_a) {
	int i,j=0;
	size_t ind;
	for(i=0; i<t_a->outsize; i=t_a->layout_size+i) {
		ind = t_a->outtoinp[i];
		if (ind <t_a->inpsize) {
			if (((t_a->property[ind] & 0x80) == 0x80)||((t_a->property[ind] & 0x10 )== 0x10))
			 	print_info_st[j++].disp_var = 1;
			else 
				print_info_st[j++].disp_var = 0;
		}
	}
}

static void  print_presentation_forms() {
	int i=0;
	do {
		multi_print(i);
	} while ( print_info_st[++i].val != 0x0000000a );
	multiprint_nextline();
}

static void  print_prolog_ps(unsigned char *s){
	int _escape = 0, first_pass=0;

	/* 4279372 New mp can't handle Japanese in mail subject */
	s = (unsigned char *)euc_to_octal((char*)s);
  	for (; s && *s; s++) {
      		switch (*s) {
			case '\\' :
				if ( !octal ) {
					FPUTS("\\\\", stdout) ;
				} else {
		 			if ( STRCHK(s) ) {
						if ( DIGITCHK(s,first_pass) && !_escape) {
							FPUTS("\\", stdout);
						} else {
							if (_escape)
								_escape = 0;
							FPUTS("\\\\", stdout);
						}
					} else {
						FPUTS("\\\\", stdout);
					}
			  	}
				break ;

			case '('  : 

				FPUTS("\\(", stdout) ;
				break ;
			case ')'  : 

				FPUTS("\\)", stdout) ;
				break ;
			default   : 
				if ((*s < 0x20) || (*s > 0x7e))
					FPRINTF(stdout,"\\%03o",*s) ;
				else
				if (isprint(*s)) PUTC(*s, stdout) ;
		}
		if (*s == '\\')
			_escape = 1;
		else
			_escape = 0;
		if(!first_pass) first_pass=1;	
	}
}

static void
nof(){}

static void
subdef(int len){
	static int first=0;

	if(first) return;
	/*
	check_n_put_newpage();
	*/
	printf("/mysub {\n");
	printf("/HeadingDict %d dict def\n", len);
	printf("HeadingDict begin\n");
	printf("/mysubdef 1 def\n");
	print_presentation_forms();
	new_page=1;
	printf("end\n");
	printf("} def\n");
	first++;
}

int
expand(unsigned char *s , int printtype, bool mix_prn)  {
	static bool ctl_flag;
	static int first_time = 0;
	transform_arr *t_arr;
	int i=0;
	static int ctl_process=0;
	static enum pform_t pformwidth = INVALID;
	int dup_cur_pt_sz=current_pt_sz;

	if ( s == NULL ) return(0);
	if(printtype==HDNGFONT) {
		current_pt_sz = hdngsize;
	}
	if ( mix_prn == TRUE ) {
		printstyle = printtype;
		if ( ++first_time == 1 ) {
			ctl_flag = ctl_check();
			!target_printer? pformwidth =  conf_presform_width():NULL;
		}
		if ( *s == NULL ) { /* '\n' was originally stripped */
			target_printer?xpr_print((unsigned char *)"",0, NRML):multiprint_nextline();
			return(0);
		}
		if ( ctl_flag == TRUE && pformwidth != WC ) {
			t_arr = ctl_processing(s);
			if(!target_printer) {
				process_ctl_line(t_arr->outbuff, t_arr->outsize, t_arr->layout_size, printtype);
				ctl_process=1;
			}
		} else {
			if(printtype==HDNGFONT) {
				if(!target_printer) wc_processing(s, printtype);
				target_printer?set_xpr_headings((char *)"Subject",s,strlen((char *)s)):(void)subdef(strlen((char*)s));
			} else {
				!target_printer ? wc_processing(s, printtype):(void)xpr_print(s,strlen((char *)s), NRML);
				!target_printer ? print_presentation_forms():(void)nof();
			}
		}
		if(ctl_process) {
			ctl_process = 0;
			set_disp_vars(t_arr);
		}
		/*
		for(i=0;i<t_arr->outsize;i++)
			fprintf(stderr,"t_arr->outbuff[%d]=0x%08x\n",i,t_arr->outbuff[i]);
			*/
		if(ctl_flag==TRUE) {
			if(printtype==HDNGFONT)  {
				target_printer?set_xpr_headings("Subject",(unsigned char *)t_arr->outbuff, t_arr->outsize):(void)subdef(t_arr->outsize);
			 } else
			target_printer? xpr_print(t_arr->outbuff,t_arr->outsize, NRML):print_presentation_forms();
			if(pformwidth!=WC)
				free_transform_arr(t_arr);
		}
	} else if ( mix_prn == FALSE ) {
		print_prolog_ps(s);
	}
	current_pt_sz=dup_cur_pt_sz;
	return(1);
}


void
mixedshow(hdr, str)     /* Display a header in mixed bold/Roman. */
char *hdr, *str ;
{
  useline() ;
  if (!multi_print_var) {
  	FPUTS("BoldFont ", stdout) ;
  	startline() ;
	expand((unsigned char *) hdr, BOLDFONT, FALSE ) ;
  	FPUTS(") SH pf (", stdout) ;
	expand((unsigned char *) str, ITALFONT, FALSE ) ;
  	endline() ;
 } else {
	char *cstr=(char *)malloc(2*llen+1);
	sprintf(cstr,"%s%s",hdr,str);
	strlen(hdr)+strlen(str)>=llen?cstr[llen]='\0':0;
  	expand((unsigned char *) cstr, ITALFONT, TRUE ) ;
	free(cstr);
	/*
  	expand((unsigned char *) hdr, BOLDFONT, TRUE ) ;
  	useline() ;
  	expand((unsigned char *) str, ITALFONT, TRUE ) ;
	*/
 }
}


int
psdef(name, def)        /* Do a PostScript define. */
char *name, *def ;
{
  unsigned int len=0;
  if(def) {
	len=strlen(def);
  	if(len>BUFFERSIZE)
		def[BUFFERSIZE]='\0';
  };
  if(target_printer) {
	if(strcmp(name, "Subject") == 0) {
		check_n_put_newpage();
		expand((unsigned char *) def, HDNGFONT, TRUE);
	} else
		set_xpr_headings(name,(unsigned char*)def, strlen(def));
	return 1;
  }

  if(multi_print_var) {
	if(strcmp(name, "Subject") == 0) {
		char *dup_def;
		int i,k;
	
		check_n_put_newpage();
		if(def) {
			dup_def=strdup(def);
			k=strlen(dup_def);
			expand((unsigned char *) def, HDNGFONT, TRUE);
			/*
			for(i=0;i<k;i++)
				dup_def[i]=' ';
				*/
  			PRINTF("/%s (%s)def\n", name, dup_def) ;
		} else {
  			PRINTF("/mysub {\n");
			PRINTF("/HeadingDict 1 dict def\n");
			PRINTF("HeadingDict begin\n");
			PRINTF("end\n");
			PRINTF("} def\n");
			PRINTF("/%s (", name) ;
			PRINTF(") def\n") ;
		}
		return 1;
	}
  }

  if(strcmp(name, "Subject") == 0) {
	PRINTF("/mysub {\n");
	PRINTF("/HeadingDict 1 dict def\n");
	PRINTF("HeadingDict begin\n");
	PRINTF("end\n");
	PRINTF("} def\n");
  }
  PRINTF("/%s (", name) ;

  if(strcmp(name, "Subject") == 0) 
  	expand((unsigned char *) def, HDNGFONT, FALSE ) ;
  else
  	expand((unsigned char *) def, 0, FALSE ) ;
  PRINTF(") def\n") ;

  if(strcmp(name, "Subject") == 0)
  	check_n_put_newpage();
  return 1;
}


void
romanshow(str)          /* Display a header all in Roman. */
char *str ;
{
  useline() ;
  if(!multi_print_var) {
  	FPUTS("pf ", stdout) ;
  	startline() ;
  	expand((unsigned char *) str, 0, FALSE ) ;
  	endline() ;
  } else {
	expand((unsigned char *) str, ROMNFONT, TRUE);
  }
}


void
set_defs()               /* Setup PostScript definitions. */
{
  int i ;

  if (article == TRUE)
    message_for = "Article from" ;                    /* MailFor. */
  else if (print_orig == TRUE && from != NULL)
    message_for = "From" ;
  psdef("MailFor", message_for) ;

  if (article == TRUE && newsgroups != NULL)          /* User. */
    {
      for (i = 0; i < strlen(newsgroups); i++)
        if (newsgroups[i] == ',' ||
            newsgroups[i] == '\0') break ;
      owner = (char *) realloc(owner, (unsigned int) i+1) ;
      STRNCPY(owner, newsgroups, i) ;
      owner[i] = '\0' ;
    }
  else if (print_orig == TRUE && from != NULL)
    {
      i = strlen(from) ;
      owner = (char *) realloc(owner, (unsigned int) i+1) ;
      STRNCPY(owner, from, i) ;
      owner[i] = '\0' ;
    }

  psdef("User", owner) ;
 
  do_date() ;                                         /* TimeNow. */
 
  if (text_doc && cmdfiles) subject = curfname ;
  psdef("Subject",
        (gsubject != NULL) ? gsubject : subject) ;    /* Subject. */
}


/* Display the PostScript prologue file for mp */
                   
void
show_prologue(pro)
char *pro ;              /* Prologue file name */
{
  FILE *cf, *pf ;
  char buf[MAXLINE], tmpstr[MAXLINE] ;
  char *cpro = NULL ;  	   /* Full pathname of the common prologue file. */
  int t2 ;                 /* Possible extract page or line length. */

  if ((pf = fopen(pro, "r")) == NULL)                                
    {
	FPRINTF(stderr , catgets(cat_fd, ERR_SET, 3, \
		"%s: Prologue file %s not found.\n"),progname, pro) ;
      exit(1) ;
    }
  while (fgets(buf, MAXLINE, pf) != NULL)
    {

      FPUTS(buf, stdout) ;

/* Check for new line or page length. */
 
     if (strstr(buf, "/LandscapeMode true def") != 0) {
	landscape=TRUE;
	if(current_pt_sz==NORMAL_PT_SZ)
		current_pt_sz=LANDSCAPE_PT_SZ;
	}
      tmpstr[0] = '\0' ;
      SSCANF(buf, "%s %d", tmpstr, &t2) ;
     if (strcmp(tmpstr, "%%PageLength") == 0)
        plen = t2 ;               /* Change the page length. */
      else if (strcmp(tmpstr, "%%LineLength") == 0)
        llen = t2 ;               /* Change the line length. */
      else if (strcmp(tmpstr, "%%NumCols")    == 0)
        numcols = t2 ;            /* Change the number of columns. */
      else if (strcmp(tmpstr, "/fontHd_size")    == 0)
        hdngsize = t2 ; 	 /* Heading size */ 
      else if (strcmp(tmpstr, "%%EndComments") == 0)
        {

/* If this is the %%EndComments line from the prologue file, then we
 *  need to read (and output to stdout), the contents of the common
 *  prologue file, mp.common.ps
 */
	  cpro = (char *)calloc(strlen(prologue)+21, 1);
	  if(!cpro) malloc_err_disp_exit(__LINE__, __FILE__);
          SPRINTF(cpro, "%s/mp.common.ps", prologue) ;
          if ((cf = fopen(cpro, "r")) == NULL)
            {
		FPRINTF(stderr , catgets(cat_fd, ERR_SET, 4, \
              		"%s: Common prologue file %s not found.\n"),
                                  progname, cpro) ;
              exit(1) ;
            }
	  if(cpro) 	
		free(cpro);
          while (fgets(buf, MAXLINE, cf) != NULL) 
	  {

/* Fix for bug 4114795 - moved code below from previous "while"
 * to this "while" statement - nana.
 */

/* NOTE: This is not nice code but...
 *
 *       Check if the line just read starts with /fullheight
 *       If this is the case, then replace it with the appropriate
 *       page height (A4 or US).
 */

	     if (!strncmp(buf, "/fullwidth", 10))
		{
		  if (paper_size == US)
		    FPUTS("/fullwidth 8.5 inch def\n", stdout) ;
		  else if (paper_size == A4)
		    FPUTS("/fullwidth 595 def\n", stdout) ;
		}
	     else if (!strncmp(buf, "/fullheight", 11))
		{
		  if (paper_size == US)
		    FPUTS("/fullheight 11 inch def\n", stdout) ;
		  else if (paper_size == A4)
		    FPUTS("/fullheight 842 def\n", stdout) ;
		}
	     else FPUTS(buf, stdout) ;
	  }
          FCLOSE(cf) ;
        }
    }
  FCLOSE(pf) ;

  if(!landscape && current_pt_sz != NORMAL_PT_SZ ) {
		int dllen=llen, dplen=plen;
		float ll, pl;
		
		ll = (float)(NORMAL_PT_SZ * llen)/(float)(current_pt_sz); 
		pl = (float)(NORMAL_PT_SZ* plen)/(float)(current_pt_sz);
		llen = ll + ll-(int)ll;
		plen = pl + pl-(int)pl;

		if(!plen || !llen || 
		(paper_size==US && current_pt_sz > MAX_US_PORTRAIT_PT_SZ) || 
		(paper_size==A4 && current_pt_sz > MAX_A4_PORTRAIT_PT_SZ)) {
			float ll, pl;
			current_pt_sz = NORMAL_PT_SZ + PT_SZ_INC;
  			ll = (float)(NORMAL_PT_SZ * dllen)/(float)current_pt_sz; 
			pl = (float)(NORMAL_PT_SZ* dplen)/(float)(current_pt_sz);
			llen = ll + ll-(int)ll;
			plen = pl + pl-(int)pl;
		}
		if(paper_size==A4) {
			float ll, pl;
			ll = (float)llen*8.268/8.5;
			pl = (float)plen*11.693/11.0;
			llen = ll + ll-(int)ll;
			plen = pl + pl-(int)pl;
		}
	if ( current_pt_sz != NORMAL_PT_SZ )
   		FPRINTF(stdout,"/FontSize %d def\n", current_pt_sz);
   } else if ( landscape && current_pt_sz != LANDSCAPE_PT_SZ ) {
		int dllen=llen, dplen=plen;
		float ll, pl;

		ll = (float)(LANDSCAPE_PT_SZ * llen)/(float)(current_pt_sz); 
		pl = (float)(LANDSCAPE_PT_SZ* plen)/(float)(current_pt_sz);
		llen = ll + ll-(int)ll;
		plen = pl + pl-(int)pl;

		if(!plen || !llen || 
		(paper_size==US && current_pt_sz > MAX_US_LANDSCAPE_PT_SZ) || 
		(paper_size==A4 && current_pt_sz > MAX_A4_LANDSCAPE_PT_SZ)) {
			float ll, pl;
			current_pt_sz = LANDSCAPE_PT_SZ + PT_SZ_INC;
  			ll = (float)(LANDSCAPE_PT_SZ * dllen)/(float)current_pt_sz; 
			pl = (float)(LANDSCAPE_PT_SZ* dplen)/(float)(current_pt_sz);
			llen = ll + ll-(int)ll;
			plen = pl + pl-(int)pl;
		}
		if(paper_size==A4) {
			float ll, pl;
			ll = (float)llen*11.693/11.0;
			pl = (float)plen*8.268/8.5;
			llen = ll + ll-(int)ll;
			plen = pl + pl-(int)pl;
		}
	if ( current_pt_sz != LANDSCAPE_PT_SZ )
   		FPRINTF(stdout,"/FontSize %d def\n", current_pt_sz);
   }
	
  if(!landscape && current_pt_sz != NORMAL_PT_SZ) {
	float ft=0.0;
	float extra_lines = 0;

	ft=((float)NORMAL_ADD_SPACING*(float)current_pt_sz)/(float)NORMAL_PT_SZ;
	extra_lines = (ft*(float)plen)/(float)current_pt_sz;
	if(extra_lines > 0.15 )
		plen-=(extra_lines);
  } else if ( landscape && current_pt_sz !=LANDSCAPE_PT_SZ) {
	float ft=0.0;
	float extra_lines = 0;

	ft=((float)LANDSCAPE_ADD_SPACING*(float)current_pt_sz)/(float)LANDSCAPE_PT_SZ;
	extra_lines = (ft*(float)plen)/(float)current_pt_sz;
	if(extra_lines > 0.10 )
		plen-=(extra_lines);
  }

  if(nodecor)	
	FPRINTF(stdout,"/nodecorations true  def\n");
  else
	FPRINTF(stdout,"/nodecorations false  def\n");

  if (multi_print_var == TRUE ) {
	clear_dict();
	pcf_tt_put_postscript_prologue();
  }
	
}


void
startline()
{
  if(!multi_print_var)
  	PRINTF("(") ;
}


void
startpage()
{
  PRINTF("%%%%Page: ? %1d\n", ++tpn) ;
  PRINTF("%1d newpage\n", tpn) ;
  new_page = 1;
  set_defs() ;
  if (!multi_print_var)
  	FPUTS("sf\n", stdout) ;

}


void
startfile()
{
}


void
textshow(s)
char *s ;
{
  useline() ;
  if (multi_print_var) {
  	expand((unsigned char *) s, ROMNFONT, TRUE) ;
  } else {
  	startline() ;
	expand((unsigned char *) s, 0, FALSE);
  	endline() ;
  }
}


void
useline()   /* Called in order to ready a line for printing. */
{
  if (++linect > plen || end_of_page == TRUE)
    {
      endcol() ;
      if (colct < numcols) return ;
      colct = 0 ;
      endpage() ;
      linect = 1 ;
      startpage() ;
    }
}

static void put_pcf_postscript(int index ) {
	
	pcf_postscript( print_info_st[index].val,\
		print_info_st[index].font_u.pcf_st.pcfbm,
		 print_info_st[index].font_u.pcf_st.Cmet,
		print_info_st[index].font_u.pcf_st.scCmet );

}

static void put_tt_postscript(int index ) {

	tt_ps_glyph_start( print_info_st[index].val,\
		print_info_st[index].scale_fact, \
		stdout);
	tt_ps_glyph_print(print_info_st[index].font_u.ttf_st.ttodata, stdout);
	tt_ps_glyph_end(print_info_st[index].val, stdout);
}

static void put_printer_postscript(int index) {
	static char *current_pfont=NULL;
	ucs4_t val;
	int pstle = print_info_st[index].print_style;

	if (new_page || current_pfont == NULL || mp_newdict_flg) {
		 FPRINTF(stdout, "FDEF\n");
	}

	if (current_pfont == NULL 
	|| strcmp( current_pfont, print_info_st[index].font_u.tp1_st.tp1_font->fontname )
	|| new_page || mp_newdict_flg) {
		current_pfont=(char *)realloc(current_pfont,strlen( print_info_st[index].font_u.tp1_st.tp1_font->fontname)+1);
		strcpy(current_pfont, print_info_st[index].font_u.tp1_st.tp1_font->fontname);
		if(pstle!=HDNGFONT)
			FPRINTF(stdout, "/%s %d SF\n",current_pfont, current_pt_sz);
		else
			FPRINTF(stdout, "fontH\n");
			
		new_page = 0;
		mp_newdict_flg = 0;
	}

	val = *(print_info_st[index].font_u.tp1_st.target_char);

	if (iswprint(val) || iswspace(val) ) {
		
		write_output("(");
		switch (val) {
			case '\\' : write_output("\\\\") ;
					break ;
			case '('  : write_output("\\(") ;
					break ;
			case ')'  : write_output("\\)") ;
					break ;
			default   : fputwc(val,stdout);
					break;
		}

		FPRINTF(stdout,") SH\n");

	}

}
static void check_n_put_newpage() {
	static int first_flg=0;
	static int done_for_page=0;

	if (!first_flg++ || pageno != done_for_page ) {
		done_for_page=pageno;
		new_page_routines();
	}
	if (SPACINGwidth  == -1 ) 
		SPACINGwidth = (float)SPACE_WIDTH_PER_PTSZ * current_pt_sz;
}
static void
multi_print(int index) {

	int c = print_info_st[index].val;
	double incr_x=0.0, incr_y=0.0;
	int i=-1;

	check_n_put_newpage();
	if ( IS_PCF(index)) {
		if ( print_info_st[index].scale_fact== -1) {
			fprintf(stdout, "%.1f 0 R\n", SPACINGwidth);
			if(abs(wcwidth(c))>1)
				fprintf(stdout, "%.1f 0 R\n", SPACINGwidth);
			free(print_info_st[index].font_u.pcf_st.Cmet);
			free(print_info_st[index].font_u.pcf_st.scCmet);
			return;
		}

		if(!gen_incremental_pos(c, &incr_x, &incr_y) ) {
			i = pcf_incremental_pos(index, &incr_x, &incr_y);
		}
		if(i==1)   put_pcf_postscript(index);
    		else if (i==0) fprintf(stdout,"C%x\n",c);
		fprintf(stdout, "%.1f 0 R\n", incr_x);
		free(print_info_st[index].font_u.pcf_st.Cmet);
		free(print_info_st[index].font_u.pcf_st.scCmet);

	} else if ( IS_TTF(index)) {
		/* if certain glyphs are missing from the font put
		 * the white space char instead
		 */
		if ( print_info_st[index].scale_fact== -1) {
			if(abs(wcwidth(c))>1)
				fprintf(stdout, "%.1f 0 R\n", (float)current_pt_sz);
			else
				fprintf(stdout, "%.1f 0 R\n", SPACINGwidth);
			return;
		}
		if(!gen_incremental_pos(c, &incr_x, &incr_y) ) {
			i = tt_incremental_pos(index, &incr_x,&incr_y);
		}
		if(i==1)   put_tt_postscript(index);
		else if(i==0) fprintf(stdout,"U%x\n",c);
		fprintf(stdout, "%.1f 0 R\n", incr_x);
		tt2ps_clear_outlinedata(print_info_st[index].font_u.ttf_st.ttodata);
        } else if (  IS_TP1(index)) {
		put_printer_postscript(index);
		free(print_info_st[index].font_u.tp1_st.target_char);
	}

}

void
pcf_tt_put_postscript_prologue(void)
{
	write_output("\n");
	write_output("/G /gsave\tload def\n");
	write_output("/Gr /grestore\tload def\n");
	write_output("/I /imagemask\tload def\n");
	write_output("/L /lineto\tload def\n");
	write_output("/C /curveto\tload def\n");
	write_output("/Tr /translate\tload def\n");
	write_output("/T /true\tload def\n");
	write_output("/S /scale\tload def\n");
	write_output("/SF /selectfont\tload def\n");
	write_output("/CL /closepath\tload def\n");
	write_output("/N /newpath\tload def\n");
	write_output("/F /fill\tload def\n");
	write_output("/R /rmoveto\tload def\n");
	write_output("/RO /rotate\tload def\n");
	write_output("/SP /showpage\tload def\n");
	write_output("/CR { CL F Gr }\tbind def\n");
	write_output("/GR { G CP Tr }\tbind def\n");
	write_output("/IG { I Gr }\tbind def\n");
	write_output("/CT { CP G Tr }\tbind def\n");
	write_output( "/FDEF { \n\t/sf () def \n\t/BoldFont () def\n  } bind def\n");
	write_output("/MP { \n\t/linecnt exch def\n\t/addpoint exch def");
	write_output("\n\t/tempypos abs-yinit linecnt FontSize addpoint add mul sub def");
	write_output("\n\tabs-xinit tempypos M\n  } bind def\n");
}

static void
new_col_routines() {
  	if (landscape == TRUE) {
		fprintf(stdout, "\ncurrentpoint ");
		fprintf(stdout, "\n/abs-yinit exch def");
		fprintf(stdout, "\n/abs-xinit exch def");
	}	
	fprintf(stdout, "\n/ypos abs-yinit def");
	fprintf(stdout, "\n/xpos abs-xinit def\n");
	fprintf(stdout, "\nxpos  ypos M\n");
	fprintf(stdout, "\nsf\n");
}
	
static void
new_page_routines() {
	fprintf(stdout, "\nCP");
	fprintf(stdout, "\n/abs-yinit exch def");
	fprintf(stdout, "\n/abs-xinit exch def");
	fprintf(stdout, "\n/ypos abs-yinit def");
	fprintf(stdout, "\n/xpos abs-xinit def\n");
	fprintf(stdout, "\nxpos  ypos M\n");
	fprintf(stdout, "\nsystemdict /setpacking known");
	fprintf(stdout, "\n{true setpacking}");
	fprintf(stdout, "\nif\n");

}
