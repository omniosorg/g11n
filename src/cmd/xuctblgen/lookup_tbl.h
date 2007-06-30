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

#ifndef	LOOKUP_TBL_H
#define	LOOKUP_TBL_H

#pragma	ident	"@(#)lookup_tbl.h	1.8 01/10/16 SMI"

#include <stdio.h>
#include <sys/types.h>
#include <sys/isa_defs.h>


/* structures */
typedef struct _LookupTableEntry {
	unsigned long	offset;
	int	target_byte;
	void	*cs_entry;
	int	entry_unit;
	void	*entry;
} LookupTableEntry;

typedef struct _LookupTable {
	int	src_size;
	int	dst_size;
	boolean_t	cs_on;
	int	tbl_ptr;
	int	length;
	int	alloc_len;
	int	alloc_unit;
	char	table_type;
	LookupTableEntry	*tbl_list;
	CARD8	*vector;
} LookupTable;

#ifdef	_BIG_ENDIAN
typedef struct _lookup_size_type_t {
	CARD32	size	:24;
	CARD32	type	:8;
} lookup_size_type_t;
#else
typedef struct _lookup_size_type_t {
	CARD32	type	:8;
	CARD32	size	:24;
} lookup_size_type_t;
#endif	/* _BIG_ENDIAN */

/*
 * Since the "size" from the lookup_size_type_t is a 24-bit unsigned unit,
 * the maximum value that the "size" can have is pow(2, 24) - 1.
 */
#define	LOOKUP_MAX_SIZE			(16777215)

/*
 * The following values are to identify the table type, esp., at table_type
 * data field of LookupTable data structure at above.
 *
 * When U8_TABLE_TYPE_TRIE is specified, the data structure will be in
 * Trie data structure at "tbl_list" data field.
 * When U8_TABLE_TYPE_VECTOR is specified, the data structure will be in
 * a Vector data structure at "vector" data field. The "length" will 
 * be the number of components in the vector and the "alloc_len" will
 * have allocated memory block size.
 */
#define	U8_TABLE_TYPE_NOT_DEFINED	0
#define	U8_TABLE_TYPE_TRIE		1
#define	U8_TABLE_TYPE_VECTOR		2
#define	U8_TABLE_TYPE_MAX_VALUE		U8_TABLE_TYPE_VECTOR


/* public functions */
LookupTable *LookupTable_create(int src_size, int dst_size, boolean_t cs_on,
		char tbl_type);
int LookupTable_add(LookupTable *lookup, unsigned long src, unsigned long dst,
		int cs, int cs_length);
int LookupTable_save(LookupTable *lookup, int fd);
void LookupTable_destroy(LookupTable *lookup);


#endif	/* LOOKUP_TBL_H */
