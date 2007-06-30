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
#pragma ident   "@(#)configuration.c	1.6 99/11/08 SMI"

#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/layout.h>
#include "general_header.h"
#include "configuration.h"
#include "pcf.h"
#include "ttf.h"
#include "tp1.h"

/* global variables used */

int pcf_ret=-2, ttf_ret=-2, tp1_ret=-2;
pcffont_t *CurrentFont;
pcffont_t *pcf_fonts;
ttffont_t *ttf_fonts;
tp1font_t *tp1_fonts;
static int printstyle=0;
static conf_t *scan_configuration(char *);
static FILE *open_config_file(char *);
static void scan_unknown_keywords(FILE *);
static int  get_keyword_count(FILE *, char *, char *);
static int get_fontnamealias_entries(FILE *, pcffont_t *, ttffont_t *, tp1font_t *);
static int get_fontgroup_entries(FILE *, fontgroup_t *) ;
static void err_missing_fields(int );
static void err_stat_file(int , char *) ;
static void err_non_numeric(int ) ;
static void assign_pcf_index(int, char *, int, int, fontgroup_t *) ;
static void assign_ttf_index(int, char *, int, int, fontgroup_t *) ;
static void assign_tp1_index(int, char *, int, int, fontgroup_t *) ;
static int get_pcffont_index(char *);
static int get_ttffont_index(char *);
static int get_tp1font_index(char *);
static int get_mapcode2font_entries( FILE *, mapcode2font_t *);
static void initialise_fontindex(void);
static int isnumstr(char *);
static int ishexstr(char *);
static unsigned int strint(char *);
static void lookup_groupname(char *, int, ucs4fontndx_t *, int);
static void set_grouptype( char * , mapcode2font_t *);
static int mmap_compare(const void *, const void *);
static int get_cnvcode2font_entries( FILE *, cnvcode2font_t *);
static void lookup_fontname(char *, int ) ;
static void shared_obj_info(conf_t *);
static int statfile(char *);
static void free_configuration(conf_t *);
static int get_afmfile_entries(FILE *);
static int afmfile_to_tp1( char *, char *, int);
static void scan_onetime_keywords(FILE *);
static void err_invalid_value(int );
static void test_pform(char *, int);
static void test_orient(char *, int);
static void test_numeral( char *, int);
static void test_textshape( char *, int);
static void test_swapping( char *, int);
static void test_context( char *, int);
static int load_tp1( ucs4fontndx_t *) ;
static int load_pcf( ucs4fontndx_t *) ;
static int load_ttf( ucs4fontndx_t *) ;
static int ucs4fontndxmap_compare(const void *, const void *);
static caddr_t getfaddr(char *,  char *);
static void env_expand(char *s) ;

/* Following are static vars used */

static conf_t conf;
static ucs4fontndx_t *ucs4fontndx;
static cparam_t *cparam=NULL;
static int pcf_no=0, ttf_no=0, tp1_no=0;
static int target_Xres = DEF_XRES;
static int target_Yres = DEF_YRES;

/* following are the global functional defined */

unsigned int read_orientation( ) { return(cparam?cparam->orient:ORIENTATION_LTR); }
unsigned int read_numerals( ) 	  { return(cparam?cparam->numeral:NUMERALS_NOMINAL); }
unsigned int read_textshaping( )   { return(cparam?cparam->textshape:TEXT_SHAPED); }
unsigned int read_swapping( )    { return(cparam?cparam->swapping:NULL); }
unsigned int read_context( )     { return(cparam?cparam->context:NULL); }

static void 
test_pform(char *st, int line) {
	if(eq(st,"WC")) {
		cparam->pform = WC;
	} else if(eq(st,"PLSOutput")) {
		cparam->pform = PLSOutput;
	} else {
		err_invalid_value(line);
	}
}

static void 
test_orient(char *st, int line) {

	if(eq(st,"ORIENTATION_LTR" )) {
		cparam->orient =ORIENTATION_LTR;
	} else if(eq(st,"ORIENTATION_RTL")) {
		cparam->orient =ORIENTATION_RTL;
	} else if(eq(st,"ORIENTATION_CONTEXTUAL")) {
		cparam->orient = ORIENTATION_CONTEXTUAL;
	} else {
		err_invalid_value(line);
	}
}
static void 
test_numeral(char *st, int line) {
	if(eq(st,"NUMERALS_NOMINAL")) {
		cparam->numeral =NUMERALS_NOMINAL;
	} else if(eq(st,"NUMERALS_NATIONAL")) {
		cparam->numeral =NUMERALS_NATIONAL;
	} else if(eq(st,"NUMERALS_CONTEXTUAL")) {
		cparam->numeral = NUMERALS_CONTEXTUAL;
	} else {
		err_invalid_value(line);
	}
}
static void 
test_textshape(char *st, int line) {
	if(eq(st,"TEXT_SHAPED" )) {
		cparam->textshape =TEXT_SHAPED;
	} else if(eq(st,"TEXT_NOMINAL")) {
		cparam->textshape =TEXT_NOMINAL;
	} else if(eq(st,"TEXT_SHFORM1")) {
		cparam->textshape = TEXT_SHFORM1;
	} else if(eq(st,"TEXT_SHFORM2")) {
		cparam->textshape = TEXT_SHFORM2;
	} else if(eq(st,"TEXT_SHFORM3")) {
		cparam->textshape = TEXT_SHFORM3;
	} else if(eq(st,"TEXT_SHFORM4")) {
		cparam->textshape = TEXT_SHFORM4;
	} else {
		err_invalid_value(line);
	}
}
static void 
test_context(char *st, int line) {

	if(eq(st,"CONTEXT_LTR" )) {
		cparam->context =CONTEXT_LTR;
	} else if(eq(st,"CONTEXT_RTL")) {
		cparam->orient =CONTEXT_RTL;
	} else {
		err_invalid_value(line);
	}
}

static void 
test_swapping(char *st, int line) {

	if(eq(st,"SWAPPING_NO" )) {
		cparam->orient =SWAPPING_NO;
	} else if(eq(st,"SWAPPING_YES")) {
		cparam->orient =SWAPPING_YES;
	} else {
		err_invalid_value(line);
	}
}
static void err_invalid_value(int line) {
	
	err_exit(catgets( cat_fd, ERR_SET , 8 ,
			"%s: config file line: %d invalid value\n"),progname ,
			line);
}

static void
scan_onetime_keywords(FILE *fp) {
	char *t;
	char buf[MAXLINE];
	int line_cnt=0;

	rewind(fp);


	while (fgets(buf, sizeof(buf), fp)) {
		line_cnt++;
		if (!ends_with_NL(buf))
			err_exit(catgets(cat_fd, ERR_SET, 9,
				"%s: config file line too long; line #%d\n" ), 
				progname, line_cnt);

		zapNL(buf);

		if ((t = strtok(buf, CFFS_STR)) == NULL) {
			continue;
		} else if (is_comment_char(*t)) {
			continue;
		} else if (eq(t, KEYWORD_PRESENTFORM)) {
			if ((t = next_tok()) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				test_pform(t, line_cnt);
			}
		} else if (eq(t, KEYWORD_ORIENT)) {
			if ((t = next_tok()) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				test_orient(t, line_cnt);
			}
		} else if (eq(t, KEYWORD_NUMERALS)) {
			if ((t = next_tok()) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				test_numeral(t, line_cnt);	
			}
		} else if (eq(t, KEYWORD_TEXTSHAPE)) {
			if ((t = next_tok()) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				test_textshape(t, line_cnt);

			}
		} else if (eq(t, KEYWORD_CONTEXT)) {
			if ((t = next_tok()) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				test_context(t, line_cnt);

			}
		} else if (eq(t, KEYWORD_SWAPPING)) {
			if ((t = next_tok()) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				test_swapping(t, line_cnt);

			}
		}
	}

}

static int 
afmfile_to_tp1( char *alias, char * file, int line_cnt) {
	int i;	
	for(i=0;i<tp1_no;i++) {
		if(eq(alias, tp1_fonts[i].name)) {
			if(statfile(file)) {
				tp1_fonts[i].afmfile = strdup(file);
				return i;
			}
		}
	}
	return -1;
}

static int 
get_afmfile_entries(FILE *fp) {
	
	char *t;
	char *tmp_hold;
	char buf[MAXLINE];
	int line_cnt=0, cnt=0;

	rewind(fp);

	while (fgets(buf, sizeof(buf), fp)) {
		line_cnt++;
		if (!ends_with_NL(buf))
			err_exit(catgets(cat_fd, ERR_SET, 9,
				"%s: config file line too long; line #%d\n" ), 
				progname, line_cnt);

		zapNL(buf);

		if ((t = strtok(buf, CFFS_STR)) == NULL) {
			continue;
		} else if (is_comment_char(*t)) {
			continue;
		} else if (eq(t, KEYWORD_AFMFILE)) {
			if ((t = next_tok()) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				tmp_hold = strdup(t);
			}
			if ((t = next_tok()) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				if (afmfile_to_tp1(tmp_hold, t, line_cnt)!= -1){
					cnt++;
				} else {
					err_exit(catgets(cat_fd, ERR_SET, 10,
						"%s: config file line # %d"
						" invalid Type1 alias name\n"),
						progname, line_cnt);
				}
					
			}
		}
	}
	return cnt;
}

static void 
free_configuration(conf_t * cf) {
	if (cf) {
		free(cf->gmap);
		free(cf->mmap);
		free(cf->cmap);
		}

}

static int
statfile(char *file) {
	struct stat st;
	if (stat(file, &st) == -1)
		return 0;
	return 1;
}


static void 
shared_obj_info(conf_t *cf) { 
	int j,k;

	/* Fill in font struct info pieces for pcf/tt/printer fonts */

	for (j = 0; j < cf->cmapN; j++) {
		for (k = 0; k < pcf_no; k++) {
			if (eq(cf->cmap[j].name, pcf_fonts[k].name)) {
				pcf_fonts[k].cufsym = strdup(cf->cmap[j].fsym);
				pcf_fonts[k].cufobj = strdup(cf->cmap[j].file);
				break;
			}
		}
	}


	for (j = 0; j < cf->cmapN; j++) {
		for (k = 0; k < ttf_no; k++) {
			if (eq(cf->cmap[j].name, ttf_fonts[k].name)) {
				ttf_fonts[k].cufsym = strdup(cf->cmap[j].fsym);
				ttf_fonts[k].cufobj = strdup(cf->cmap[j].file);
				break;
			}
		}
	}

	for (j = 0; j < cf->cmapN; j++) {
		for (k = 0; k < tp1_no; k++) {
			if (eq(cf->cmap[j].name, tp1_fonts[k].name)) {
				tp1_fonts[k].cufsym = 
						strdup(cf->cmap[j].fsym);
				tp1_fonts[k].cufobj = 
						strdup(cf->cmap[j].file);
				break;
			}
		}
	}

}
static void
lookup_fontname(char *name, int line) {
	int j;

	for (j = 0; j < pcf_no; j++) {
		if (eq(name, pcf_fonts[j].name)) {
			return ;
		}
	}
	for (j = 0; j < ttf_no; j++) {
		if (eq(name, ttf_fonts[j].name)) {
			return ;
		}
	}
	for (j = 0; j < tp1_no; j++) {
		if (eq(name, tp1_fonts[j].name)) {
			return ;
		}
	}
		err_exit(catgets( cat_fd, ERR_SET, 11 ,
				"%s: config file line: %d; "
				"unknown font name (%s).\n"), progname
				,line, name);
}
static int 
get_cnvcode2font_entries( FILE *fp, cnvcode2font_t *cu) {
	char *t;
	char buf[MAXLINE];
	int line_cnt=0, cnt=0;

	rewind(fp);

	while (fgets(buf, sizeof(buf), fp)) {
		line_cnt++;
		if (!ends_with_NL(buf))
			err_exit(catgets(cat_fd, ERR_SET, 9,
				"%s: config file line too long; line #%d\n" ), 
				progname, line_cnt);

		zapNL(buf);

		if ((t = strtok(buf, CFFS_STR)) == NULL) {
			continue;
		} else if (is_comment_char(*t)) {
			continue;
		} else if (eq(t, KEYWORD_CNVCODE2FONT)) {
			if ((t = next_tok()) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				cu->name = strdup(t);
				lookup_fontname(cu->name, line_cnt);
			}

			if ((t = strtok(NULL, CFFS_STR)) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				cu->fsym = strdup(t);
			}

			if ((t = strtok(NULL, CFFS_STR)) == NULL) {
				err_missing_fields(line_cnt);
			} else {
				cu->file = strdup(t);
			}
			env_expand(cu->file);
			if (!statfile(cu->file)) {
				err_stat_file(line_cnt, cu->file);
			} else {
				cu++;
				cnt++;
			}
		}
	}

	if (ferror(fp))
		err_exit(catgets( cat_fd, ERR_SET, 7,
			"%s: error reading configuration file\n"), progname);

	return cnt;
}
					
int
presform_fontndx(ucs4_t val) {
	int ndx, ndx1, ndx2, grp_idx;
	ucs4fontndx_t uf;
	ucs4fontndx_t *p;
	static int save_pt_sz=0;

	pcf_ret = ttf_ret = tp1_ret = -2;
	uf.n1 = val;
	uf.n2 = val;
	printstyle = 0;
	if ((p = (ucs4fontndx_t*) bsearch((void*) &uf, ucs4fontndx, conf.mmapN,
			sizeof(ucs4fontndx_t), ucs4fontndxmap_compare)) == NULL) {
		return XU_UCS4UNDEF;
	} else { 
		ndx = p->pcf_ndx[printstyle];	/* index to pcf font , if not exists -1 */
		ndx1 = p->ttf_ndx[printstyle];	/* index to tt font, if not exists -1 */
		ndx2 = p->tp1_ndx[printstyle];	/* index to printer font , if not exists -1 */
		/*
		printf("pcf font index : %d\nttf font index : %d\ntp1 font index :%d\n", ndx, ndx1, ndx2);
		*/
		/* Defaulting in case of pstyle is not configured */
		if ( ndx == -1 && ndx1 == -1 && ndx2 == -1 ) {
			printstyle=ROMAN;
			ndx = p->pcf_ndx[printstyle];
			ndx1 = p->ttf_ndx[printstyle];
		   	ndx2 = p->tp1_ndx[printstyle];
		}

		for(grp_idx=0; grp_idx<TOT_FONT_TYPES; grp_idx++) {

			if ( p->group_order[grp_idx] == TP1_STYLE ) {
				if ( ndx2 != -1 ) {
					if (tp1_fonts[ndx2].loaded == 0 ) 
						tp1_ret = load_tp1(p);
					else if (tp1_fonts[ndx2].loaded == 1 )
						tp1_ret = ndx2;
					if (tp1_ret >= 0) return(tp1_ret);
				}
			} else if ( p->group_order[grp_idx] == TTF_STYLE ) {
				if ( ndx1 != -1 ) {
					if (ttf_fonts[ndx1].loaded == 0 ) 
						ttf_ret = load_ttf(p);
					else if (ttf_fonts[ndx1].loaded == 1 )
						ttf_ret = ndx1;
					if (ttf_ret >= 0) return(ttf_ret);
				}
			} else if ( p->group_order[grp_idx] == PCF_STYLE ) {
				if ( ndx != -1 ) {
					if (pcf_fonts[ndx].loaded == 0 || 
							save_pt_sz != current_pt_sz) {
						save_pt_sz = current_pt_sz;
						pcf_ret = load_pcf(p);
						
					} else if (pcf_fonts[ndx].loaded == 1 )
						pcf_ret = ndx;
					if (pcf_ret >= 0) return(pcf_ret);
				}
			}
		}
		fprintf(stderr,catgets(cat_fd, WARN_SET, 1,\
				"%s: cannot load Type1/pcf/TT fonts "
				"for val 0x%08x\n"),progname,uf.n1);
		return(-1);
	}
}

enum pform_t conf_presform_width(void) {
	return (cparam->pform);
}
	
static int
ucs4fontndxmap_compare(const void *p1, const void *p2)
{
	ucs4fontndx_t *u1 = (ucs4fontndx_t*) p1;
	ucs4fontndx_t *u2 = (ucs4fontndx_t*) p2;

	if (u1->n1 < u2->n1 && u1->n1 < u2->n2) {
		return -1;
	} else if (u1->n1 > u2->n1 && u1->n1 > u2->n2) {
		return 1;
	} else if (u1->n1 >= u2->n1 && u1->n1 <= u2->n2) {
		return 0;
	} else {
		err_exit( catgets(cat_fd, ERR_SET, 5, "%s: font indexing error"
				" for presentation form ranges:  "
				"%x %x\n \t\t\t and %x %x\n"), progname, 
				u1->n1, u1->n2,
				u2->n1, u2->n2);
	}
}
static int
mmap_compare(const void *p1, const void *p2) {

	mapcode2font_t *u1 = (mapcode2font_t*) p1;
	mapcode2font_t *u2 = (mapcode2font_t*) p2;

	if (u1->n1 > u2->n1 && u1->n2 > u2->n2) {
		return 1;
	} else if (u1->n1 < u2->n1 && u1->n2 < u2->n2) {
		return -1;
	} else if (u1->n1 == u2->n1 && u1->n2 == u2->n2) {
		return 0;
	} else {
		err_exit( catgets(cat_fd, ERR_SET,6, "%s: incorrectly "
				"specfified %s values found. %s %x %x\n\t "
				"%s %x %x\n"), progname, KEYWORD_MAPCODE2FONT
				, KEYWORD_MAPCODE2FONT, KEYWORD_MAPCODE2FONT,
				u1->n1, u1->n2,
				u2->n1, u2->n2);
	}
}

static void
set_grouptype( char *ch , mapcode2font_t *mf) {
	int j;
	
	for (j = 0; j < conf.gmapN; j++) {
		if (eq(ch, conf.gmap[j].name)) {
			if ( TYPE_PCF == conf.gmap[j].type) {
				mf->pcf_name= strdup(conf.gmap[j].name);
			} else if( TYPE_TTF == conf.gmap[j].type) {
				mf->ttf_name= strdup(conf.gmap[j].name);
			} else if( TYPE_TP1 == conf.gmap[j].type) {
				mf->tp1_name= strdup(conf.gmap[j].name);
			}
		}
	}
}

static void
lookup_groupname(char *gn, int line, ucs4fontndx_t *ut, int grp_index) {
	int j,i;

	for (j = 0; j < conf.gmapN; j++) {
		if (eq(gn, conf.gmap[j].name)) {
			if(conf.gmap[j].type == TYPE_PCF ) {
				for(i=0;i<TOT_INDEX;i++)
					ut->pcf_ndx[i] = conf.gmap[j].gndx[i];
				ut->group_order[grp_index] = PCF_STYLE;
			} else if (conf.gmap[j].type == TYPE_TTF ) {
				for(i=0;i<TOT_INDEX;i++)
					ut->ttf_ndx[i] = conf.gmap[j].gndx[i];
				ut->group_order[grp_index] = TTF_STYLE;
			} else if (conf.gmap[j].type == TYPE_TP1 ) {
				for(i=0;i<TOT_INDEX;i++)
					ut->tp1_ndx[i] = conf.gmap[j].gndx[i];
				ut->group_order[grp_index] = TP1_STYLE;
			} else {
				err_exit( catgets( cat_fd, ERR_SET, 12 ,
					"%s: wrong groupmap: internal structure"
					" error\n"),progname);
			}
			break;
		}
    	}

	if (j >= conf.gmapN)
		err_exit( catgets( cat_fd, ERR_SET, 13 ,
				"%s: config file line: %d; "
				"unknown group name (%s).\n"), progname,
				line, gn);
}

static unsigned int 
strint(char *s) {
	char *p;
	unsigned long rval;

	errno=0;
	rval = strtoul(s, &p, 0);

	if ((rval == 0 && p == s) || (*p != '\0'))
		err_exit(catgets( cat_fd, ERR_SET, 31,
			"%s: config file: should be a number: %s.\n"), progname
			, s);

	if (rval == ULONG_MAX && errno == ERANGE) {
		err_exit( catgets( cat_fd, ERR_SET, 14,
				"%s: config file: string to integer conversion "
				"overflow: %s.\n"), progname, s); 
	}

	return rval;
}


static void 
err_non_numeric(int line) {
	err_exit( catgets( cat_fd, ERR_SET, 15,
			"%s: config file line: %d; "
			"non-numeric in numeric filed.\n"), progname, line);
}

static int
ishexstr(char *s)
{
	uchar_t c;

	while (c = *s++) {
		if (!isxdigit(c))
		return 0;
	}

	return 1;
}

static int
isnumstr(char *s)
{
	uchar_t c;

	if (eqn(s, "0x", 2)) {
		if (ishexstr(s+2))
			return 1;
		return 0;
	}

	while (c = *s++) {
		if (!isdigit(c))
			return 0;
	}

	return 1;
}
	
static 
void initialise_fontindex() {
	int i ,j, numucs4;
	numucs4 = conf.mmapN;

	for (j = 0; j < numucs4; j++) {
		ucs4fontndx[j].n1 = 0;
		ucs4fontndx[j].n2 = 0;
		for(i=0;i<TOT_INDEX;i++)
			ucs4fontndx[j].pcf_ndx[i]=-1; /* pcf index init */
		for(i=0;i<TOT_INDEX;i++)
			ucs4fontndx[j].ttf_ndx[i]=-1; /* ttf index init */
		for(i=0;i<TOT_INDEX;i++)
			ucs4fontndx[j].tp1_ndx[i]=-1; /* tp1 index init */
	}
}

static int 
get_mapcode2font_entries( FILE *fp, mapcode2font_t *mcf) {
	char *t;
	char buf[MAXLINE];
	int line_cnt=0, cnt=0,i;

	/* ufmap_t is a struct consisting of the index names  
	 * and the pointers to the pcf_fonts and ttf_fonts
	 * and tp1 fonts.
	 */

	if (( ucs4fontndx = (ucs4fontndx_t *) malloc(sizeof(ucs4fontndx_t)* \
			conf.mmapN)) == NULL ) {
		err_exit(catgets( cat_fd, ERR_SET , 26,
			"%s: Malloc failure; line %d, file %s\n"),progname,
			__LINE__, __FILE__ );
	}

	initialise_fontindex();

	rewind(fp);

	while (fgets(buf, sizeof(buf), fp)) {
		line_cnt++; 
		if (!ends_with_NL(buf))
			err_exit(catgets(cat_fd, ERR_SET, 9,
				"%s: config file line too long; line #%d\n" ), 
				progname, line_cnt);

		zapNL(buf);

		if ((t = strtok(buf, CFFS_STR)) == NULL) {
			continue;
		} else if (is_comment_char(*t)) {
			continue;
		} else if (eq(t, KEYWORD_MAPCODE2FONT)) {
			if ((t = next_tok())== NULL) {
				err_missing_fields(line_cnt);
			} else if (!isnumstr(t)) {
				err_non_numeric(line_cnt);	
			} else {
				mcf->n1 = strint(t);
			}
			if ((t = next_tok())== NULL) {
				err_missing_fields(line_cnt);
			} else if (isnumstr(t)) {
				mcf->n2 =  strint(t);
			} else if (*t == '-') {
				mcf->n2 = mcf->n1;
			} else
				err_non_numeric(line_cnt);
			if(mcf->n1 > mcf->n2)
				err_exit( catgets( cat_fd, ERR_SET, 16,
						"%s: config file line:%d"
						" starting range greater than"
						" ending range\n"), progname,
						line_cnt);
			if ((t = next_tok())== NULL) {
				err_missing_fields(line_cnt);
			} else {
				mcf->pcf_name="";
				mcf->ttf_name="";
				mcf->tp1_name="";
				ucs4fontndx[cnt].n1=mcf->n1;
				ucs4fontndx[cnt].n2=mcf->n2;
				lookup_groupname(t,line_cnt, &(ucs4fontndx[cnt]),0);
				set_grouptype(t,mcf);
			}

			/*
			 * Support for more than one font group type
			 * More than one group is not mandatory though!
			 */

			for(i=1;i<TOT_FONT_TYPES;i++) {
				if ((t = next_tok())!= NULL) {
					lookup_groupname(t,line_cnt, &(ucs4fontndx[cnt]), i);
					set_grouptype(t,mcf);
				}
			}

			mcf++;
			cnt++;
		}
	}

	if (ferror(fp)) 
		err_exit(catgets( cat_fd, ERR_SET, 7,
			"%s: error reading configuration file\n"), progname);
	return cnt;
}
	
static int
get_pcffont_index(char *pcfname) {

	int i;
	for(i=0; i<pcf_no; i++) {
		if(eq(pcf_fonts[i].name, pcfname)) {
			return(i);
		}
	}
	return -1;
}

static int
get_ttffont_index(char *ttfname) {
	int i;
	for(i=0; i<ttf_no; i++) {
		if(eq(ttf_fonts[i].name, ttfname)) {
			return(i);
		}
	}
	return -1;
}

static int
get_tp1font_index(char *tp1name) {
	int i;
	for(i=0; i<tp1_no; i++) {
		if(eq(tp1_fonts[i].name, tp1name)) {
			return(i);
		}
	}
	return -1;
}

static void 
assign_pcf_index(int line, char *t, int pos, int fmap_cnt, fontgroup_t *fg) {

	int ret;

	if( (ret = get_pcffont_index(t)) == -1 ) {
		err_exit( catgets( cat_fd, ERR_SET, 17,
			"%s: config file line: %d; unknown pcf font name \
				(%s).\n"),progname, line, t);
	} else {
		fg[fmap_cnt].gndx[pos]=ret;
	}
}
static void 
assign_ttf_index(int line, char *t, int pos, int fmap_cnt, fontgroup_t *fg) {

	int ret;

	if( (ret=get_ttffont_index(t)) == -1 ) {
		err_exit( catgets( cat_fd, ERR_SET, 18, 
				"%s: config file line: %d; unknown ttf font "
				"name (%s).\n"), progname, line, t);
	} else {
		fg[fmap_cnt].gndx[pos]=ret;
	}
}
static void 
assign_tp1_index(int line, char *t, int pos, int fmap_cnt, fontgroup_t *fg) {
	
	int ret;

	if( (ret = get_tp1font_index(t)) == -1 ) {
		err_exit(catgets( cat_fd, ERR_SET, 19,
			"%s: config file line: %d; unknown tp1 font name\
				(%s).\n"),progname, line, t);
	} else {
		fg[fmap_cnt].gndx[pos]=ret;
	}
}

static void err_missing_fields(int line) {
	err_exit( catgets( cat_fd, ERR_SET, 20,
		"%s: config file line: %d; "
		"missing field(s).\n"), progname,  line);
}
static void err_stat_file(int line, char *fname) {
	err_exit(catgets( cat_fd, ERR_SET, 21,
			"%s: config file line: %d,"
			"cannot stat font file (%s)\n"), progname,
			line, fname);
}

static int 
get_fontgroup_entries(FILE *fp, fontgroup_t *fgroup) {
	char *t;
	char *tmp_hold;
	char buf[MAXLINE];
	int line_cnt=0, fgroup_cnt=0;

	rewind(fp);

	while (fgets(buf, sizeof(buf), fp)) {
		line_cnt++;
		if (!ends_with_NL(buf)) {
			err_exit(catgets(cat_fd, ERR_SET, 9,
				"%s: config file line too long; line #%d\n" ), 
				progname, line_cnt);
		}
		zapNL(buf);
		if((t = strtok(buf, CFFS_STR)) == NULL) {
			continue;
		} else if(is_comment_char(*t)) {
			continue;
		} else if (eq(t, KEYWORD_FONTGROUP )) {
			if ((t = next_tok())== NULL) {
				err_missing_fields(line_cnt);
			} else
				tmp_hold =  strdup(t);

			if ((t = next_tok())== NULL) {
				err_missing_fields(line_cnt);
			} else if (eq(t, KEYWORD_PCF)) {	
				fgroup[fgroup_cnt].type = TYPE_PCF;
				fgroup[fgroup_cnt].name =  tmp_hold;
				if ((t = next_tok())== NULL) {
					err_missing_fields(line_cnt);
				}  else {
					assign_pcf_index(line_cnt, t, 
							ROMAN_INDEX, 
							fgroup_cnt,
							fgroup);
				} 
				if ((t = next_tok()) != NULL) {
					assign_pcf_index(line_cnt, t,
							BOLD_INDEX, fgroup_cnt,
							fgroup);
				}
				if ((t = next_tok()) != NULL) {
					assign_pcf_index(line_cnt, t,
							ITALIC_INDEX, 
							fgroup_cnt,
							fgroup);
				}
				if ((t = next_tok()) != NULL) {
					assign_pcf_index(line_cnt, t,
							BOLDITALIC_INDEX, 
							fgroup_cnt,
							fgroup);
				}
				fgroup_cnt++;
				continue;
			} else if (eq(t, KEYWORD_TTF)) {	
				fgroup[fgroup_cnt].type = TYPE_TTF;
				fgroup[fgroup_cnt].name =  tmp_hold;
				if ((t = next_tok())== NULL) {
					err_missing_fields(line_cnt);
				}  else {
					assign_ttf_index(line_cnt, t, 
							ROMAN_INDEX, 
							fgroup_cnt,
							fgroup);
				} 
				if ((t = next_tok()) != NULL) {
					assign_ttf_index(line_cnt, t,
							BOLD_INDEX, fgroup_cnt,
							fgroup);
				}
				if ((t = next_tok()) != NULL) {
					assign_ttf_index(line_cnt, t,
							ITALIC_INDEX, 
							fgroup_cnt,
							fgroup);
				}
				if ((t = next_tok()) != NULL) {
					assign_ttf_index(line_cnt, t,
							BOLDITALIC_INDEX, 
							fgroup_cnt,
							fgroup);
				}
				fgroup_cnt++;
				continue;
			} else if (eq(t, KEYWORD_TP1)) {	
				fgroup[fgroup_cnt].type = TYPE_TP1;
				fgroup[fgroup_cnt].name =  tmp_hold;

				if ((t = next_tok())== NULL) {
					err_missing_fields(line_cnt);
				}  else {
					assign_tp1_index(line_cnt, t, 
							ROMAN_INDEX, 
							fgroup_cnt,
							fgroup);
				} 
				if ((t = next_tok()) != NULL) {
					assign_tp1_index(line_cnt, t,
							BOLD_INDEX, fgroup_cnt,
							fgroup);
				}
				if ((t = next_tok()) != NULL) {
					assign_tp1_index(line_cnt, t,
							ITALIC_INDEX, 
							fgroup_cnt,
							fgroup);
				}
				if ((t = next_tok()) != NULL) {
					assign_tp1_index(line_cnt, t,
							BOLDITALIC_INDEX, 
							fgroup_cnt,
							fgroup);
				}
				fgroup_cnt++;
				continue;
			}
		}
	}

	if(ferror(fp))
		err_exit(catgets(cat_fd, ERR_SET, 7,
			"%s: error reading configuration file\n"), progname);
	return (fgroup_cnt);
}
		

static int 
get_fontnamealias_entries(FILE *fp, pcffont_t *pcf, ttffont_t *ttf, tp1font_t *tp1) {
	char *t;
	char buf[MAXLINE];
	char *tmp_hold;
	int line_cnt=0, pcf_cnt=0, ttf_cnt=0, tp1_cnt=0;

	rewind(fp);

	while (fgets(buf, sizeof(buf), fp)) {
		line_cnt++;
		if(!ends_with_NL(buf)) {
			err_exit(catgets(cat_fd, ERR_SET, 9,
				"%s: config file line too long; line #%d\n" ), 
				progname, line_cnt);
		}
		zapNL(buf);
		if((t = strtok(buf, CFFS_STR)) == NULL) {
			continue;
		} else if(is_comment_char(*t)) {
			continue;
		} else if (eq(t, KEYWORD_FONTNAMEALIAS)) {
			if ((t = next_tok())== NULL) {
				err_missing_fields(line_cnt);
			} else 
				tmp_hold = strdup(t);	
				
			if ((t = next_tok())== NULL) {
				err_missing_fields(line_cnt);
			} else if (eq(t, KEYWORD_PCF)) {	
				pcf[pcf_cnt].name = tmp_hold;
				if ((t = next_tok())== NULL) {
					err_missing_fields(line_cnt);
				} else {
					pcf[pcf_cnt].file =  strdup(t);	
					env_expand(pcf[pcf_cnt].file);
				}
				pcf_cnt++;
			} else if (eq(t, KEYWORD_TTF)) {	
				ttf[ttf_cnt].name = tmp_hold;
				if ((t = next_tok())== NULL) {
					err_missing_fields(line_cnt);
				} else {
					ttf[ttf_cnt].file =  strdup(t);	
					env_expand(ttf[ttf_cnt].file);
				}
				ttf_cnt++;
			} else if (eq(t, KEYWORD_TP1)) {
				tp1[tp1_cnt].name = tmp_hold;
				if ((t = next_tok())== NULL) {
					err_missing_fields(line_cnt);
				} else {
					tp1[tp1_cnt].file =  strdup(t);	
				}
				tp1_cnt++;
			}
		}
	}

	if (ferror(fp))
		err_exit(catgets( cat_fd, ERR_SET, 7,
			"%s: error reading configuration file\n"), progname);

	return(pcf_cnt+ttf_cnt+tp1_cnt);
}

/*
 * This function is not general , see the position of primary
 * and secondary keyword is hardcoded as 1 and 3
 */

static int  get_keyword_count(FILE *fp, char *primary, char *secondary) {
	int line_cnt=0, kwc=0;
	char *t;
	char buf[MAXLINE];

	rewind(fp);

	while (fgets(buf, sizeof(buf), fp)) {
		line_cnt++;
		if (!ends_with_NL(buf)) {
			err_exit(catgets(cat_fd, ERR_SET, 9,
				"%s: config file line too long; line #%d\n" ), 
				progname, line_cnt);
		}
		zapNL(buf);
		if ((t = strtok(buf, CFFS_STR)) == NULL) {
			continue;
		} else if (is_comment_char(*t)) {
			continue;
		} else if (eq(t, primary)) {
			next_tok();
			if ((t = next_tok()) == NULL) {

				err_exit( catgets( cat_fd, ERR_SET, 20,
				"%s: config file line: %d; "
				"missing field(s).\n"), progname,  line_cnt);

			} else if ( secondary == NULL ) {
				kwc++;
			} else if (( secondary ) && (eq(t, secondary))) {
				kwc++;
			} else {
				continue;
			}
		
		}
	}
	if (ferror(fp))
		err_exit(catgets( cat_fd, ERR_SET, 7,
			"%s: error reading configuration file\n"), progname);
	return kwc;
}


static void scan_unknown_keywords(FILE *conf_fp) {
	int line_cnt=0;
	char *t;
	char buf[MAXLINE];

	rewind(conf_fp);
	while(fgets(buf, sizeof(buf), conf_fp)) {
		line_cnt++;
		if(!ends_with_NL(buf)) {
			err_exit(catgets(cat_fd, ERR_SET, 9,
				"%s: config file line too long; line #%d\n" ), 
				progname, line_cnt);
		}
		zapNL(buf);
		if ((t = strtok(buf, CFFS_STR)) == NULL) {
			continue;
		} else if (is_comment_char(*t)) {
			continue;
		} else if( !eq(t,KEYWORD_PRESENTFORM) &&
				!eq(t,KEYWORD_ORIENT) &&
				!eq(t,KEYWORD_NUMERALS) &&
				!eq(t,KEYWORD_TEXTSHAPE) &&
				!eq(t,KEYWORD_FONTNAMEALIAS) &&
				!eq(t,KEYWORD_FONTGROUP) &&
				!eq(t,KEYWORD_AFMFILE) &&
				!eq(t,KEYWORD_MAPCODE2FONT) &&
				!eq(t,KEYWORD_CNVCODE2FONT)) {
			err_exit(catgets( cat_fd, ERR_SET, 22,
				"%s: unknown keyword: %s in line %d\n"),
				progname, t, line_cnt);
		}
	}

	if (ferror(conf_fp))
		err_exit(catgets( cat_fd, ERR_SET, 7,
			"%s: error reading configuration file\n"), progname);
}

void err_exit(char *msg, ...) {
	va_list *ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	if(target_printer) {
		xpr_print(NULL, 0, FEND);	
		xpr_end();
	}
	exit(1);
}

static void env_expand(char *s) {
	char *temp;
	char path_sep='/';
	char doll='$';
	char *temp1=strdup(s);

	while((temp=strchr(temp1,doll))!=NULL) {
		char *tc,*td;
		
		tc=strdup(temp);
		if((td=strchr(tc,path_sep))!=NULL) { 
			*td='\0';
			temp1=++td;
		} else {
			temp1=NULL;
		}
		if(tc) {
			char *genv=(char *)getenv(tc+1);

			replace_str(s,tc,genv?genv:"");
		}
		if (!temp1) break;
	}
}

static FILE *open_config_file(char *path){
	FILE *fp;
	if (!statfile(path)) return((FILE*)NULL);

	if((fp = fopen(path,"r"))==NULL) {
		err_exit(catgets( cat_fd, ERR_SET, 23,
			"%s: config file %s don't have read permissions. "
			"Exiting..\n"), progname, path);
	}
	return(fp);
}

static conf_t *scan_configuration(char *path) {
	FILE *conf_fp;
	int tot_no=0, pcfg_no=0, ttfg_no=0, tp1g_no=0;
	int fnum=0, tnum=0, mnum=0, gnum=0, cnum=0;
	int i,j;
	fontgroup_t *fontgroup;
	mapcode2font_t *mapcode2font;
	cnvcode2font_t *cnvcode2font;


	conf_fp = open_config_file(path);
	if ( conf_fp == (FILE *)NULL ) return ((conf_t *)NULL);
	scan_unknown_keywords(conf_fp);

	if((cparam = (cparam_t *) malloc(sizeof(cparam_t)))==NULL)
		err_exit(catgets( cat_fd, ERR_SET , 26,
			"%s: Malloc failure; line %d, file %s\n"),progname,
			__LINE__, __FILE__ );

	memset(cparam, 0, sizeof(cparam_t));

	scan_onetime_keywords(conf_fp);

	pcf_no = get_keyword_count(conf_fp, KEYWORD_FONTNAMEALIAS, KEYWORD_PCF);
	ttf_no = get_keyword_count(conf_fp, KEYWORD_FONTNAMEALIAS, KEYWORD_TTF);
	tp1_no = get_keyword_count(conf_fp, KEYWORD_FONTNAMEALIAS, KEYWORD_TP1);

	tot_no = pcf_no+ttf_no+tp1_no;

	if ((pcf_fonts = (pcffont_t*)
			malloc(sizeof(pcffont_t) * pcf_no)) == NULL)
		err_exit(catgets( cat_fd, ERR_SET , 26,
			"%s: Malloc failure; line %d, file %s\n"),progname,
			__LINE__, __FILE__ );

	bzero(pcf_fonts, sizeof(pcffont_t) * pcf_no);

	if ((ttf_fonts = (ttffont_t*)
			malloc(sizeof(ttffont_t) * ttf_no)) == NULL)
		err_exit(catgets( cat_fd, ERR_SET , 26,
			"%s: Malloc failure; line %d, file %s\n"),progname,
			__LINE__, __FILE__ );

	bzero(ttf_fonts, sizeof(ttffont_t) * ttf_no);

	if ((tp1_fonts = (tp1font_t*)
			malloc(sizeof(tp1font_t) * tp1_no)) == NULL)
		err_exit(catgets( cat_fd, ERR_SET , 26,
			"%s: Malloc failure; line %d, file %s\n"),progname,
			__LINE__, __FILE__ );

	bzero(tp1_fonts, sizeof(tp1font_t) * tp1_no);

	fnum = get_fontnamealias_entries(conf_fp, pcf_fonts, ttf_fonts, tp1_fonts);
	if ( fnum != tot_no )
		err_exit(catgets( cat_fd, ERR_SET , 24,
		"%s: number of %s entries processed != %s"
		" count.\n"), progname, KEYWORD_FONTNAMEALIAS,
		KEYWORD_FONTNAMEALIAS);

	get_keyword_count(conf_fp, KEYWORD_AFMFILE, NULL);
	get_afmfile_entries(conf_fp);

	pcfg_no = get_keyword_count(conf_fp, KEYWORD_FONTGROUP, KEYWORD_PCF);
	ttfg_no = get_keyword_count(conf_fp, KEYWORD_FONTGROUP, KEYWORD_TTF);
	tp1g_no = get_keyword_count(conf_fp, KEYWORD_FONTGROUP, KEYWORD_TP1);
	tnum = pcfg_no + ttfg_no + tp1g_no;

	conf.gmapN = tnum;

	if (( fontgroup = ( fontgroup_t *)
			 malloc(sizeof(fontgroup_t)*tnum)) == NULL)
		err_exit(catgets( cat_fd, ERR_SET , 26,
			"%s: Malloc failure; line %d, file %s\n"),progname,
			__LINE__, __FILE__ );

	for(i=0;i<tnum;i++) {
		fontgroup[i].type=-1;
		for(j=0;j<TOT_INDEX;j++)
			fontgroup[i].gndx[j]=-1;
	}

	gnum = get_fontgroup_entries(conf_fp, fontgroup);

	if(tnum != gnum)
		err_exit(catgets( cat_fd, ERR_SET , 24,
		"%s: number of %s entries processed != %s"
		" count.\n"), progname, KEYWORD_FONTGROUP, KEYWORD_FONTGROUP);
	conf.gmap = fontgroup;

	tnum = get_keyword_count(conf_fp, KEYWORD_MAPCODE2FONT, NULL);

	conf.mmapN = tnum;

	if(( mapcode2font = ( mapcode2font_t *)
			malloc(sizeof(mapcode2font_t)*tnum)) == NULL)
		err_exit(catgets( cat_fd, ERR_SET , 25,
			"%s: Malloc failure; line %d, file %s\n"),progname,
			__LINE__, __FILE__ );

	mnum = get_mapcode2font_entries(conf_fp, mapcode2font);

	if ( tnum != mnum )
		err_exit(catgets( cat_fd, ERR_SET , 24,
		"number of %s entries processed != %s"
		" count.\n"), progname, KEYWORD_MAPCODE2FONT,
		KEYWORD_MAPCODE2FONT);

	conf.mmap = mapcode2font;

	/* sort the configuration  mapcode2font */
	qsort(conf.mmap, conf.mmapN, sizeof(mapcode2font_t), mmap_compare);

	tnum = get_keyword_count(conf_fp, KEYWORD_CNVCODE2FONT, NULL);

	conf.cmapN = tnum;

	if (( cnvcode2font = (cnvcode2font_t *)
			malloc(sizeof(cnvcode2font_t)*tnum)) == NULL)

		err_exit(catgets( cat_fd, ERR_SET , 26,
			"%s: Malloc failure; line %d, file %s\n"),progname,
			__LINE__, __FILE__ );

	cnum = get_cnvcode2font_entries( conf_fp, cnvcode2font);

	if ( tnum != cnum )
		err_exit(catgets( cat_fd, ERR_SET , 24,
		"%s: number of %s entries processed != %s"
		" count.\n"), progname, KEYWORD_CNVCODE2FONT, 
		KEYWORD_CNVCODE2FONT);

	conf.cmap = cnvcode2font;

#ifdef DEBUG 
	for (i=0;i<mnum;i++) {
		fprintf(stderr,"ucs4fontndx[%d].n1 = 0x%08x\n", i,ucs4fontndx[i].n1);
		fprintf(stderr,"ucs4fontndx[%d].n2 = 0x%08x\n", i,ucs4fontndx[i].n2);
		for(j=0;j<TOT_INDEX;j++) {
			fprintf(stderr,"ucs4fontndx[%d].pcf_ndx[%d] = %d\n", i, j, ucs4fontndx[i].pcf_ndx[j]);
		}
		for(j=0;j<TOT_INDEX;j++) {
			fprintf(stderr,"ucs4fontndx[%d].ttf_ndx[%d] = %d\n", i, j, ucs4fontndx[i].ttf_ndx[j]);
		}
		for(j=0;j<TOT_INDEX;j++) {
			fprintf(stderr,"ucs4fontndx[%d].tp1_ndx[%d] = %d\n", i, j, ucs4fontndx[i].tp1_ndx[j]);
		}
	}
#endif

	fclose(conf_fp);
	return &conf;
}
	
static caddr_t
getfaddr(char *symname, char *file)
{
    void *dlh;
    caddr_t pf;

    if ((dlh = dlopen(file, RTLD_NOW)) == NULL)
    	err_exit(catgets(cat_fd, ERR_SET,26, "%s: config file %s dlopen error: %s\n"), progname, file, dlerror()); 

    if ((pf = (caddr_t) dlsym(dlh, symname)) == NULL)
    	err_exit(catgets(cat_fd, ERR_SET,27, "%s: config file %s dlsym error: %s\n"), progname, file, dlerror()); 

    return pf;
}

static int
load_tp1( ucs4fontndx_t *ufm ) {

	int ndx=ufm->tp1_ndx[printstyle];
	tp1_fonts[ndx].cuf =  (int(*)(ucs4_t)) getfaddr( \
			tp1_fonts[ndx].cufsym, tp1_fonts[ndx].cufobj);
	load_tp1_font(&(tp1_fonts[ndx])); 
	tp1_fonts[ndx].loaded = 1;
	return ndx;
}


static int 
load_pcf( ucs4fontndx_t *ufm ) {

	int ndx=ufm->pcf_ndx[printstyle];

	double target_ptsz = current_pt_sz * PTSZ_SCALE;
#ifdef SDEBUG
	fprintf(stderr, "%f --- target_ptsz\n", target_ptsz);
#endif	

	if (load_pcf_font(&(pcf_fonts[ndx])) == -1) {
	/*
		err_exit(catgets(cat_fd, WARN_SET,2,\
		"%s: cannot load font file (%s)\n"), progname,
		pcf_fonts[ndx].file);
		*/
		return -1;
	}
	
	scaling_factors(&(pcf_fonts[ndx]), target_ptsz, target_Xres, 
			target_Yres);
	scale_Fmetrics(&(pcf_fonts[ndx]));
	pcf_fonts[ndx].loaded = 1;
	pcf_fonts[ndx].cuf = (int(*)(ucs4_t)) getfaddr(pcf_fonts[ndx].cufsym, \
			pcf_fonts[ndx].cufobj);
	CurrentFont = &pcf_fonts[ndx];
	return ndx;
}

static int 
load_ttf( ucs4fontndx_t *ufm ) {
	int ret;
	int tt_ndx=ufm->ttf_ndx[printstyle];

	if (( ret = tt2ps_open_ttfont(ttf_fonts[tt_ndx].file, 
			&(ttf_fonts[tt_ndx]), ttf_fonts[tt_ndx].ttc_num)) < 0) {
			/*
		if(ret == TT_NOTFOUND){
	    		err_exit( catgets(cat_fd, ERR_SET, 28, "%s: config file "
					"can't open ttfont %s\n"), 
					progname,	
					ttf_fonts[tt_ndx].file);
		} else if (ret == TT_BADFONT) {
	    		err_exit( catgets(cat_fd, ERR_SET, 29, "%s: config file "
					":%s is not a true type font\n"), 
					progname,  
					ttf_fonts[tt_ndx].file);
		} else if (ret == TT_BADFORMAT) {
			err_exit(catgets(cat_fd, ERR_SET, 30, "%s: config file "
					":%s does not have valid cmap.\n"), 
					progname,  
					ttf_fonts[tt_ndx].file);
		}
		*/
		return(-1);
	} else {
		ttf_fonts[tt_ndx].loaded = 1;
		ttf_fonts[tt_ndx].cuf = (int(*)(ucs4_t)) 
				getfaddr(ttf_fonts[tt_ndx].cufsym,
				ttf_fonts[tt_ndx].cufobj);
		return(tt_ndx);
	}
}
/*
 * Font configuration file reading
 * 
 */
int config_scan(char *path) {
	conf_t  *t_conf;

	if((t_conf = scan_configuration(path)) == (conf_t *)NULL)
		return 0;
	shared_obj_info(t_conf);
	free_configuration(t_conf);
	return 1;
}

