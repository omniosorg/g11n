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

#pragma	ident	"@(#)range_tbl.h	1.3 01/08/17 SMI"

#ifndef	RANGE_TBL_H
#define	RANGE_TBL_H

#include <sys/types.h>

/* structures */
typedef struct _RangeTblEntry{
	unsigned long	begin;
	unsigned long	end;
} RangeTblEntry;

typedef struct _RangeTbl{
	unsigned long	length;
	RangeTblEntry	*entry;
	unsigned long	alloc_len;
	unsigned long	alloc_unit;
} RangeTbl;

/* public functions */
RangeTbl	*RangeTbl_create(int num);
int	RangeTbl_add(RangeTbl *tbl, RangeTblEntry *add_entry);
boolean_t	RangeTbl_in(RangeTbl *tbl, unsigned long code);
void	RangeTbl_destroy(RangeTbl *tbl, int num);

#endif	/* RANGE_TBL_H */
