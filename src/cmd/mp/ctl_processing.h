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
#pragma ident   "@(#)ctl_processing.h	1.2 99/07/25 SMI"

#ifndef _CTL_PROCESSING_H
#define _CTL_PROCESSING_H
#include <sys/types.h>

typedef struct {
	unsigned char *outbuff;
	size_t inpsize;
	size_t outsize;
	size_t *inptoout;
	size_t *outtoinp;
	unsigned char *property;
	size_t inpbufindex;
	int layout_size;
} transform_arr;

extern char *prolog_locale;

extern unsigned int read_orientation();
extern unsigned int read_numerals();
extern unsigned int read_textshaping();
extern unsigned int read_swapping();
extern unsigned int read_context();

#endif /* _CTL_PROCESSING_H */
