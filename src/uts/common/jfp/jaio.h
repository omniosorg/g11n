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
 * Copyright (c) 1991, Sun Microsystems, Inc.
 */

#ifndef _JAIO_H
#define _JAIO_H

#ident  "@(#)jaio.h 1.4     93/05/19 SMI"

#define JA_SKIOC   _IOW('J', 1, struct kioc)
#define JA_GKIOC   _IOR('J', 2, struct kioc)

struct kioc {
    char ki;
    char ko;
};
typedef struct kioc kioc_t;

#endif /* !_JAIO_H */

