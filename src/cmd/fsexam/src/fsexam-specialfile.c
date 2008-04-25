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

#include <locale.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "fsexam-header.h"
#include "fsexam-specialfile.h"

/* -------------  Compress file type implementation ------------ */
typedef enum {
    COMP_TARGZ = 0,
    COMP_TARBZ2,
    COMP_TAR,
    COMP_ZIP,
    COMP_Z,
}Compress_Type; 

typedef struct {
    gchar *result_path;      /* used to generate hist path */
    gchar *search_path;      /* used to generate hist search path */
    gchar *log_path;         /* used to generate log_path */
    gint  temp_dir_len;
}Compress_info;

static gchar    *iso8859_locale = NULL;
const gchar *compress_pair[][3] = {
    /* Type         UNCOMPRESS                 COMPRESS */
    {".tar.gz",  "LC_ALL=%s gzcat %s | tar -xf - 2>/dev/null",    "LC_ALL=%s tar -cf - . | gzip > ../%s"},
    {".tar.bz2", "LC_ALL=%s bzcat %s | tar -xf - 2>/dev/null",    "LC_ALL=%s tar -cf - . | bzip2 -z > ../%s"},
    {".tar",     "LC_ALL=%s tar -xf %s 2>/dev/null",              "LC_ALL=%s tar -cf - . > ../%s"},
    {".zip",     "unzip -q %s 2>/dev/null",             "zip -qr %s .; mv %s .."},
    {".tar.Z",   "uncompress -c %s | tar -xf - 2>/dev/null", "tar -cf - . | compress -fc > ../%s"},
};

#define TARCMD  "/usr/sfw/bin/gtar"

static gboolean fsexam_special_is_compress (const gchar *, Compress_Type *);
static gboolean compress_common_convert (const gchar *,
        Compress_info *,
        FSEXAM_setting *,
        gboolean restore,
        gboolean nameconvert);
static gboolean compress_convert_content (const gchar *,
        Compress_info *,
        FSEXAM_setting *,
        gboolean restore);

static Compress_info *
compress_info_new (const gchar *result_path,
        const gchar *search_path,
        const gchar *log_path)
{
    Compress_info *info = NULL;

    if (NULL == result_path)
        return NULL;

    info = g_new0 (Compress_info, 1);
    info->result_path = g_strdup (result_path);
    info->search_path = g_strdup (search_path ? search_path : result_path);
    info->log_path = g_strdup (log_path ? log_path : result_path);

    return info;
}
            
static void
compress_info_free (Compress_info *info)
{
    if (info == NULL) {
        return;
    }

    g_free (info->result_path);
    g_free (info->search_path);
    g_free (info->log_path);
    g_free (info);

    return;
}

/*
 *  ensure $PWD is the directory of temparay directory
 */
static gchar *
compose_compress_cmd (Compress_Type type, const gchar *bname)
{
    if (type == COMP_ZIP) {
        return g_strdup_printf (compress_pair[type][2], bname, bname);
    }
   
    if (type <= COMP_TAR) {
        return g_strdup_printf (compress_pair[type][2], iso8859_locale, bname);
    }

    return g_strdup_printf (compress_pair[type][2], bname);
}

/* 
 * Generate history path or log path for compress file.
 *      Please note that we may nested to handling compress file. so 
 *      orig_compress_file 
 *      may be in the form "/tmp/test/test.tar[src/test.tar][cmd/data.tar]
 *                          -------------------------------  ------------
 *                          orig_compress_file               fullpath - temp..
 *
 *      fullpath: the full path of compress file in question such as /tmp/a.tar
 *
 * The sequence of converting compress filename and content:
 *      regurlar: first file name , then file content. otherwise the log/hist
 *                info is not consistent with disk data.
 *      resotre ; first filecontent, then file name itself, otherwise 
 *                        can't search the history  correctly
 */
static gchar *
compose_path (const gchar *fullpath, 
        Compress_info *info, 
        gboolean forhist,
        gboolean restore)
{
    const gchar *relative_path;
    gchar       *result = NULL;
#define URI_HEAD_LEN    7

    if ((NULL == fullpath) || (NULL == info))
        return NULL;
    
    relative_path = fullpath + info->temp_dir_len + 1;

    if (forhist) { 
        /* 
         *  Convert hist_path to uri, remove 'file://' then concat with
         *  relative_path
         */
        gchar *uri;
        
        uri = g_filename_to_uri (
                restore ? info->search_path : info->result_path, 
                NULL, 
                NULL);
        result = g_strdup_printf ("%s\t%s", 
                uri + URI_HEAD_LEN, 
                relative_path);

        g_free (uri);
    }else {
        gchar *esc_path = NULL;

        if (! g_utf8_validate (relative_path, -1, NULL)) {
            esc_path = g_strescape (relative_path, NULL);
        }

        if (g_utf8_validate (info->log_path, -1, NULL)) {
            result = g_strdup_printf ("%s [%s]", 
                    info->log_path, 
                    esc_path ? esc_path : relative_path);
        }else{
            gchar *uri = g_filename_to_uri (info->log_path, NULL, NULL);
            result = g_strdup_printf ("%s [%s]", 
                    uri, 
                    esc_path ? esc_path : relative_path);
            g_free (uri);
        }
    }

    return result;
}

/*
 *  caller pass in orig_compress_file;
 */
static gboolean
compress_write_to_disk (FSEXAM_setting *setting, 
            const gchar *path,      /* the path of current file */
            const gchar *origname,  /* old base name            */
            const gchar *utf8name,  /* new base name            */
            Compress_info *info,
            short from_encoding,    /* from encoding            */
            short to_encoding,      /* to encoding              */
            gchar **actualname)      /* return the actual used   */
{
    static gchar    oldname[PATH_MAX];
    static gchar    newname[PATH_MAX];
    gchar           *retname = NULL;
    gchar           *fname = NULL;
    gchar           *log_path = NULL;
    gboolean        ret = FALSE;

    fsexam_errno = ERR_OK;

    /* construct full old name and full new name */
    g_snprintf (oldname, PATH_MAX - 1, "%s/%s", path, origname);
    g_snprintf (newname, PATH_MAX - 1, "%s/%s", path, utf8name);

    if (g_file_test (newname, G_FILE_TEST_EXISTS)) {
        fsexam_errno = ERR_NAME_EXIST;
        fname = find_non_exist_name (newname);

        if (fname == NULL) {
            fsexam_errno = ERR_CANNOT_RENAME;
        }else if (rename(oldname, fname) == 0){
            retname = g_strdup (basename (fname));
            ret = TRUE;
        }else{
            fsexam_errno = ERR_CANNOT_RENAME;
        }
    }else{
        if (rename(oldname, newname) == 0){
            retname = g_strdup (utf8name);
            ret = TRUE;
        }else{
            fsexam_errno = ERR_CANNOT_RENAME;
        }
    }

    /* history file; GUI vs. CLI */
    if ((ret)  && (! (setting->flags & FSEXAM_SETTING_FLAGS_UNDO))) {
        gboolean same_serial = TRUE;
        if (setting->flags & FSEXAM_SETTING_FLAGS_DIFF_SERIAL) {
            setting->flags &= ~FSEXAM_SETTING_FLAGS_DIFF_SERIAL;
            same_serial = FALSE;
        }

        gchar *hist_path =  compose_path (newname, info, TRUE, FALSE);

        fsexam_history_put (setting->hist_info, 
                            ConvNameSpecial, 
                            hist_path,
                            from_encoding, 
                            to_encoding, 
                            same_serial);

        g_free (hist_path);
    } 
  
    ret ? ++setting->succ_num : ++setting->fail_num;

    log_path = compose_path (ret ? newname : oldname, info, FALSE, FALSE);

    /* log file */
    if (fsexam_errno == ERR_OK) {
        gchar   *msg = g_strdup_printf ("%s -> %s", 
                        id2encoding (from_encoding), 
                        id2encoding (to_encoding));
        fsexam_log_puts (setting->log_info, log_path, msg);
        if (setting->display_msg) {
            setting->display_msg (log_path, msg);
        }
        g_free (msg);
    }else if (fsexam_errno == ERR_NAME_EXIST) {
        fsexam_log_puts (setting->log_info, log_path, NULL);
        if (setting->display_msg) {
            setting->display_msg (log_path, fsexam_error_get_msg ());
        }
    }else{
        fsexam_log_puts (setting->log_info, log_path, NULL);
        if (setting->display_msg) {
            setting->display_msg (log_path, fsexam_error_get_msg ());
        }

    }

    if (actualname != NULL)
        *actualname = retname;  //freed outside
    else
        g_free (retname);

    g_free (log_path);
    g_free (fname);

    return ret;
}

/*
 * Restore single file.
 *
 * Note that search path is in info directly, don't use compose any more.
 * This is diff with non-restore operation.
 */
static gboolean
compress_restore_single_filename (FSEXAM_setting *setting,
        const gchar *fullpath,
        const gchar *dname,
        const gchar *bname,
        Compress_info *info,
        gchar **newname)
{
    gboolean ret = FALSE;
    Hist_item *item = NULL;
    gchar     *converted_text = NULL;
    gchar     *log_path;

    fsexam_errno = ERR_OK;

    item = fsexam_history_search (setting->hist_info, info->search_path, TRUE);

    if ((item == NULL) || (item->convtype != ConvNameSpecial)) {
        fsexam_errno = ERR_HIST_NO_ITEM;
        goto done;
    }

    converted_text = g_convert (bname,
            strlen (bname),
            id2encoding (item->from_encoding),
            id2encoding (item->to_encoding),
            NULL,
            NULL,
            NULL);
    
    if (converted_text == NULL) {
        fsexam_errno = ERR_CANNOT_CONVERT;
        goto done;
    }

    ret = compress_write_to_disk (setting,
            dname,
            bname,
            converted_text,
            info,
            item->to_encoding,
            item->from_encoding,
            newname);

    g_free (converted_text);

    return ret;

done:
    log_path = compose_path (fullpath, info, FALSE, FALSE);
    fsexam_log_puts (setting->log_info, log_path, NULL);
    if (setting->display_msg)
        setting->display_msg (log_path, fsexam_error_get_msg());

    g_free (log_path);


    return ret;
}

/*
 *
 */
static gboolean
compress_convert_single_filename (FSEXAM_setting *setting,
        const gchar *fullpath,
        const gchar *dname, /* info->log path, info->dname */
        const gchar *bname,
        Compress_info *info,
        gchar **newname)
{
    Score       score;
    Encoding    *encoding = NULL;
    gboolean    ret = FALSE;

    fsexam_errno = ERR_OK;  /* initialize fsexam_errno */

    if (setting->pref->auto_detect) { /* handle encoding auto detection */
        GList *detected_encoding = str_encoding_detect (bname, 
                DEFAULT_DETECTING_FLAG);
        setting->pref->encode_list = fsexam_encoding_add_auto (
                setting->pref->encode_list, 
                detected_encoding);
        auto_encoding_free (detected_encoding);
    }

    score = fsexam_encoding_decode (setting->pref->encode_list, 
                             ConvName, 
                             (char *)bname, 
                             strlen(bname), 
                             setting->pref->force);


    if (setting->pref->dry_run){    /* dry run */
        //don't support dryrun
    } else {                    /* real convert */  
        gint     index = 0;
        gchar    *actualname = NULL;

        if ((score == FAIL) || (score == ORIGINAL)){
            fsexam_errno = (score == FAIL) ? ERR_NO_PROPER_ENCODING 
                                           : ERR_NAME_UTF8_ALREADY;
            
            goto done;
        }   
       
        if (setting->gold_index != -1) {
            index = setting->gold_index;
	    } else if (setting->pref->auto_conversion) {
            index = fsexam_encoding_get_first_index (
                    setting->pref->encode_list);
        } else {
            index = setting->get_index (setting->pref->encode_list, TRUE);
        }

        if (-1 == index) {
            goto cleanup;
        }

        encoding = (Encoding *)g_list_nth_data (setting->pref->encode_list, 
                index);
        if (NULL == encoding){
            fsexam_errno = ERR_ENCODING_INDEX_INVALID;
            goto done;
        }

        ret = compress_write_to_disk (setting, 
                dname, 
                bname, 
                encoding->u.converted_text, 
                info,
                encoding->encodingID, 
                encoding2id ("UTF-8"),
                &actualname);

        if ((ret) && (newname)) {   // Return new name
            *newname = actualname;
        }else{
            g_free (actualname);
        }

        return ret;
    }
    
done:
    if (fsexam_errno != ERR_OK && !setting->pref->dry_run) {
        gchar *log_path;

        log_path = compose_path (fullpath, info, FALSE, FALSE);
        fsexam_log_puts (setting->log_info, log_path, NULL);
        if (setting->display_msg)
            setting->display_msg (log_path, fsexam_error_get_msg());

        g_free (log_path);
    }

cleanup:

    if (setting->pref->auto_detect)
        setting->pref->encode_list = fsexam_encoding_remove_auto (
                setting->pref->encode_list);

    return ret;
}

/*
 * Convert the name of files in archive/compress type file
 * And will handle flags
 */
static gboolean
compress_convert_filename (const gchar *fullpath, 
        Compress_info *info,
        FSEXAM_setting *setting, 
        gboolean restore)
{
    gchar           *bname = NULL;
    gchar           *dname = NULL;
    gchar           *newname = NULL;
    gchar           *abs_path = NULL;
    struct stat     statbuf;
    gboolean        ret = FALSE;
    gboolean        need_free_newname = FALSE;

    if ((NULL == fullpath) || (NULL == info) || (NULL == setting))
        return FALSE;

    fsexam_errno = ERR_OK;
    bname = g_path_get_basename (fullpath);
    dname = g_path_get_dirname (fullpath);

    if (lstat (fullpath, &statbuf) == -1) 
        fsexam_errno = ERR_FILE_NONEXIST;
    else if (!(S_ISREG(statbuf.st_mode)) 
            && !(S_ISDIR(statbuf.st_mode)) && !(S_ISLNK(statbuf.st_mode))){
        fsexam_errno = ERR_FILE_TYPE_NOT_SUPPORT; 
    }else if ((! setting->pref->hidden) && (*bname == '.')){
        fsexam_errno = ERR_IGNORE_HIDDEN_FILE;
    }else if ((!setting->pref->remote) 
            && (is_remote_file (setting->remote_path, fullpath))) {
        fsexam_errno = ERR_IGNORE_REMOTE_FILE;
    }

    if (fsexam_errno != ERR_OK) {
        gchar *log_path = compose_path (fullpath, info, FALSE, FALSE);

        fsexam_log_puts (setting->log_info, log_path, NULL);
        if (setting->display_msg)
            setting->display_msg (log_path, fsexam_error_get_msg());
        g_free (log_path);

        goto done;
    }
   
    if (restore && (strlen (fullpath) != info->temp_dir_len)) {
       ret = compress_restore_single_filename (setting,
                fullpath, 
                dname,
                bname,
                info, 
                &newname);
    }else if (strlen (fullpath) != info->temp_dir_len) {
        if ((!setting->pref->force) 
                && (str_isutf8 (bname, DEFAULT_DETECTING_FLAG))) {
            gchar *log_path = compose_path (fullpath, info, FALSE, FALSE);
            fsexam_errno = ERR_NAME_UTF8_ALREADY;

            fsexam_log_puts (setting->log_info, log_path, NULL);
            if (setting->display_msg)
                setting->display_msg (log_path, fsexam_error_get_msg());
            g_free (log_path);
        }else{
            ret = compress_convert_single_filename (setting,
                fullpath,
                dname,
                bname,
                info,
                &newname);
        }
    }

    if (ret && newname != NULL) {
        abs_path = g_strdup_printf ("%s/%s", dname, newname);
        need_free_newname = TRUE;
    }else{
        abs_path = (gchar *)fullpath;
    }

    if (setting->flags & FSEXAM_SETTING_FLAGS_STOP)
        goto done;

    if (fsexam_special_is_compress (abs_path, NULL)){ 
        /* nested compress file */
        gchar   *log_path = compose_path (abs_path, info, FALSE, FALSE);
        gchar   *result_path = compose_path (abs_path, info, TRUE, FALSE);
        gchar   *search_path; 
       
        if (restore) {
            search_path = g_filename_to_uri (info->search_path, NULL, NULL);

        }else{
            search_path = compose_path (abs_path, info, TRUE, TRUE);
        }
            
        Compress_info *new_info = compress_info_new (result_path, 
                search_path + URI_HEAD_LEN, 
                log_path);

        g_free (log_path); 
        g_free (result_path);
        g_free (search_path);

        if (NULL == new_info) {
            goto done;
        }

        compress_common_convert (abs_path, 
                new_info,
                setting, 
                restore, 
                TRUE);

        compress_info_free (new_info);
    } else if (setting->pref->recursive 
            && S_ISDIR (statbuf.st_mode)) { /* Directory */
        const gchar *filename = NULL;
        gchar       *old_search_path = g_strdup (info->search_path);
        GDir        *dp = g_dir_open (abs_path, 0, NULL);

        if (dp == NULL)
            goto done;

        while ((filename = g_dir_read_name (dp)) != NULL) {
            gchar *childname = g_strdup_printf ("%s/%s", abs_path, filename);

            if (restore) {
                gchar *tmp;
                tmp = g_strdup_printf (
                        info->temp_dir_len == strlen (abs_path) ? "%s\t%s" 
                                                                : "%s/%s",
                        old_search_path, 
                        filename);
                g_free (info->search_path);
                info->search_path = tmp;
            }

            compress_convert_filename (childname, 
                    info,
                    setting, 
                    restore);

            g_free (childname);

            if (setting->flags & FSEXAM_SETTING_FLAGS_STOP)
                break;
        }

        g_free (old_search_path);
        g_dir_close (dp);
    }

done:
    if (need_free_newname) {
        g_free (newname);
    }

    g_free (bname);
    g_free (dname);

    return ret;
}

/*
 * Modify file content on the disk
 */
static gboolean
compress_write_back_contents (FSEXAM_setting *setting, 
        const gchar *fullpath, 
        Compress_info *info,
        gchar *converted_contents, 
        short fid, 
        short tid)
{
    gsize       length;
    gchar       *hist_path = NULL;
    gchar       *log_path = NULL;
    gboolean    same_serial = TRUE;
    gboolean    need_free = FALSE;
    gboolean    ret = FALSE;
    const gchar  *from_encoding = id2encoding (fid);
    const gchar  *to_encoding = id2encoding (tid);

    if (converted_contents == NULL) {
        gchar        *contents = NULL;
        gboolean    err;

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

    /* Modify file contents on the disk */
    if (g_file_set_contents (fullpath, converted_contents, length, NULL)) {
        ret = TRUE;
        ++setting->succ_num;
    }else{
        fsexam_errno = ERR_CANNOT_WRITE;
        ++setting->fail_num;
        goto done;
    }
    

    if (setting->flags & FSEXAM_SETTING_FLAGS_UNDO) {
        goto done;
    }

    if (setting->flags & FSEXAM_SETTING_FLAGS_DIFF_SERIAL) {
        setting->flags &= ~FSEXAM_SETTING_FLAGS_DIFF_SERIAL;
        same_serial = FALSE;
    }

    hist_path = compose_path (fullpath, info, TRUE, FALSE);
    
    fsexam_history_put (setting->hist_info, 
                        ConvContentSpecial, 
                        hist_path,
                        fid,
                        tid,
                        same_serial);

done:
    log_path = compose_path (fullpath, info, FALSE, FALSE);

    /* log file */
    if (fsexam_errno == ERR_OK) {
        gchar   *msg = g_strdup_printf (_("[Content] %s -> %s"), 
                        from_encoding, 
                        to_encoding);
        fsexam_log_puts (setting->log_info, log_path, msg);
        if (setting->display_msg) {
            setting->display_msg (log_path, msg);
        }
        g_free (msg);
    }else{
        fsexam_log_puts (setting->log_info, log_path, NULL);
        if (setting->display_msg) {
            setting->display_msg (log_path, fsexam_error_get_msg ());
        }
    }


    g_free (hist_path);
    g_free (log_path);

    if (need_free)
        g_free (converted_contents);

    return ret;
}

/*
 * Restore single file's content.
 */
static gboolean
compress_restore_single_file_content (const gchar *fullpath, 
        Compress_info *info,
        FSEXAM_setting *setting)
{
    gboolean    ret = FALSE;
    gchar       *hist_path = NULL;
    gchar       *log_path = NULL;
    Hist_item   *item = NULL;

    ++setting->total_num;
    fsexam_errno = ERR_OK;

    hist_path = compose_path (fullpath, info, TRUE, TRUE);
    item = fsexam_history_search (setting->hist_info, hist_path, FALSE);

    if ((item == NULL) || (item->convtype != ConvContentSpecial)) {
        fsexam_errno = ERR_HIST_NO_ITEM;
        goto done;
    }

    ret = compress_write_back_contents (setting, 
            fullpath, 
            info,                   /* Compress info struct */
            NULL,                   /* contents pointer */
            item->to_encoding, 
            item->from_encoding);

    goto cleanup;
    
done:
    log_path = compose_path (fullpath, info, FALSE, FALSE);

    fsexam_log_puts (setting->log_info, log_path, NULL);
    if (setting->display_msg)
        setting->display_msg (log_path, fsexam_error_get_msg());
   
cleanup:
    g_free (hist_path);
    g_free (log_path);

    return ret;
}

/* 
 * Convert single one file's content. Don't care flags
 */
static gboolean
compress_convert_single_file_content (const gchar *fullpath, 
        Compress_info *info,
        FSEXAM_setting *setting)
{
    Score       score;
    Encoding    *encoding = NULL;
    gboolean    err;
    gchar       *sample_contents = NULL;
    gchar       *contents = NULL;
    gsize       length;
    gint        index = 0;
    gboolean    need_free_contents = FALSE;
    gboolean    ret = FALSE;

    fsexam_errno = ERR_OK;
    ++setting->total_num;

    if (! file_validate_for_contentconv (fullpath, setting)) { 
        /* Don't need convert current file's content */
        goto done; 
    }

    /* handle encoding auto detection, plain text need this */
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

    need_free_contents = TRUE;      /* free contents in encoding_list */

    if (setting->pref->dry_run) {
        /* Don't support dry run for special file */
    }else{
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

        if (index == -1) {
            goto cleanup; 
        }

        encoding = (Encoding *)g_list_nth_data (setting->pref->encode_list, 
                                                index);
        if (NULL == encoding){
            fsexam_errno = ERR_ENCODING_INDEX_INVALID;
            goto done;
        }

        ret = compress_write_back_contents (setting, 
                    fullpath, 
                    info,
                    NULL,       /* contents pointer */
                    encoding->encodingID, 
                    encoding2id ("UTF-8"));
    }
        
done:
    if (fsexam_errno != ERR_OK && !setting->pref->dry_run) {
        gchar *log_path = compose_path (fullpath, info, FALSE, FALSE);
        
        fsexam_log_puts (setting->log_info, log_path, NULL);
        if (setting->display_msg)
            setting->display_msg (log_path, fsexam_error_get_msg());

        g_free (log_path);
    }

cleanup:
    if (setting->pref->auto_detect)
        setting->pref->encode_list = fsexam_encoding_remove_auto (
                                            setting->pref->encode_list);

    if (need_free_contents)
        fsexam_encoding_cleanup_content (setting->pref->encode_list);

    return ret;
}

/*
 *  Convert the content of unarchive/uncompress files.
 *  don't handle symlink for special type file.
 */
static gboolean
compress_convert_content (const gchar *fullpath, 
        Compress_info *info,
        FSEXAM_setting *setting, 
        gboolean restore)
{
    gboolean ret = FALSE;
    gchar   *abs_path = get_abs_path (fullpath);
    struct  stat statbuf;

    if ((abs_path == NULL) || (setting == NULL) || (info == NULL))
        return FALSE;

    if (lstat (abs_path, &statbuf) == -1) {
        fsexam_errno = ERR_CANNOT_OPEN;
        goto done;
    }

    if (fsexam_special_is_compress (abs_path, NULL)){ 
        /* nested compress file */
        gchar   *search_path = compose_path (abs_path, info, TRUE, TRUE);
        gchar   *result_path = compose_path (abs_path, info, TRUE, FALSE);
        gchar   *log_path = compose_path (abs_path, info, FALSE, FALSE);
        Compress_info *new_info = compress_info_new (result_path, 
                                                    search_path, 
                                                    log_path);
        if (NULL == new_info) {
            goto done;
        }

        compress_common_convert (abs_path, 
                new_info,
                setting, 
                restore, 
                FALSE);

        compress_info_free (new_info);
    }else if (S_ISREG (statbuf.st_mode)) {  /* regular file */
        if (restore) {
            compress_restore_single_file_content (abs_path, 
                    info,
                    setting);
        }else{
            compress_convert_single_file_content (abs_path, 
                    info,
                    setting);
        }
    } else if (setting->pref->recursive 
            && S_ISDIR (statbuf.st_mode)) { /* Directory */
        const gchar *filename = NULL;
        GDir        *dp = g_dir_open (abs_path, 0, NULL);

        if (dp == NULL)
            goto done;

        while ((filename = g_dir_read_name (dp)) != NULL) {
            gchar *childname = g_strdup_printf ("%s/%s", abs_path, filename);

            compress_convert_content (childname, 
                    info,
                    setting, 
                    restore);

            g_free (childname);

            if (setting->flags && FSEXAM_SETTING_FLAGS_STOP)
                break;
        }

        g_dir_close (dp);
    }

done:
    g_free (abs_path);

    return ret;
}

/*
 * Whether give file is supported archive/compress file or not
 */
static gboolean
fsexam_special_is_compress (const gchar *filename, Compress_Type *type)
{
    int i;

    if (NULL == filename) {
        return FALSE;
    }

    for (i = 0; i < sizeof (compress_pair)/sizeof (compress_pair[0]); i++) {
        if (g_str_has_suffix (filename, compress_pair[i][0])) {
            if (type != NULL)
                *type = (Compress_Type) i;

            return TRUE;
        }
    }
    
    return FALSE;
}

static gboolean
get_iso8859_locale (FSEXAM_setting *setting)
{
    static gboolean first = TRUE;

    if (first) {
        char  *old_locale = setlocale (LC_ALL, NULL);

        if ((setlocale (LC_ALL, "en_US.ISO8859-1")) != NULL) {
            iso8859_locale = "en_US.ISO8859-1";
        }else if ((setlocale (LC_ALL, "fr_FR.ISO8859-1")) != NULL) {
            iso8859_locale = "fr_FR.ISO8859-1";
        }else if ((setlocale (LC_ALL, "de_DE.ISO8859-1")) != NULL) {
            iso8859_locale = "de_DE.ISO8859-1";
        }else if ((setlocale (LC_ALL, "es_ES.ISO8859-1")) != NULL) {
            iso8859_locale = "es_ES.ISO8859-1";
        }else if ((setlocale (LC_ALL, "it_IT.ISO8859-1")) != NULL) {
            iso8859_locale = "it_IT.ISO8859-1";
        }else if ((setlocale (LC_ALL, "sv_SE.ISO8859-1")) != NULL) {
            iso8859_locale = "sv_SE.ISO8859-1";
        }else{
            iso8859_locale = NULL;
        }

        /* reset locale */
        if (old_locale != NULL)
            setlocale (LC_ALL, old_locale);

        first = FALSE;
    }

    if (iso8859_locale == NULL && setting->display_msg) 
        setting->display_msg (NULL, _("Can not run tar command due to lack of iso8859-1 locale, please see fsexam(1) man page for more info."));
 
    return (iso8859_locale != NULL);
}

static gboolean
run_cmd (gchar **argv, gchar **envp, FSEXAM_setting *setting)
{
    FILE *fp = NULL;
    gchar tmp[200];
    GError *error = NULL;
    gboolean ret = FALSE;
    gint child_stderr;

    if (! g_spawn_async_with_pipes (NULL, argv, envp, 0,
                NULL, NULL, NULL, NULL,
                NULL,
                &child_stderr,
                &error)) {
        setting->display_msg (NULL, error->message); 
        g_error_free (error);
        goto DONE;
    }

    fp = fdopen (child_stderr, "r");
    if (fp == NULL) {
        goto DONE;
    }

    if (fgets (tmp, sizeof (tmp), fp) != NULL) {
        setting->display_msg (NULL, tmp); 
        goto DONE;
    }

    ret = TRUE;

DONE:
    if (fp != NULL)
        fclose (fp);

    return ret;
}

static gboolean
uncompress_pre (const gchar *fullpath, const gchar *tmpdir,
        FSEXAM_setting *setting)
{
    gchar **argv = NULL;
    gboolean ret = FALSE;

    if (! get_iso8859_locale (setting)) {
       return FALSE;
    }

    /* cp target file into current directory */
    argv = g_malloc (sizeof (gchar **) * 4);
    argv[0] = "/usr/bin/cp";
    argv[1] = (gchar *)fullpath;
    argv[2] = (gchar *)tmpdir;
    argv[3] = NULL;

    if (! run_cmd (argv, NULL, setting))
        goto DONE;

    ret = TRUE;

DONE:
    g_free (argv);

    return ret;
}

static gboolean
delete_files (const gchar *filename, FSEXAM_setting *setting)
{
    gchar **argv = NULL;
    gboolean ret = FALSE;

    /* rm target file into current directory */
    argv = g_malloc (sizeof (gchar **) * 4);
    argv[0] = "/usr/bin/rm";
    argv[1] = "-rf";
    argv[2] = (gchar *)filename;
    argv[3] = NULL;

    if (! run_cmd (argv, NULL, setting))
        goto DONE;

    ret = TRUE;

DONE:
    g_free (argv);

    return ret;
}

static gboolean
uncompress_post (const gchar *filename, FSEXAM_setting *setting)
{
   if (! delete_files (filename, setting))
       return FALSE;

   return TRUE;
}

static gboolean
uncompress_bz2 (const gchar *fullpath, const gchar *tmpdir,
        FSEXAM_setting *setting)
{
    gchar *bname = NULL;
    gchar **argv = NULL;
    gchar **envp = NULL;
    gboolean ret = FALSE;
        
    if (! uncompress_pre (fullpath, tmpdir, setting)) {
        return FALSE;
    }

    bname = g_path_get_basename (fullpath);

    argv = g_malloc (sizeof (gchar **) * 4);
    argv[0] = "/usr/bin/bunzip2";
    argv[1] = bname;
    argv[2] = NULL;

    /* bunzip2 */
    if (! run_cmd (argv, NULL, setting)) {
        goto DONE;
    }

    /* untar */
    *(bname + strlen (bname) - 4) = '\0';  /* 4 is strlen(".bz2") */
    argv[0] = TARCMD;
    argv[1] = "-xf";
    argv[2] = bname;
    argv[3] = NULL;

    envp = g_malloc (sizeof (gchar **) * 2);
    envp[0] = iso8859_locale;
    envp[1] = NULL;

    if (! run_cmd (argv, envp, setting)) 
        goto DONE;

    /* delete bname */
    if (! uncompress_post (bname, setting))
        goto DONE;


    ret = TRUE;

DONE:
    g_free (bname);
    g_free (argv);
    g_free (envp);

    return ret;
}

static gboolean
uncompress_gz (const gchar *fullpath, const gchar *tmpdir,
        FSEXAM_setting *setting)
{
    gchar *bname = NULL;
    gchar **argv = NULL;
    gchar **envp = NULL;
    gboolean ret = FALSE;
        
    if (! uncompress_pre (fullpath, tmpdir, setting)) {
        return FALSE;
    }

    bname = g_path_get_basename (fullpath);

    argv = g_malloc (sizeof (gchar **) * 4);
    argv[0] = "/usr/bin/gunzip";
    argv[1] = bname;
    argv[2] = NULL;

    /* bunzip2 */
    if (! run_cmd (argv, NULL, setting)) {
        goto DONE;
    }

    /* untar */
    *(bname + strlen (bname) - 3) = '\0';  /* 3 is strlen(".gz") */
    argv[0] = TARCMD;
    argv[1] = "-xf";
    argv[2] = bname;
    argv[3] = NULL;

    envp = g_malloc (sizeof (gchar **) * 2);
    envp[0] = iso8859_locale;
    envp[1] = NULL;

    if (! run_cmd (argv, envp, setting)) 
        goto DONE;

    /* delete bname */
    if (! uncompress_post (bname, setting))
        goto DONE;


    ret = TRUE;

DONE:
    g_free (bname);
    g_free (argv);
    g_free (envp);

    return ret;
}

static gboolean
uncompress_tar (const gchar *fullpath, FSEXAM_setting *setting)
{
    gchar **argv = NULL;
    gchar **envp = NULL;
    gboolean ret = FALSE;
        
    if (! get_iso8859_locale (setting))
        return FALSE;

    argv = g_malloc (sizeof (gchar **) * 4);
    argv[0] = TARCMD;
    argv[1] = "-xf";
    argv[2] = (gchar *)fullpath;
    argv[3] = NULL;

    envp = g_malloc (sizeof (gchar **) * 2);
    envp[0] = iso8859_locale;
    envp[1] = NULL;

    if (! run_cmd (argv, envp, setting)) 
        goto DONE;

    ret = TRUE;

DONE:
    g_free (argv);
    g_free (envp);

    return ret;
}

static gboolean
uncompress_unzip (const gchar *fullpath, FSEXAM_setting *setting)
{
    gchar **argv = NULL;
    gboolean ret = FALSE;
        
    argv = g_malloc (sizeof (gchar **) * 4);
    argv[0] = "/usr/bin/unzip";
    argv[1] = "-q";
    argv[2] = (gchar *)fullpath;
    argv[3] = NULL;

    if (! run_cmd (argv, NULL, setting)) 
        goto DONE;

    ret = TRUE;

DONE:
    g_free (argv);

    return ret;
}

static gboolean
uncompress_Z (const gchar *fullpath, const gchar *tmpdir,
        FSEXAM_setting *setting)
{
    gchar *bname = NULL;
    gchar **argv = NULL;
    gchar **envp = NULL;
    gboolean ret = FALSE;
        
    if (! uncompress_pre (fullpath, tmpdir, setting)) {
        return FALSE;
    }

    bname = g_path_get_basename (fullpath);

    argv = g_malloc (sizeof (gchar **) * 4);
    argv[0] = "/usr/bin/uncompress";
    argv[1] = "-c";
    argv[2] = bname;
    argv[3] = NULL;

    /* bunzip2 */
    if (! run_cmd (argv, NULL, setting)) {
        goto DONE;
    }

    /* untar */
    *(bname + strlen (bname) - 2) = '\0';  /* 2 is strlen(".Z") */
    argv[0] = TARCMD;
    argv[1] = "-xf";
    argv[2] = bname;
    argv[3] = NULL;

    envp = g_malloc (sizeof (gchar **) * 2);
    envp[0] = iso8859_locale;
    envp[1] = NULL;

    if (! run_cmd (argv, envp, setting)) 
        goto DONE;

    /* delete bname */
    if (! uncompress_post (bname, setting))
        goto DONE;


    ret = TRUE;

DONE:
    g_free (bname);
    g_free (argv);
    g_free (envp);

    return ret;
}

/*
 * Uncompress .bz2, .gz, .zip, .Z
 */
static gboolean
fsexam_uncompress (const gchar *fullpath, const gchar *tmpdir,
        Compress_Type type, FSEXAM_setting *setting)
{
    /* uncompress */
    if (type == COMP_TARGZ) {
        return uncompress_gz (fullpath, tmpdir, setting);
    } else if (type == COMP_TARBZ2) {
        return uncompress_bz2 (fullpath, tmpdir, setting);
    } else if (type == COMP_TAR) {
        return uncompress_tar (fullpath, setting);
    } else if (type == COMP_ZIP) {
        return uncompress_unzip (fullpath, setting);
    } else if (type == COMP_Z) {
        return uncompress_Z (fullpath, tmpdir, setting);
    } else {
         if (setting->display_msg)
             setting->display_msg (fullpath,
                     _("Don't support this file type"));
    }

    return FALSE;
}

static gboolean
compress_bz2 (const gchar *target_name, FSEXAM_setting *setting)
{
    gchar **argv = NULL;
    gboolean ret = FALSE;

    argv = g_malloc (sizeof (gchar **) * 5);
    argv[0] = TARCMD;
    argv[1] = "-cjf";
    argv[2] = (gchar *)target_name;
    argv[3] = ".";
    argv[4] = NULL;

    if (! run_cmd (argv, NULL, setting)) {
        goto DONE;
    }

    ret = TRUE;

DONE:
    g_free (argv);

    return ret;
}

static gboolean
compress_tar (const gchar *target_name, FSEXAM_setting *setting)
{
    gchar **argv = NULL;
    gboolean ret = FALSE;

    argv = g_malloc (sizeof (gchar **) * 5);
    argv[0] = TARCMD;
    argv[1] = "-cf";
    argv[2] = (gchar *)target_name;
    argv[3] = ".";
    argv[4] = NULL;

    if (! run_cmd (argv, NULL, setting)) {
        goto DONE;
    }

    ret = TRUE;

DONE:
    g_free (argv);

    return ret;
}

static gboolean
compress_Z (const gchar *target_name, FSEXAM_setting *setting)
{
    gchar **argv = NULL;
    gboolean ret = FALSE;

    argv = g_malloc (sizeof (gchar **) * 5);
    argv[0] = TARCMD;
    argv[1] = "-cZf";
    argv[2] = (gchar *)target_name;
    argv[3] = ".";
    argv[4] = NULL;

    if (! run_cmd (argv, NULL, setting)) {
        goto DONE;
    }

    ret = TRUE;

DONE:
    g_free (argv);

    return ret;
}

/*
 * zip in the current directory, then move to target dir.
 * Otherwise zip(1) will append the new files into old file
 */
static gboolean
compress_zip (const gchar *target_name, const gchar *tmpdir,
        FSEXAM_setting *setting)
{
    gchar **argv = NULL;
    gboolean ret = FALSE;
    gchar *bname = NULL;
    gchar *tmpname = NULL;

    bname = g_path_get_basename (target_name);
    tmpname = g_strdup_printf ("%s/%s", tmpdir, bname);

    argv = g_malloc (sizeof (gchar **) * 5);
    argv[0] = "/usr/bin/zip";
    argv[1] = "-qr";
    argv[2] = tmpname;
    argv[3] = ".";
    argv[4] = NULL;

    if (! run_cmd (argv, NULL, setting)) {
        goto DONE;
    }

    argv[0] = "/usr/bin/cp";
    argv[1] = tmpname;
    argv[2] = (gchar *)target_name;
    argv[3] = NULL;

    if (! run_cmd (argv, NULL, setting))
        goto DONE;

    ret = TRUE;

DONE:
    g_free (argv);
    g_free (bname);
    g_free (tmpname);

    return ret;
}


static gboolean
compress_gz (const gchar *target_name, FSEXAM_setting *setting)
{
    gchar **argv = NULL;
    gboolean ret = FALSE;

    argv = g_malloc (sizeof (gchar **) * 5);
    argv[0] = TARCMD;
    argv[1] = "-czf";
    argv[2] = (gchar *)target_name;
    argv[3] = ".";
    argv[4] = NULL;

    if (! run_cmd (argv, NULL, setting)) {
        goto DONE;
    }

    ret = TRUE;

DONE:
    g_free (argv);

    return ret;
}


/*
 * Compress .bz2, .gz, .zip, .Z
 *  dname: the directory name which will contain the result file
 *  bname: the result filename, such as *.tar.bz2
 */
static gboolean
fsexam_compress (const gchar *target_name, const gchar *tmpdir,
        Compress_Type type, FSEXAM_setting *setting)
{
    /* uncompress */
    if (type == COMP_TARGZ) {
        return compress_gz (target_name, setting);
    } else if (type == COMP_TARBZ2) {
        return compress_bz2 (target_name, setting);
    } else if (type == COMP_TAR) {
        return compress_tar (target_name, setting);
    } else if (type == COMP_ZIP) {
        return compress_zip (target_name, tmpdir, setting);
    } else if (type == COMP_Z) {
        return compress_Z (target_name, setting);
    } else {
         if (setting->display_msg)
             setting->display_msg (target_name,
                     _("Don't support this file type"));
    }

    return FALSE;
}

/*
 * Description: 
 *      Unarchive/Uncompress file, execute name or content conversion, then
 *      Rearchive/Recompress and replace the original file.
 * 
 * Parameter:
 *      fullpath: the full path of compress file, such as /tmp/a.tar
 *      info:     Path needed by compress conversion 
 */
static gboolean
compress_common_convert (const gchar *fullpath, 
        Compress_info *info,
        FSEXAM_setting *setting, 
        gboolean restore, 
        gboolean nameconvert)
{
    Compress_Type   type;
    gchar    oldcwd[PATH_MAX];
    gchar    *bname = NULL;
    gchar    *dname = NULL;
    gchar    *tempdir = NULL;       /* temparay directory */
    gchar    *template = NULL;      /* template for temp dir */
    gchar    *compress_cmd = NULL;

    if (! fsexam_special_is_compress (fullpath, &type)) {
        return FALSE;
    }

    /* 
     * cd to temparary directory at the same directory 
     * with original file  and then uncompress.
     */
    if (getcwd (oldcwd, PATH_MAX) == NULL) {
        fsexam_errno = ERR_GET_CWD;
        goto done;
    }

    bname = g_path_get_basename (fullpath);
    dname = g_path_get_dirname (fullpath);

    template = g_strdup_printf ("%sXXXXXX", fullpath);
    tempdir = mkdtemp (template);

    if (tempdir == NULL) {
        if (fsexam_debug () & FSEXAM_DBG_ARCHIVE)
            g_print ("mkdtemp return NULL\n");

        fsexam_errno = ERR_NO_RIGHTS;
        goto done;
    }

    if (fsexam_debug () & FSEXAM_DBG_ARCHIVE)
        g_print ("Succeed to create tempdir %s\n", tempdir);

    if (chdir (tempdir)) {          /* uncompress file into temp dir */
        fsexam_errno = ERR_NO_RIGHTS;
        goto done;
    }
    
    if (! fsexam_uncompress (fullpath, tempdir, type, setting)) {
        if (fsexam_debug () & FSEXAM_DBG_ARCHIVE) {
            g_print ("Error when uncompress %s to %s\n", fullpath, tempdir);
        }
        /* chdir to orig dir at error */
        chdir (oldcwd);
        goto done;
    }

    if (fsexam_debug () & FSEXAM_DBG_ARCHIVE)
        g_print ("Succeed to uncompress %s to %s\n", fullpath, tempdir);

    /* 
     * Before converting, ensure the tempdir is not empty.
     * If it is empty, this means that the archive file
     * may have absolute path, this will dangerous. Or
     * we met with unknow error.
     */
    if (rmdir (tempdir) == 0) {
        if (fsexam_debug () & FSEXAM_DBG_ARCHIVE) {
            g_print ("Error: archive use absolute path or met unknow error\n");
        }

        if (setting->display_msg)
            setting->display_msg (NULL, 
                    _("The archive file is empty or has absolute path, so can not convert files in this archive file"));
        chdir (oldcwd);
        goto done;
    }

    info->temp_dir_len = strlen (tempdir);

    if (nameconvert) {      /* name conversion */
        compress_convert_filename (tempdir,     /* contain unarchived files */
                info,
                setting, 
                restore);
    } else {                /* content conversion */
        compress_convert_content (tempdir,      /* contain unarchived files */
                info,
                setting, 
                restore);
    }

    /* recompress */
    if (! fsexam_compress (fullpath, tempdir, type, setting)) {
        /* chdir to orig dir at error */
        if (fsexam_debug () & FSEXAM_DBG_ARCHIVE) {
            g_print ("Error when recompress %s to %s\n", tempdir, fullpath);
        }
    }

    if (fsexam_debug () & FSEXAM_DBG_ARCHIVE)
        g_print ("Succeed to recompress %s to %s\n", tempdir, fullpath);

    chdir (oldcwd);
    if (! delete_files (tempdir, setting)) {
        if (fsexam_debug () & FSEXAM_DBG_ARCHIVE) {
            g_print ("Error when delete tempdir: %s\n", tempdir);
        }
    }

    if (fsexam_debug () & FSEXAM_DBG_ARCHIVE)
        g_print ("Succeed to delete tempdir %s\n", tempdir);
    
done:
    g_free (dname);
    g_free (bname);
    g_free (template);

    return TRUE;
}

/* content conversion */
static gboolean
fsexam_compress_convert_content (const gchar *fullpath, 
        FSEXAM_setting *setting, 
        gboolean restore)
{
    Compress_info *info;
    gboolean      ret;

    info = compress_info_new (fullpath, NULL, NULL);

    if (info == NULL)
        return FALSE;

    ret = compress_common_convert (fullpath,   /* abs path of archive file */
                info,
                setting, 
                restore,    /* Convert or Restore */
                FALSE);     /* Content conversion instead of name conversion */

    compress_info_free (info);

    return ret;
}




/* ----------- For content conversion --------------------------- */

gboolean 
fsexam_special_is_special_for_content (const gchar *fullpath, 
        FSEXAM_setting *setting)
{
    return fsexam_special_is_compress (fullpath, NULL);
}

/*
 * Iterate every special file handling module. This assumes that no two special
 * module will handle the same one file.  Return True if file has been 
 * proccessed by one special module; otherwise return False.
 */
gboolean 
fsexam_special_convert_content (const gchar *fullpath, 
        FSEXAM_setting *setting,
        gboolean restore)
{
    g_return_val_if_fail ((fullpath != NULL) && (setting != NULL), FALSE);

    gboolean        ret;

    if (fsexam_compress_convert_content (fullpath, setting, restore)) {
        ret = TRUE;
    }else{
        ret = FALSE;
    }

    return ret;
}


/* ------------- For name convert ------------------- */

/*
 *  If fullpath is supported archive or compress file, then 
 *  return TRUE, otherwise return FALSE.
 */
gboolean 
fsexam_special_is_special_for_name (const gchar *fullpath, 
                                    FSEXAM_setting *setting)
{
    return fsexam_special_is_compress (fullpath, NULL);
}

/*===================================================================
 *  Description:
 *      Convert or Restore the name of special type file.
 *
 *  Parameters:
 *      setting: Contain Total setting/preference information
 *      fullpath: Absolute path or archive or compress file, such as
 *                /tmp/test/test.tar.gz
 *      hist_search_path: Use the oldname(before restore) as history search
 *               path. This is only used for restore. This can ensure path
 *               in history is consistent with path in disk.
 *      restore: convert name or restore name
 *
 *  Return value:
 *=====================================================================*/
gboolean 
fsexam_compress_convert_name (FSEXAM_setting *setting, 
                            const gchar *fullpath, 
                            const gchar *hist_search_path, 
                            gboolean restore)
{
    gboolean      ret;
    Compress_info *info;
    gchar         *search_path;

    /* 
     * hist_search_path is passed by caller, and it is different when
     * restore is performed. When restore, search_path is the direct
     * path used to search in history, don't need compose
     */
    search_path = g_filename_to_uri (hist_search_path, NULL, NULL);
    info = compress_info_new (fullpath, search_path +URI_HEAD_LEN, fullpath);

    if (NULL == info) {
        return FALSE;
    }
    
    ret = compress_common_convert (fullpath,   /* abs path of archive file */
                info,
                setting, 
                restore,            /* Convert or Restore */
                TRUE);              /* Convert Name instead of content */

    compress_info_free (info);
    g_free (search_path);

    return ret;
}
