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
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma	ident	"@(#)cstream.c	1.3 01/08/17 SMI"

#include <stdio.h>

#include "conv_info.h"
#include "cstream.h"

#include "conv_def.h"

static int	CStream_getc(CStream *stream);
static void	CStream_putc(CStream *stream, int c);
static int	CStream_parseHexNum(CStream *stream, unsigned long *num);

void
CStream_initFile(CStream *stream, FILE *fp)
{
	stream->fp = fp;
	stream->cp = (char *)NULL;
	stream->ptr = 0;
	stream->last_c = (char)NULL;
}
	
void
CStream_initString(CStream *stream, char *cp)
{
	stream->fp = (FILE *)NULL;
	stream->cp = cp;
	stream->ptr = 0;
	stream->last_c = (char)NULL;
}

int
CStream_parseConvInfo(CStream *stream, ConvInfoEntry *cnv_info)
{
	int	c;
	unsigned long	num;

	if ((c = CStream_getc(stream)) == DELIMIT_CHAR)
		c = CStream_getc(stream);

	if (c == RANGE_BEGIN_CHAR){
		if(CStream_parseHexNum(stream, &num))
			return -1;
		cnv_info->cs_begin = num;

		if ((c = CStream_getc(stream)) != RANGE_MIDDLE_CHAR)
			return -1;

		if(CStream_parseHexNum(stream, &num))
			return -1;
		cnv_info->cs_end = num;

		if ((c = CStream_getc(stream)) != RANGE_END_CHAR)
			return -1;
	}
	else{
		CStream_putc(stream, c);
		if(CStream_parseHexNum(stream, &num))
			return -1;
		cnv_info->cs_begin = num;
		cnv_info->cs_end = num;
	}

	if ((c = CStream_getc(stream)) != RANGE_TRANS_CHAR1)
		return -1;
	if ((c = CStream_getc(stream)) != RANGE_TRANS_CHAR2)
		return -1;

	if(CStream_parseHexNum(stream, &num))
		return -1;
	cnv_info->wc_begin = num;
	cnv_info->wc_end = num + cnv_info->cs_end - cnv_info->cs_begin;

	return 0;
}
				

static int
CStream_getc(CStream *stream)
{
	int	c;

	if (stream->last_c){
		c = stream->last_c;
		stream->last_c = NULL;
	}else{
		if(stream->fp){
			c = getc(stream->fp);
			if (c == '\n')
				c = getc(stream->fp);

			while (c == COMMENT_CHAR) {
				do {
					c = getc(stream->fp);
				} while (c != '\n' && c != EOF);
				
				if (c != EOF)
					c = getc(stream->fp);
			}
		}else{
			c = *(stream->cp + stream->ptr);
			if (c == NULL)
				c = EOF;
			(stream->ptr)++;
		}
	}
	return c;
}

static void
CStream_putc(CStream *stream, int c)
{
	stream->last_c = c;
}

static int
CStream_parseHexNum(CStream *stream, unsigned long *hexnum)
{
	int	c, num, count;

	if ((c = CStream_getc(stream)) != '\\')
		return 0;
	switch(c = CStream_getc(stream)){
	    case 'X':
	    case 'x':
		*hexnum = 0;
		count = 0;
		while(1){
			c = CStream_getc(stream);
			if (c >= '0' && c <= '9')
				num = c - '0';
			else if (c >= 'A' && c <= 'F')
				num = c - 'A' + 10;
			else if (c >= 'a' && c <= 'f')
				num = c - 'a' + 10;
			else{
				CStream_putc(stream, c);
				break;
			}
			(*hexnum) <<= 4;
			(*hexnum) += num;
			count++;
		}
		if (!count)
			return -1;
		else
			return 0;
		break;
	    default:
		return -1;
		break;
	}
}
