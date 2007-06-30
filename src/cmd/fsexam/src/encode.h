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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */


#ifndef _ENCODE_H_
#define _ENCODE_H_

typedef enum { FAIL, LOW, HIGH, ORIGINAL } Score;
typedef enum { ConvName,
	       ConvContent = 0x10,
	       ConvContentWithCRLF,
	       ConvContentCRLFOnly,
	       ConvNameReverse = 0x100,
	       ConvContentReverse = 0x1000,
	       ConvContentWithCRLFReverse,
	       ConvContentCRLFOnlyReverse
             } ConvType;

#define GetConvTypeReverse(c) ((c == ConvName) ? ConvNameReverse : \
                               (c - ConvContent) + ConvContentReverse)

typedef struct _Encoding Encoding;
struct _Encoding {
  char   codename[20];
  GIConv icd;
  Score  score;
  ConvType convtype;
  union {
    char converted_text[256];
    char *contents;
  } u;
};

typedef gboolean (*EncodeFunc) (Encoding *, gint, va_list);

GList *init_encode (GSList *);
Score decode_analyzer (GList *, ConvType, gchar *, size_t);
void  iterate_encode_with_func (GList *, EncodeFunc, ...);
gboolean get_encode_elements (Encoding *, gint, va_list);
gboolean  translate_encode_index (Encoding *, gint, va_list);
gint  get_first_encode_index (GList *);
void destroy_encode (GList *);
void cleanup_encode (GList *);

#endif
