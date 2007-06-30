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
/*  @(#)mp.h 3.5 99/07/25
 *
 *  Contains all the global definitions used by mp.
 *
 *  Copyright (c) Steve Holden and Rich Burridge.
 *                All rights reserved.
 *
 *  Permission is given to distribute these sources, as long as the
 *  copyright messages are not removed, and no monies are exchanged.
 *
 *  No responsibility is taken for any errors or inaccuracies inherent
 *  either to the comments or the code of this program, but if
 *  reported to me then an attempt will be made to fix them.
 */

#ifndef _MP_H
#define _MP_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/param.h>
#include <ctype.h>
#include <time.h>
#include <pwd.h>

#ifdef   SYSV
#include <string.h>
#endif /*SYSV*/

#define  FCLOSE       (void) fclose
#define  FPRINTF      (void) fprintf
#define  FPUTS        (void) fputs
#define  PRINTF       (void) printf
#define  PUTC         (void) putc
#define  SPRINTF      (void) sprintf
#define  SSCANF       (void) sscanf
#define  STRCPY       (void) strcpy
#define  STRNCPY      (void) strncpy
#define  UNGETC       (void) ungetc

/*  For all function declarations, if ANSI then use a prototype. */

#if  defined(__STDC__)                                              
#define P(args)  args
#else  /* ! __STDC__ */
#define P(args)  ()
#endif  /* STDC */

/* Configuration constants */

/* For message catalog file */

#define CATALOG_ROOT    "/usr"
#define NLS_PATH_STRING  CATALOG_ROOT "/lib/locale/%L/LC_MESSAGES/%N.cat:" \
CATALOG_ROOT "/lib/locale/C/LC_MESSAGES/%N.cat"
#define NLS_PATH_ENVIRON "NLSPATH"
#define MP_MSG_FILE     "mp"

#ifndef  PROLOGUE            /* PostScript prologue file */
#define  PROLOGUE     "/usr/local/lib"
#endif /*PROLOGUE*/

#define PROLOG_STR_OWN	"/usr/openwin/lib/locale/%s/print/"
#define PROLOG_STR	"/usr/lib/lp/locale/%s/mp/"
#define DEF_PROLOG_DIR	"/usr/lib/lp/locale/C/mp/"

#define  EQUAL(val)   (!strncmp(val, nextline, strlen(val)))
#define  INC          argc-- ; argv++ ;

#ifdef  NOINDEX
#define  index        strchr
#endif /*NOINDEX*/

#define  LINELENGTH   80     /* Number of characters per line. */

#ifndef  MAXPATHLEN
#define  MAXPATHLEN   810
#endif /*MAXPATHLEN*/

#define  MAXSIZES     4      /* Maximum number of different sizes. */

#define  NAMEFIELDS   3      /* Default no. of "words" from passwd gecos. */
#define  NAMELENGTH   18     /* Maximum allowable real user name. */
#define  PAGELENGTH   60     /* Number of lines per page. */
#define  MAXCONT      10     /* Maximum no of continuation header lines */
#if 0
#define  MAXLINE      1024   /* Maximum string length. */
#endif
#define  NORMAL_PT_SZ 10	/* Before scaling, all printing except landscape */
#define  LANDSCAPE_PT_SZ 7	/* For mixed script printing except landscpae */
#define  PT_SZ_INC 		0
#define  ABS_Y_DEC 		((float)(current_pt_sz * 0.80))
				/* This is used for setting the default print
				   size  different from NORMAL_PT_SZ or
				   LANDSCAPE_PT_SZ without need to change
				   .ps files */
#define  MAX_A4_PORTRAIT_PT_SZ	275 /* Maximum pt size values. */
#define  MAX_A4_LANDSCAPE_PT_SZ	175 /* Can't exceed these due to */
#define  MAX_US_PORTRAIT_PT_SZ	275 /* double width characters that may */
#define  MAX_US_LANDSCAPE_PT_SZ	175 /* be present in some locales */

#define  NORMAL_ADD_SPACING	1.0 /* Normal spacing for 10 ptsz font */
#define  LANDSCAPE_ADD_SPACING	0.3 /* additional interline spacing for landscape */
 
#ifndef TRUE
#define TRUE          1
#define FALSE         0
#endif  /*TRUE*/
 
typedef enum {DO_MAIL, DO_NEWS, DO_TEXT} document_type ;
typedef enum { A4, US }                  paper_type ;
typedef char bool;

extern time_t time          P((time_t *)) ;
extern struct tm *localtime P((const time_t *)) ;

bool emptyline              P((char *)) ;

extern FILE *fopen          P((const char *, const char *)) ;
extern void exit            P((int)) ;
extern char *asctime        P((const struct tm *)) ;
extern char *getlogin       P(()) ;
extern char *gets           P((char *)) ;
#ifndef _STRINGS_H
extern char *index          P((char *, char)) ;
#endif

#define BUFFERSIZE          1024
#define MAXSUBLEN           512
#define isEUC(c)            ((c) & 0x80 ? 1 : 0)
extern char  *euc_to_octal();

#if 0
/*
#ifndef SYSV
*/

extern char *strchr         P((char *, int)) ;
extern char *strcpy         P((char *, char *)) ;
extern char *strncpy        P((char *, char *, int)) ;
#endif /*SYSV*/

void boldshow                P((char *, char *)) ;
void endcol                  P(()) ;
void endfile                 P(()) ;
void endline                 P(()) ;
void endpage                 P(()) ;
int expand                  P((unsigned char *, int , char)) ;
int get_opt                 P((int, char **, char *)) ;
int get_options             P((int, char **)) ;
int hdr_equal               P((char *)) ;
int main                    P((int, char **)) ;
void mixedshow               P((char *, char *)) ;
int printfile               P(()) ;
int process_name_field      P((char *, char *, int, int)) ;
int process_postscript      P(()) ;
int psdef                   P((char *, char *)) ;
void romanshow               P((char *)) ;
void show_prologue           P((char *)) ;
int show_trailer            P(()) ;
void startline               P(()) ;
void startfile               P(()) ;
void startpage               P(()) ;
void textshow                P((char *)) ;
int usage                   P(()) ;
void useline                 P(()) ;

void do_date                P(()) ;
void get_header             P((char *, char **)) ;
void get_mult_hdr           P((char *, char **)) ;
void init_setup             P(()) ;
void parse_headers          P((int)) ;
void readline               P(()) ;
void reset_headers          P(()) ;
void set_defs               P(()) ;
void show_headers           P((int)) ;
void show_mult_hdr          P((char *, char **)) ;
char *replace_str	    P((char *, char *, char *)) ;
#endif
