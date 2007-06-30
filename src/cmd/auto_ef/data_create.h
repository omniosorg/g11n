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
 * Copyright (c) 2003, by Sun Microsystems, Inc.
 * All rights reserved.
 */

#ifndef	_DATA_CREATE_H
#define	_DATA_CREATE_H

#pragma	ident	"@(#)data_create.h	1.2	03/05/12	SMI"

#ifdef	__cplusplus
extern "C" {
#endif

#define BUFSIZE 65536
#define HASHSIZE 8192
#define TRUE 1
#define FALSE 2
#define ICONV_LOCALE_MAX 128
#define LOCALE_NAME_LENGTH 32
#define SINGLE_ENCODING_MAX 256

/* Hash Record */
typedef struct score_record *srd;
typedef struct score_record{
  unsigned char keyword[2];
  int score;
  srd nextsrd;
}SRD;

/* Hash Table */
srd hashtable[HASHSIZE];

#ifdef	__cplusplus
}
#endif

#endif	/* _DATA_CREATE_H */
