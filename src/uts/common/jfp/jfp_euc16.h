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

#pragma ident "@(#)%M	1.1	97/09/12 SMI"

#ifndef	_JFP_EUC16_H
#define	_JFP_EUC16_H

#define	_EUC16_DEFAULTCHAR	0xa2ae	/* GETA */

#define	ISEUC16_CS0(w)		(((w) & 0x8080) == 0x0000)
#define	ISEUC16_CS1(w)		(((w) & 0x8080) == 0x8080)
#define	ISEUC16_CS2(w)		(((w) & 0x8080) == 0x0080)
#define	ISEUC16_CS3(w)		(((w) & 0x8080) == 0x8000)
#define	EUC16_CS0_GET1(w)	(unsigned char)((w) & 0x7f)
#define	EUC16_CS1_GET1(w)	(unsigned char)(((w) >> 8) & 0xff)
#define	EUC16_CS1_GET2(w)	(unsigned char)(((w) >> 0) & 0xff)
#define	EUC16_CS2_GET1(w)	(unsigned char)(((w) >> 0) & 0xff)
#define	EUC16_CS3_GET1(w)	(unsigned char)(((w) >> 8) & 0xff)
#define	EUC16_CS3_GET2(w)	(unsigned char)((((w) >> 0) & 0xff) | 0x80)

#define	ISEUC1FIRST(c)	((c) >= 0xa1 && (c) <= 0xfe)
#define	ISEUC2FIRST(c)	((c) == SS2)

#define	ISEUC_MBFIRST(c)	( \
	(((c) >= 0xa1) && ((c) <= 0xfe)) || ((c) == SS2) || ((c) == SS3))
#define	ISEUC_SUBSEQ(c)	(((c) >= 0xa1) && ((c) <= 0xfe))
#define	EUCLEN(c)	(((c) == SS3) ? 3 : 2)
#define	EUC_LEN_MAX	3

#endif	/* !_JFP_EUC16_H */
