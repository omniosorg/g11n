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
#ifndef _FSEXAM_ENCODING_DIALOG_H
#define _FSEXAM_ENCODING_DIALOG_H

typedef enum
{
    FSEXAM_ENCODING_8859_1,	/* 0 */
    FSEXAM_ENCODING_8859_10,
    FSEXAM_ENCODING_8859_11,
    FSEXAM_ENCODING_8859_13,
    FSEXAM_ENCODING_8859_14,
    FSEXAM_ENCODING_8859_15,
    FSEXAM_ENCODING_8859_16,
    FSEXAM_ENCODING_8859_2,
    FSEXAM_ENCODING_8859_3,
    FSEXAM_ENCODING_8859_4,
    FSEXAM_ENCODING_8859_5,	/* 10 */
    FSEXAM_ENCODING_8859_6,
    FSEXAM_ENCODING_8859_7,
    FSEXAM_ENCODING_8859_8,
    FSEXAM_ENCODING_8859_9,
    FSEXAM_ENCODING_BIG5,
    FSEXAM_ENCODING_HKSCS,
    FSEXAM_ENCODING_CP1250, 
    FSEXAM_ENCODING_CP1251,
    FSEXAM_ENCODING_CP1252,
    FSEXAM_ENCODING_CP1253,	/* 20 */
    FSEXAM_ENCODING_CP1254,
    FSEXAM_ENCODING_CP1255,
    FSEXAM_ENCODING_CP1256,
    FSEXAM_ENCODING_CP1257,
    FSEXAM_ENCODING_CP1258,
    FSEXAM_ENCODING_CP437,
    FSEXAM_ENCODING_CP737,
    FSEXAM_ENCODING_CP775,
    FSEXAM_ENCODING_CP850,
    FSEXAM_ENCODING_CP852,	/* 30 */
    FSEXAM_ENCODING_CP855,
    FSEXAM_ENCODING_CP857,
    FSEXAM_ENCODING_CP860,
    FSEXAM_ENCODING_CP861,
    FSEXAM_ENCODING_CP862,
    FSEXAM_ENCODING_CP863,
    FSEXAM_ENCODING_CP864,
    FSEXAM_ENCODING_CP865,
    FSEXAM_ENCODING_CP866,
    FSEXAM_ENCODING_CP869,	/* 40 */
    FSEXAM_ENCODING_CP874,
    FSEXAM_ENCODING_CP935,
    FSEXAM_ENCODING_CP937,
    FSEXAM_ENCODING_CP949,
    FSEXAM_ENCODING_EUC_KR,
    FSEXAM_ENCODING_EUC_TH,
    FSEXAM_ENCODING_EUC_TW,
    FSEXAM_ENCODING_eucJP,
    FSEXAM_ENCODING_GB18030,
    FSEXAM_ENCODING_GB2312,	/* 50 */
    FSEXAM_ENCODING_GBK,
    FSEXAM_ENCODING_ISO_2022_CN,
    FSEXAM_ENCODING_ISO_2022_JP,
    FSEXAM_ENCODING_ISO_2022_KR,
    FSEXAM_ENCODING_JOHAB,
    FSEXAM_ENCODING_SJIS,
    FSEXAM_ENCODING_UTF_16,
    FSEXAM_ENCODING_UTF_16BE,
    FSEXAM_ENCODING_UTF_16LE,
    FSEXAM_ENCODING_UTF_8	/* 60 */
}FsexamEncodingIndex;

typedef struct {
    gint    index;              /* Numeric name of encoding */
    gchar   *charset;           /* External encoding name for user */
    gchar   *normalized_name;   /* normalized name for comparison */
    gchar   *name;          	/* Country or region for this charset */
    gchar   *locale;            /* used for file(1) */
}FsexamEncoding;

/* encoding & ID map */
gchar   *id2encoding (short id);
gchar   *encoding_to_locale (const gchar *encoding_name);
short   encoding2id (const gchar *encoding_name);
void    show_avail_encoding (void);

void    cb_create_encoding_dialog (GtkWidget *parent);

#endif
