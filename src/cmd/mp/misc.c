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
/*  @(#)misc.c 3.9 99/10/26
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
#include "patchlevel.h"
#include "extern.h"
#include "general_header.h"

int set_pt_sz=0;
void malloc_err_disp_exit(int , char *);

void
do_date()        /* Output Postscript definition for the date and time. */
{
  char *ptr ;            /* Pointer to current time or date string. */
  int len ;
  long clock ;           /* Used by the localtime function call. */
  struct tm *tm ;        /* Used by the localtime and asctime calls. */
  char *timenow ;        /* Used to set TimeNow field with users name. */

  if (date == NULL)
    {
      clock = time((time_t *) 0) ;
      tm = localtime(&clock) ;
      ptr = asctime(tm) ;
      ptr[24] = '\0' ;
    }
  else ptr = date ;

  if (article != TRUE) psdef("TimeNow", ptr) ;
  else
    {
      len = strlen(ptr) ;
      timenow = (char *)malloc((unsigned) (len + 6 + strlen(whoami) + 1)) ;
      SPRINTF(timenow, "%s  - (%s)", ptr, whoami) ;
      psdef("TimeNow", timenow) ;
    }
}


int
get_opt(argc, argv, options)
int argc ;
char **argv, *options ;
{
  char opch, *str, *ptr ;
  static int flag = 0 ;
  static int cur_argc ;
  static char **cur_argv ;

  if (flag == 0)
    {
      cur_argc = argc ;
      cur_argv = argv ;
      flag = 1 ;
      optind = 1 ;
    }

  if (cur_argc <= 1) return -1 ;

  if (--cur_argc >= 1)
    {
      str = *++cur_argv ;
      if (*str != '-') return -1 ;    /* Argument is not an option   */
      else
        {                             /* Argument is an option */
          if ((ptr = (char *)strchr(options, opch = *++str)) != (char *) 0)
            {
              ++optind ;
              optarg = ++str ;        /* Point to rest of argument if any  */
              if ((*++ptr == ':') && (*optarg == '\0'))
                {
                  if (--cur_argc <= 0) return '?' ;
                  optarg = *++cur_argv ;
                  ++optind ;
                }
              return opch ;
            }
          else if (opch == '-')
            {                         /* End of options */
              ++optind ;
              return -1 ;
            }
          else return '?' ;
        }
    } 
  return 0 ;                          /* Should never be reached. */
}


int
get_options(argc, argv)      /* Read and process command line options. */
int argc ;
char *argv[] ;
{
  int opch, set_sz=0 ;
  unsigned int tmp_len=0;

  set_pt_sz = current_pt_sz;
  while ((opch = get_opt(argc, argv, "aA:c:CdefFlL:D:mMnop:P:s:t:u:U:vw:z:")) != -1)
    switch (opch)
      {
        case 'a' : article = TRUE ;      /* "Article from" format. */
                   break ;
        case 'A' : if (!strcmp(optarg, "4"))    /* A4 paper size. */
                     paper_size = A4 ;
                   break ;
        case 'C' : content = TRUE ;      /* Use Content-Length: header. */
                   break ;
        case 'c' : if (strlen(optarg))                /* Gecos (chars). */
                     namelength = atoi(optarg) ;
                   break ;
        case 'D' : if (strlen(optarg))
			target_printer = optarg;  /* target printer name */
                   break ;
        case 'd' : digest = TRUE ;       /* Print digest. */
                   break ;
        case 'e' : elm_if = TRUE ;       /* ELM intermediate file format. */
                   folder = TRUE ;       /* Kind of folder. */
                   break ;
        case 'F' : print_orig = TRUE ;   /* Print originators name. */
                   break ;
        case 'f' :      if (!strcmp(optarg, "f"))     /* Filofax output. */
                     SPRINTF(proname, "%s/mp.pro.ff.%%s", prologue) ;
                   else if (!strcmp(optarg, "p"))     /* Franklin Planner. */
                     SPRINTF(proname, "%s/mp.pro.fp.%%s", prologue) ;
                   break ;
        case 'l' : if (!strcmp(optarg, "l")) {
			   landscape = TRUE ;    /* Landscape printing. */
			   if(current_pt_sz==NORMAL_PT_SZ)
			   	current_pt_sz = LANDSCAPE_PT_SZ;
			   SPRINTF(proname, "%s/mp.pro.ll.%%s", prologue) ;
		   } else {
			   landscape = TRUE ;    /* Landscape printing. */
			   if(current_pt_sz==NORMAL_PT_SZ)
			   	current_pt_sz = LANDSCAPE_PT_SZ;
			   SPRINTF(proname, "%s/mp.pro.l.%%s", prologue) ;
		   }
                   break ;
	case 'L' : prolog_locale = (char *)strdup(optarg) ;  /* Locale of prolog file */
			   break ;
        case 'M' : use_mp_conf = TRUE;
		     break;
        case 'm' : folder = TRUE ;       /* Print mail folder. */
                   break ;
        case 'n' : nodecor = TRUE ;       /* Print without decors */
                   break ;
        case 'o' : text_doc = TRUE ;     /* Print ordinary text file */
                   break ;
        case 'P' : if (!strcmp(optarg, "S"))    /* Print PostScript files. */
                     print_ps = TRUE ;
		   else {
			target_printer = optarg;  /* target printer name */
			print_spool=TRUE;
		   }
                   break ;
        case 'p' : if ((tmp_len=strlen(optarg))) {
			int tlen=strlen(proname);
			if(tlen<(tmp_len+1))
				proname=(char*)realloc(proname,tmp_len+1);
			if(!proname) malloc_err_disp_exit(__LINE__, __FILE__);
                        STRCPY(proname, optarg) ;  /* New prologue file. */
		   }
                   break ;
        case 's' : if ((tmp_len=strlen(optarg))) {
                     	gsubject = optarg ;        /* New subject line. */
			if(tmp_len>MAXSUBLEN)
				gsubject[MAXSUBLEN]='\0';
		   }
                   break ;
        case 't' :      if (!strcmp(optarg, "m"))     /* Time Manager. */
                     SPRINTF(proname, "%s/mp.pro.tm.%%s", prologue) ;
                   else if (!strcmp(optarg, "s"))     /* Time/System Int. */
                     SPRINTF(proname, "%s/mp.pro.ts.%%s", prologue) ;
                   break ;
        case 'u' : if (strlen(optarg))
                   	mpconf_path=optarg ;
                   break ;
        case 'U' : if (!strcmp(optarg, "S"))    /* US paper size. */
                     paper_size = US ;
                   break ;
        case '?' :
        case 'v' : usage() ;
                   break ;
        case 'w' : if (strlen(optarg))                /* Gecos (words). */
                     namefields = atoi(optarg) ;
		     break;
        case 'z' : if (strlen(optarg)) {               /* point size */
			int fsz= atoi(optarg);
			if(fsz) {
                     		set_pt_sz = current_pt_sz = fsz;
				set_sz=1;
			}
		}	
      }
	if(!set_sz && landscape == TRUE )
		current_pt_sz = LANDSCAPE_PT_SZ+PT_SZ_INC;
	else if (!set_sz)
		current_pt_sz = NORMAL_PT_SZ+PT_SZ_INC;
}


void
init_setup()            /* Set default values for various options. */
{
  char *c ;
  int amp_cnt = 0 ;     /* Number of ampersands in gecos field. */
  int i, len ;
  struct passwd *pp ;

  c = getlogin() ;      /* Pointer to users login name. */
  if (c == NULL)        /* Get username from password file */
    {
      pp = getpwuid(geteuid()) ;
      if (pp == NULL) c = "printing" ;
      else c = pp->pw_name ;
    }
  owner = (char *)malloc((unsigned) (strlen(c) + 1)) ;
  STRCPY(owner, c) ;
  whoami = (char *)malloc((unsigned) (strlen(c) + 1)) ;   /* Save User login name */
  STRCPY(whoami, c) ;
 
/*  Have a look for the users gecos (normally real name), so that its a bit
 *  more recognisable. If this field is too long, then we need to truncate
 *  sensibly. We also need to check a few things. If we've extracted
 *  namefields "words" or have found a comma, then exit. If an ampersand is
 *  found, this is expanded to the users name in capitals.
 */    
     
  pp = getpwnam(owner) ;
  if (pp != NULL && pp->pw_gecos && pp->pw_gecos[0] != '\0')
    {  
      len = strlen(pp->pw_gecos) ;
      for (i = 0; i < len; i++)
        if (pp->pw_gecos[i] == '&') amp_cnt++ ;
 
      if ((nameptr = (char *)getenv("NAME")) != NULL) {
        owner = (char *)realloc(owner, (unsigned) (strlen(nameptr) + 1)) ;
        process_name_field(c, nameptr, namefields, namelength) ;
      } else {
        owner = (char *)realloc(owner, (unsigned) (strlen(pp->pw_gecos) +
                                 amp_cnt * strlen(c) + 1)) ;
        process_name_field(c, pp->pw_gecos, namefields, namelength) ;
      }
    }

  if (text_doc) doc_type = DO_TEXT ;
  switch (doc_type)
    {
      case DO_TEXT : message_for = "Listing for ";
                     digest = FALSE ;
                     break ;
      case DO_MAIL : message_for = digest ? "Mail digest for " : "Mail for " ;
                     break ;
      case DO_NEWS : message_for = digest ? "News digest for " : "News for " ;
                     break ;
    }
}


/* Extract user name from $NAME or passwd GECOS. */

int
process_name_field(c, ptr, fields, length)
char *c, *ptr ;
int fields, length ;
{
  int i, j, len, n, spaces, slen ;

  n = spaces = 0 ;
  slen = strlen(ptr) ;
  for (i = 0; i < slen; i++)
    {
           if (*ptr == ',') break ;
      else if (*ptr == '&')
        {
          if (islower(c[0])) owner[n++] = toupper(c[0]) ;
          len = strlen(c) ;
          for (j = 1; j < len; j++)
            owner[n++] = c[j] ;
          ptr++ ;
        } 
      else if (*ptr == ' ' || *ptr == '\t')
        {
          if (++spaces == fields) break ;
          else
            while (*ptr == ' ' || *ptr == '\t') owner[n++] = *ptr++ ;
        } 
      else owner[n++] = *ptr++ ;
      if (n >= length) break ;
    } 
  if (n > length) n = length ;
  owner[n] = '\0' ;
}


int
usage()     /* Print usage message and exit. */
{
  FPRINTF(stderr, catgets(cat_fd, USAGE_SET, 1, 
		"%s version 2.5.%1d\n\n"), progname, PATCHLEVEL) ;
  FPRINTF(stderr, catgets(cat_fd, USAGE_SET, 2, "Usage: %s "), progname );
  FPRINTF(stderr, "[-A4] [-C] [-D target printer] [-F] [-L localename]\n");
  FPRINTF(stderr, "\t[-P target spool printer] [-PS] [-US] [-a] [-c chars] [-d] \n");
  FPRINTF(stderr, "\t[-e] [-ff] [-fp] [-l] [-ll] [-M] [-m] [-n] [-o] [-p prologue] \n");
  FPRINTF(stderr, "\t[-s subject] [-tm] [-ts] [-u user specified mp.conf file] [-v]\n");
  FPRINTF(stderr, catgets(cat_fd, USAGE_SET, 3, "\t[-w words] [-z pointsize] [-?] filename ...\n")) ;
  exit(1) ;
}

/* The following routine is specific to using FMapType 3 composite fonts
 * in postscript.  Asian charset specific.
 */
char *
euc_to_octal(srcStr)
char *srcStr;
{
	int inKanji = FALSE;
	char buf[64];
	static char *dstStr = NULL;
	int i;
	unsigned int len = 0;
	unsigned int dstlen = 0;
	
	if(srcStr) {
		len=strlen(srcStr);
		len=len>BUFFERSIZE?BUFFERSIZE:len;
		dstlen=4*len+1;
		dstStr = (char *)realloc(dstStr, dstlen);
		if(!dstStr)
			malloc_err_disp_exit(__LINE__, __FILE__);
	} else return NULL;
#ifdef SVR4
	memset(dstStr, 0, dstlen);
#else
	bzero(dstStr, dstlen);
#endif "SVR4"
	octal = 0;
	for (i = 0; i < len; i++) {
		if (inKanji) {
			if (!isEUC(srcStr[i])) {
				inKanji = FALSE;
				/* NOT NEEDED FOR FMapType 4 (or 5)
				strcat(dstStr, "\\377\\000");
				*/
			}
		}
		else {
			if (isEUC(srcStr[i])) {
				inKanji = TRUE;
				/* NOT NEEDED FOR FMapType 4 (or 5)
				strcat(dstStr, "\\377\\001");
				*/
			}
		}
		if (inKanji) {
			if(i+1<len) {
				sprintf(buf, "\\%3.3o\\%3.3o", srcStr[i] & 0xff, srcStr[i+1] & 0xff);
				i++;
			} else 
				sprintf(buf, "\\%3.3o", srcStr[i] & 0xff);
			octal = 1;
		}
		else {
			sprintf(buf, "%c", srcStr[i]);
		}
		strcat(dstStr, buf);
	}
	dstStr[strlen(dstStr)]='\0';
	return dstStr;
}


char *replace_str(char *str, char *oldstr, char *newstr) {
	int oldlen, newlen;
	char *p, *q;

	if ((p=(char *)strstr(str, oldstr)) == NULL) return p;
	oldlen=strlen(oldstr);
	newlen=strlen(newstr);
	memmove(q=p+newlen, p+oldlen, strlen(p+oldlen)+1);
	memcpy(p,newstr,newlen);
	return q;
}

void malloc_err_disp_exit(int line_no, char *file) {
	err_exit(catgets(cat_fd, ERR_SET, 25,
		"%s: Malloc failure; line %d, file %s\n"), progname, \
		line_no, file);
}

