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
#pragma ident   "@(#)tp1.c	1.3 99/08/20 SMI"
#include <stdio.h>
#include <dlfcn.h>
#include "general_header.h"
#include "mp.h"
#include "tp1.h"

void load_tp1_font(tp1font_t *) ;

void 
load_tp1_font(tp1font_t *pf) {

	FILE *fp;
	char buf[MAXLINE];
	char fpath[MAXLINE];
	char tmpstr[MAXLINE];
	char tmpname[MAXLINE];
	char *t;
	int line=0, cnt=0, ref=0;
	int i, hold_int=0, width_int=0;

	if(pf->file!=(char *)NULL) {
		if((t=(char *)strchr(pf->file, '/')) != NULL) {
			if( (fp = fopen ( pf->file, "r")) == NULL ) 
				err_exit(catgets(cat_fd, ERR_SET, 1, "%s: \
					Cannot open %s\n"),
					progname, pf->file);
				
	    			while ( fgets(buf, MAXLINE, fp) != NULL ) {
					FPUTS(buf, stdout);
      					tmpstr[0] = '\0' ;
      					SSCANF(buf, "%s %s", tmpstr, tmpname);
					if(!strcmp(tmpstr,"/FontName")) {
						pf->fontname = (char*)strdup(tmpname+1); /* To avoid '/' in front */
					}
				}
				if(tmpname[0] == '\0') {
					err_exit(catgets(cat_fd, ERR_SET, 
						1, "%s: \
						unable to get fontname from %s\n"),
						progname, pf->file);
						

				}

		} else {
			pf->fontname = pf->file;
		}
	} 

	return;
#if 0
			

	if(pf->afmfile!=NULL) {
		sprintf(fpath, pf->afmfile);
	} else {
		sprintf(fpath, "/usr/openwin/lib/X11/fonts/Type1/afm/%s.afm", pf->file+1);
	}
	if( (fp = fopen ( fpath, "r")) != NULL ) {
		pf->tp1fd_t = (tp1_data_t *) 
				malloc(MAX_AFM_ELMS*sizeof(tp1_data_t));
		for(i=0;i<MAX_AFM_ELMS;i++)
			pf->tp1fd_t[i].width=current_pt_sz;
		while(fgets(buf,sizeof(buf),fp)) {
			line++;
			if (!ends_with_NL(buf)) {
				fprintf(stderr,catgets(cat_fd,ERR_SET,32,\
						"%s: file: %s, line:%d "
						"too long\n"), progname,
						fpath, line);
				exit(1);
			} else {
				zapNL(buf);	
			}

			if ((t = strtok(buf, CFFS_STR)) == NULL) {
				continue;
			} else if (is_comment_char(*t)) {
				continue;
			} else if (!eq(t, "StartCharMetrics") && !ref ) {
				continue;
			} else if(eq(t, "StartCharMetrics")) {
				ref = 1;
				continue;
			} 

			if(eq(t, "EndCharMetrics")) { 
				fclose(fp); 
				break; 
			}
			cnt=0;
			while((t =  strtok(NULL,CFFS_STR))!=NULL) {
				cnt++;

				if (cnt==1) {
					if (atoi(t)>=0) {
						hold_int=atoi(t);
					} else {
						break;
					}
				}

				if(cnt==4) { 
					width_int = atoi(t); 
					pf->tp1fd_t[hold_int].width=\
							(float)width_int*\
							((float)current_pt_sz/1000.0);
					break; 
			    	}
			}
		}
	} else {
		err_exit(catgets(cat_fd, ERR_SET, 1, "%s: Cannot open %s\n"),
				progname, fpath);
	}
#endif
}
