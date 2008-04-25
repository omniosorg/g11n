/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _ENCODE_H_
#define _ENCODE_H_

typedef enum { 
        FAIL, 
        LOW, 
        HIGH, 
        ORIGINAL 
} Score;

typedef enum { 
        ConvName,
        ConvNameSpecial,

        ConvContent = 0x10,
        ConvContentSpecial,
        ConvContentWithCRLF,
        ConvContentCRLFOnly,

        RestoreConvName = 0x100,
        RestoreConvNameSpecial,
        RestoreConvContent = 0x1000,
        RestoreConvContentWithCRLF,
        RestoreConvContentCRLFOnly,

        DryRunName,
        DryRunContent,

	ConvInvalid,
} ConvType;

#define VALID_CONVTYPE(v) (((v) == ConvName) || ((v) & ConvContent) \
                    || ((v) & ConvNameSpecial) || ((v) && ConvContentSpecial))

#define MATCH_CONVTYPE(v, w) ((v) == ConvName ? (w) == ConvName : \
                  ((w) == ConvContent || (w) == ConvContentWithCRLF \
                   || (w) == ConvContentCRLFOnly))

#define GetConvTypeReverse(c) ((c == ConvName) ? ConvNameReverse : \
                               (c - ConvContent) + ConvContentReverse)

#define TEXT_LEN 256

typedef struct _Encoding Encoding;
struct _Encoding {
    short   encodingID;
    GIConv  icd;
    Score   score;
    ConvType convtype;
    gboolean autodetected;
    union {
        gchar converted_text[TEXT_LEN];
        gchar *contents;    /* don't use fixed array due to no way to trunc */
    } u;
};

typedef gboolean (*EncodeFunc) (Encoding *, gint, va_list);

GList   *fsexam_encoding_init (GSList *);

Score   fsexam_encoding_decode (GList *, ConvType, gchar *, size_t, gboolean);
void    fsexam_encoding_iterate_with_func (GList *, EncodeFunc, ...);

gint    fsexam_encoding_get_length (GList *encoding_list);

gboolean fsexam_encoding_get_elements (Encoding *, gint, va_list);
gboolean fsexam_encoding_translate_index (Encoding *, gint, va_list);
gint     fsexam_encoding_get_first_index (GList *);

void    fsexam_encoding_destroy (GList *);
void    fsexam_encoding_cleanup_content (GList *);
GList * fsexam_encoding_remove_auto (GList *);
GList * fsexam_encoding_add_auto (GList *, GList *);
void    print_encoding (gpointer en, gpointer data);

#endif
