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
 * Copyright (c) 1996, 2001 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma	ident	"@(#)lookup_tbl.c	1.12 01/10/16 SMI"

#include <X11/Xmd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/isa_defs.h>

#include "lookup_tbl.h"

#define LUT_CODESET_UNKOWN	0xff
#define LUT_CODE_UNDEFINED	0x00

#define CS_ENTRY		CARD8

static int LookupTable_addtable(LookupTable *lookup, int target_byte);
static int LookupTableEntry_add(LookupTable *lookup, int tbl_num,
		unsigned long src, unsigned long dst, int dst_size,
			int cs, int cs_length);
static int LookupTable_save_vector_type(LookupTable *lookup, int fd);


LookupTable *
LookupTable_create(int src_size, int dst_size, boolean_t cs_on, char tbl_type)
{
	LookupTable *lookup;
	int i;
	int cardinality;

	lookup = (LookupTable *)malloc(sizeof(LookupTable));
	if (lookup == (LookupTable *)NULL)
		return (LookupTable *)NULL;

	lookup->src_size = src_size;
	lookup->dst_size = dst_size;
	lookup->cs_on = cs_on;
	lookup->table_type = tbl_type;
	lookup->length = 0;

	if (tbl_type == U8_TABLE_TYPE_VECTOR) {
		lookup->tbl_list = (LookupTableEntry *)NULL;

		/* The following three lines are "pow(2, 8 * src_size)". */
		lookup->alloc_len = 1;
		for (i = 0; i < src_size; i++)
			lookup->alloc_len = lookup->alloc_len * 256;
		cardinality = lookup->alloc_len;

		/*
		 * If we need to have CodeSet id for each of the component,
		 * i.e., cs_on == True, then we allocate one more byte for
		 * that. (The "codeset id" is also known as "font id" and
		 * they are appended after "cs" and "fs" as like "cs0", "cs1"
		 * and so on at the XLC_LOCALE file.)
		 */
		if (dst_size > 4 || dst_size < 1)
			return ((LookupTable *)NULL);
		lookup->alloc_unit = (dst_size == 3 ? dst_size + 1 : dst_size)
					+ (cs_on ? 1 : 0);
		lookup->alloc_len = lookup->alloc_len * lookup->alloc_unit;

		lookup->vector = (CARD8 *)malloc(lookup->alloc_len);
		if (lookup->vector == (CARD8 *)NULL) {
			lookup->alloc_len = lookup->alloc_unit = 0;
			return ((LookupTable *)NULL);
		}

		/*
		 * Let's initialize with LUT_CODESET_UNKOWN and
		 * LUT_CODE_UNDEFINED. The first byte in each entry/component
		 * is for the codeset id and the following byte(s) is/are for
		 * glyph id.
		 */
		memset((void *)lookup->vector, LUT_CODE_UNDEFINED,
			lookup->alloc_len);
		if (cs_on) {
			for (i = 0; i < cardinality; i++)
				lookup->vector[lookup->alloc_unit * i] =
					(CARD8)LUT_CODESET_UNKOWN;
		}

		return (lookup);
	}

	/*
	 * The following is only for if tbl_type != U8_TABLE_TYPE_VECTOR.
	 * In other words, the table type is either U8_TABLE_TYPE_NOT_DEFINED
	 * or U8_TABLE_TYPE_TRIE; in these cases, the "Trie" is used by
	 * default.
	 */
	lookup->alloc_len = 0;
	lookup->alloc_unit = 256;
	lookup->tbl_list = (LookupTableEntry *)NULL;
	lookup->vector = (CARD8 *)NULL;

	lookup->tbl_ptr = LookupTable_addtable(lookup, src_size - 1);
	if (lookup->tbl_ptr < 0) {
		free(lookup);
		return (LookupTable *)NULL;
	}

	return lookup;
}

	
int
LookupTable_add(LookupTable *lookup, unsigned long src, unsigned long dst,
		int cs, int cs_length)
{
	return LookupTableEntry_add(lookup, lookup->tbl_ptr, src, dst,
		lookup->dst_size, cs, cs_length);
}


int
LookupTable_save(LookupTable *lookup, int fd)
{
	int	i, j, size;
	CARD32	total_size;
	LookupTableEntry	null_tbl[3];
	int	null_tbl_offset[4];
	int	valid_null_tbl;
	lookup_size_type_t	size_type;

	if (lookup->table_type == U8_TABLE_TYPE_VECTOR) {
		return (LookupTable_save_vector_type(lookup, fd));
	}

	/* count table size & set table offset */
	total_size = 0;
	for (i = 0; i < lookup->length; i++) {
		lookup->tbl_list[i].offset = total_size;
		if (lookup->tbl_list[i].cs_entry)
			total_size += sizeof(CS_ENTRY);
		total_size += lookup->tbl_list[i].entry_unit;
	}

	/* create null table */
	for (i = 0; i < lookup->src_size - 1; i++) {
		null_tbl[i].target_byte = i;
		null_tbl[i].cs_entry = (void *)NULL;
		null_tbl[i].entry_unit = 0;
		null_tbl[i].entry = (void *)NULL;
	}

	null_tbl_offset[0] = total_size;
	if (lookup->cs_on == B_TRUE) {
		null_tbl[0].cs_entry = (CS_ENTRY *)malloc(sizeof(CS_ENTRY) *
						256);
		if (null_tbl[0].cs_entry == (CS_ENTRY *)NULL)
			goto err_return;
		memset((void *)(null_tbl[0].cs_entry), LUT_CODESET_UNKOWN,
			sizeof(CS_ENTRY) * 256);
		null_tbl_offset[1] = null_tbl_offset[0] + sizeof(CS_ENTRY);
	} else {
		null_tbl[0].entry_unit = lookup->dst_size;
		null_tbl[0].entry = (void *)malloc(null_tbl[0].entry_unit *
						256);
		if (null_tbl[0].entry == (void *)NULL)
			goto err_return;
		memset(null_tbl[0].entry, LUT_CODE_UNDEFINED,
			null_tbl[0].entry_unit * 256);
		null_tbl_offset[1] = null_tbl_offset[0] +
					null_tbl[0].entry_unit;
	}

	for (i = 1; i < lookup->src_size - 1; i++) {
		null_tbl[i].entry_unit = sizeof(CARD32);
		null_tbl[i].entry = (void *)malloc(null_tbl[i].entry_unit *
						256);
		if (null_tbl[i].entry == (void *)NULL)
			goto err_return;
		for (j = 0; j < 256; j++)
			((CARD32 *)(null_tbl[i].entry))[j] =
						null_tbl_offset[i - 1];
		null_tbl_offset[i + 1] = null_tbl_offset[i] +
						null_tbl[i].entry_unit;
	}

	/* change table entry (table_num -> offset) */
	valid_null_tbl = -1;
	for (i = 0; i < lookup->length; i++) {
		if (lookup->tbl_list[i].target_byte <= 0)
			continue;
		for (j = 0; j < 256; j++) {
			if (lookup->src_size == 2) {
				if (((CARD16 *)(lookup->tbl_list[i].entry))[j] == 0) {
					((CARD16 *)(lookup->tbl_list[i].entry))[j] = null_tbl_offset[lookup->tbl_list[i].target_byte - 1];
					if (valid_null_tbl < lookup->tbl_list[i].target_byte - 1)
						valid_null_tbl = lookup->tbl_list[i].target_byte - 1;
				} else
					((CARD16 *)(lookup->tbl_list[i].entry))[j] = lookup->tbl_list[((CARD16 *)(lookup->tbl_list[i].entry))[j]].offset;
			} else {
				if (((CARD32 *)(lookup->tbl_list[i].entry))[j] == 0) {
					((CARD32 *)(lookup->tbl_list[i].entry))[j] = null_tbl_offset[lookup->tbl_list[i].target_byte - 1];
					if (valid_null_tbl < lookup->tbl_list[i].target_byte - 1)
						valid_null_tbl = lookup->tbl_list[i].target_byte - 1;
				} else
					((CARD32 *)(lookup->tbl_list[i].entry))[j] = lookup->tbl_list[((CARD32 *)(lookup->tbl_list[i].entry))[j]].offset;
			}
		}
	}

	/* adjust table size */
	if (valid_null_tbl >= 0)
		total_size = null_tbl_offset[valid_null_tbl + 1];

	/* Write the table to the file. */

	/*
	 * We save first the actual size in bytes. Since we are using
	 * unsigned 24-bit unit, the max size we can have is 16MB.
	 * (Previously, it was just the number of 256 byte tables and
	 * xlcUTF-8.so.2 did read and multiply by 256 which is absolutely
	 * not necessary considering we will practically never make out
	 * anything near to 16MB size file.
	 */
	total_size = total_size * 256;
	if (total_size > LOOKUP_MAX_SIZE)
		goto err_return;

	/*
	 * Write the table size and the type so that xlcUTF-8.so.2 can apply
	 * correct index calculation and data retrieval from the bit block.
	 */
	size_type.size = total_size;
	size_type.type = lookup->table_type;
	if (write(fd, (const void *)&size_type, sizeof(lookup_size_type_t)) !=
	    sizeof(lookup_size_type_t))
		goto err_return;

	/*
	 * Write the actual bit block, i.e., the table in multi-level
	 * Trie data structure.
	 */
	for (i = 0; i < lookup->length; i++) {
		if (lookup->tbl_list[i].cs_entry) {
			size = sizeof(CS_ENTRY) * 256;
			if (write(fd,
			    (const void *)lookup->tbl_list[i].cs_entry, size)
			    != size)
				goto err_return;
		}
		size = lookup->tbl_list[i].entry_unit * 256;
		if (write(fd, (const void *)lookup->tbl_list[i].entry, size)
		    != size)
			goto err_return;
	}
	if (valid_null_tbl >= 0) {
		if (null_tbl[0].cs_entry) {
			size = sizeof(CS_ENTRY) * 256;
			if (write(fd, (const void *)null_tbl[0].cs_entry, size)
			    != size)
				goto err_return;
		} else {
			size = null_tbl[0].entry_unit * 256;
			if (write(fd, (const void *)null_tbl[0].entry, size)
			    != size)
				goto err_return;
		}
	}
	for (i = 1; i <= valid_null_tbl; i++) {
		size = null_tbl[i].entry_unit * 256;
		if (write(fd, (const void *)null_tbl[i].entry, size) != size)
			goto err_return;
	}

	/* free resource */
	for (i = 0; i < lookup->src_size - 1; i++) {
		if (null_tbl[i].cs_entry)
			free(null_tbl[i].cs_entry);
		if (null_tbl[i].entry)
			free(null_tbl[i].entry);
	}
	
	return 0;
					
err_return:
	for (i = 0; i < lookup->src_size - 1; i++) {
		if (null_tbl[i].cs_entry)
			free(null_tbl[i].cs_entry);
		if (null_tbl[i].entry)
			free(null_tbl[i].entry);
	}
	return -1;
}


static int
LookupTable_save_vector_type(LookupTable *lookup, int fd)
{
	lookup_size_type_t size_type;

	/* Check if the request is made to a correct lookup table. */
	if (lookup->alloc_len <= 0 || lookup->alloc_unit <= 0 ||
	    lookup->vector == (CARD8 *)NULL)
		return (-1);

	/* If the length is zero, we also don't write anything for this. */
	if (lookup->length == 0)
		return (-1);

	/*
	 * Write the size and the type of the table. The size, i.e.,
	 * lookup->alloc_len, does not include bytes for the size and type
	 * (a CARD32).
	 */
	size_type.size = lookup->alloc_len;
	size_type.type = (CARD8)U8_TABLE_TYPE_VECTOR;

	if (write(fd, (const void *)&size_type, sizeof(lookup_size_type_t))
	    != sizeof(lookup_size_type_t))
		return (-1);

	/* Write the vector. */
	if (write(fd, (const void *)lookup->vector, lookup->alloc_len) !=
	    lookup->alloc_len)
		return (-1);

	return (0);
}


void
LookupTable_destroy(LookupTable *lookup)
{
	int	i;

	if (lookup->tbl_list) {
		for (i = 0; i < lookup->length; i++) {
			if (lookup->tbl_list[i].entry)
				free(lookup->tbl_list[i].entry);
			if (lookup->tbl_list[i].cs_entry)
				free(lookup->tbl_list[i].cs_entry);
		}
		free(lookup->tbl_list);
	}

	if (lookup->vector)
		free(lookup->vector);

	free(lookup);
}


static int
LookupTable_addtable(LookupTable *lookup, int target_byte)
{
	int	new_length;
	LookupTableEntry	*new_list;

	if (lookup->length >= lookup->alloc_len) {
		new_length = lookup->alloc_len + lookup->alloc_unit;
		new_list = (LookupTableEntry *)malloc(sizeof(LookupTableEntry)
						* new_length);
		if (new_list == (LookupTableEntry *)NULL)
			return -1;

		if (lookup->tbl_list) {
			memcpy(new_list, lookup->tbl_list,
				sizeof(LookupTableEntry) * lookup->length);
			free(lookup->tbl_list);
		}
		lookup->tbl_list = new_list;
		lookup->alloc_len = new_length;
	}

	lookup->tbl_list[lookup->length].target_byte = target_byte;
	lookup->tbl_list[lookup->length].cs_entry = (CS_ENTRY *)NULL;
	lookup->tbl_list[lookup->length].entry_unit = 0;
	lookup->tbl_list[lookup->length].entry = (void *)NULL;
	(lookup->length)++;

	return lookup->length - 1;
}


static int
LookupTableEntry_add(LookupTable *lookup, int tbl_num, unsigned long src,
			unsigned long dst, int dst_size, int cs, int cs_length)
{
	LookupTableEntry *tbl;
	unsigned char num;
	unsigned long i;

	if (lookup->table_type == U8_TABLE_TYPE_VECTOR) {
		/* Check if the request is made to a correct lookup table. */
		if (lookup->alloc_len == 0 || lookup->alloc_unit == 0 ||
		    lookup->vector == (CARD8 *)NULL)
			return (-1);

		/* Calculate the component's base address from the vector. */
		i = lookup->alloc_unit * src;
		if (i > (lookup->alloc_len - lookup->alloc_unit))
			return (-1);

		/* Save the codeset id first if applicable. */
		if (lookup->cs_on && cs >= 0) {
			lookup->vector[i] = (CARD8)cs;
			i++;
		}

		/*
		 * And then, save the glyph id, or, the wide character
		 * code if the current lookup table is a CS to WC table.
		 */
		switch (dst_size) {
		case 4:
		case 3:
			/*
			 * We cannot do something like the following:
			 *
			 *	*(CARD32 *)(lookup->vector + i) = (CARD32)dst;
			 *
			 * if "cs_on" is true since SPARC SysV ABI has rigid
			 * alignment rule.
			 */
#ifdef	_BIG_ENDIAN
			lookup->vector[i++] = (CARD8)((dst >> 24) & 0xff);
			lookup->vector[i++] = (CARD8)((dst >> 16) & 0xff);
			lookup->vector[i++] = (CARD8)((dst >> 8) & 0xff);
			lookup->vector[i] = (CARD8)(dst & 0xff);
#else
			lookup->vector[i++] = (CARD8)(dst & 0xff);
			lookup->vector[i++] = (CARD8)((dst >> 8) & 0xff);
			lookup->vector[i++] = (CARD8)((dst >> 16) & 0xff);
			lookup->vector[i] = (CARD8)((dst >> 24) & 0xff);
#endif	/* _BIG_ENDIAN */
			break;
		case 2:
			/*
			 * If "cs_on" is true, this is a WC_TO_CS table.
			 * And, if "cs_length" is 1, we save the glyph index at
			 * the first byte so that we will be able to save
			 * computation time at xlcUTF-8.so.2 later.
			 */
			if (lookup->cs_on && cs >= 0 && cs_length == 1) {
				lookup->vector[i] = (CARD8)(dst & 0xff);
			} else {
#ifdef	_BIG_ENDIAN
				lookup->vector[i++] = (CARD8)((dst>>8) & 0xff);
				lookup->vector[i] = (CARD8)(dst & 0xff);
#else
				lookup->vector[i++] = (CARD8)(dst & 0xff);
				lookup->vector[i] = (CARD8)((dst >> 8) & 0xff);
#endif	/* _BIG_ENDIAN */
			}
			break;
		case 1:
			lookup->vector[i] = (CARD8)dst;
			break;
		default:
			return (-1);
			break;
		}

		/*
		 * The lookup->length here is the number of mappings in
		 * this lookup table.
		 */
		lookup->length++;

		return (0);
	}


	/*
	 * If lookup table type is U8_TABLE_TYPE_NOT_DEFINED or
	 * U8_TABLE_TYPE_TRIE, we make a Trie at below.
	 */

	tbl = &(lookup->tbl_list[tbl_num]);

	if (tbl->entry == (void *)NULL) {
		if (tbl->target_byte > 0) {
			switch(lookup->src_size) {
			    case 4:
			    case 3:
				tbl->entry_unit = sizeof(CARD32);
				break;
			    case 2:
			    case 1:
				tbl->entry_unit = sizeof(CARD16);
				break;
			    default:
				return -1;
				break;
			}
		} else {
			tbl->entry_unit = (dst_size == 3) ? dst_size + 1
							  : dst_size;
		}

		tbl->entry = (void *)malloc(tbl->entry_unit * 256);
		if (tbl->entry == (void *)NULL)
			return -1;

		if (tbl->target_byte > 0)
			memset(tbl->entry, 0, tbl->entry_unit * 256);
		else
			memset(tbl->entry, LUT_CODE_UNDEFINED,
					tbl->entry_unit * 256);
	}

	if (tbl->target_byte > 0) {
		num = (src >> (tbl->target_byte * 8)) & 0xff;
		if (tbl->entry_unit == sizeof(CARD16)) {
			if (((CARD16 *)(tbl->entry))[num] == 0) {
				((CARD16 *)(tbl->entry))[num] =
					LookupTable_addtable(lookup,
						tbl->target_byte - 1);
				if (((CARD16 *)(tbl->entry))[num] < 0)
					return -1;
			}
			return LookupTableEntry_add(lookup,
				((CARD16 *)(tbl->entry))[num], src, dst,
					dst_size, cs, cs_length);
		} else {
			if (((CARD32 *)(tbl->entry))[num] == 0) {
				((CARD32 *)(tbl->entry))[num] =
					LookupTable_addtable(lookup,
						tbl->target_byte - 1);
				if (((CARD32 *)(tbl->entry))[num] < 0)
					return -1;
			}
			return LookupTableEntry_add(lookup,
				((CARD32 *)(tbl->entry))[num], src, dst,
					dst_size, cs, cs_length);
		}
	}
	
	num = src & 0xff;
	switch (dst_size) {
	    case 4:
	    case 3:
		((CARD32 *)(tbl->entry))[num] = dst;
		break;
	    case 2:
		((CARD16 *)(tbl->entry))[num] = dst;
		break;
	    case 1:
		((CARD8 *)(tbl->entry))[num] = dst;
		break;
	    default:
		return -1;
		break;
	}

	if (cs >= 0) {
		if (tbl->cs_entry == (CS_ENTRY *)NULL) {
			tbl->cs_entry = (CS_ENTRY *)malloc(sizeof(CS_ENTRY) *
							256);
			if (tbl->cs_entry == (CS_ENTRY *)NULL)
				return 1;
			memset(tbl->cs_entry, LUT_CODESET_UNKOWN,
				sizeof(CS_ENTRY) * 256);
		}	
		((CS_ENTRY *)(tbl->cs_entry))[num] = cs;
	}

	return 0;
}
