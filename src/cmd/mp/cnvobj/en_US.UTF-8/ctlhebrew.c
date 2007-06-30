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
 * Copyright (c) 1998 by Sun Microsystems, Inc.
 * All rights reserved.
 */
#pragma ident "@(#)ctlhebrew.c	1.2 99/11/10 SMI"

unsigned int
_ctlhebrew(unsigned int k) {
	k=k&0x0000ffff;
	if (k>=0x20 && k<=0x7e) return(k);
	if (k>=0xa0 && k<=0xa9) return(k);
	if (k==0xaa) return(0xd7);
	if (k>=0xab && k<=0xb9) return(k);
	if (k==0xba) return(0xf7);
	if (k>=0xbb && k<=0xbe) return(k);
	if (k==0xdf) return(0x2017);
	if (k>=0xe0 && k<=0xfa) return(0x4f0+k);
	return(0);
}
