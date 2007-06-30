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
 * Copyright (c) 1997, by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma ident "@(#)jfp_ucsutf.h	1.1	97/09/12 SMI"

#ifndef	_JFP_UCSUTF_H
#define	_JFP_UCSUTF_H

/*
 * ISUCS2_nB(ucs2)
 * 	give UCS-2 value as unsigned short argument ucs2.
 *	if number of bytes needed to hold UTF-8 representation of
 *	the UCS-2 character is n, returns true.
 */
#define	ISUCS2_1B(ucs2)	((ucs2) <= 0x7fU)
#define	ISUCS2_2B(ucs2)	((ucs2) <= 0x7ffU)
#define	ISUCS2_3B(ucs2)	((ucs2) <= 0xffffU)

/*
 * UCS2_mB_GETn(ucs2)
 * 	give UCS-2 value as unsigned short argument ucs2.
 *	assume the UCS-2 value consumes m bytes when converted to UTF-8.
 *	returns n'th byte of the character's UTF-8 representation.
 */
#define	UCS2_1B_GET1(ucs2)	(unsigned char)((ucs2) & 0x7f)
#define	UCS2_2B_GET1(ucs2)	(unsigned char)((((ucs2) >>  6) & 0x1f) | 0xc0)
#define	UCS2_2B_GET2(ucs2)	(unsigned char)((((ucs2) >>  0) & 0x3f) | 0x80)
#define	UCS2_3B_GET1(ucs2)	(unsigned char)((((ucs2) >> 12) & 0x0f) | 0xe0)
#define	UCS2_3B_GET2(ucs2)	(unsigned char)((((ucs2) >>  6) & 0x3f) | 0x80)
#define	UCS2_3B_GET3(ucs2)	(unsigned char)((((ucs2) >>  0) & 0x3f) | 0x80)

#define	UTF8_LEN_MAX	6

/*
 * ISUTF8_nB(c)
 * 	give a byte of a UTF-8 string as c.
 *	returns true if the byte is legitimate leading byte of a UTF-8
 *	character and length in bytes of the character is n.
 */
#define	ISUTF8_1B(c)	(((c) & 0x80) == 0x00)
#define	ISUTF8_2B(c)	(((c) & 0xe0) == 0xc0)
#define	ISUTF8_3B(c)	(((c) & 0xf0) == 0xe0)
#define	ISUTF8_4B(c)	(((c) & 0xf8) == 0xf0)
#define	ISUTF8_5B(c)	(((c) & 0xfc) == 0xf8)
#define	ISUTF8_6B(c)	(((c) & 0xfe) == 0xfc)

/*
 * ISUTF8_MBFIRST(c)
 * 	give a byte of a UTF-8 string as c.
 *	returns true if the byte is legitimate leading byte of a
 *	UTF-8 character of 2 byte length or longer.
 */
#define	ISUTF8_MBFIRST(c)	( \
	ISUTF8_2B(c) || ISUTF8_3B(c) || ISUTF8_4B(c) || \
	ISUTF8_5B(c) || ISUTF8_6B(c) \
)

/*
 * ISUTF8_SUBSEQ(c)
 * 	give a byte of a UTF-8 string as c.
 *	returns true if the byte is legitimate subsequent byte of a
 *	UTF-8 character of 2 byte length or longer.
 */
#define	ISUTF8_SUBSEQ(c)	(((c) & 0xc0) == 0x80)

/*
 * UTF8_LEN(c)
 * 	give a byte of a UTF-8 string as c.
 *	if the byte is legitimate leading byte of a UTF-8 character,
 *	return length in bytes of the character. otherwise, return -1.
 */
#define	UTF8_LEN(c)	( \
	ISUTF8_1B(c) ? 1 : ( \
		ISUTF8_2B(c) ? 2 : ( \
			ISUTF8_3B(c) ? 3 : ( \
				ISUTF8_4B(c) ? 4 : ( \
					ISUTF8_5B(c) ? 5 : ( \
						ISUTF8_6B(c) ? 6 : -1 \
					) \
				) \
			) \
		) \
	))

#endif	/* !_JFP_UCSUTF_H */
