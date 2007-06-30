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
    
#pragma	ident	"@(#)misc.h	1.3 99/05/03 SMI"

#ifndef MISC_H
#define MISC_H
#include <stdio.h>
#include <strings.h>

#define NBPB	8

#ifndef UCS4_MAXVAL
#define UCS4_MAXVAL 0x0000ffff
#endif

#undef bzero	
#undef bcopy

#define basename(s)	(rindex('/',(s))==NULL?(s):(rindex('/',(s))+1))
#define bcopy(b1,b2,n)	(memcpy(b2,b1,n))
#define bzero(b,n)	(memset((b),0,(n)))
#define eq(s1,s2)	(strcmp((s1),(s2)) == 0)
#define eqn(s1,s2,n)	(strncmp((s1),(s2),n) == 0)
#define htoi(s)		((int)strtol(s,NULL,16))
#define rindex(c,s)	(strrchr((s),(c)))

#define is_tab(c)	((c)=='\t')
#define is_newline(c)	((c)=='\n')
#define is_backspace(c)	((c)=='\010')
#define is_formfeed(c)	((c)=='\014')

#ifndef DEBUG
#define debug(x)
#define Ndebug(x)
#else
#define debug(x)        x
#define Ndebug(x)
#endif

int statfile(char*);
int ishexstr(char*);
int isnumstr(char*);
unsigned int strint(char*);
double strfloat(char*);
void errxit(int);
void error(char*, ...);
void print2(char*, ...);
#endif
