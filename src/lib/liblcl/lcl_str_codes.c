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
/* Copyright (c) 2000, Sun Microsystems, Inc. All rights reserved. */
 
#pragma ident	"@(#)lcl_str_codes.c	1.2	00/01/06 SMI"

/*////////////////////////////////////////////////////////////////////////
Copyright (c) 1992 Electrotechnical Laboratry (ETL)

Permission to use, copy, modify, and distribute this material
for any purpose and without fee is hereby granted, provided
that the above copyright notice and this permission notice
appear in all copies, and that the name of ETL not be
used in advertising or publicity pertaining to this
material without the specific, prior written permission
of an authorized representative of ETL.
ETL MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
/////////////////////////////////////////////////////////////////////////
ontent-Type: program/C; charset=US-ASCII
Program:      codess.h
Author:       Yutaka Sato <ysato@etl.go.jp>
Description:

     This program redirects the file I/O of codes.c
     from/to strings on memory.

History:
	92.05.18   created
///////////////////////////////////////////////////////////////////////*/
#include <stdio.h>

int _to64(), _from64(), _toqp(), _fromqp();
static
str_callfunc(func,in,isize,out,osize,arg3,arg4)
	int (*func)();
	unsigned char *in,*out;
	long arg3,arg4;
{	long In,Out;
	int rcode;
	int len;

	In = str_fopen(in,isize);
	Out = str_fopen(out,osize);
out[0] = 0;
	rcode = (*func)(In,Out,arg3,arg4);
	len = str_ftell(Out);
	out[len] = 0;
	str_fflush(Out);
	str_fclose(In);
	str_fclose(Out);
	return len;
}
str_to64(in,isize,out,osize,pnl)
	unsigned char *in,*out;
{	int len;
	len = str_callfunc(_to64,in,isize,out,osize,pnl);
	return len;
}
str_from64(in,isize,out,osize,pnl)
	unsigned char *in,*out;
{	int len;

	return str_callfunc(_from64,in,isize,out,osize,0,NULL,pnl);
}
str_toqp(in,isize,out,osize)
	unsigned char *in,*out;
{	int len;

	len = str_callfunc(_toqp,in,isize,out,osize);
	if( 2 < len && out[len-2] == '=' && out[len-1] == '\n' ){
		out[len-2] = 0;
		len -= 2;
	}
	return len;
}
str_fromqp(in,isize,out,osize)
	unsigned char *in,*out;
{
	return str_callfunc(_fromqp,in,isize,out,osize,0,NULL);
}

#include "lcl_str_stdio.h"

#define basis_64	_basis_64
#define to64		_to64
#define output64chunk	_output64chunk
#define PendingBoundary	_PendingBoundary
#define from64		_from64
#define toqp		_toqp
#define fromqp		_fromqp
#define	basis_hex	_basis_hex
#define	toqp		_toqp
#define	fromqp		_fromqp
#define	almostputc	_almostputc
#define	nextcharin	_nextcharin

#include "lcl_endecode.c"

