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
 
#pragma ident	"@(#)lcl_str_stdio.h	1.3	00/01/07 SMI"

/*
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

Author:       Yutaka Sato <ysato@etl.go.jp>

     This program redirects the file I/O from/to strings on memory.
     Include "str_stdio.h" instead of <stdio.h>

///////////////////////////////////////////////////////////////////////*/

#include <stdio.h>
#undef getc
#undef putc

#define getc(file)		str_getc(file)
#define fgetc(file)		str_getc(file)
#define ungetc(ch,file)		str_ungetc(ch,file)
#define fgets(buf,size,file)	str_fgets(buf,size,file)
#define putc(ch,file)		str_putc(ch,file)
#define fputc(ch,file)		str_putc(ch,file)
#define fputs(buf,file)		str_fputs(buf,file)
#define fflush(file)		str_fflush(file)
#define fseek(file,off,where)	str_fseek(file,off,where)
#define ftell(file)		str_ftell(file)
#define fclose(file)		str_fclose(file)

#define fprintf			str_fprintf
