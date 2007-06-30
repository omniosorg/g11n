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

#pragma ident  "@(#)xwc2PCK.c  1.3     04/10/26 SMI"

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

#define Uint	unsigned int
#define Uchar	unsigned char

Uchar mbc[2];

Uint _sjis2jis();

Uint
_xwc2jisx0201(Uint k) {
    return k;
}

Uint
_xwc2jisx0208(Uint k) {
  register int len;

  len = wctomb((char *)mbc, (wchar_t)k);

  return (len == 2)? _sjis2jis((mbc[0]<<8)|mbc[1]):k;
}


Uint
_sjis2jis(Uint sjis)
{
  register Uint hib, lob;

  hib = (sjis >> 8) & 0xff;
  lob = sjis & 0xff;
  hib -= (hib <= 0x9f) ? 0x71 : 0xb1;
  hib = (hib << 1) + 1;
  if (lob > 0x7f)
    lob--;
  if (lob >= 0x9e) {
    lob -= 0x7d;
    hib++;
  } else
    lob -= 0x1f;

  return (hib << 8) | lob;
}

_xwc2ucs4(Uint k) {
  /* 
   * See iconv/common/PCK_TO_Unicode.c for detail 
   */
  if (wctomb((char *)mbc, (wchar_t)k) > 0) {
    if (ISASC(mbc[0])) {
        return (Uint)_jfp_tbl_jisx0201roman_to_ucs2[mbc[0]];
    } else if (ISSJKANA(mbc[0])) {
	return (Uint)_jfp_tbl_jisx0201kana_to_ucs2[(mbc[0] - 0xa1)];
    } else if (ISSJKANJI1(mbc[0])) {
	mbc[0] = sjtojis1[(mbc[0] - 0x80)];
	if (mbc[1] >= 0x9f) {
		mbc[0]++;
	}
	return (Uint)_jfp_tbl_jisx0208_to_ucs2[
		((mbc[0] - 0x21) * 94) + (sjtojis2[mbc[1]] - 0x21)];
    } else if (ISSJSUPKANJI1(mbc[0])) {
	mbc[0] = sjtojis1[(mbc[0] - 0x80)];
	if (mbc[1] >= 0x9f) {
		mbc[0]++;
	}
	return (Uint)_jfp_tbl_jisx0212_to_ucs2[
		((mbc[0] - 0x21) * 94) + (sjtojis2[mbc[1]] - 0x21)];
    } else if (ISSJIBM(mbc[0]) || ISSJNECIBM(mbc[0])) {
 	Uint dest, index;
	dest = (mbc[0] << 8) + mbc[1];
	if ((0xed40 <= dest) && (dest <= 0xeffc))
		REMAP_NEC(dest);
	if ((dest == 0xfa54) || (dest == 0xfa5b)) {
		if (dest == 0xfa54) {
			index = (2 - 1) * 94 + (44 - 1);
		} else {
			index = (2 - 1) * 94 + (72 - 1);
		}
		return (Uint)_jfp_tbl_jisx0208_to_ucs2[index];
	} else {
		Uint upper, lower;
		dest = dest - 0xfa40 - (((dest>>8) - 0xfa) * 0x40);
		dest = sjtoibmext[dest];
		upper = ((dest >> 8) & 0x7f) - 0x21;
		lower = (dest & 0x7f) - 0x21;
		index = (unsigned int)(upper * 94 + lower);
		return (Uint)_jfp_tbl_jisx0212_to_ucs2[index];
	}
    }
  }
  return (0x3000);
}

Uint
_xwc2jisx0212(Uint k) {
  Uchar 	 *src;
  unsigned short s;
  unsigned short dest;
  int		 count;

  wctomb((char *)mbc, (wchar_t)k);
  src = mbc;

  count = (Uchar)*src - 0xfa; /* 'raw' number */
  s = ( (Uchar)*src << 8); src++;
  s += (Uchar)*src;

  s = s -  0xfa40 - (count * 0x40);
  dest = sjtoibmext[s];

  if(dest != 0xffff)
    dest -= 0x8080;

  return dest;
}
