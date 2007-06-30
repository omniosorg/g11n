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
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident  "@(#)xwc2eucJP.c  1.2     04/10/26 SMI"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <euc.h>

#define JFP_ICONV_STATELESS
#include "japanese.h"
#include "jfp_iconv_unicode.h"
#ifdef JAVA_CONV_COMPAT
#define JFP_J2U_ICONV_JAVA
#else
#define JFP_J2U_ICONV
#endif
#include "jfp_jis_to_ucs2.h"

#define Uchar	unsigned char
#define Uint	unsigned int
#define SPACE	(0x2121)	/* space */
#define GL(jis)	((jis)&0x7F7F)

Uchar mbc[3];

Uint
_xwc2jisx0201(Uint k) {
    return k;
}

Uint
_xwc2jisx0208(Uint k) {

  /* convert wc to mbc and get GL*/
  
  if (wctomb((char *)mbc, (wchar_t)k))
    return (GL(mbc[0]<<8|mbc[1]));
  else
    return (SPACE);
}

Uint
_xwc2jisx0212(Uint k) {

  /* convert wc to mbc and get GL*/
  
  if (wctomb((char *)mbc, (wchar_t)k))
    return(GL(mbc[1]<<8|mbc[2]));
  else
    return (SPACE);
}

Uint
_xwc2ucs4(Uint k) {
  if (wctomb((char *)mbc, (wchar_t)k) > 0) {
    if (ISASC(mbc[0])) {
	return (Uint)_jfp_tbl_jisx0201roman_to_ucs2[mbc[0]];
    } else if (ISCS1(mbc[0])) {
	return (Uint)_jfp_tbl_jisx0208_to_ucs2[
		(mbc[0] - 0xa1) * 94 + (mbc[1] - 0xa1)];
    } else if (mbc[0] == SS2) {
	return (Uint)_jfp_tbl_jisx0201kana_to_ucs2[(mbc[1] - 0xa1)];
    } else if (mbc[0] == SS3) {
	return (Uint)_jfp_tbl_jisx0212_to_ucs2[
		(mbc[1] - 0xa1) * 94 + (mbc[2] - 0xa1)];
    }
  }
  return (0x3000);
}
