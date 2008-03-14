/*
 * Copyright (c) 1996,1999 by Sun Microsystems, Inc.
 * All Rights Reserved.
 */

#pragma	ident	"@(#)common.h	1.1 99/03/05 SMI"


#ifndef	_COMMON_H_
#define	_COMMON_H_


#define	K_UTF_1_BYTE(x)		((x) < 0x80)
#define	K_UTF_2_BYTE(x)		(((x) & 0xe0) == 0xc0)
#define	K_UTF_3_BYTE(x)		(((x) & 0xf0) == 0xe0)
#define	K_UTF_4_BYTE(x)		(((x) & 0xf8) == 0xf0)
#define	K_UTF_5_BYTE(x)		(((x) & 0xfc) == 0xf8)
#define	K_UTF_6_BYTE(x)		(((x) & 0xfe) == 0xfc)

#define	NON_IDENTICAL		'?'

typedef struct {
	unsigned int    u8;
	signed char     size;
} to_utf8_table_component_t;

typedef struct {
	unsigned int    u8;
	unsigned char   sb;
} to_sb_table_component_t;


#endif	/* _COMMON_H_ */
