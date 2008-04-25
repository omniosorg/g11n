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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <stdlib.h>
#include <strings.h>

#ifdef   HAVE_AUTO_EF_H
#include <auto_ef.h>
#endif

#include "encoding.h"
#include "auto-detect.h"

short encoding2id (const gchar *encoding_name);

#ifndef  HAVE_AUTO_EF_H
gboolean 
file_isutf8 (const gchar *filename, gint flags)
{
    gchar *contents = NULL;

    if (filename == NULL)
        return FALSE;

    if (g_file_get_contents (filename, &contents, NULL, NULL)) {
        gboolean ret = FALSE;

        if (g_utf8_validate (contents, -1, NULL))
            ret = TRUE;

        g_free (contents);

        return ret;
    }else{
        //fsexam_errno = ERR_CANNOT_READ;   delay error handling 
    }

    return FALSE;
}

GList *
file_encoding_detect (const gchar *filename, gint flags)
{
    return NULL;
}

GList *
str_encoding_detect (const gchar *string, gint flags)
{
    return NULL;
}

void
auto_encoding_free (GList *list)
{
    return;
}

#else

static const gchar *autoen_map[][2] = {
  /* auto detected          canonical */
    {"8859-1",              "ISO-8859-1"},
    {"8859-2",              "ISO-8859-2"},
    {"8859-5",              "ISO-8859-5"},
    {"8859-6",              "ISO-8859-6"},
    {"8859-7",              "ISO-8859-7"},
    {"8859-8",              "ISO-8859-8"},
    {"ascii",               "UTF-8"},
    {"iso-8859-1",          "ISO-8859-1"},
    {"iso-8859-2",          "ISO-8859-2"},
    {"iso-8859-5",          "ISO-8859-5"},
    {"iso-8859-6",          "ISO-8859-6"},
    {"iso-8859-7",          "ISO-8859-7"},
    {"iso-8859-8",          "ISO-8859-8"},
    {"ko_KR.cp949",         "cp949"}, 
    {"ko_KR.euc",           "EUC-KR"},
    {"koi8-r",              "8859-5"},
    {"zh_CN.euc",           "GB18030"},
    {"zh_CN.GB18030",       "GB18030"},
    {"zh_CN.iso2022-CN",    "GB2312"},
    {"zh_HK.hkscs",         "BIG5-HKSCS"},
    {"zh_TW-big5",          "BIG5"},
    {"zh_TW-euc",           "EUC-TW"},
};

/* convert encoding name detected to canonical encoding name */
static const gchar *
auto_encoding_to_canonical_name (const gchar *auto_encoding_name)
{
    short low = 0;  
    short high = sizeof (autoen_map) / sizeof (autoen_map[0]) - 1;
    short mid;

    if (auto_encoding_name == NULL)
        return NULL;

    while (low <= high) {
        gint result;

        mid = (low + high) / 2;
        result = g_ascii_strcasecmp (auto_encoding_name, autoen_map[mid][0]);

        if (result == 0)
            return autoen_map[mid][1];
        else if (result > 0)
            low = mid + 1;
        else
            high = mid - 1;
    }

    /* may be in canonical name already */
    if (encoding2id (auto_encoding_name) != -1)
        return auto_encoding_name;

    return NULL;
}

gboolean
file_isutf8 (const gchar *filename, gint flags)
{
    auto_ef_t   *array_info = NULL;
    size_t      number = 0;
    gchar       *encoding = NULL;
    double      score = 0;
    gint        ret = FALSE;

    if (filename == NULL)
        return ret;

    number = auto_ef_file (&array_info, filename, flags);
    if (number != 1){
        ret = FALSE;
        goto free;
    }

    score = auto_ef_get_score (array_info[0]);
    if (abs(score - 100.0) > INACCURACY) {
        ret = FALSE;
        goto free;
    }
    encoding = auto_ef_get_encoding (array_info[0]);
    if ((strcmp (encoding, "UTF-8") == 0) || (strcmp (encoding, "ASCII") == 0)){
        ret = TRUE;
    }

free:
    auto_ef_free (array_info);

    return ret;
}

GList *
file_encoding_detect (const gchar *filename, gint flags)
{
    EncodingPair    *pair = NULL;
    GList           *result = NULL;
    auto_ef_t       *array_info = NULL;
    size_t          number = 0;
    gint            i;

    if (filename == NULL)
        return NULL;

    number = auto_ef_file (&array_info, filename, flags);
    if (-1 == number)
        return NULL;

    for (i = number - 1; i >= 0; i--){
        const gchar *canonical_name = NULL;

        canonical_name =  auto_encoding_to_canonical_name (
                        auto_ef_get_encoding (array_info[i]));

        if (canonical_name == NULL) {
            g_print (_("Warning: can not convert encoding %s to canonical encoding name, will ignore it.\n"), 
                      auto_ef_get_encoding (array_info[i]));
            continue;
        }

        pair = g_new (EncodingPair, 1);

        pair->encoding_name = g_strdup (canonical_name);
        pair->score = auto_ef_get_score (array_info[i]);

        result = g_list_append (result, pair);
    }

free:
    auto_ef_free (array_info);
    return result;
}

GList *
str_encoding_detect (const gchar *string, gint flags)
{
    EncodingPair    *pair = NULL;
    GList           *result = NULL;
    auto_ef_t       *array_info = NULL;
    size_t          number = 0;
    gint            i = 0;

    if (string == NULL)
        return NULL;

    number = auto_ef_str (&array_info, string, strlen(string), flags);
    if (-1 == number)
        return NULL;

    for (i = number - 1; i >= 0; i--){
        const gchar *canonical_name = NULL;

        canonical_name =  auto_encoding_to_canonical_name (
                        auto_ef_get_encoding (array_info[i]));

        if (canonical_name == NULL) {
            g_print (_("Warning: can not convert encoding %s to canonical encoding name, will ignore it.\n"), 
                      auto_ef_get_encoding (array_info[i]));
            continue;
        }

        pair = g_new (EncodingPair, 1);

        pair->encoding_name = g_strdup (canonical_name);
        pair->score = auto_ef_get_score (array_info[i]);

        result = g_list_append (result, pair);
    }

free:
    auto_ef_free (array_info);

    return result;
}

void
auto_encoding_free (GList *list)
{
    EncodingPair    *pair = NULL;

    if (NULL == list)
        return;

    while (list){
        pair = list->data;
        if (pair != NULL)
            g_free (pair->encoding_name);

        list = list->next;
    }

    g_list_free (list);

    return;
}
#endif

gboolean
str_isutf8 (const gchar *string, gint flags)
{
    if (g_utf8_validate (string, -1, NULL))
        return TRUE;
    else
        return FALSE;
}
