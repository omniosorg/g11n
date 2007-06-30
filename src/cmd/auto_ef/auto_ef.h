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
 * Copyright (c) 2003 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#ifndef _AUTO_EF_H
#define _AUTO_EF_H

#ident  "@(#)auto_ef.h 1.6 03/07/18 SMI"

#include <sys/types.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct auto_ef_info *auto_ef_t;

#define AE_LEVEL_0 0
#define AE_LEVEL_1 1
#define AE_LEVEL_2 2
#define AE_LEVEL_3 3

extern size_t auto_ef_file(auto_ef_t **, const char *, int);
extern size_t auto_ef_str (auto_ef_t **, const char *, size_t, int);
extern void auto_ef_free(auto_ef_t *);
extern char *auto_ef_get_encoding(auto_ef_t);
extern double auto_ef_get_score(auto_ef_t);

#ifdef __cplusplus
}
#endif

#endif /* _AUTO_EF_H */
