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
/*  @(#)main.c 3.9 99/09/16
 *
 *  Takes a mail file, a news article or an ordinary file
 *  and pretty prints it on a Postscript printer.
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

#include "mp.h"
#include "general_header.h"
#include <stdlib.h>
#include <sys/param.h>
#include <nl_types.h>
#include <locale.h>
#include <sys/stat.h>

extern void pcf_tt_put_postscript_prologue(void);
Bool set_current_locale(void);
static void put_environment_var ( char *, char *);
static void PS_out();

/* Command line option flags */
 
bool use_mp_conf  = FALSE ;   /* Use mp.conf even if prolog.ps file is present*/
bool article  = FALSE ;       /* Set for news in "Article from " format. */
bool content  = FALSE ;       /* Set if Content-Length: has message length. */
bool digest   = FALSE ;       /* Are we are printing a mail digest (-d) */
bool elm_if   = FALSE ;       /* ELM mail frontend intermediate file format */
bool folder   = FALSE ;       /* Set if we are printing a mail folder. */
bool nodecor   = FALSE ;       /* Set if we are printing without decorations */
bool landscape = FALSE ;      /* Set if we are printing in landscape mode. */
bool print_orig = FALSE ;     /* Print From rather than To in mail header. */
bool print_ps = FALSE ;        /* Print PostScript files if set. */
bool text_doc = FALSE ;       /* Printing normal text (-o) */
bool multi_print_var = FALSE ;    /* Whether to print using the configuration mech or as print server client*/
 
/* Header definitions. */
 
char *POSTSCRIPT_MAGIC = "%!" ;            /* First line of PS file. */

/* Bug # 4010691 */
char *X_UNIX_FROM	= "X-Unix-From:" ; /* mail seperator in a folder */

char *FROMHDR       = "From:" ;
char *FROM_HDR      = "From " ;            /* UNIX From header */
char *APP_FROMHDR   = "Apparently_from:" ;
char *TOHDR         = "To:" ;
char *APP_TOHDR     = "Apparently_to:" ;
char *CCHDR         = "Cc:" ;
char *SUBJECTHDR    = "Subject:" ;
char *DATEHDR       = "Date:" ;
char *NEWSGROUPSHDR = "Newsgroups:" ;
char *NEWSGROUPHDR  = "Newsgroup:" ;
char *REPLYHDR      = "Reply_to:" ;
char *CONTENT_LEN   = "Content-Length:" ;

/* Header lines. */

char *from            = NULL ;    /* From: */
char *from_           = NULL ;    /* From_ (UNIX from) */
char *apparently_from = NULL ;    /* Apparently_from: */
char *to[MAXCONT+1] ;             /* To: (can have multiple lines) */
char *apparently_to   = NULL ;    /* Apparently_to: */
char *cc[MAXCONT+1] ;             /* Cc: (can have multiple lines) */
char *subject         = NULL ;      /* Subject: (can be set from command line) */
char *gsubject        = NULL ;    /* Global subject set from command line. */
char *date            = NULL ;    /* Date: */
char *newsgroups      = NULL ;    /* Newsgroups: (news articles only) */
char *reply_to        = NULL ;    /* Reply-to: */
char *content_len     = NULL ;    /* Content-Length: */

/* Strings used in page processing. */

char *curfname = NULL;       /* Current file being printed. */
char *message_for = "" ;          /* "[Mail,News,Listing] for " line */
char *nameptr ;                   /* Used to getenv the NAME variable. */
char *optarg = NULL;                    /* Optional command line argument. */
char *owner       = NULL ;        /* Name of owner (usually equal to 'to') */
char *progname    = NULL ;        /* Name of this program. */
char *prologue    = PROLOGUE ;    /* Name of PostScript prologue file. */
char *proname = NULL;        /* Full pathname of the prologue file. */
char *whoami      = NULL ;        /* Login name of user. */
char *prolog_locale = NULL ;      /* Locale of prolog file */
char *target_printer = NULL ;	  /* Name of target printer */
char *mpconf_path = NULL ;	  /* User specified config. path */
int octal = 0 ;                   /* Does "nextline" contain octal numbers */
int dup_stdout_des ;		  /* duplicate stdout descriptor */

/* newly defined globals */
nl_catd  cat_fd = NULL;
int current_pt_sz = NORMAL_PT_SZ;

/* Other globals. */

document_type doc_type = DO_MAIL ;  /* Printing type - default mail */
paper_type paper_size = US ;        /* Paper size - default US */

#ifdef GECOSFIELDS
int namefields = GECOSFIELDS ;  /* New no. of "words" from passwd gecos. */
#else
int namefields = NAMEFIELDS ;   /* Not supplied; use default. */
#endif /*GECOSFIELDS*/

#ifdef GECOSLENGTH
int namelength = GECOSLENGTH ;  /* New max. no. of chars. from passwd gecos. */
#else
int namelength = NAMELENGTH ;   /* Not supplied; use default. */
#endif /*GECOSLENGTH*/

int clen = 0 ;              /* Current line length (including newline). */
int colct = 0;              /* Column count on current page. */
int cmdfiles = 0 ;          /* Set if file to print given on command line. */
int linect = 0 ;            /* Line count on current page. */
int llen = LINELENGTH ;     /* Number of characters per line. */
int mlen = 0 ;              /* Number of characters in message (-C option). */
int numcols = 1 ;           /* Number of columns per page */
int optind ;                /* Optional command line argument indicator. */
int pageno = 1 ;            /* Page number within message. */
int plen = PAGELENGTH ;     /* Number of lines per page. */
int tpn    = 0 ;            /* Total number of pages printed. */

/* Read-ahead variables. */

char *nextline=NULL;  	       /* Read-ahead of the mail message, minus nl */

bool end_of_file = FALSE ;     /* EOF indicator */
bool end_of_line ;             /* Is a newline removed from this line */
bool end_of_page = FALSE ;     /* end-of-page indicator - ^L on input */
bool print_spool = FALSE ; 	/* Spool output in xpr client mode */

FILE *fp ;                     /* File pointer for current file. */
static int printed_once=0;

int
main(argc, argv)
int argc ;
char **argv ;
{
  FILE **prolog_fp;

  to[0] = cc[0] = NULL ;

  progname = argv[0] ;        /* Save this program name. */

  /* For setting the path of tne message catalog file */

  get_def_style_file();

  get_options(argc, argv) ;   /* Read and process command line options. */

  (void)setlocale(LC_ALL, "");

  put_environment_var(NLS_PATH_ENVIRON,NLS_PATH_STRING); 

  /* Opening the catalog file handle */

  cat_fd = catopen(MP_MSG_FILE, NL_CAT_LOCALE);

  (void) set_current_locale();

  /* Select suitable page format files for xprt client/ps printing */
  replace_str(proname,"%s",target_printer?"xpr":"ps");
  target_printer?multi_print_var=TRUE:NULL;

  prolog_fp=(FILE **)calloc(sizeof(FILE *),1);
  *prolog_fp=(FILE *)calloc(sizeof(FILE),1);

  if(target_printer==NULL) {	/* Not xprt client */
  	postscript_onetime_routine(prolog_fp);
  } else {	
	dup_stdout_des = dup(1);	/* create a dupli.desc for stdout */
	freopen("/dev/null","wb",stdout);
  	if(target_printer)
  		xpr_init();
  }
	
  if (argc - optind != 0) cmdfiles = 1 ;
  if (!cmdfiles)
    {
      fp = stdin ;                 /* Get input from standard input. */
      curfname=(char *)strdup("stdin");
      STRCPY(curfname, "stdin") ;
      if(!target_printer) {
	      if(!test_postscript(fp)) {
		      postscript_out(prolog_fp) ;
		      readline();
		      printfile() ;   /* Pretty print *just* standard input. */
		     show_trailer() ; /* Send trailer file to output. */
	     }
     } else {
	printfile() ;
     } 
    }
  else
    for (; optind < argc; ++optind)
      {
        if(argv[optind]!=NULL) {
		unsigned int tmp_len=strlen(argv[optind]);
		curfname=(char*)realloc(curfname, tmp_len+1);
		if(!curfname)
			malloc_err_disp_exit(__LINE__, __FILE__);
        	STRCPY(curfname, argv[optind]) ;    /* Current file to print. */
	} else {
		continue;			/* File is empty ! */
	}
	if ((fp = fopen(curfname, "r"))==NULL) {
		FPRINTF(stderr , catgets(cat_fd, ERR_SET, 1, \
		"%s: cannot open file %s \n"), progname, curfname) ;
		continue;
	}
        colct = 0 ;
        pageno = 1 ;       /* Initialise current page number. */
	tpn = 0;
        end_of_file = 0 ;  /* Reset in case there's another file to print. */
	if (!target_printer) {
		if(*prolog_fp)
			fseek(*prolog_fp, 0L, SEEK_SET);
		if(!test_postscript(fp)) {
			postscript_out(prolog_fp) ;
			readline();
			printfile() ;      /* Pretty print current file. */
			show_trailer() ;   /* Send trailer file to output. */
		}
	} else {
		printfile() ;
	}
     }
  if(*prolog_fp)
  	FCLOSE(*prolog_fp);
  if(printed_once && target_printer )
	xpr_end();
  exit(0) ;
/*NOTREACHED*/
}


int
printfile()    /* Create PostScript to pretty print the current file. */
{
  int blankslate ;    /* Nothing set up for printing. */
  bool eop ;          /* Set if ^L (form-feed) found. */

  if (target_printer)
	readline();

  if (end_of_file)
    {
	FPRINTF(stderr , catgets(cat_fd, ERR_SET, 2, \
		"%s: empty input file %s, nothing printed\n"), progname, curfname) ;
      return(1) ;
    }

  printed_once=1;
  if (!text_doc)
    parse_headers(FALSE) ;    /* Parse headers of mail or news article */
  init_setup() ;              /* Set values for remaining globals. */

  startfile();
  startpage() ;               /* Output initial definitions. */
  blankslate = 0 ;
  eop        = FALSE ;

/* Print the document */

  if (doc_type != DO_TEXT)
    {
      show_headers(FALSE) ;
#ifdef WANTED
      FPUTS("sf ", stdout) ;
#endif /*WANTED*/
    }
  while (!end_of_file)
    {
      if (blankslate)
        {
          startfile() ;
          startpage() ;               /* Output initial definitions. */
          blankslate = 0 ;
        }

      if (content && folder && mlen <= 0)
        {

/*  If the count has gone negative, then the Content-Length is wrong, so go
 *  back to looking for "\nFrom".
 */

          if (mlen < 0) content = FALSE ;
          else if ((hdr_equal(FROM_HDR) || hdr_equal(FROMHDR)) &&
                    isupper(nextline[0]))
            {
              eop    = FALSE ;
              linect = plen ;
              reset_headers() ;
              parse_headers(FALSE) ;
              show_headers(FALSE) ;
            }
          else content = FALSE ;
        }

		/* bug#4010691 */
      if (!content && folder && hdr_equal(X_UNIX_FROM)
          /* (!elm_if && hdr_equal(FROM_HDR) ||
            elm_if && hdr_equal(FROMHDR)) */ && isupper(nextline[0]))
        {
          eop    = FALSE ;
          linect = plen ;
          reset_headers() ;
          parse_headers(FALSE) ;
          show_headers(FALSE) ;
        }
      if (digest &&
         (hdr_equal(FROMHDR) || hdr_equal(DATEHDR) || hdr_equal(SUBJECTHDR)) &&
          isupper(nextline[0]))
        {
          linect = plen ;
          parse_headers(TRUE) ;
          show_headers(TRUE) ;
        }

      if (print_ps && hdr_equal(POSTSCRIPT_MAGIC))
        {
          if (numcols) endcol() ;
          endpage() ;
          endfile() ;
          process_postscript() ;
          blankslate = 1 ;
        }
      else if (folder && end_of_page) 
	{
	/*  Bug # 4010691 */
	/* This line causes a blank page to print inbetween mails */
	/*
	eop = TRUE ;
	*/
	}
      else
        {
          if (eop == TRUE) end_of_page = TRUE ;
          textshow(nextline) ;
          eop = FALSE ;
        }

      if (content) mlen -= clen ;
      readline() ;

    }    

  if(target_printer && end_of_file)
	xpr_print(NULL, 0, FEND);	
  if (!blankslate)
    {
      if (numcols) endcol() ;
      endpage() ;
      endfile() ;
    }
  
  FCLOSE(fp) ;
  return(1);
}
static void 
PS_out() {
  int firstline = 1 ;   /* To allow a newline after the first line. */
  FILE *fpprt = fdopen(dup_stdout_des, "w");
  while (!hdr_equal(FROMHDR)    && !hdr_equal(DATEHDR) &&
         !hdr_equal(SUBJECTHDR) && !end_of_file)
    {
      fprintf(fpprt,"%s", nextline) ;
      if (firstline) fprintf(fpprt,"\n");
      firstline = 0 ;
      if (fgets(nextline, MAXLINE, fp) == NULL) end_of_file = TRUE ;
    }
    fflush(fpprt);
    fclose(fpprt);
}


int
process_postscript()
{
  int firstline = 1 ;   /* To allow a newline after the first line. */
  if(target_printer) { 
	xpr_print(NULL, 0, FEND);	
  	xpr_end();
	PS_out();
	return(0);
  }
  startpage() ;
  while (!hdr_equal(FROMHDR)    && !hdr_equal(DATEHDR) &&
         !hdr_equal(SUBJECTHDR) && !end_of_file)
    {
      PRINTF("%s", nextline) ;
      if (firstline) FPUTS("\n", stdout) ;
      firstline = 0 ;
      if (fgets(nextline, MAXLINE, fp) == NULL) end_of_file = TRUE ;
    }
  endpage() ;
}


int
show_trailer()
{
  FPUTS("%%Trailer\n", stdout) ;
  PRINTF("%%%%Pages: %1d\n", tpn) ;
}

static void 
put_environment_var ( char *env_var_name , char *env_var_value )
{
	char *after_environ_array;
     	char before_environ_array[MAX_ENV_STRING];
	char *tempString;

	tempString = (char *)getenv(env_var_name);
	/*
	 * this variable has to be malloc'ed as putenv won't
	 * work good with auto vars.
	 */
	after_environ_array = (char *)malloc(MAX_ENV_STRING);
	if ( after_environ_array == NULL )
	{
		fprintf(stderr, catgets( cat_fd, ERR_SET , 26,
			"%s: Malloc failure; line %d, file %s\n"),progname,
			__LINE__, __FILE__ );
		exit(1);
	}

	if (!tempString)
     	{
		sprintf(after_environ_array,"%s%s%s",
			env_var_name,
			"=",
			env_var_value
			);
	} else 
	{
		 sprintf(before_environ_array,"%s%s%s",
			env_var_name ,
			"=",
			tempString
			);
		sprintf(after_environ_array,"%s%s%s%s%s",
			env_var_name ,
			"=",
			tempString,
			":",
			env_var_value
			);
	}
	putenv(after_environ_array);
	/*
	free(after_environ_array);
	*/
	 
}

Bool
set_current_locale(void) {
	
  char *tmp = NULL;
  char *locale = NULL;
  Bool ret=FALSE;

  if ( prolog_locale != NULL && strcmp(prolog_locale,"") ) {     /* command line arg */
	if ( (locale = setlocale(LC_ALL,prolog_locale)) != NULL) 
		ret=TRUE;
  } else if ( (tmp = (char *)getenv("MP_LANG")) && strcmp(tmp,"") ) {    /* environment var */
	if ( (locale = setlocale(LC_ALL,tmp)) != NULL ) 
		ret=TRUE;
  } else if ( (tmp = (char *)getenv("LANG")) != NULL && strcmp(tmp,"") ) {    /* environment var */
	if ( (locale = setlocale(LC_ALL,tmp)) != NULL ) 
		ret=TRUE;
  } else if ( (locale = setlocale(LC_ALL, (char *)NULL)) != NULL ) {
		ret=TRUE;
  } 
  /* this is a fix for bug#4311552, to make decimal point to appear thus, not 
  as a semicolon in all the solaris locales */

  (void)setlocale(LC_NUMERIC, "C");

  return ret;
}

int 
test_hdrequal(val, lline)
char val[MAXLINE];
char lline[MAXLINE];
{

  register char *nptr = lline ;
  register char *wptr = val ;
  register int n, w ;

  do
    {
      n = *nptr++ ;
      w = *wptr++ ;
      if (isupper(n)) n = tolower(n) ;
      if (isupper(w)) w = tolower(w) ;
      if (n != w && w != '\0') return(0) ;
    }
  while (n != '\0' && w != '\0') ;
  return(1) ;
}

int
test_postscript(fp) 
FILE *fp;
{
char localline[MAXLINE];
int c;

	if ((c = getc(fp)) == EOF ) { end_of_file = TRUE ; }
	else UNGETC(c, fp) ;
	if (end_of_file) {
		FPRINTF(stderr , catgets(cat_fd, ERR_SET, 2, \
			"%s: empty input file %s, nothing printed\n"), progname, curfname) ;
		return(1);
	} else {
		localline[0]=getc(fp);		
		localline[1]=getc(fp);		
		localline[2]='\n';
		UNGETC(localline[1], fp);
		UNGETC(localline[0], fp);

#if 0
		memset(localline, 0, MAXLINE);
  		readline() ;
		strncpy(localline, nextline, clen);
		localline[clen-1]='\n';
		/* 
		fgets(localline, MAXLINE, fp);
		*/
#endif
		if(!text_doc && test_hdrequal(POSTSCRIPT_MAGIC, localline)) {
			while (!end_of_file) {
				if (fgets(localline, MAXLINE, fp) == NULL ) end_of_file = TRUE ;
				else PRINTF("%s", localline) ;
			}
		end_of_file = FALSE;
		return 1;
		}
	return 0;
	}
}

int 
postscript_onetime_routine(prolog_fp) 
  FILE **prolog_fp;
{
  char *i18n_prologfile0=NULL;
  char *i18n_prologfile0_tmp=NULL;
  char *i18n_prologfile=NULL;
  char *i18n_prologfile_tmp=NULL;
  char *confpath=NULL;
  char *confpath_tmp=NULL;
  char *tmp = NULL;
  char *t = NULL;
  char *locale = NULL;
  int setuplocale = 0;

  unsigned int i1=0, i2=0;

  i1=strlen(PROLOG_STR);
  i2=strlen(PROLOG_STR_OWN);


/*  Try to get location of the mp prologue file from an environment variable.
 *  If it's not found, then use the default value.
 */

  if (prolog_locale && (strcmp(prolog_locale,"C") == 0)){
      *prolog_fp = NULL;
      setuplocale = 0;
  /* Figure out which locale document is in */
  } else if ( prolog_locale != NULL && strcmp(prolog_locale,"") && strcmp(prolog_locale,"C")) {     /* command line arg */
      tmp = prolog_locale;
      setuplocale = 1;
  } else if (((tmp = (char *)getenv("MP_LANG")) != NULL || (tmp = (char *)getenv("LANG")) != NULL) && strcmp(tmp,"") && strcmp(tmp,"C")) {    /* environment var */
      setuplocale = 1;
  } else { /* query locale */ 
      if ( (locale = setlocale(LC_ALL, (char *)NULL)) != NULL ) {
	  if ( strcmp(locale, "C") == 0 ) { /* in C locale */
	      *prolog_fp = NULL;
              setuplocale = 0;
	  } else {
		tmp=strtok(locale,"/");
	      	setuplocale = 1;
	  }
      }
  }

  if(setuplocale) {
	  char *pl=NULL;
	  unsigned int st=strlen(tmp)+1;
	  i1+=st+20;
	  i2+=st+20;
	  pl = (char *)calloc(st+1, 1);
	  if(!pl) 
		malloc_err_disp_exit(__LINE__, __FILE__);
	  strcpy(pl,tmp);
  	  i18n_prologfile = (char *)calloc(i1, 1);
	  if(!i18n_prologfile) 
		malloc_err_disp_exit(__LINE__, __FILE__);
  	  i18n_prologfile_tmp = (char *)calloc(i1, 1);
	  if(!i18n_prologfile_tmp) 
		malloc_err_disp_exit(__LINE__, __FILE__);
  	  i18n_prologfile0 = (char *)calloc(i2, 1);
	  if(!i18n_prologfile0) 
		malloc_err_disp_exit(__LINE__, __FILE__);
  	  i18n_prologfile0_tmp = (char *)calloc(i2, 1);
	  if(!i18n_prologfile0_tmp) 
		malloc_err_disp_exit(__LINE__, __FILE__);
	  confpath = (char *)calloc(i1, 1);
	  if(!confpath) 
		malloc_err_disp_exit(__LINE__, __FILE__);
	  confpath_tmp = (char *)calloc(i1, 1);
	  if(!confpath_tmp) 
		malloc_err_disp_exit(__LINE__, __FILE__);
	  SPRINTF(i18n_prologfile0, PROLOG_STR_OWN, tmp);	
	  SPRINTF(i18n_prologfile, PROLOG_STR, tmp);	
	  if ((t=(char *)strchr(pl,'@')) && (*t=NULL)) ;
	  SPRINTF(i18n_prologfile_tmp, PROLOG_STR, pl);	
	  SPRINTF(i18n_prologfile0_tmp, PROLOG_STR_OWN, pl);	
	  free(pl);
  }

  if (mpconf_path) {
	struct stat st;
	if (stat(mpconf_path,&st) == -1) {
		fprintf(stderr, catgets(cat_fd, ERR_SET,1, \
			"%s: Cannot open %s\n"), progname, mpconf_path);
		exit(-1);
	}
  }

  if ( setuplocale ) {
	  if(mpconf_path) {
		strcpy(confpath, mpconf_path);
	  } else {
  	  	SPRINTF(confpath, "%s/mp.conf", i18n_prologfile);
  	  	SPRINTF(confpath_tmp, "%s/mp.conf", i18n_prologfile_tmp);
	  }
	  strcat(i18n_prologfile0, "/prolog.ps");
	  strcat(i18n_prologfile, "/prolog.ps");
	  strcat(i18n_prologfile_tmp, "/prolog.ps");
	  strcat(i18n_prologfile0_tmp, "/prolog.ps");
	  /* Check whether prolog.ps exist */
	  /* This order of checking is really important */
	  if (((*prolog_fp = fopen(i18n_prologfile0, "r")) == NULL) &&
	      	((*prolog_fp = fopen(i18n_prologfile0_tmp, "r")) == NULL) &&
	      	((*prolog_fp = fopen(i18n_prologfile, "r")) == NULL) &&
	      	((*prolog_fp = fopen(i18n_prologfile_tmp, "r")) == NULL) ||
		use_mp_conf ) {
		if (config_scan(confpath) || 
			config_scan(confpath_tmp)) {
			multi_print_var = TRUE;
			*prolog_fp = NULL;
		}
		/* prolog.ps does not exist - such as in European locales */
		setuplocale = 0;
	  } else {
		setuplocale = 1;
	  }
  } else {
	if(mpconf_path) {
		if(!confpath) {
			confpath=(char*)calloc(strlen(mpconf_path)+1,1);
	  		if(!confpath) 
				malloc_err_disp_exit(__LINE__, __FILE__);
		}
		strcpy(confpath, mpconf_path);
	}

	if (confpath && config_scan(confpath)) multi_print_var = TRUE;
	setuplocale = 0;
 }

  if(i18n_prologfile)
	free(i18n_prologfile);
  if(i18n_prologfile_tmp)
	free(i18n_prologfile_tmp);
  if(i18n_prologfile0)
	free(i18n_prologfile0);
  if(i18n_prologfile0_tmp)
	free(i18n_prologfile0_tmp);
  if(confpath)
	free(confpath);
  if(confpath_tmp)
	free(confpath_tmp);

	return setuplocale;
}
void output_prolog_ps(prolog_fp) 
FILE **prolog_fp;
{
    char buf[MAXLINE];
    while ( fgets(buf, MAXLINE, *prolog_fp) != NULL ) {
	FPUTS(buf, stdout);
    }
}
int postscript_out(prolog_fp) 
FILE **prolog_fp;
{
      if(*prolog_fp!=(FILE *)NULL) {
	output_prolog_ps(prolog_fp);
	FPUTS("/localeprolog 1 def\n\n", stdout);
      } else {
	FPUTS("%!PS-Adobe-1.0\n", stdout);
	FPUTS("/localeprolog 0 def\n\n", stdout);
      }

      show_prologue(proname) ;    /* Send prologue file to output. */
      FPUTS("%%EndProlog\n", stdout) ;
      write_output("/CP /currentpoint\tload def\n");
      write_output("/M /moveto\tload def\n");
      write_output("/SH /show\tload def\n");
      return 0;
}
int  get_def_style_file()
{
  unsigned int pron_size = 0;
  if ((prologue = (char *)getenv("MP_PROLOGUE")) == NULL)
    {
          char buf[MAXPATHLEN] ;

          SPRINTF(buf, DEF_PROLOG_DIR) ;
          prologue = (char *) strdup(buf) ;
    }
  pron_size=strlen(prologue)+21;
  proname=(char*)realloc(proname, pron_size);
  if(!proname) 
	  malloc_err_disp_exit(__LINE__, __FILE__);
  SPRINTF(proname, "%s/mp.pro.%%s", prologue) ;
  return 0;
}
