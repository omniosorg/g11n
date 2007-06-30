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

#pragma	ident	"@(#)conv_info.c	1.3 01/08/17 SMI"


#include <stddef.h>

#include "conv_info.h"

ConvInfo *
ConvInfo_create()
{
	ConvInfo	*tbl;

	tbl = (ConvInfo *)malloc(sizeof(ConvInfo));
	if (tbl == (ConvInfo *)NULL)
		return (ConvInfo *)NULL;

	tbl->length = 0;
	tbl->entry = (ConvInfoEntry *)NULL;
	tbl->alloc_len = 0;
	tbl->alloc_unit = 256;
	
	return tbl;
}

int
ConvInfo_add(ConvInfo *tbl, ConvInfoEntry *add_entry)
{
	int	i;

	if(tbl->length >= tbl->alloc_len){
		int new_length = tbl->alloc_len + tbl->alloc_unit;
		ConvInfoEntry *new_entry = (ConvInfoEntry *)malloc(sizeof(ConvInfoEntry) * new_length);
		if(new_entry == (ConvInfoEntry *)NULL)
			return -1;

		for(i = 0; i < tbl->alloc_len; i++){
			new_entry[i].cs_begin = tbl->entry[i].cs_begin;
			new_entry[i].cs_end = tbl->entry[i].cs_end;
			new_entry[i].wc_begin = tbl->entry[i].wc_begin;
			new_entry[i].wc_end = tbl->entry[i].wc_end;
		}

		if(tbl->entry)
			free(tbl->entry);

		tbl->entry = new_entry;
		tbl->alloc_len = new_length;
	}

	tbl->entry[tbl->length].cs_begin = add_entry->cs_begin;
	tbl->entry[tbl->length].cs_end = add_entry->cs_end;
	tbl->entry[tbl->length].wc_begin = add_entry->wc_begin;
	tbl->entry[tbl->length].wc_end = add_entry->wc_end;
	(tbl->length)++;

	return 0;
}

void
ConvInfo_destroy(ConvInfo *tbl)
{
	if(tbl->entry)
		free(tbl->entry);
	free(tbl);
}
