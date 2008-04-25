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

/*
 * auto-detect.h
 *
 * Auto detect the filename/filecontent/string's encoding
 */

#ifndef _AUTO_DETECT_H
#define _AUTO_DETECT_H

#define INACCURACY  0.001

#ifdef HAVE_AUTO_EF_H
#include <auto_ef.h>
#define DEFAULT_DETECTING_FLAG (AE_LEVEL_2)
#else
#define DEFAULT_DETECTING_FLAG 0
#endif

typedef struct _EncodingPair EncodingPair;

struct _EncodingPair{
    gchar   *encoding_name;
    double  score;
};

/*====================================================================
 *  Function Name:  file_isutf8
 *
 *  Parameters:
 *      const gchar *filename: the name of file
 *      gint   flags: used by underlying library to detect encoding. 
 *              eg: auto_ef use  detect level
 *
 *  Desc:
 *      Determine whether the file's content is UTF-8 or not.
 *
 *  Return value:
 *      True if file content is UTF-8, otherwise False.
 *
 *  Author:     Yandong Yao 2006/09/06
 ========================================================================*/ 
gboolean file_isutf8 (const gchar *filename, gint flags);

/*====================================================================
 *  Function Name:  file_encoding_detect
 *
 *  Parameters:
 *      const gchar *filename
 *      gint flags: used by underlying library to detect encoding. 
 *                  eg: auto_ef use detect level
 *
 *  Desc:
 *      Detect the file's content encoding, and return detected result.
 *
 *  Return value:
 *      Return one list of EncodingPair
 *      The encoding with higher score will appear ahead of encoding with lower score.
 *
 *      Need free the returned GList manually use auto_encoding_free when don't
 *      use it again.
 *
 *  Author:     Yandong Yao 2006/09/06
 ========================================================================*/ 
GList * file_encoding_detect (const gchar *filename, gint flags);

/*====================================================================
 *  Function Name:  str_isutf8
 *
 *  Parameters:
 *      const gchar *string: one null-terminated string.
 *      gint flag: used by underlying library to detect encoding. 
 *                  eg: auto_ef use detect level
 *
 *  Desc:   
 *      Determine whether one string is UTF-8 or not. Internally use 
 *      g_utf8_validate now.
 *
 *  Return value:
 *      True if is UTF-8, otherwise False.
 *
 *  Author:     Yandong Yao 2006/09/06
 ========================================================================*/ 
gboolean str_isutf8 (const gchar *string, gint flags);

/*====================================================================
 *  Function Name:  str_encoding_detect
 *
 *  Parameters:
 *      const gchar *string: one null-terminated string
 *      gint flag: used by underlying library to detect encoding. 
 *                  eg: auto_ef use detect level
 *
 *  Desc:
 *      Detect the possible encoding of one string and return one list which 
 *      contain the encoding name and its score. Higher score pair is ahead 
 *      of lower score pair
 *
 *      Need free the returned GList manually use auto_encoding_free when don't
 *      use it again.
 *
 *  Return value:
 *
 *  Author:     Yandong Yao 2006/09/06
 ========================================================================*/ 
GList * str_encoding_detect (const gchar *string, gint flags);

/*==================================================================
 *  Free encoding pair in list and list itself.
 ==================================================================*/
void auto_encoding_free (GList *list);

#endif  //_AUTO_DETECT_H
