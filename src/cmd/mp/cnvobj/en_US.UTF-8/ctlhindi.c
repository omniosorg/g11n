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
 * Copyright (c) 2002 by Sun Microsystems, Inc.
 * All rights reserved.
 */
#pragma ident "@(#)ctlhindi.c 1.4 02/09/04 SMI"

unsigned int 
_ctlhindi_ucs4(unsigned int k) {

	k = k & 0x0000ffff;

	if ( k >= 0x00000000 && k <= 0x0000007e)
		return(k);
	if ( k >= 0x00000900 && k <= 0x00000903)
		return(k);
	if ( k >= 0x00000905 && k <= 0x00000939)
		return(k);
	if ( k >= 0x0000093c && k <= 0x0000094d)
		return(k);
	if ( k >= 0x00000950 && k <= 0x00000954)
		return(k);
	if ( k >= 0x00000958 && k <= 0x00000970)
		return(k);
	if ( k >= 0x00000981 && k <= 0x00000983)
		return(k);
	if ( k >= 0x00000985 && k <= 0x0000098c)
		return(k);
	if ( k >= 0x0000098f && k <= 0x00000990)
		return(k);
	if ( k >= 0x00000993 && k <= 0x000009a8)
		return(k);
	if ( k >= 0x000009aa && k <= 0x000009b0)
		return(k);
	if ( k == 0x000009b2)
		return(k);
	if ( k >= 0x000009b6 && k <= 0x000009b9)
		return(k);
	if ( k == 0x000009bc)
		return(k);
	if ( k >= 0x000009be && k <= 0x000009c4)
		return(k);
	if ( k >= 0x000009c7 && k <= 0x000009c8)
		return(k);
	if ( k >= 0x000009cb && k <= 0x000009cd)
		return(k);
	if ( k == 0x000009d7)
		return(k);
	if ( k >= 0x000009dc && k <= 0x000009dd)
		return(k);
	if ( k >= 0x000009df && k <= 0x000009e3)
		return(k);
	if ( k >= 0x000009e6 && k <= 0x000009fa)
		return(k);
	if ( k == 0x00000a02)
		return(k);
	if ( k >= 0x00000a05 && k <= 0x00000a0a)
		return(k);
	if ( k >= 0x00000a0f && k <= 0x00000a10)
		return(k);
	if ( k >= 0x00000a13 && k <= 0x00000a28)
		return(k);
	if ( k >= 0x00000a2a && k <= 0x00000a30)
		return(k);
	if ( k >= 0x00000a32 && k <= 0x00000a33)
		return(k);
	if ( k >= 0x00000a35 && k <= 0x00000a36)
		return(k);
	if ( k >= 0x00000a38 && k <= 0x00000a39)
		return(k);
	if ( k == 0x00000a3c)
		return(k);
	if ( k >= 0x00000a3e && k <= 0x00000a42)
		return(k);
	if ( k >= 0x00000a47 && k <= 0x00000a48)
		return(k);
	if ( k >= 0x00000a4b && k <= 0x00000a4d)
		return(k);
	if ( k >= 0x00000a59 && k <= 0x00000a5c)
		return(k);
	if ( k == 0x00000a5e)
		return(k);
	if ( k >= 0x00000a66 && k <= 0x00000a74)
		return(k);
	if ( k >= 0x00000a81 && k <= 0x00000a83)
		return(k);
	if ( k >= 0x00000a85 && k <= 0x00000a8b)
		return(k);
	if ( k == 0x00000a8d)
		return(k);
	if ( k >= 0x00000a8f && k <= 0x00000a91)
		return(k);
	if ( k >= 0x00000a93 && k <= 0x00000aa8)
		return(k);
	if ( k >= 0x00000aaa && k <= 0x00000ab0)
		return(k);
	if ( k >= 0x00000ab2 && k <= 0x00000ab3)
		return(k);
	if ( k >= 0x00000ab5 && k <= 0x00000ab9)
		return(k);
	if ( k >= 0x00000abc && k <= 0x00000ac5)
		return(k);
	if ( k >= 0x00000ac7 && k <= 0x00000ac9)
		return(k);
	if ( k >= 0x00000acb && k <= 0x00000acd)
		return(k);
	if ( k == 0x00000ad0)
		return(k);
	if ( k == 0x00000ae0)
		return(k);
	if ( k >= 0x00000ae6 && k <= 0x00000aef)
		return(k);
	if ( k >= 0x00000b82 && k <= 0x00000b83)
		return(k);
	if ( k >= 0x00000b85 && k <= 0x00000b8a)
		return(k);
	if ( k >= 0x00000b8e && k <= 0x00000b90)
		return(k);
	if ( k >= 0x00000b92 && k <= 0x00000b95)
		return(k);
	if ( k >= 0x00000b99 && k <= 0x00000b9a)
		return(k);
	if ( k == 0x00000b9c)
		return(k);
	if ( k >= 0x00000b9e && k <= 0x00000b9f)
		return(k);
	if ( k >= 0x00000ba3 && k <= 0x00000ba4)
		return(k);
	if ( k >= 0x00000ba8 && k <= 0x00000baa)
		return(k);
	if ( k >= 0x00000bae && k <= 0x00000bb5)
		return(k);
	if ( k >= 0x00000bb7 && k <= 0x00000bb9)
		return(k);
	if ( k >= 0x00000bbe && k <= 0x00000bc2)
		return(k);
	if ( k >= 0x00000bc6 && k <= 0x00000bc8)
		return(k);
	if ( k >= 0x00000bca && k <= 0x00000bcd)
		return(k);
	if ( k == 0x00000bd7)
		return(k);
	if ( k >= 0x00000be7 && k <= 0x00000bf2)
		return(k);
	if ( k >= 0x00000c01 && k <= 0x00000c03)
		return(k);
	if ( k >= 0x00000c05 && k <= 0x00000c0c)
		return(k);
	if ( k >= 0x00000c0e && k <= 0x00000c10)
		return(k);
	if ( k >= 0x00000c12 && k <= 0x00000c28)
		return(k);
	if ( k >= 0x00000c2a && k <= 0x00000c33)
		return(k);
	if ( k >= 0x00000c35 && k <= 0x00000c39)
		return(k);
	if ( k >= 0x00000c3e && k <= 0x00000c44)
		return(k);
	if ( k >= 0x00000c46 && k <= 0x00000c48)
		return(k);
	if ( k >= 0x00000c4a && k <= 0x00000c4d)
		return(k);
	if ( k >= 0x00000c55 && k <= 0x00000c56)
		return(k);
	if ( k >= 0x00000c60 && k <= 0x00000c61)
		return(k);
	if ( k >= 0x00000c66 && k <= 0x00000c6f)
		return(k);
	if ( k >= 0x00000c82 && k <= 0x00000c83)
		return(k);
	if ( k >= 0x00000c85 && k <= 0x00000c8c)
		return(k);
	if ( k >= 0x00000c8e && k <= 0x00000c90)
		return(k);
	if ( k >= 0x00000c92 && k <= 0x00000ca8)
		return(k);
	if ( k >= 0x00000caa && k <= 0x00000cb3)
		return(k);
	if ( k >= 0x00000cb5 && k <= 0x00000cb9)
		return(k);
	if ( k >= 0x00000cbe && k <= 0x00000cc4)
		return(k);
	if ( k >= 0x00000cc6 && k <= 0x00000cc8)
		return(k);
	if ( k >= 0x00000cca && k <= 0x00000ccd)
		return(k);
	if ( k >= 0x00000cd5 && k <= 0x00000cd6)
		return(k);
	if ( k == 0x00000cde)
		return(k);
	if ( k >= 0x00000ce0 && k <= 0x00000ce1)
		return(k);
	if ( k >= 0x00000ce6 && k <= 0x00000cef)
		return(k);
	if ( k >= 0x00000d02 && k <= 0x00000d03)
		return(k);
	if ( k >= 0x00000d05 && k <= 0x00000d0c)
		return(k);
	if ( k >= 0x00000d0e && k <= 0x00000d10)
		return(k);
	if ( k >= 0x00000d12 && k <= 0x00000d28)
		return(k);
	if ( k >= 0x00000d2a && k <= 0x00000d39)
		return(k);
	if ( k >= 0x00000d3e && k <= 0x00000d43)
		return(k);
	if ( k >= 0x00000d46 && k <= 0x00000d48)
		return(k);
	if ( k >= 0x00000d4a && k <= 0x00000d4d)
		return(k);
	if ( k == 0x00000d57)
		return(k);
	if ( k >= 0x00000d60 && k <= 0x00000d61)
		return(k);
	if ( k >= 0x00000d66 && k <= 0x00000d6f)
		return(k);
	if ( k >= 0x0000f31f && k <= 0x0000f33d)	/* Punjabi ( Gurumukhi) */
		return(k);
	/*
	if ( k >= 0x0000f360 && k <= 0x0000f46d)
	*/
	if ( k >= 0x0000f360 && k <= 0x0000f478)	/*	Bengali	*/
		return(k);
	if ( k >= 0x0000f480 && k <= 0x0000f4c8)	/*	Malayalam */
		return(k);
	/*
	if ( k >= 0x0000f4fb && k <= 0x0000f558)
	*/
	if ( k >= 0x0000f4eb && k <= 0x0000f577)	/* Gujarati	*/
		return(k);
	/*
	if ( k >= 0x0000f56a && k <= 0x0000f577)
		return(k);
	*/
	/*
	if ( k >= 0x0000f59d && k <= 0x0000f5a7)
	*/
	if ( k >= 0x0000f59c && k <= 0x0000f61e)	/* Telugu */
		return(k);
	/*
	if ( k >= 0x0000f5aa && k <= 0x0000f5b1)
		return(k);
	if ( k >= 0x0000f5b3 && k <= 0x0000f5bb)
		return(k);
	if ( k >= 0x0000f5bd && k <= 0x0000f5cb)
		return(k);
	if ( k >= 0x0000f5cd && k <= 0x0000f5da)
		return(k);
	if ( k >= 0x0000f5dc && k <= 0x0000f5e1)
		return(k);
	if ( k >= 0x0000f5e3 && k <= 0x0000f5e5)
		return(k);
	if ( k >= 0x0000f5e7 && k <= 0x0000f61e)
		return(k);
	*/
/*
	if ( k >= 0x0000f64c && k <= 0x0000f6ad)
		return(k);
	if ( k >= 0x0000f64c && k <= 0x0000f6ad)
		return(k);
*/
	if ( k >= 0x0000f64c && k <= 0x0000f6c0)	/* Kannada */
		return(k);
	if ( k >= 0x0000f6e2 && k <= 0x0000f765)	/* Tamil */
		return(k);
	if ( k >= 0x0000f7bf && k <= 0x0000f84d)	/* Devnagari */
		return(k);
        return(0);
}
