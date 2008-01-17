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

#pragma	ident	"@(#)range_tbl.c	1.3 01/08/17 SMI"

#include <stdlib.h>

#include "range_tbl.h"

RangeTbl *
RangeTbl_create(int num)
{
	RangeTbl	*tbl;
	int	i;

	tbl = (RangeTbl *)malloc(sizeof(RangeTbl) * num);
	if(tbl == (RangeTbl *)NULL)
		return (RangeTbl *)NULL;

	for(i = 0; i < num; i++){
		tbl[i].length = 0;
		tbl[i].entry = (RangeTblEntry *)NULL;
		tbl[i].alloc_len = 0;
		tbl[i].alloc_unit = 256;
	}

	return tbl;
}

int
RangeTbl_add(RangeTbl *tbl, RangeTblEntry *add_entry)
{
	int	i;
	unsigned long	new_length;
	RangeTblEntry	*new_entry;

	if(tbl == (RangeTbl *)NULL)
		return -1;

	if(tbl->length >= tbl->alloc_len){
		new_length = tbl->alloc_len + tbl->alloc_unit;
		new_entry = (RangeTblEntry *)malloc(sizeof(RangeTblEntry) * new_length);
		if(new_entry == (RangeTblEntry *)NULL)
			return -1;

		for(i = 0; i <tbl->alloc_len; i++){
			new_entry[i].begin = tbl->entry[i].begin;
			new_entry[i].end = tbl->entry[i].end;
		}

		if(tbl->entry)
			free(tbl->entry);

		tbl->entry = new_entry;
		tbl->alloc_len = new_length;
	}

	tbl->entry[tbl->length].begin = add_entry->begin;
	tbl->entry[tbl->length].end = add_entry->end;
	(tbl->length)++;

	return 0;
}

boolean_t
RangeTbl_in(RangeTbl *tbl, unsigned long code)
{
	int	i;

	if (tbl == (RangeTbl *)NULL)
		return B_TRUE;

	if (tbl->length == 0) 
		return B_TRUE;

	for(i = 0; i < tbl->length; i++){
		if((code >= tbl->entry[i].begin) && (code <= tbl->entry[i].end))
			return B_TRUE;
	}

	return B_FALSE;
}
	
void
RangeTbl_destroy(RangeTbl *tbl, int num)
{
	int	i;

	if (tbl == (RangeTbl *)NULL)
		return;

	for(i = 0; i < num; i++){
		if (tbl[i].entry)
			free(tbl[i].entry);
	}
	free(tbl);
}
