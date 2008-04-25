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

#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>

#include "fsexam-header.h"
#include "fsexam-convcontent.h"

/* Append SUFFIX when has duplicated name */
#define SUFFIX  "utf-8"

static gboolean write_to_disk (FSEXAM_setting *,
                               const gchar *path,
                               const gchar *origname,
                               const gchar *utf8name,
                               short from_encoding,
                               short to_encoding,
                               gchar **actualname);

static gboolean real_convert (FSEXAM_setting *setting, 
                              const gchar *dname, 
                              const gchar *bname, 
                              gchar **newname);
static gboolean convert_symlink_target_node (
                              GNode *node, 
                              FSEXAM_setting *setting, 
                              gboolean restore);
static gboolean wrapped_convert_node (GNode *node, FSEXAM_setting *setting);
static gboolean wrapped_restore_convert_node (GNode *node, gpointer data);
static gboolean convert_node (GNode *node, gpointer data);
static gboolean dryrun_convert_node (GNode *node, gpointer data);
static gboolean restore_convert_node (GNode *node, gpointer data);
static gboolean fsexam_convert_scenario_for_name (FSEXAM_setting *setting);
static gboolean fsexam_restore_name (FSEXAM_setting *setting, GList *list);
static gboolean _convert_directory (FSEXAM_setting *setting, const gchar *);
static void     convert_directory (FSEXAM_setting *setting, GNode *node);
static gint     construct_candidate_list (GList *list, gboolean forname);


/*
 *  Recreate symlink from file in symlink_node to file in target_node
 */
static gboolean
relink_symlink (GNode *symlink_node, 
                GNode *target_node, 
                FSEXAM_setting *setting)
{
    gchar     *symlink_contents = NULL;
    gchar     *symlink_path = NULL;
    gchar     *target_path = NULL;
    gchar     *log_msg = NULL;
    gboolean  ret = FALSE;

    if ((NULL == symlink_node) || (NULL == target_node) || (NULL == setting))
        return FALSE;

    symlink_path = fsexam_tree_get_path (symlink_node, FALSE);
    symlink_contents = g_file_read_link (symlink_path, NULL);
    
    if (NULL == symlink_contents) {
        fsexam_errno = ERR_CANNOT_READ;
        goto done;
    }

    if (remove (symlink_path) == -1) {  /* delete original symlink */
        fsexam_errno = ERR_CANNOT_RM_SYMLINK;
        goto done;
    }

    if (*symlink_contents == '/') {     /* absolute symlink */
        target_path = fsexam_tree_get_path (target_node, FALSE);
    }else{
        /* reconstruct symlink path for relative symlink */
        gchar     *tmp = symlink_contents;
        gchar     *cur = tmp;
        GNode     *parent_node = symlink_node->parent;
        gboolean  finish = FALSE;

        while (!finish) {
            if ((*tmp != '/') && (*tmp != '\0')) {
                tmp++;
                continue;
            }else{                      /* got one subpath */
                if (*tmp == '\0')
                    finish = TRUE;
                else
                    *tmp = '\0';

                if (strcmp (cur, ".") == 0) {
                    /* no op */
                }else if (strcmp (cur, "..") == 0) {
                    parent_node = parent_node->parent;
                }else if (strcmp (cur, "") == 0) {
                    /* this means '//' in the symlink_contents */
                    cur = "/";      
                }else{
                    GNode *gnode = fsexam_tree_search_name (parent_node, cur);
                    if (gnode == NULL) {  /* fallback to use absolute path */
                        g_free (target_path);
                        target_path = fsexam_tree_get_path (target_node, FALSE);

                        break;
                    }else{
                        TreeNode *tnode = (TreeNode *)gnode->data;

                        if (TREENODE_IS_CONVERTED (tnode)) 
                            cur = TREENODE_GET_UTF8 (tnode);
                        else
                            cur = TREENODE_GET_ORIG (tnode);
                    }
                    
                    parent_node = parent_node->parent;
                }
               
                if (target_path == NULL) {
                    target_path = g_strdup (cur);
                }else{
                    if (strcmp (cur, "/") == 0) {
                        cur = g_strdup_printf ("%s%s", target_path, cur);
                    }else{
                        cur = g_strdup_printf ("%s/%s", target_path, cur);
                    }

                    g_free (target_path);
                    target_path = cur;
                }

                cur = ++tmp;
            }
        } /* endof while */
    }

    if (symlink (target_path, symlink_path) == -1) {
        fsexam_errno = ERR_LOST_SYMLINK_FILE;
        goto done;
    }

    ret = TRUE;

done:
    if (ret)    
        log_msg = _("Re-create symbolic link success"); 

    fsexam_log_puts (setting->log_info, symlink_path, log_msg);
    if (setting->display_msg)
        setting->display_msg (symlink_path, log_msg);

    g_free (symlink_contents);
    g_free (target_path);
    g_free (symlink_path);

    return ret;
}

/*====================================================================
 *  Function Name:  write_to_disk
 *
 *  Parameters:
 *      FSEXAM_setting *setting:
 *      gchar *path:    The dir name which contain file to be renamed
 *      gchar *orig:    The original name
 *      gchar *utf8name:    The utf8 name
 *      gchar **actualname: the actual name used if succeed
 *
 *  Desc:
 *      Rename the file from orig to utf8name. If newname exists, then will 
 *      find one new name, and actualname point to it.
 *
 *      Will handle log/history in this function, dryrun/restore
 *      may call this function directly.
 *
 *  Return value:
 *      If rename success, then return TRUE; 
 *      otherwise(such as can't rename, can't write to log) return FALSE.
 *
 *  Author:     Yandong Yao 2006/08/31
 ========================================================================*/ 
static gboolean
write_to_disk (FSEXAM_setting *setting, 
               const gchar *path,      /* the path of current file */
               const gchar *origname,  /* old base name            */
               const gchar *utf8name,  /* new base name            */
               short from_encoding,    /* from encoding            */
               short to_encoding,      /* to encoding              */
               gchar **actualname)      /* return the actual used   */
{
    static gchar    oldname[PATH_MAX];
    static gchar    newname[PATH_MAX];
    gchar           *retname = NULL;
    gchar           *fname = NULL;
    gchar           *msg = NULL;
    gboolean        ret = FALSE;

    if ((NULL == setting) || (NULL == path) || (NULL == origname) 
            || (NULL == utf8name))
        return FALSE;

    fsexam_errno = ERR_OK;

    /* construct full old name and full new name */
    g_snprintf (oldname, PATH_MAX - 1, "%s/%s", path, origname);
    g_snprintf (newname, PATH_MAX - 1, "%s/%s", path, utf8name);

    /* whether newname == oldname? */
    if (strcmp (utf8name, origname) == 0) {
        fsexam_errno = ERR_NAME_SAME;
        goto done;
    }

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
            fsexam_errno = ERR_CANNOT_RENAME; /* inc dup name */
        }
    }

    if (ret) { 
        if (! (setting->flags & FSEXAM_SETTING_FLAGS_UNDO)) {
            gboolean same_serial = TRUE;

            if (setting->flags & FSEXAM_SETTING_FLAGS_DIFF_SERIAL) {
                setting->flags &= ~FSEXAM_SETTING_FLAGS_DIFF_SERIAL;
                same_serial = FALSE;
            }

            fsexam_history_put (setting->hist_info, 
                                ConvName, 
                                newname,
                                from_encoding, 
                                to_encoding, 
                                same_serial);
        }

        if (setting->update_gui) {
            setting->update_gui (setting, path, origname, retname);
        }
        
        ++setting->succ_num;
    } else {
        ++setting->fail_num;
    }
       
done:
    msg = g_strdup_printf (_("[Name] %s -> %s"), 
                        id2encoding (from_encoding), 
                        id2encoding (to_encoding));

    /* Display messages to user */
    if (setting->display_msg) {
        if (ret) 
            setting->display_msg (newname, msg);
        else
            setting->display_msg (oldname, fsexam_error_get_msg ());
    }

    /* Writting log */
    if (fsexam_errno == ERR_OK) {
        fsexam_log_puts (setting->log_info, newname, msg);
    }else if (fsexam_errno == ERR_NAME_EXIST) {
        fsexam_log_puts (setting->log_info, fname, NULL);
    }else{
        fsexam_log_puts (setting->log_info, newname, NULL);
    }
    
    if (actualname != NULL)
        *actualname = retname;  //freed outside
    else
        g_free (retname);

    g_free (msg);
    g_free (fname);

    return ret;
}

/*====================================================================
 *  Function Name:  real_convert
 *
 *  Parameters:
 *      FSEXAM_setting *setting: 
 *      const gchar *dname: the directory in which file reside
 *      const gchar *bname: the filename without any directory name
 *      gchar **newname: return the new name when success. Need free by caller
 *
 *  Desc:
 *      real_convert() will convert the filename using provided encoding list,
 *      and display candidates to user in interactive mode.
 *
 *      If success to rename file on the disk, then newname will contain the 
 *      new name.
 *
 *      This function will handle log information also.
 *
 *      write_to_disk() will handle log either, because it can be call by
 *      other funcs.
 *
 *  Return value:
 *      If succeed to rename the file, return TRUE, otherwise return FALSE.
 *      In dryrun mode, always return FALSE.
 *
 *      The most important usage for the return value is to determine whether
 *      fsexam change the real file name on disk or not.
 *
 *  Author:     Yandong Yao 2006/09/01
 ========================================================================*/ 
static gboolean
real_convert (FSEXAM_setting *setting, 
              const gchar *dname, 
              const gchar *bname, 
              gchar **newname)
{
    Score       score;
    gchar       *fullname = NULL;
    gboolean    ret = FALSE;

    if ((NULL == setting) || (NULL == dname) || (NULL ==bname))
        return FALSE;

    fsexam_errno = ERR_OK; 

    fullname = g_strdup_printf ("%s/%s", dname, bname);

    if (setting->pref->auto_detect) { /* handle encoding auto detection */
        GList *detected_encoding;

        detected_encoding = str_encoding_detect (bname, DEFAULT_DETECTING_FLAG);
        setting->pref->encode_list = fsexam_encoding_add_auto (
                                                setting->pref->encode_list, 
                                                detected_encoding);
        auto_encoding_free (detected_encoding);
    }

    score = fsexam_encoding_decode (setting->pref->encode_list, 
                                    ConvName, 
                                    (gchar *)bname, 
                                    strlen(bname), 
                                    setting->pref->force);


    if (setting->pref->dry_run){    /* dry run */
        ret = fsexam_dryrun_puts (setting->dryrun_info, 
                                  fullname, 
                                  score, 
                                  setting->pref->encode_list, 
                                  ConvName);    
        ret ? ++setting->succ_num : ++setting->fail_num;
    } else {                        /* real convert */  
        gint        index = 0;
        gchar       *actualname = NULL;
        Encoding    *encoding = NULL;

        if ((score == FAIL) || (score == ORIGINAL)){
            fsexam_errno = (score == FAIL) ? ERR_NO_PROPER_ENCODING 
                                           : ERR_NAME_UTF8_ALREADY;
            fsexam_log_puts_folder_and_name (setting->log_info, 
                                             dname, bname, 
                                             NULL);
            if (setting->display_msg)
                setting->display_msg (fullname, fsexam_error_get_msg());

            goto done;
        }   
       
	    /* 
	     * User may select don't ask me again
	     */ 
	    if (setting->gold_index != -1) {
            index = setting->gold_index;
	    } else if (setting->pref->auto_conversion) {
            index = fsexam_encoding_get_first_index (
                                        setting->pref->encode_list);
        } else {
            index = setting->get_index (setting->pref->encode_list, TRUE);
        }

        if (index == -1) {          /* cancel the selection */
            fsexam_errno = ERR_CANCEL_CONVERSION;
            goto done;
        }

        encoding = (Encoding *)g_list_nth_data (setting->pref->encode_list, 
                                                index);
        if (NULL == encoding){
            fsexam_errno = ERR_ENCODING_INDEX_INVALID;
            fsexam_log_puts_folder_and_name (setting->log_info, 
                                             dname, bname, 
                                             NULL);
            if (setting->display_msg)
                setting->display_msg (fullname, fsexam_error_get_msg());

            goto done;
        }

        ret = write_to_disk (setting, 
                             dname, 
                             bname, 
                             encoding->u.converted_text, 
                             encoding->encodingID, 
                             encoding2id ("UTF-8"),
                             &actualname);

        if ((ret) && (newname)) {   // Return new name
            *newname = actualname;
        }else{
            g_free (actualname);
        }
    }
    
done:
    if (setting->pref->auto_detect)
        setting->pref->encode_list = fsexam_encoding_remove_auto (
                                                setting->pref->encode_list);

    g_free (fullname);

    return ret;
}

/*
 * convert one directory recursively. won't care about symlink
 */
static gboolean
common_convert_directory (FSEXAM_setting *setting, 
                          const gchar *dname, 
                          gboolean restore)
{
    struct stat     statbuf;
    struct dirent   *dirp = NULL;
    DIR             *dp = NULL;
    gchar           *childname = NULL;
    gboolean        ret = FALSE;

    if ((NULL == dname) || (NULL == setting))
        return FALSE;

    if ((dp = opendir (dname)) == NULL){
        fsexam_errno = ERR_CANNOT_OPEN;
        fsexam_log_puts (setting->log_info, dname, NULL);
            if (setting->display_msg)
                setting->display_msg (dname, fsexam_error_get_msg());

        return ret;
    }

    while ((dirp = readdir (dp)) != NULL){
        if ((strcmp (dirp->d_name, ".") == 0)
                || (strcmp (dirp->d_name, "..") == 0))
            continue;

        fsexam_errno = ERR_OK;

        childname = g_strdup_printf ("%s/%s", dname, dirp->d_name);

        /* need convert this filename or not? */
        if (lstat (childname, &statbuf) == -1) 
            fsexam_errno = ERR_FILE_NONEXIST;
        else if (!(S_ISREG(statbuf.st_mode)) 
                && !(S_ISDIR(statbuf.st_mode)) && !(S_ISLNK(statbuf.st_mode))){
            fsexam_errno = ERR_FILE_TYPE_NOT_SUPPORT; 
        }else if ((! setting->pref->hidden) && (*(dirp->d_name) == '.')){
            fsexam_errno = ERR_IGNORE_HIDDEN_FILE;
        }else if ((!setting->pref->remote) 
                && (is_remote_file (setting->remote_path, childname))) {
            fsexam_errno = ERR_IGNORE_REMOTE_FILE;
        } 

        if (fsexam_errno != ERR_OK) {
            fsexam_log_puts (setting->log_info, childname, NULL);
            if (setting->display_msg)
                setting->display_msg (childname, fsexam_error_get_msg());
            g_free (childname);
            continue;
        }
    
        if ((!setting->pref->force) 
                && (str_isutf8 (dirp->d_name, DEFAULT_DETECTING_FLAG))) {
            fsexam_errno = ERR_NAME_UTF8_ALREADY;
        }

        if (restore) {
            //Currently tree for restore name has no -R node, so no-op
        }else if (fsexam_errno != ERR_NAME_UTF8_ALREADY) {
            gchar  *newname = NULL;

            ret = real_convert (setting, dname, dirp->d_name, &newname);

            if (ret && newname != NULL){
                g_free (childname);
                childname = g_strdup_printf ("%s/%s", dname, newname);
                lstat(childname, &statbuf); 
            }
                
            g_free (newname);
        }

        if (setting->flags & FSEXAM_SETTING_FLAGS_STOP) {
            g_free (childname);
            goto done;
        }

        if (! setting->pref->dry_run 
                && setting->pref->special 
                && fsexam_special_is_special_for_name (childname, setting)) {
            gchar *hist_search_path;

            /* history search path is for search Hist_item in history file */
            hist_search_path = g_strdup_printf ("%s/%s", dname, dirp->d_name);
            fsexam_compress_convert_name (setting, 
                                          childname, 
                                          hist_search_path, 
                                          restore);
            g_free (hist_search_path);
        } else if (S_ISDIR(statbuf.st_mode)) {
            _convert_directory(setting, childname);
        }

        g_free (childname);
    }

done:
    closedir (dp);

    return ret;
}

static gboolean
_convert_directory (FSEXAM_setting *setting, const gchar *dname)
{
    return common_convert_directory (setting, dname, FALSE);
}

/*
static gboolean
_restore_directory (FSEXAM_setting *setting, const gchar *dname)
{
    return common_convert_directory (setting, dname, TRUE);
}
*/

/*
 *  convert to fullpath, then call _convert_directory
 */
static void
convert_directory (FSEXAM_setting *setting, GNode *node)
{
    gchar *dir = fsexam_tree_get_path (node, FALSE); 

    _convert_directory (setting, dir);

    return;
}

/*
 * Convert the symlink target, and convert parent first if has not do.
 */
static gboolean
convert_symlink_target_node (GNode *node, 
                             FSEXAM_setting *setting, 
                             gboolean restore)
{
    TreeNode    *tnode = NULL;

    if ((NULL == node) || (NULL == setting))
        return FALSE;
    
    tnode = (TreeNode *)node->data;

    if (TREENODE_IS_TRAVERSED (tnode)) {
        return TREENODE_IS_CONVERTED (tnode) ? TRUE : FALSE;
    }

    if (setting->flags & FSEXAM_SETTING_FLAGS_STOP)
        return FALSE;    /* stop the recursive */

    if ((node->parent != NULL) 
        && (! TREENODE_IS_TRAVERSED ((TreeNode *)((node->parent)->data)))) {
        convert_symlink_target_node (node->parent, setting, restore);
    }

    if (setting->flags & FSEXAM_SETTING_FLAGS_STOP)
        return FALSE;    /* stop the recursive */

    if (restore)
        return wrapped_restore_convert_node (node, (gpointer)setting);
    else
        return wrapped_convert_node (node, (gpointer)setting);
}

/*============================================================================
 * Function Name:   convert_node
 *
 * Parameters:
 *      FSEXAM_setting *:   contain preference information
 *      GNode *node:        The node which will be handled
 *
 * Desc:
 *      GNode Traverse function. Convert one node in the whole tree. 
 *      Need care about recursive flag
 *
 *      Because this function will called by g_node_traverse, so we need 
 *      handle various error information here.
 *
 * Return value:
 *      Return TRUE to stop g_node_traverse().
 *
 * Author:
 *      Yandong Yao 2006/08/22
 ============================================================================*/
static gboolean 
convert_node (GNode *node, gpointer data)
{
    FSEXAM_setting  *setting = (FSEXAM_setting *)data;

    wrapped_convert_node (node, setting);

    if (setting->flags & FSEXAM_SETTING_FLAGS_STOP)
        return TRUE;

    return FALSE;
}

/*
 *  Return TRUE if current node has been converted.
 */
static gboolean
wrapped_convert_node (GNode *node, FSEXAM_setting *setting)
{
    TreeNode        *tnode = node->data;
    gboolean        node_converted = FALSE;
    gchar           *fullpath = NULL;

    if ((NULL == setting) || (NULL == tnode) 
            || (TREENODE_IS_TRAVERSED (tnode))){
        return FALSE;
    }

    TREENODE_SET_TRAVERSED (tnode, 1);  /* avoid infinite loop for symlink */

    fsexam_errno = ERR_OK;

    /* get stored error during tree construction */
    if (TREENODE_FLAG_REMOTE_FILE (tnode)) {
        fsexam_errno = ERR_IGNORE_REMOTE_FILE;
    }else if (TREENODE_FLAG_HIDDEN_FILE (tnode)) {
        fsexam_errno = ERR_IGNORE_HIDDEN_FILE;
    }else if (TREENODE_FLAG_UTF8_ALREADY (tnode)) {
        fsexam_errno = ERR_NAME_UTF8_ALREADY;
    }

    if (fsexam_errno != ERR_OK) {
        fullpath = fsexam_tree_get_path (node, FALSE);
        fsexam_log_puts (setting->log_info, fullpath, NULL);
        if (setting->display_msg) {
            setting->display_msg (fullpath, fsexam_error_get_msg ());
        }

        if (fsexam_errno != ERR_NAME_UTF8_ALREADY)
            goto done;
    }

    if (TREENODE_IS_NEED_CONVERT (tnode)) {
        gchar    *dir = fsexam_tree_get_path (node->parent, FALSE);
        gchar    *bname = tnode->orig;
        gchar    *newname = NULL;

        node_converted = real_convert (setting, dir, bname, &newname);

        if (node_converted && newname != NULL) {
            TREENODE_SET_CONVERTED (tnode, 1);
            TREENODE_SET_UTF8 (tnode, newname);
        }
        g_free (fullpath);
        fullpath = g_strdup_printf ("%s/%s", dir, newname ? newname : bname);

        g_free (dir);

        if (setting->flags & FSEXAM_SETTING_FLAGS_STOP)
            goto done;
    }

    if (TREENODE_IS_SPECIAL (tnode)) {
        if (fullpath == NULL)
                fullpath = fsexam_tree_get_path (node, FALSE);

        /* 
         * archive or compress filename may be converted already,
         * so get the original name.
         */
        gchar *hist_search_path = fsexam_tree_get_path (node, TRUE);

        fsexam_compress_convert_name (setting, 
                                      fullpath, 
                                      hist_search_path, 
                                      FALSE);

        g_free (hist_search_path);
    }else if (TREENODE_IS_SYMLINK (tnode)) {
        if (TREENODE_FLAG_TARGET_NOTEXIST (tnode)) {
            if (fullpath == NULL)
                fullpath = fsexam_tree_get_path (node, FALSE);
            
            fsexam_errno = ERR_SYMLINK_TARGET_NOEXIST;
            fsexam_log_puts (setting->log_info, fullpath, NULL);
            if (setting->display_msg) {
                setting->display_msg (fullpath, fsexam_error_get_msg ());
            }
        }else if (!setting->pref->no_check_symlink_content) {
            /* follow has been handled during tree construction */
            GNode   *gnode = TREENODE_GET_TARGET (tnode);
            
            if ((gnode != NULL) && (gnode->data != NULL)) 
                if (convert_symlink_target_node (gnode, setting, FALSE)) 
                    relink_symlink (node, gnode, setting);
        }
    } else if ((setting->pref->recursive && TREENODE_IS_RECURSIVE (tnode))
            && (!setting->pref->follow) 
            && setting->pref->no_check_symlink_content){
        convert_directory (setting, node);
    }

done:
    g_free (fullpath);

    return node_converted;
}

/*====================================================================
 *  Function Name:  dryrun_convert_node
 *
 *  Parameters:
 *      GNode *node:    one node created from dryrun result file.
 *      gpointer data:  is FSEXAM_setting passed from g_node_traverse.
 *
 *  Desc:
 *      Convert one node come from dryrun result file.
 *
 *      We need handle various error here.
 *
 *  Return value:
 *      Always return TRUE, otherwise g_node_traverse will stop.
 *
 *  Exception:
 *
 *  Author:     Yandong Yao 2006/09/05
 ========================================================================*/ 
static gboolean
dryrun_convert_node (GNode *node, gpointer data)
{
    FSEXAM_setting  *setting = (FSEXAM_setting *)data;
    TreeNode        *tnode = node->data;
    gchar            *path = NULL;
    gboolean        ret;

    if ((NULL == setting)
            || (NULL == tnode)
            || TREENODE_IS_TRAVERSED (tnode) 
            || (TREENODE_GET_UTF8 (tnode) == NULL) 
            || (! TREENODE_IS_NEED_CONVERT (tnode))) {   /* Ingore symlink */
        return FALSE;
    }

    TREENODE_SET_TRAVERSED (tnode, 1);

    path = fsexam_tree_get_path (node->parent, FALSE);  /* get real path */

    ret = write_to_disk (setting, 
                        path, 
                        TREENODE_GET_ORIG (tnode),
                        TREENODE_GET_UTF8 (tnode),
                        TREENODE_GET_ID (tnode),        /* from encoding    */
                        encoding2id ("UTF-8"),          /* to encoding      */
                        NULL);

    /* update TreeNode */
    if (ret) {
        TREENODE_SET_CONVERTED (tnode, 1);  
    }

    g_free (path);

    return FALSE;
}

/*====================================================================
 *  Function Name:  restore_convert_node
 *
 *  Parameters:
 *      GNode *node:   node need restore
 *      gpointer data: is FSEXAM_setting passed from g_node_traverse.
 *
 *  Desc:
 *      restore one node come from history file
 *      We need handle various error here.
 *
 *  Return value:
 *      Always return FALSE, otherwise g_node_traverse will stop.
 *
 *  Author:     Yandong Yao 2006/11/27
 ========================================================================*/ 
static gboolean
restore_convert_node (GNode *node, gpointer data)
{
    wrapped_restore_convert_node (node, data);

    return FALSE;
}

/* 
 * Return TRUE if node has been converted.
 */
static gboolean
wrapped_restore_convert_node (GNode *node, gpointer data)
{
    FSEXAM_setting  *setting = (FSEXAM_setting *)data;  
    TreeNode        *tnode = (node->data);
    gboolean        node_converted = FALSE;
    gchar           *dirpath = NULL;
    gchar           *fullpath = NULL;

    if ((NULL == setting) || (NULL == tnode) 
            || TREENODE_IS_TRAVERSED (tnode)) { 
        return FALSE;
    }

    TREENODE_SET_TRAVERSED (tnode, 1);

    dirpath = fsexam_tree_get_path (node->parent, FALSE);
    fullpath = fsexam_tree_get_path (node, FALSE);

    if (dirpath == NULL) {
        g_free (fullpath);
        return FALSE;
    }

    fsexam_errno = ERR_OK;

    /* do we need convert it? */
    if (TREENODE_FLAG_REMOTE_FILE (tnode)) {
        fsexam_errno = ERR_IGNORE_REMOTE_FILE;
    }else if (TREENODE_FLAG_HIDDEN_FILE (tnode)) {
        fsexam_errno = ERR_IGNORE_HIDDEN_FILE;
    }

    if (fsexam_errno != ERR_OK) {
        fsexam_log_puts (setting->log_info, fullpath, NULL);
        if (setting->display_msg) {
            setting->display_msg (fullpath, fsexam_error_get_msg ());
        }

        goto done;
    }

    if (TREENODE_GET_ORIG (tnode) != NULL && TREENODE_GET_UTF8 (tnode) != NULL){
        if (TREENODE_IS_REVERSE (tnode)) {
            node_converted = write_to_disk (setting,
                                            dirpath,
                                            TREENODE_GET_ORIG (tnode),
                                            TREENODE_GET_UTF8 (tnode),
                                            encoding2id ("UTF-8"),
                                            TREENODE_GET_ID (tnode),
                                            NULL);

        }else{
            node_converted = write_to_disk (setting,
                                            dirpath,
                                            TREENODE_GET_ORIG (tnode),
                                            TREENODE_GET_UTF8 (tnode),
                                            TREENODE_GET_ID (tnode),
                                            encoding2id ("UTF-8"),
                                            NULL);
        }

        if (node_converted)
            TREENODE_SET_CONVERTED (tnode, 1);

        g_free (fullpath);
        fullpath = fsexam_tree_get_path (node, FALSE);
    }

    if (TREENODE_IS_SPECIAL (tnode)) {
        /* hist_search_path use the original name of special file */
        gchar *hist_search_path = fsexam_tree_get_path (node, TRUE);

        /* Restore special type file name */
        fsexam_compress_convert_name (setting, 
                                      fullpath, 
                                      hist_search_path, 
                                      TRUE);

        g_free (hist_search_path);
    }else if (TREENODE_IS_SYMLINK (tnode)) {
        if (TREENODE_FLAG_TARGET_NOTEXIST (tnode)) {
            fsexam_errno = ERR_SYMLINK_TARGET_NOEXIST;
            fsexam_log_puts (setting->log_info, fullpath, NULL);
            if (setting->display_msg) {
                setting->display_msg (fullpath, fsexam_error_get_msg ());
            }
        }else if (!setting->pref->no_check_symlink_content) {
            GNode   *gnode = TREENODE_GET_TARGET (tnode);

            if ((gnode != NULL) && (gnode->data != NULL)) 
                if (convert_symlink_target_node (gnode, setting, TRUE)) 
                    relink_symlink (node, gnode, setting);
        }
    }

done:
    g_free (fullpath);
    g_free (dirpath);

    return node_converted;
}

/*
 *  Display candidate list for CLI.
 *  Return the total number of candidates.
 */
static gint
construct_candidate_list (GList *list, gboolean forname)
{
    gint num_candidate = 0;

    if (list == NULL)
        return -1;

    fprintf (stdout, _("Candidate list:\n"));
    fprintf (stdout, _("\t[No.] Encoding name\tConversion Result\n"));

    while (list != NULL) {
        Encoding *encode = (Encoding *)list->data;

        list = g_list_next (list);

        if (encode->score == FAIL) 
            continue;

        if (forname)
            fprintf (stdout, "\t[%i]   %s\t\t%s\n", 
                     num_candidate, 
                     id2encoding (encode->encodingID),
                     encode->u.converted_text);
        else {                          /* content */
            fprintf (stdout, "\t[%i]   %s\t\t%s\n", 
                     num_candidate, 
                     id2encoding (encode->encodingID),
                     encode->u.contents);
        }

        num_candidate++;
    }

    return num_candidate;
}

static gboolean
fsexam_convert_scenario_for_name (FSEXAM_setting *setting)
{
    g_return_val_if_fail (setting != NULL, FALSE);

    GSList   *slist = NULL;
    GNode    *root = NULL;
    gboolean ret;

    ret = fsexam_dryrun_process (setting->dryrun_info, &slist);

    if ((! ret) || (NULL == slist)) {
        fsexam_log_puts (setting->log_info, NULL, NULL);
        if (setting->display_msg)
            setting->display_msg (NULL, fsexam_error_get_msg ());

        setting->display_stats (setting);   /* display statastics */
        fsexam_log_flush (setting->log_info);
        return FALSE;
    }

    root = fsexam_tree_construct_from_dryrun (slist, setting);
    fsexam_dryrun_item_slist_free (slist);

    if (root == NULL) {
        fsexam_errno = ERR_TREE_IS_EMPTY;
        fsexam_log_puts (setting->log_info, NULL, NULL);
        if (setting->display_msg)
            setting->display_msg (NULL, fsexam_error_get_msg ());

        return FALSE;
    }

    if (fsexam_debug () 
            & (FSEXAM_DBG_OPTION | FSEXAM_DBG_TREE | FSEXAM_DBG_FILELIST)) {
        return TRUE;
    }

    setting->flags |= FSEXAM_SETTING_FLAGS_DIFF_SERIAL;

    g_node_traverse (root, 
                    G_PRE_ORDER, 
                    G_TRAVERSE_ALL, 
                    -1, 
                    dryrun_convert_node, 
                    setting);

    setting->display_stats (setting);   /* display statastics */
    fsexam_log_flush (setting->log_info);
    setting->flags = 0;
    fsexam_tree_destroy (root);

    return TRUE;

}

static gboolean
fsexam_restore_name (FSEXAM_setting *setting, GList *list)
{
    g_return_val_if_fail (setting != NULL, FALSE);

    GNode *root = NULL;

    root = fsexam_tree_construct_from_history (list, 
                                               setting->hist_info, 
                                               setting);

    if ((list == NULL) || (root == NULL)){
        fsexam_errno = ERR_TREE_IS_EMPTY;
        fsexam_log_puts (setting->log_info, NULL, NULL);
        if (setting->display_msg)
            setting->display_msg (NULL, fsexam_error_get_msg ());

        return FALSE;
    }

    setting->flags |= FSEXAM_SETTING_FLAGS_DIFF_SERIAL;

    if (fsexam_debug () 
            & (FSEXAM_DBG_OPTION | FSEXAM_DBG_TREE | FSEXAM_DBG_FILELIST)) {
        return TRUE;
    }

    g_node_traverse (root, 
                    G_PRE_ORDER, 
                    G_TRAVERSE_ALL, 
                    -1, 
                    restore_convert_node, 
                    setting);

    setting->display_stats (setting);   /* display statastics */
    fsexam_log_flush (setting->log_info);
    setting->flags = 0;
    fsexam_tree_destroy (root);

    return TRUE;
}

/* --------------   Public API  ------------------------ */

/*
 *  find one non exist name through append suffix.
 *  need free by caller
 */
gchar *
find_non_exist_name (const gchar *name)
{
    gchar   tmp[PATH_MAX];
    gint    i;

    for (i = 0; ; i++) {
        if (i) {
            g_snprintf (tmp, PATH_MAX - 1, "%s.%s.%d", name, SUFFIX, i);
        }else{
            g_snprintf (tmp, PATH_MAX - 1, "%s.%s", name, SUFFIX);
        }

        if (! g_file_test (tmp, G_FILE_TEST_EXISTS))
            return g_strdup (tmp);
    }
}

/*
 * Convert single file's fullpath, don't care any flags
 */
gboolean
fsexam_convert_single_filename (FSEXAM_setting *setting, 
                                const gchar *filename,
                                gchar **result_name)
{
    GNode    *root = NULL;
    GNode    *tmp = NULL;
    gboolean node_converted = FALSE;

    if ((filename == NULL) || (setting == NULL)) {
        return FALSE;
    }

    fsexam_setting_reset_stats (setting);

    if ((root = fsexam_tree_construct_from_single_file (filename)) == NULL) {
        fsexam_errno = ERR_TREE_IS_EMPTY;
        fsexam_log_puts (setting->log_info, NULL, NULL);
        if (setting->display_msg)
            setting->display_msg (NULL, fsexam_error_get_msg ());

        return FALSE;
    }

    setting->flags |= FSEXAM_SETTING_FLAGS_DIFF_SERIAL;

    tmp = root;
    while ((tmp = g_node_first_child (tmp)) != NULL) {
        TreeNode *tnode = tmp->data;
        gchar    *bname = tnode->orig;
        gchar    *dir = NULL;
        gchar    *newname = NULL;

        if (! TREENODE_IS_NEED_CONVERT (tnode)) {
            continue;
        }

        dir = fsexam_tree_get_path (tmp->parent, FALSE);
        node_converted = real_convert (setting, dir, bname, &newname);
        g_free (dir);

        if (node_converted) {
            TREENODE_SET_CONVERTED (tnode, 1);
            TREENODE_SET_UTF8 (tnode, newname);

            if ((G_NODE_IS_LEAF (tmp)) && (result_name != NULL)) {
                /* last node */ 
                *result_name = fsexam_tree_get_path (tmp, FALSE);
            }
        }else{
            g_free (newname);
            break;      /* fsexam_errno store the reason */
        }
    }
    
    setting->flags = 0;
    fsexam_tree_destroy (root);

    return node_converted;
}

gboolean
fsexam_convert_filename (FSEXAM_setting *setting, const gchar *filename)
{
    g_return_val_if_fail (setting != NULL, FALSE);

    GList       *list = NULL;
    gboolean    ret;

    if (filename != NULL) 
        list = g_list_prepend (list, (gpointer) filename);

    ret = fsexam_convert_filename_batch (setting, list);

    fsexam_list_free (list);

    return ret;
}

gboolean
fsexam_convert_filename_batch (FSEXAM_setting *setting, GList *list)
{
    g_return_val_if_fail (setting != NULL, FALSE);

    GNode *root = NULL;

    fsexam_setting_reset_stats (setting);

    if (list != NULL) 
        root = fsexam_tree_construct_from_list (list, setting);

    if ((NULL == list) || (NULL == root)){
        fsexam_errno = ERR_TREE_IS_EMPTY;
        fsexam_log_puts (setting->log_info, NULL, NULL);
        if (setting->display_msg)
            setting->display_msg (NULL, fsexam_error_get_msg ());

        return FALSE;
    }

    if (fsexam_debug () 
            & (FSEXAM_DBG_OPTION | FSEXAM_DBG_TREE | FSEXAM_DBG_FILELIST)) {
        return TRUE;
    }

    setting->flags |= FSEXAM_SETTING_FLAGS_DIFF_SERIAL;

    g_node_traverse (root, 
                     G_PRE_ORDER, 
                     G_TRAVERSE_ALL, 
                     -1, 
                     convert_node,  /* Return TRUE will stop the traverse */
                     setting);

    setting->display_stats (setting);   /* display statastics */
    fsexam_log_flush (setting->log_info);
    setting->flags = 0;             /* clear all flags after conversion */
    fsexam_tree_destroy (root);

    return TRUE;
}


//TODO: Add format validation
gboolean
fsexam_convert_scenario (FSEXAM_setting *setting)
{
    g_return_val_if_fail (setting != NULL, FALSE);

    ConvType    type;
    gboolean    ret;

    fsexam_setting_reset_stats (setting);

    if (setting->dryrun_info == NULL) {
        fprintf (stderr, _("Can't access dry run result information.\n"));
        return FALSE;
    }

    if (!fsexam_dryrun_get_convtype (setting->dryrun_info, &type))
        return FALSE;

    if (type == ConvName) {
        ret = fsexam_convert_scenario_for_name (setting);
    }else{
        ret = fsexam_convert_scenario_for_content (setting);
    }

    return ret;
}

gboolean 
fsexam_restore (FSEXAM_setting *setting, GList *list, ConvType restore_type)
{
    gboolean ret;

    fsexam_setting_reset_stats (setting);

    if (restore_type == RestoreConvName) {
        ret = fsexam_restore_name (setting, list);
    }else if (restore_type == RestoreConvContent) {
        ret = fsexam_restore_content (setting, list);
    }else{
        g_print (_("Don't support this restore type.\n"));
        ret = FALSE;
    }

    return ret;
}


/* ---------------  call back function for CLI ------------  */

#define INPUT_LEN 10

/*
 *  Get the index of user's selection.
 *  -1 returned if no selection.
 */
gint 
get_index_default (GList *encoding_list, gboolean forname)
{
    gint     num_candidate;
    gchar    input[INPUT_LEN];
    gint     response;
    gint     result_index;

    num_candidate = construct_candidate_list (encoding_list, forname);

    if (num_candidate < 1) {
        fprintf (stderr, _("No proper encoding.\n"));
        return -1;
    }

    do {
        fprintf (stdout, _("Please select No.('s' to skip): "));
        memset (input, 0, sizeof (input));  
        fflush (stdin);             /* discard excess data */
        fgets (input, sizeof(input), stdin);

        if ((*input == '\n') || (*input == '\0'))   /* Return or Empty */
            continue;

        input[strlen (input) - 1] = '\0';   /* remove trailing '\n' */

        if (g_ascii_strcasecmp (input, "s") == 0) {
            response = -1;
            break;
        }else{
            gchar *tmp = input;

            for (; ((*tmp >= '0') && (*tmp <= '9') && (*tmp != '\0')); tmp++)
                ;

            if (*tmp != '\0')
                continue;

            response = atoi (input);    /* input belong [0-9]{1, 9}  */

            if ((response < 0) || (response > num_candidate - 1))
                continue;
            else
                break;
        }
    } while (TRUE);

    if (response == -1)
        return -1;

    fsexam_encoding_iterate_with_func (encoding_list,
                fsexam_encoding_translate_index,
                &response,
                &result_index);

    return result_index;
}

/*
 *  display_msg func for CLI
 */
void
display_msg_default (const gchar *filename, const gchar *msg)
{
    if ((filename == NULL) || (msg == NULL))
        return;

    if (filename) {
        if (g_utf8_validate (filename, -1, NULL)) {
            fprintf (stderr, "%s: %s\n", filename, msg ? msg : "");
        }else{
            gchar *uri = g_filename_to_uri (filename, NULL, NULL);

            if (uri == NULL) {  /* such as filename doesn't exist */
                uri = fsexam_string_escape (filename);
            }

            fprintf (stderr, "%s: %s\n", uri, msg ? msg : "");
            g_free (uri);
        }
    }else{ 
        fprintf (stderr, "%s\n", msg);
    }

    return;
}

