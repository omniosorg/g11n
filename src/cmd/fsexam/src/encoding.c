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

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

#include <glib.h>

#include "encoding.h"
#include "fsexam-debug.h"
#include "fsexam-header.h"

gint indexg;

void 
print_encoding (gpointer en, gpointer data)
{
    Encoding *encoding = (Encoding *)en;

    /* Flawfinder: ignore */
    printf ("\tencoding name: %s\n", id2encoding (encoding->encodingID));

    return;
}

void 
print_encoding_list (GList *list)
{
    if (list == NULL) {
        printf ("encoding list is NULL\n");
        return;
    }

    while (list) {
        Encoding *encode = (Encoding *)list->data;

        /* Flawfinder: ignore */
        printf ("\tencoding name: %s\ttext=%s\n", 
                id2encoding (encode->encodingID), 
                encode->convtype == ConvName 
                ? (encode->u.converted_text ? encode->u.converted_text : "NULL")
                : (encode->u.contents ? encode->u.contents : "NULL"));

        list = g_list_next (list);
    }

    return;
}

static Encoding *
encoding_new (const gchar *encoding_name, gboolean auto_detected)
{
    Encoding    *encode = NULL;
    GIConv      icd;
    short       en_id;

    if (encoding_name == NULL)
        return NULL;
    
    en_id = encoding2id (encoding_name);

    if (en_id == -1) {
        g_print (_("%s is not supported encoding name, use "
                   "'fsexamc -l' to list supported encoding.\n"), 
                encoding_name);

        return NULL;
    } 
   
    /*
     * Workaround:
     * 		When use ISO-2022-CN, we must use zh_CN.iso2022-CN to
     * 		open GIConv.
     */
    if (en_id == encoding2id("ISO-2022-CN"))
        icd = g_iconv_open ("UTF-8", "zh_CN.iso2022-CN");
    else
        icd = g_iconv_open ("UTF-8", id2encoding (en_id));

    if (icd == (GIConv)-1) {
        g_print (_("Your system do not support conversion from '%s' to 'UTF-8',"
                   "will ignore it.\n"), 
                encoding_name);

        return NULL;
    }

    encode = g_new0 (Encoding, 1);
    encode->icd = icd;
    encode->encodingID = en_id;
    encode->score = FAIL;
    encode->convtype = ConvName;
    encode->autodetected = auto_detected;

    return encode;
}

GList *
fsexam_encoding_init (GSList *list)
{
    GList   *encode_list = NULL;

    if (list == NULL)
        return NULL;

    while (list) {
        Encoding    *encode = NULL;

        encode = encoding_new ((char *)list->data, FALSE);
        list = g_slist_next (list);

        if (encode == NULL)
            continue;

        encode_list = g_list_prepend (encode_list, encode);
    }

    encode_list = g_list_reverse (encode_list);

    return encode_list;
}

/* 
 * append auto detected encoding into existing encoding list
 */
GList *fsexam_encoding_add_auto (GList *encoding_list, 
                                 GList *detected_encoding)
{
    while (detected_encoding) {
        EncodingPair    *pair = (EncodingPair *)detected_encoding->data;
        GList           *tmp = NULL;

        for (tmp = encoding_list; tmp; tmp = g_list_next (tmp)) {
            Encoding    *encode = (Encoding *)tmp->data;
            const gchar *name = id2encoding (encode->encodingID);

            if (g_ascii_strcasecmp (pair->encoding_name, name) == 0) {
                break;
            }
        }

        if (tmp == NULL) {  /* new encoding found */
            Encoding    *en;

            en = encoding_new (pair->encoding_name, TRUE);
            if (en != NULL)
                encoding_list = g_list_append (encoding_list, en);
        }

        detected_encoding = g_list_next (detected_encoding);
    }

    return encoding_list;
}

void
fsexam_encoding_destroy (GList *encode_list)
{
    if (encode_list == NULL)
        return;

    while (encode_list) {
        Encoding *encode = (Encoding *)encode_list->data;

        if (encode->convtype == ConvContent && encode->u.contents != NULL)
            g_free (encode->u.contents);
      
        g_iconv_close (encode->icd);
        g_free (encode);

        encode_list = g_list_next (encode_list);
    }

    g_list_free (encode_list);
}

/*
 * free memory allocated for file content in Encoding
 */
void
fsexam_encoding_cleanup_content (GList *encode_list)
{
    while (encode_list) {
        Encoding *encode = (Encoding *)encode_list->data;

        if (encode->convtype == ConvContent) {
            g_free (encode->u.contents);
            encode->u.contents = NULL;
        }

        encode_list = g_list_next (encode_list);
    }

    return;
}

/*
 * free memory allocated for auto detected Encoding
 */ 
GList * 
fsexam_encoding_remove_auto (GList *encode_list)
{
    GList *tmp = encode_list;

    while (tmp != NULL) {   
        Encoding *encode = (Encoding *)tmp->data;
        
        if (encode->autodetected) {
            GList *next = NULL;

            if (encode->convtype == ConvContent && encode->u.contents)
                g_free (encode->u.contents);

            g_iconv_close (encode->icd);
            g_free (encode);
     
            /* delete current node from list */
            next = g_list_next (tmp);
            encode_list = g_list_delete_link (encode_list, tmp);
            tmp = next;
        } else {
            tmp = g_list_next (tmp);
        }
    }
   
   return encode_list;
}


/*
 * Use give encoding in list to convert given text and 
 * store result into list again
 */
Score
fsexam_encoding_decode (GList *encode_list,
                        ConvType convtype,
                        gchar *text,    /* filename or content sample */
                        size_t textlen,
                        gboolean force)
{
    GList    *list_for_dbg = encode_list;
    gboolean success = FAIL;

    if (fsexam_debug() & FSEXAM_DBG_ENCODING) {
        print_encoding_list (list_for_dbg);
    }

    if ((encode_list == NULL) || (text == NULL))
        return FAIL;

    if ((g_utf8_validate (text, textlen, NULL)) && !force) 
        return ORIGINAL;
    
    while (encode_list != NULL) {
        Encoding *encode = (Encoding *)encode_list->data;
        gchar    *inbuf = text;
        size_t   inbytes_left = textlen;
        gchar    *outbuf;
        gchar    *old_outbuf;
        size_t   outbytes_left;
        size_t   num_uconv = 0;

        encode->convtype = convtype;
        if (convtype == ConvName) {
            outbuf = encode->u.converted_text;
            outbytes_left = TEXT_LEN;
        } else {
            outbytes_left = 3 * inbytes_left;
            outbuf = encode->u.contents = g_new0 (gchar, outbytes_left);
        }

        memset (outbuf, 0, outbytes_left);
        old_outbuf = outbuf;

        num_uconv = g_iconv (encode->icd, &inbuf, &inbytes_left,
                            &outbuf, &outbytes_left);

        if ((E2BIG == errno) && ((size_t)-1 == num_uconv))
            num_uconv = 0;                  /* very few condition */

        /*
         * Sometimes, g_iconv will convert successfully, but the result
         * maybe not real UTF-8 sequence. Check this before continuing.
         * I do encounter such problem when handling ISO-2022-KR.
         */
        if (! g_utf8_validate (old_outbuf, -1, NULL))
            num_uconv = (size_t)-1;

        switch (num_uconv) {
        case 0:
            encode->score = HIGH;
            success = TRUE;
            break;
        case (size_t)-1:
            encode->score = FAIL;
            break;
        default:
            encode->score = LOW;
            success = TRUE;
            break;
        }

        encode_list = g_list_next (encode_list);
    }

    return success ? HIGH : FAIL;
}

/*
 * Get the number of encoding from which can convert to UTF-8
 */
gint
fsexam_encoding_get_length (GList *encoding_list)
{
    gint    length = 0;

    while (encoding_list) {
        Encoding *en = encoding_list->data;

        encoding_list = g_list_next (encoding_list);

        if (en == NULL || en->score == FAIL)
            continue;

        length++;
    }

    return length;
}

/*
gboolean
fsexam_encoding_get_elements (Encoding *encode,
             gint index,
             va_list args)
{
    gint *n_elements;

    n_elements = va_arg (args, gint *);
    (*n_elements)++;

    return TRUE;
}
*/

/* 
 * Iterate encode_list using given function 
 * Stop traverse if user function return FALSE
 */
void
fsexam_encoding_iterate_with_func (GList *encode_list,
                                   EncodeFunc func, ...)
{
    va_list args;
    gint    encode_idx = -1;

    if (encode_list == NULL)
        return;

    va_start (args, func);

    while (encode_list != NULL) {
        Encoding *encode = (Encoding *)encode_list->data;

        encode_list = g_list_next (encode_list);
        ++encode_idx;

        if (encode->score == FAIL) 
            continue;

        if (! (*func)(encode, encode_idx, args))
            break;
    }

    va_end (args);

    return;
}

/*
 * Tanslate the user provided index to index in encode_list.
 *
 * It is one kind of func of fsexam_encoding_iterate_with_func () 
 */
gboolean
fsexam_encoding_translate_index (Encoding *encode, 
                                 gint index, /* the index in GList */
                                 va_list args)
{
    gint *old_index = va_arg (args, gint *);  /* the user's selection */
    gint *new_index = va_arg (args, gint *);  /* store result */

    if (*old_index) {  
        --*old_index;
        return TRUE;
    } else {
        *new_index = index; 
        return FALSE;
    }
}

gint
fsexam_encoding_get_first_index (GList *encode_list)
{
    gint encode_idx = -1, best_idx = -1;

    while (encode_list != NULL) {
        Encoding *encode = (Encoding *)encode_list->data;

        encode_list = g_list_next (encode_list);
        ++encode_idx;

        if (encode->score == HIGH) {
            // found it, the first encoding with score as HIGH
            best_idx = encode_idx;
                break;
        } else if (encode->score == LOW) {
            // record the first encoding with score as LOW
            if (best_idx == -1) 
                best_idx = encode_idx;
        }
    }

    return best_idx;
}
