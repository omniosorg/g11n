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

#pragma	ident	"@(#)cstream.h	1.3 01/08/17 SMI"

#ifndef	CSTREAM_H
#define	CSTREAM_H

#include <stdio.h>

/* structure */
typedef struct _CStream{
	FILE	*fp;
	char	*cp;
	int	ptr;
	int	last_c;
} CStream;

/* public functions */
void	CStream_initFile(CStream *stream, FILE *fp);
void	CStream_initString(CStream *stream, char *cp);
int	CStream_parseConvInfo(CStream *stream, ConvInfoEntry *cnv_info);

#endif	/* CSTREAM_H */
