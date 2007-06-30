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
 
#pragma ident	"@(#)lcl_buf.c	1.2	00/01/06 SMI"

#include "lcl.h"
#include "lcl_internal.h"

LclBuffer *
_LclBuffer_create(int length)
{
	LclBuffer *buf;

	buf = (LclBuffer *)malloc(sizeof(LclBuffer));
	if(buf == (LclBuffer *)NULL)
		return (LclBuffer *)NULL;

	buf->ptr = (char *)malloc(length);
	if(buf->ptr == (char *)NULL){
		free(buf);
		return (LclBuffer *)NULL;
	}

	buf->pos = 0;
	buf->length = length;

	return buf;
}

int
_LclBuffer_add(LclBuffer *buf, char *str, int len)
{
	if(len == 0)
		return 0;

	if((buf->length - buf->pos) < len){
		while((buf->length - buf->pos) < len)
			buf->length *= 2;
		buf->ptr = (char *)realloc(buf->ptr, buf->length);
		if(buf->ptr == (char *)NULL)
			return -1;
	}
	memcpy(buf->ptr + buf->pos, str, len);
	buf->pos += len;
	return 0;
}

char *
_LclBuffer_get_string(LclBuffer *buf)
{
	char	*ret;

	if(buf->pos == 0){
		return (char *)NULL;
	}
	else{
		buf->length = buf->pos + 1;
		buf->ptr = (char *)realloc(buf->ptr, buf->length);
		if(buf->ptr == (char *)NULL)
			return (char *)NULL;
		*(buf->ptr + buf->pos) = (char)NULL;
		ret = buf->ptr;

		buf->ptr = (char *)NULL;
		buf->pos = 0;
		buf->length = 0;

		return ret;
	}
}

void
_LclBuffer_destroy(LclBuffer *buf)
{
	if(buf->ptr)
		free(buf->ptr);
	free(buf);
}
