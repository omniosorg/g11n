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

/*  @(#)io.c 3.7 94/01/20
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
#pragma ident "@(#)io.c	3.14	00/10/05	SMI"


#include <locale.h>
#include <widec.h>
#include <errno.h>
#include "mp.h"
#include "extern.h"
#include "general_header.h"

extern bool multi_print_var;

/* Emptyline returns true if its argument is empty or whitespace only */
         
bool
emptyline(str)
char *str ;
{
  while (*str)
    {
      if (!isspace(*str)) return(FALSE) ;
      str++ ;
    }
  return(TRUE) ;
}


/*  Read an input line into nextline, setting end_of_file, end_of_page
 *  and end_of_line appropriately.
 */

void
readline() {
	int c = EOF;
	int i = 0 ;      /* Index into current line being read. */
	int len = 0 ;    /* Length of the current line. */
	static int first_time=0;
	static wchar_t *wclinebuf=NULL;
	static wchar_t *wcline = NULL;
	static int cur_len=MAXLEN;
	static unsigned char *line;
	wchar_t wcbuf[MAXLINE];

	if ( !first_time ) {
		first_time++;
		wclinebuf = (wchar_t *)calloc(MAXLEN, sizeof(wchar_t));
		line = (unsigned char *)calloc(MAXLEN, 1 );
		nextline = (char *)calloc(MAXLEN, 1 );
		set_current_locale();
	}
	if (end_of_file) return ;
	end_of_page = end_of_line = FALSE ;

	if (wcline == NULL) {
		while ((c = getc(fp)) != EOF && c != '\n' && c != '\f') {
			if(i+9 >= cur_len) {
				cur_len+=512;
				wclinebuf = (wchar_t *)realloc(wclinebuf, cur_len*sizeof(wchar_t));
				line = (unsigned char *)realloc(line, cur_len);
				nextline = (char *)realloc(nextline, cur_len);
			}
			if (c == '\t') {
				do {
					line[i++] = ' ' ;
					len++ ;
				} while (len % 8 != 0);
			} else { 
            			line[i++] = c ;
            			len++ ;
          		}
        		if (c == '\b') {
            			len -= 2 ;
            			i -= 2 ;
          		}
      		}
    		line[i] = '\0' ;

    		if (elm_if && c == '\f') {
        		len-- ;
        		i-- ;
      		}

		switch (c) {
			case EOF  : 	if (i == 0) {
						end_of_file = TRUE ;
					} else { 
						UNGETC(c, fp) ;
					}
					break ;
			case '\n' : 
					break ;

	/*  /usr/ucb/mail for some unknown reason, appends a bogus formfeed at
	 *  the end of piped output. The next character is checked; if it's an
	 *  EOF, then end_of_file is set, else the character is put back.
	 */

			case '\f' : 	if ((c = getc(fp)) == EOF) 
						end_of_file = TRUE ;
				    	else UNGETC(c, fp) ;

				    	end_of_page = TRUE ;
				    	break ;
		}
    
/*  save the line to a buffer in wide char format
 */		errno = 0;	
		if (mbstowcs(wclinebuf, line, strlen((char*)line) + 1)!=(size_t)-1) {
    			wcline = wclinebuf;
		} else if(errno==EILSEQ) {
			/* Temporarily not delivered to
			 * message file.
			err_exit(catgets(cat_fd, ERR_SET, 43, "s: Illegal input sequence in line number %d\n"), progname, i);
			 */
			 err_exit("%s: Illegal input sequence, possible mismatch between print locale codeset and input data codeset. Exiting.\n", progname );
		}
		
	}

	if (wscol(wcline) <= llen) {
		wcstombs(line, wcline, MAXLINE);
		wcline = NULL;
		end_of_line = TRUE ;
	} else {
		for (i = 0, wcbuf[0] = '\0'; wscol(wcbuf) <= llen; i++) {
			wcbuf[i] = wcline[i];
			wcbuf[i+1] = '\0';
		}
		wcbuf[--i] = '\0';
		wcline += i;
		wcstombs(line, wcbuf, MAXLINE);
	}


  	clen = strlen((char *)line) + 1; 	/* Current line length */
						/* (includes newline). */
	strcpy(nextline,(char *)line);
}
