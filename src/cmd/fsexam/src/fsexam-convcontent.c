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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>

#include <glib/gi18n.h>

#include "fsexam-header.h"
#include "encoding.h"
#include "fsexam-convcontent.h"

/* Store processed files, avoid loops */
static GHashTable   *name_hash = NULL;

static gboolean     convert_file (const gchar *name, 
                                  FSEXAM_setting *setting, 
                                  gboolean restore);
static gboolean     convert_single_file (const gchar *fullpath, 
                                  FSEXAM_setting *setting);
static void         puts_msg (FSEXAM_setting *setting, 
                                  const gchar *path, 
                                  const gchar *from_en, 
                                  const gchar *to_en);

/*
 * wrapper for fsexam_log_puts() and display_msg ()
 */
static void
puts_msg (FSEXAM_setting *setting, 
          const gchar *path, 
          const gchar *from_en, 
          const gchar *to_en)
{
    if ((setting == NULL) || (path == NULL))
        return;

    if (fsexam_errno == ERR_OK) {
        gchar *msg;

        if ((from_en == NULL) || (to_en == NULL))
            return;

        msg = g_strdup_printf (_("[Content] %s -> %s"), from_en, to_en);

        fsexam_log_puts (setting->log_info, path, msg);
        if (setting->display_msg)
            setting->display_msg (path, msg);

        g_free (msg);
    }else{
        fsexam_log_puts (setting->log_info, path, NULL);
        if (setting->display_msg)
            setting->display_msg (path, fsexam_error_get_msg ());
    }

    return;
}

/*
 * Modify file contents, and write history info if needed
 */
static gboolean
write_back_contents (FSEXAM_setting *setting, 
                     const gchar *fullpath, 
                     gchar *converted_contents, 
                     short fid, 
                     short tid)
{
    gboolean    ret = FALSE;
    gboolean    need_free = FALSE;
    gsize       length;

    if ((NULL == setting) || (NULL == fullpath))
        return FALSE;

    if (converted_contents == NULL) {
        const gchar  *from_encoding = id2encoding (fid);
        const gchar  *to_encoding = id2encoding (tid);
        gchar        *contents = NULL;
        gboolean     err;

        if ((NULL == from_encoding) || (NULL == to_encoding))
            return FALSE;

        err = g_file_get_contents (fullpath, &contents, &length, NULL);
        if (err && !contents) {
            fsexam_errno = ERR_CANNOT_READ;
            goto done;
        } else if (length == 0) {
            fsexam_errno = ERR_EMPTY_FILE;
            goto done;
        }
    
        converted_contents = g_convert (contents,
                                        length,
                                        to_encoding,
                                        from_encoding,
                                        NULL,
                                        NULL,
                                        NULL);

        g_free (contents);

        if (converted_contents == NULL) {
            fsexam_errno = ERR_CANNOT_CONVERT;
            goto done;
        }

        need_free = TRUE;
    }
        
    length = strlen (converted_contents);

    if (g_file_set_contents (fullpath, converted_contents, length, NULL)) {
        ret = TRUE;
        ++setting->succ_num;

        /* UNDO don't need write history */
        if (! (setting->flags & FSEXAM_SETTING_FLAGS_UNDO)) {
            gboolean same_serial = TRUE;
            if (setting->flags & FSEXAM_SETTING_FLAGS_DIFF_SERIAL) {
                setting->flags &= ~FSEXAM_SETTING_FLAGS_DIFF_SERIAL;
                same_serial = FALSE;
            }

            fsexam_history_put (setting->hist_info, 
                            ConvContent, 
                            fullpath,
                            fid,
                            tid,
                            same_serial);
        }
    }else{
        ++setting->fail_num;
        fsexam_errno = ERR_CANNOT_WRITE;
    }

done:
    if (need_free)
        g_free (converted_contents);

    return ret;
}

/*
 * Restore single one file. Won't care about follow or recursive 
 */
static gboolean
restore_single_file (const gchar *fullpath, FSEXAM_setting *setting)
{
    gboolean    ret = FALSE;
    Hist_item   *item = NULL;

    if ((NULL == fullpath) || (NULL == setting))
        return FALSE;

    fsexam_errno = ERR_OK;

    if (! setting->pref->dry_run  /* special type don't support dryrun */
            && setting->pref->special 
            && fsexam_special_convert_content (fullpath, setting, TRUE)) {
        ++setting->succ_num;
        return TRUE;    
    }

    item = fsexam_history_search (setting->hist_info, fullpath, FALSE);
    if (item == NULL) {
        fsexam_errno = ERR_HIST_NO_ITEM;
        ++setting->ignore_num;
        goto done;
    }

    ret = write_back_contents (setting, 
                               fullpath, 
                               NULL, 
                               item->to_encoding, 
                               item->from_encoding);
    
done:
    puts_msg (setting, fullpath, 
              item ? id2encoding (item->to_encoding) : NULL, 
              item ? id2encoding (item->from_encoding) : NULL);

    return ret;
}

/*
 * Convert single one file, display candidate or dryrun.
 * Won't care follow or recursive
 */
static gboolean
convert_single_file (const gchar *fullpath, FSEXAM_setting *setting)
{
    Score       score;
    Encoding    *encoding = NULL;
    gboolean    ret = FALSE;
    gboolean    err;
    gsize       length;
    gchar       *contents = NULL;
    gchar       *sample_contents = NULL;
    gboolean    need_free_contents = FALSE;

    if ((NULL == fullpath) || (NULL == setting))
        return FALSE;

    fsexam_errno = ERR_OK;

    if (! file_validate_for_contentconv (fullpath, setting)) {  
        goto done;      /* fsexam_errno has been set, goto log */
    }

    if (! setting->pref->dry_run
            && setting->pref->special 
            && fsexam_special_convert_content (fullpath, setting, FALSE)) { 
        ++setting->succ_num;
        return TRUE;    
    }

    /* 
     * handle encoding auto detection.
     * And fsexam_is_plain_text/file(1) need these encodings 
     */
    if (setting->pref->auto_detect) { 
        GList *detected_encoding;
        
        detected_encoding = file_encoding_detect (fullpath, 
                                                  DEFAULT_DETECTING_FLAG);
        setting->pref->encode_list = fsexam_encoding_add_auto (
                                                setting->pref->encode_list, 
                                                detected_encoding);
        auto_encoding_free (detected_encoding);
    }

    if (! fsexam_is_plain_text (fullpath, setting)) {
        /* This func need auto detected encoding */
        fsexam_errno = ERR_FILE_TYPE_NOT_SUPPORT;
        goto done;
    }

    /* plain text now */
    err = g_file_get_contents (fullpath, &contents, &length, NULL);
    if (err && !contents) {
        fsexam_errno = ERR_CANNOT_READ;
        goto done;
    } else if (length == 0) {
        fsexam_errno = ERR_EMPTY_FILE;
        goto done;
    }

    sample_contents = get_sample_text (contents, &length);
    score = fsexam_encoding_decode (setting->pref->encode_list,
                                    ConvContent,
                                    sample_contents,
                                    length,
                                    setting->pref->force);
    g_free (contents);
    g_free (sample_contents);

    need_free_contents = TRUE;      /* need free contents in encoding_list */

    if (setting->pref->dry_run){    /* dry run */
        ret = fsexam_dryrun_puts (setting->dryrun_info, 
                                  fullpath, 
                                  score, 
                                  setting->pref->encode_list, 
                                  ConvContent); 
        ret ? ++setting->succ_num : ++setting->fail_num;
    } else {                        /* real convert */  
        gint  index = 0;

        if ((score == FAIL) || (score == ORIGINAL)){
            fsexam_errno = (score == FAIL) ? ERR_NO_PROPER_ENCODING 
                                           : ERR_CONTENT_UTF8_ALREADY;
            goto done;
        }   
       
        if (setting->gold_index != -1) {
            index = setting->gold_index;
        } else if (setting->pref->auto_conversion) {
            index = fsexam_encoding_get_first_index (
                                            setting->pref->encode_list);
        } else {
            index = setting->get_index (setting->pref->encode_list, FALSE);
        }

        if (-1 == index)
            goto done;              /* cancel the selection */

        encoding = (Encoding *)g_list_nth_data (setting->pref->encode_list, 
                                                index);
        if (NULL == encoding){
            fsexam_errno = ERR_ENCODING_INDEX_INVALID;
            goto done;
        }

        ret = write_back_contents (setting, 
                                   fullpath, 
                                   NULL,
                                   encoding->encodingID, 
                                   encoding2id ("UTF-8"));
    }
 
    goto cleanup;

done:
    ++setting->ignore_num;

cleanup:
    if (!setting->pref->dry_run) {
        puts_msg (setting, fullpath, 
                  encoding ? id2encoding (encoding->encodingID) : NULL, 
                  "UTF-8");
    }

    if (setting->pref->auto_detect)
        setting->pref->encode_list = fsexam_encoding_remove_auto (
                                               setting->pref->encode_list);

    if (need_free_contents)
        fsexam_encoding_cleanup_content (setting->pref->encode_list);

    return ret;
}

/* 
 * Convert given file, handling various flags also
 */
static gboolean
convert_file (const gchar *name, FSEXAM_setting *setting, gboolean restore)
{
    gchar    *abs_path = get_abs_path (name);
    gboolean ret = FALSE;

    if ((abs_path == NULL) || (setting == NULL)) {
        goto done;
    }

    /* Global hash pointer */
    if (g_hash_table_lookup (name_hash, abs_path)) {
        ret = TRUE;
        goto done;
    }

    g_hash_table_insert (name_hash, g_strdup (abs_path), (gpointer)-1);
    ++setting->total_num;

    if (restore) {
        restore_single_file (abs_path, setting);
    }else{
        convert_single_file (abs_path, setting);
    }
    
    if (setting->pref->follow 
            && g_file_test (abs_path, G_FILE_TEST_IS_SYMLINK)) {
        /* Symlink file */
        gchar    *target = get_abs_path_for_symlink_target (abs_path);
        ++setting->ignore_num;      /* ignore symlink file itself */

        if (target == NULL)
            goto done;

        convert_file (target, setting, restore);

        g_free (target);
    }else if (setting->pref->recursive 
            && g_file_test (abs_path, G_FILE_TEST_IS_DIR)) {
        const gchar *filename = NULL;
        GDir        *dp = g_dir_open (abs_path, 0, NULL);
        ++setting->ignore_num;      /* ignore directory file itself */

        if (dp == NULL)
            goto done;

        while ((filename = g_dir_read_name (dp)) != NULL) {
            gchar *childname = g_strdup_printf ("%s/%s", abs_path, filename);
            convert_file (childname, setting, restore);
            g_free (childname);

            if (setting->flags & FSEXAM_SETTING_FLAGS_STOP)
                break;
        }

        g_dir_close (dp);
    }
    
    ret = TRUE;

done:
    g_free (abs_path);

    return ret;
}

static void
dryrun_convert_contents (GSList *slist, FSEXAM_setting *setting)
{
    if ((NULL == slist) || (NULL == setting))
        return;

    while (slist != NULL) {
        Dryrun_item *item = slist->data;
        
        ++setting->passin_num;
        ++setting->total_num;
       
        write_back_contents (setting, item->path, NULL, 
                             encoding2id (item->encoding), 
                             encoding2id ("UTF-8"));

        puts_msg (setting, item->path,  item->encoding, "UTF-8");

        slist = slist->next;
    }

    return;
}

static gboolean
_convert_or_restore_contents (FSEXAM_setting *setting, 
                              GList *list, 
                              gboolean restore)
{
    if ((list == NULL) || (setting == NULL))
        return FALSE;

    name_hash = g_hash_table_new_full (
                    g_str_hash,
                    g_str_equal,
                    (GDestroyNotify) g_free,
                    (GDestroyNotify) NULL);

    if (name_hash == NULL) {
        g_print (_("Can't new hash table.\n"));
        return FALSE;
    }

    fsexam_setting_reset_stats (setting);

    setting->passin_num = g_list_length (list);     /* passin No. of files */
    setting->flags |= FSEXAM_SETTING_FLAGS_DIFF_SERIAL;

    while (list) {
        convert_file ((char *)list->data, setting, restore);

        if (setting->flags & FSEXAM_SETTING_FLAGS_STOP)
            break;

        list = g_list_next (list);
    }

    setting->flags = 0;     /* clear all flags */
    setting->display_stats (setting);
    fsexam_log_flush (setting->log_info);

    g_hash_table_destroy (name_hash);
    name_hash = NULL;

    return TRUE;

}

/*----------- Public API ---------------------*/

/*
 * Convert single file
 */
gboolean
fsexam_convert_content (FSEXAM_setting *setting, const gchar *filename)
{   
    GList    *list = NULL;
    gboolean ret;

    if ((filename == NULL) || (setting == NULL))
        return FALSE;

    list = g_list_prepend (list, g_strdup (filename));
    ret = fsexam_convert_content_batch (setting, list);
    fsexam_list_free (list);

    return ret;
}

/*
 * Convert multiple files
 */
gboolean
fsexam_convert_content_batch (FSEXAM_setting *setting, GList *list)
{
    return _convert_or_restore_contents (setting, list, FALSE);    
}

/*
 * Restore file content
 */
gboolean
fsexam_restore_content (FSEXAM_setting *setting, GList *list)
{
    return _convert_or_restore_contents (setting, list, TRUE);
}

/*
 * Scenario based conversion: convert file content according to
 * previous dryrun result.
 */
gboolean
fsexam_convert_scenario_for_content (FSEXAM_setting *setting)
{
    g_return_val_if_fail (setting != NULL, FALSE);
    GSList   *slist = NULL;
    gboolean ret;
   
    fsexam_setting_reset_stats (setting);
    setting->flags |= FSEXAM_SETTING_FLAGS_DIFF_SERIAL;
    
    /* construct file list from dryrun result */
    ret = fsexam_dryrun_process (setting->dryrun_info, &slist);

    if ((! ret) || (NULL == slist)) {
        fsexam_log_puts (setting->log_info, NULL, NULL);
        if (setting->display_msg)
            setting->display_msg (NULL, fsexam_error_get_msg ());
        setting->display_stats (setting);       /* display stats infor */
        fsexam_log_flush (setting->log_info);

        return FALSE;
    }

    dryrun_convert_contents (slist, setting);   /* real convert */

    fsexam_dryrun_item_slist_free (slist);  /* free file list just create */
    setting->flags = 0;                     /* clear all flags */
    setting->display_stats (setting);       /* display stats infor */
    fsexam_log_flush (setting->log_info);

    return TRUE;
}
