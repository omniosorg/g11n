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
 * Copyright 2002 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>

static xu2jisx0212_0[] = {
#include "JISX0212-0_tbl"
};  

static xu2jisx0213_1[] = {
#include "JISX0213-1_tbl"
};  

static xu2jisx0213_2[] = {
#include "JISX0213-2_tbl"
};  

#define Uchar		unsigned char
#define Uint		unsigned int
#define TBL_MAX		(0x2FFFF)
#define INDEX(x)	((x)&TBL_MAX)	/* Work around for (x) over TBL_MAX. */

Uint
_xu2jis0212 (Uint k)
{
  /* fprintf (stderr, "_xu2jis0212: <0x%x>:%x\n", k, k&0xFFFF); */

  return (xu2jisx0212_0[INDEX(k)]);
}

Uint
_xu2jis02131(Uint k)
{
  /* fprintf (stderr, "_xu2jis02131: <0x%x>:%x\n", k, k&0xFFFF); */

  return (xu2jisx0213_1[INDEX(k)]);
}

Uint
_xu2jis02132(Uint k)
{
  /* fprintf (stderr, "\t_xu2jis02132: <0x%x>:%x\n", k, k&0xFFFF); */

  return (xu2jisx0213_2[INDEX(k)]);
}

Uint
_xu2jisx(Uint k)
{
  /* fprintf (stderr, "\t_xu2unicode: <0x%x>:%x\n", k, k&0xFFFF); */
  
  return (k);
}
