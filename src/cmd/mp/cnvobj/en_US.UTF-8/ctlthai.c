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
#pragma ident "@(#)ctlthai.c	1.2 99/11/10 SMI"

unsigned int
_ctlthai(unsigned int k) {
	k=k&0x0000ffff;
	if (k>=0x20 && k<=0x7e) return(k);
	if (k==0x7f) return(0xf71b);
	if (k>=0x80 && k<=0x84) return(0xf680+k);
	if (k==0x85) return(0x2026);
	if (k>=0x86 && k<=0x90) return(0xf681+k);
	if (k>=0x91 && k<=0x97) return(0x1f87+k);
	if (k>=0x98 && k<=0x9f) return(0xf678+k);
	if (k>=0xa1 && k<=0xda) return(0xd60+k);
	if (k>=0xdb && k<=0xdc) return(0xf641+k);
	if (k>=0xdf && k<=0xfb) return(0xd60+k);
	if (k>=0xfc && k<=0xfe) return(0xf61c+k);
	return(0);
}
