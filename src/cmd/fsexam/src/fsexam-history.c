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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "fsexam-debug.h"
#include "encoding.h"
#include "fsexam-history.h"
#include "fsexam-header.h"

/*
 *  Print one Hist_item data structure 
 */
static void
print_hist_item (gpointer data, gpointer user_data)
{
    Hist_item *item = (Hist_item *)data;

    if (item == NULL)
        return;

    printf ("%d:\t%d -> %d: %s\n", item->serial, item->from_encoding, 
                           item->to_encoding, 
                           g_filename_from_uri (item->uri, NULL, NULL));

    return;
}

/*
 * Print the whole history info
 */
void
print_history (Hist_info *info)
{
    gint current;
    gint head;

    if (NULL == info)
        return;

    head = info->head;
    current = info->current;
    if (current == -1) {
        g_print ("History list is empty\n");
        return;
    }

    g_print ("History_Info: head = %d, current = %d, max_len = %d," 
            "last_serial = %d\n", 
            current, head, 
            info->max_len, info->last_serial);

    current = (current + 1) % info->max_len;
    head = (head + 1) % info->max_len;

    while (head != current) {
        GList *tmp = info->hist_array[head];
        g_print ("------------------------------------------\n");
        g_list_foreach (tmp, print_hist_item, NULL);

        head = (head + 1) % info->max_len;
    }

    return;
}

static void 
release_each_item (gpointer data_ptr, gpointer p)
{
    Hist_item *h = (Hist_item *) data_ptr;

    if (h == NULL)
        return;

    g_free (h->uri);
    g_free (h);

    return;
}

/*
 *  free the memory for Hist_item array list
 */
static void
release_hist_array (Hist_info *info)
{
    gint head, current;

    if (NULL == info)
        return;

    head = info->head;
    current = info->current;

    if (current == -1)
        return;

    while (current != head) {
        g_list_foreach (info->hist_array[current], release_each_item, NULL);
        g_list_free (info->hist_array[current]);
        
        current = (current + info->max_len - 1) % info->max_len;
    }

    /* restore to orignal state */
    info->head = 0;
    info->current = -1;
    info->last_serial = 0;

    return;
}

/* 
 * when current catch up head, need free some of space, currently hardcode
 * free 1/3 of space.
 */
static void
reclaim_hist_elements (Hist_info *info)
{
    gint newhead, current;

    if (NULL == info)
        return;

    newhead = (info->head + info->max_len / 3) % info->max_len;
    current = newhead;

    while (current != info->head) {
        g_list_foreach (info->hist_array[current], release_each_item, NULL);
        g_list_free (info->hist_array[current]);
        info->hist_array[current] = NULL;

        current = (current + info->max_len - 1) % info->max_len;
    }

    info->head = newhead;

    return;
}

/*
 *  Store one Hist_item into history stream
 */
static void 
serialize_history_item (gpointer data_ptr, gpointer fp)
{
    FILE        *stream = (FILE *) fp;
    Hist_item   *h = (Hist_item *) data_ptr;

    if (VALID_CONVTYPE (h->convtype)) {
        fprintf (stream, "%ud\27%d\27%d\27%d\27%s\n", 
                 h->serial, 
                 h->convtype, 
                 h->from_encoding, 
                 h->to_encoding, 
                 h->uri);
    }
 
    return;
}

/* Initialize in-memory structure from disk file */
static void
deserialize_history_data (Hist_info *hist_info)
{
    Hist_item   *item = NULL;
    gchar       linebuf[4096];
    gint32      prev_serial = -1;
    gint32      current = -1;
    gboolean    first = TRUE;
    
    if ((NULL == hist_info) || (NULL == hist_info->fp)) {
        g_print ("deserialize_history_data: file pointer is NULL\n");
        return;
    }

    memset (linebuf, 0, sizeof(linebuf));

    while ( fgets (linebuf, sizeof(linebuf), hist_info->fp) ) 
    {
        gchar    *serial = NULL;
        gchar    *conv = NULL;
        gchar    *from_encoding = NULL;
        gchar    *to_encoding = NULL;
        gchar    *uri = NULL;
        gchar    *last_char = NULL;

        if ( linebuf [sizeof(linebuf)-1] != '\0' &&
             linebuf [sizeof(linebuf)-1] != '\n')
        {
            memset (linebuf, 0, sizeof(linebuf));
            continue;       /* overflow */
        }

	if (linebuf[strlen (linebuf) - 1] == '\n')
        	linebuf [strlen(linebuf)-1] = '\0';

        serial = strtok_r (linebuf, "\27", &last_char);
        conv = strtok_r (NULL, "\27", &last_char);
        from_encoding = strtok_r (NULL, "\27", &last_char);
        to_encoding = strtok_r (NULL, "\27", &last_char);
        uri = last_char;

        if (!serial || !conv || !from_encoding || !to_encoding || !uri)
            continue;

        item = g_new0 (Hist_item, 1);

        item->serial = atoi (serial);
        item->convtype = atoi (conv);
        item->from_encoding = atoi (from_encoding);
        item->to_encoding = atoi (to_encoding);
        item->uri = g_strdup (uri);

        if ((first) || (item->serial != prev_serial)) {
            if (first) {
                first = FALSE;
                current = 1;
            }else{
                current = (current + 1) % hist_info->max_len;

                /* current catch up head, free space */
                if (current == hist_info->head) { 
                    reclaim_hist_elements (hist_info);
                }
            }

            prev_serial = item->serial;

            hist_info->hist_array[current] = g_list_append (
                                        hist_info->hist_array[current], 
                                        item);
        }else{  /* The same pass of action, won't increment current */
            hist_info->hist_array[current] = g_list_append (
                                        hist_info->hist_array[current], 
                                        item);
        }
    }

    if (current == -1) {        /* empty history */
        hist_info->head = 0;
        hist_info->current = -1;
        hist_info->last_serial = 0;
    }else{
        Hist_item *item = (Hist_item *)(hist_info->hist_array[current])->data;
        hist_info->current = current;
        hist_info->last_serial = item->serial;
    }

    return;
}

/* write in-memory hist info into disk file */
static gboolean
serialize_history_data (Hist_info *info)
{
    gint head;
    gint current;

    if (NULL == info)
        return FALSE;

    if (! info->changed)
        return TRUE;

    if (info->current == -1)
        return TRUE;

    /* skip the head, cause head always pointer to NULL */
    head = (info->head + 1) % info->max_len;
    current = (info->current + 1) % info->max_len;

    while (head != current) {
        g_list_foreach (info->hist_array[head], 
                        serialize_history_item, 
                        info->fp);

        head = (head + 1) % info->max_len;
    }

    return TRUE;
}

/*
 * Remove the last array element in history array
 */
void
fsexam_history_remove_last (Hist_info *info)
{
    gint      current;

    if (NULL == info)
        return;

    current = info->current;

    if (current == -1)
        return;

    g_list_foreach (info->hist_array[current], release_each_item, NULL);
    g_list_free (info->hist_array[current]);

    info->changed = TRUE;
    info->hist_array[current] = NULL;

    info->current = (current + info->max_len - 1) % info->max_len;

    if (info->current == info->head) { /* empty now */
        info->head = 0;
        info->current = -1;
        info->last_serial = 0;
    } else {
        Hist_item *item = (info->hist_array[info->current])->data;
        info->last_serial = item->serial;
    }

    return;
}

/*
 *  Get the last conversion list in history file
 *      For archive or compress file, we only get the compress or archive 
 *      file name
 */
GList *
fsexam_history_get_last_list (Hist_info *info, ConvType *conv_type)
{
    GList       *file_list = NULL; 
    GList       *hist_list = NULL;
    GHashTable  *special_file_hash = NULL;
    Hist_item   *item = NULL;

    if (info == NULL)
        return NULL;

    if ((hist_list = info->hist_array[info->current])== NULL)
        return NULL;

    if (conv_type != NULL) {
        item = hist_list->data;
        *conv_type = item ? item->convtype : ConvInvalid;
    }

    while (hist_list) {
        gchar *filename = NULL;

        item = hist_list->data;
        filename = g_filename_from_uri (item->uri, NULL, NULL);

        if ((item->convtype == ConvNameSpecial) 
                || (item->convtype == ConvContentSpecial)) {
            gint  len = 0;
            gchar *tmp = filename;

            if (special_file_hash == NULL) {
                special_file_hash = g_hash_table_new_full (
                        g_str_hash, 
                        g_str_equal,
                        (GDestroyNotify) g_free,
                        NULL);
            }
            
            while ((*filename != '\t') && (*filename != '\0')) {
                filename ++;
                len ++;
            }

            filename = g_strndup (tmp, len);
            g_free (tmp);

            if (! g_hash_table_lookup (special_file_hash, filename)) {
                file_list = g_list_prepend (file_list, g_strdup (filename));
                g_hash_table_insert (special_file_hash, filename, (gchar *)-1);
            }
        }else{
            file_list = g_list_prepend (file_list, filename);
        }


        hist_list = g_list_next (hist_list);
    }

    if (special_file_hash != NULL) {
        g_hash_table_destroy (special_file_hash);
    }

    return file_list;
}

/*
 * put a history item in the info->histlist.
 */
void
fsexam_history_put (Hist_info *info, 
                    ConvType convtype,
                    const gchar *filename, 
                    gint from_encoding,
                    gint to_encoding,
                    gboolean same_serial)
{
    if ((NULL == filename) || (NULL == info))
        return;

    Hist_item *h = g_new0 (Hist_item, 1);

    h->convtype = convtype;
    h->from_encoding = from_encoding;
    h->to_encoding = to_encoding;
    h->uri = g_filename_to_uri (filename, NULL, NULL);

    if (! same_serial) {
        info->last_serial++;
        if (info->current == -1)
            info->current = 1;
        else
            info->current = (info->current + 1) % info->max_len;

        if (info->current == info->head) {
            reclaim_hist_elements (info);
        }
        
    }

    h->serial = info->last_serial;
    info->hist_array[info->current] = g_list_append (
                                            info->hist_array[info->current], 
                                            h);

    info->changed = TRUE;
    
    return;
}

/*
 * If the file specified by filename exist and open properly, 
 * deserialize the data from file to list of History_items.
 */
Hist_info *
fsexam_history_open (const gchar *filename, gint max_len)
{
    struct stat stat_buf;
    Hist_info *info = NULL;

    if (NULL == filename)
        return NULL;
    
    info = g_new0 (Hist_info, 1);

    if (stat (filename, &stat_buf) == -1)
        info->fp = fopen (filename, "w+b");     /* create file if not exist */
    else
        info->fp = fopen (filename, "r+b");     /* open for update */

    /* allocate hist_array */
    info->max_len = max_len;
    info->hist_array = g_new0 (GList *, info->max_len);

    /* Initial state of Hist_info */
    info->head = 0;
    info->current = -1;
    info->last_serial = 0;
    info->changed = FALSE;

    deserialize_history_data (info);    /* init history array */

    if (fsexam_debug () & FSEXAM_DBG_HISTORY) {
        print_history (info);
    }
    
    return info;
}

/*
 * During search in the history list, we only differentiate 
 * ConvName and non-ConvName
 */
Hist_item *
fsexam_history_search (Hist_info *info, const gchar *path, gboolean convname)
{
    if ((NULL == info) || (NULL == path))
        return NULL;

    Hist_item   *item = NULL;
    gint        current = info->current;
    gint        head = info->head;
    gchar       *path_uri = NULL;

    if (current == -1)
        return NULL;

    path_uri = g_filename_to_uri (path, NULL, NULL);

    while (current != head) {
        GList *one_pass = info->hist_array[current];

        while (one_pass) {
            Hist_item   *tmp = (Hist_item *)one_pass->data;
            if ((tmp == NULL) || (tmp->uri == NULL))
                continue;
            
            if ((strcmp (tmp->uri , path_uri) == 0)
                    && ((convname && ((tmp->convtype == ConvName) 
                                    || (tmp->convtype == ConvNameSpecial))) 
                        || (!convname 
                                    && (tmp->convtype != ConvName) 
                                    && (tmp->convtype != ConvNameSpecial)))) {
                item = tmp;                             /* found match */
            }


            if (item != NULL)
                break;
            
            one_pass = g_list_next (one_pass);
        }

        if (item != NULL)
            break;

        current = (current - 1 + info->max_len) % info->max_len;
    }

    g_free (path_uri);

    return item;
}

/* 
 * Get the latest serial
 */
gint32
fsexam_history_get_serial (Hist_info *info)
{
    if (NULL == info)
        return -1;

    return info->last_serial;
}

/* 
 * Serialize list of History_items, and save to info->fp.
 * At last, close info->fp, and release info->histlist.
 */
gboolean
fsexam_history_close (Hist_info *info)
{
    if (info->fp) {
        fseek (info->fp, SEEK_SET, 0);
        serialize_history_data (info);      /* save data into disk */
        fclose (info->fp);
    }

    release_hist_array (info);

    g_free (info->hist_array);
    g_free (info);

    return TRUE;
}

