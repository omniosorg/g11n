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

#pragma	ident	"@(#)conv_info.h	1.3 01/08/17 SMI"

#ifndef	CONV_INFO_H
#define	CONV_INFO_H

/* structures */
typedef struct _ConvInfoEntry{
	unsigned long	cs_begin;
	unsigned long	cs_end;
	unsigned long	wc_begin;
	unsigned long	wc_end;
} ConvInfoEntry;

typedef struct _ConvInfo{
	unsigned long	length;
	ConvInfoEntry	*entry;
	unsigned long	alloc_len;
	unsigned long	alloc_unit;
} ConvInfo;

/* public functions */
ConvInfo	*ConvInfo_create();
int	ConvInfo_add(ConvInfo *tbl, ConvInfoEntry *add_entry);
void	ConvInfo_destroy(ConvInfo *tbl);

#endif	/* CONV_INFO_H */
