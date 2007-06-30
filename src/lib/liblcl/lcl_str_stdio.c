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
 
#pragma ident	"@(#)lcl_str_stdio.c	1.2	00/01/06 SMI"

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
Content-Type: program/C; charset=US-ASCII
Program:      str_stdio.h
Author:       Yutaka Sato <ysato@etl.go.jp>
Description:

     This program redirects the file I/O from/to strings on memory.
     Include "str_stdio.h" file after <stdio.h>

History:
	92.05.18   created
///////////////////////////////////////////////////////////////////////*/
#include <stdio.h>
typedef unsigned char Uchar;

#define Str_MAGIC 0x12345678
typedef struct {
	FILE	s_FILE;
	int	s_magic;
	Uchar  *s_base;
	int	s_peak;
	int	s_maxsize;		/* limit of auto expansion */
	int	s_size;
} String;

#define str_isSTR(Str)	(Str->s_magic == Str_MAGIC)

str_isStr(Str)
	register String *Str;
{
	return str_isSTR(Str);
}
String *
str_fopen(buf,size)
	unsigned char *buf;
{	String *Str;

	Str = (String*)calloc(1,sizeof(String));
	Str->s_magic = Str_MAGIC;
	Str->s_base = buf;
	Str->s_size = size;
	Str->s_peak = 0;
	return Str;
}
str_fclose(Str)
	String *Str;
{
	if( !str_isSTR(Str) )
		return fclose((FILE*)Str);

	str_fflush(Str);
	free(Str);
	return 0;
}

str_getc(Str)
	register String *Str;
{
	if( !str_isSTR(Str) )
		return fgetc((FILE*)Str);

	if( Str->s_size <= Str->s_peak )
		return EOF;

	return Str->s_base[Str->s_peak++];
}
str_feof(Str)
	String *Str;
{
	if( !str_isSTR(Str) )
		return feof((&Str->s_FILE));
	return Str->s_size <= Str->s_peak;
}
str_ungetc(ch,Str)
	String *Str;
{
	if( !str_isSTR(Str) )
		return ungetc(ch,(FILE*)Str);

	if( Str->s_peak <= 0)
		return EOF;

	Str->s_base[--Str->s_peak] = ch;
	return ch;
}
char *
str_fgets(buf,size,Str)
	char *buf;
	String *Str;
{	int rsize,nlx;
	Uchar *top,*nlp;

	if( !str_isSTR(Str) )
		return fgets(buf,size,(FILE*)Str);

	rsize = Str->s_size - Str->s_peak;
	if( rsize <= 0 )
		return NULL;
	if( rsize < size )
		size = rsize;

	top = &Str->s_base[Str->s_peak];
	for(nlx = 0; nlx < rsize; nlx++)
		if( top[nlx] == '\n' ){
			size = nlx+1;
			break;
		}
	strncpy(buf,top,size);
	Str->s_peak += size;
	buf[size] = 0;
	return buf;
}

str_putc(ch,Str)
	String *Str;
{
	if( !str_isSTR(Str) )
		return fputc(ch,(FILE*)Str);

	if( Str->s_size <= Str->s_peak )
		return EOF;

	Str->s_base[Str->s_peak++] = ch;

	return ch;
}

str_fputs(buf,Str)
	char *buf;
	String *Str;
{	int size,rsize;

	if( !str_isSTR(Str) )
		return fputs(buf,(FILE*)Str);

	rsize = Str->s_size - Str->s_peak;
	if( rsize <= 0 )
		return EOF;

	size = strlen(buf);
	if( size == 0 )
		return(0);
	if( rsize < size )
		size = rsize;

	strncpy(&Str->s_base[Str->s_peak],buf,size);
	Str->s_peak += size;

	return 0;
}

str_fflush(Str)
	String *Str;
{
	if( !str_isSTR(Str) )
		return fflush((FILE*)Str);

	Str->s_base[Str->s_peak] = 0;
	return 0;
}
str_fprintf(Str,form,a,b,c,d,e,f)
	String *Str;
	char *form;
	long a,b,c,d,e,f;
{	int wlen;
	unsigned char *peakp;

	if( !str_isSTR(Str) )
		return fprintf((FILE*)Str,form,a,b,c,d,e,f);

	peakp = &Str->s_base[Str->s_peak];
	sprintf((char*)peakp,form,a,b,c,d,e,f);
	wlen = strlen(peakp);

	Str->s_peak += wlen;
	return  wlen;
}
str_fseek(Str,off,where)
	String *Str;
{	int noff;

	if( !str_isSTR(Str) )
		return fseek((FILE*)Str,off,where);

	switch( where ){
		case 0: noff = off; break;
		case 1: noff = Str->s_peak + off; break;
		case 2: noff = Str->s_size-1 + off; break;
		default: return -1;
	}

	if( noff < 0 || Str->s_size <= noff )
		return -1;
	Str->s_peak = noff;
}
str_ftell(Str)
	String *Str;
{
	if( !str_isSTR(Str) )
		return ftell((FILE*)Str);

	return Str->s_peak;
}
