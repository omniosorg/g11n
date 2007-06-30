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

#ident  "@(#)auto_ef_methods.c 1.7 04/03/15 SMI"

#include "auto_ef_lib.h"

void auto_ef_free(auto_ef_t *rtp) {
	int i = 0;
	if (rtp != NULL) {
		while (rtp[i] != (auto_ef_t)NULL) {
			free(rtp[i]);
			i++;
		}
		*rtp = NULL;
	}
	free(rtp);
}

char *auto_ef_get_encoding(auto_ef_t p) {
	if (p != NULL)
		return (p->encoding);
	else
		return (NULL);
}

double auto_ef_get_score(auto_ef_t p) {
	if (p != NULL)
		return (p->score);
	else
		return (NULL);
}
