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
#pragma ident "@(#)ctlarabic.c	1.2 99/11/10 SMI"

unsigned int
_ctlarabic(unsigned int k) {
	k=k&0x0000ffff;
	if (k>=0x20 && k<=0x7e) return(k);
	if (k>=0xad && k<=0xff) return(0x560+k);
	if (k>=0x600 && k<=0x6ff) return(k);
	if (k>=0xfb50 && k<=0xfffe) return(k);
	return(0);
}
