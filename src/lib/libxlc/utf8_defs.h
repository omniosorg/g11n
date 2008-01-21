/*
 * Copyright 2001-2002 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 *
 * Important note:
 * This header file contains same macros what xuctblgen(1) internal utility at
 * ../../../../programs/xuctblgen/ directory is using. Therefore, if any of
 * the macro values at here or there need to be changed, that change must be
 * synchronized. Otherwise, possibly, xlcUTF-8.so.2 won't work correctly.
 */

#ifndef	UTF8_DEFS_H
#define	UTF8_DEFS_H

#pragma	ident	"@(#)utf8_defs.h	1.3 02/02/02 SMI"

#include <sys/isa_defs.h>
#include <wchar.h>

#define	U8_MAX_PLANES		17
#define U8_MAX_VALUE_IN_UTF32	((unsigned)0x10ffff)


#ifdef	_BIG_ENDIAN

typedef struct {
	wchar_t	group	:8;
	wchar_t	plane	:8;
	wchar_t	rowcell	:16;
} into_octets_t;

typedef struct {
	CARD32  size    :24;
	CARD32  type    :8;
} lookup_size_type_t;

#else

typedef struct {
	wchar_t	rowcell	:16;
	wchar_t	plane	:8;
	wchar_t	group	:8;
} into_octets_t;

typedef struct {
	CARD32  type    :8;
	CARD32  size    :24;
} lookup_size_type_t;

#endif	/* _BIG_ENDIAN */


#define	U8_FALLBACK_ENCODING_NAME	"UNICODE-FONTSPECIFIC"


/*
 * The following values are to identify the table type of conversion
 * tables between wide character and charsets of fonts from the fontset
 * definion of the locale.
 *
 * When U8_TABLE_TYPE_TRIE is specified, the data structure will be in
 * Trie data structure at "tbl_list" data field.
 * When U8_TABLE_TYPE_VECTOR is specified, the data structure will be in
 * a Vector data structure at "vector" data field. The "length" will 
 * be the number of components in the vector and the "alloc_len" will
 * have allocated memory block size.
 */
#define U8_TABLE_TYPE_NOT_DEFINED	0
#define U8_TABLE_TYPE_TRIE		1
#define U8_TABLE_TYPE_VECTOR		2
#define U8_TABLE_TYPE_MAX_VALUE		U8_TABLE_TYPE_VECTOR


#endif	/* UTF8_DEFS_H */
