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
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>

#include <glib.h>

#include "fsexam-debug.h"
#include "fsexam-header.h"

static gboolean compare_filename (GNode *node, gchar *filename);
static gboolean fsexam_tree_node_data_destroy (GNode *node, gpointer data);
static gboolean fsexam_tree_fill_target (GNode *node, gpointer root);
static GNode    *fsexam_tree_insert_single_file_full (GNode *root, 
                                const gchar *fullname, 
                                short from_id, 
                                short to_id, 
                                FSEXAM_setting *setting);
static GNode    *fsexam_tree_insert_single_file (GNode *root, 
                                const gchar *fullname, 
                                FSEXAM_setting *setting);
static gboolean fsexam_tree_insert_file (GNode *root, 
                                const gchar *fullname, 
                                FSEXAM_setting *setting, 
                                GNode **node);
static gboolean fsexam_tree_insert_file_for_restore (GNode *root, 
                                const gchar *filename, 
                                FSEXAM_setting *setting, GNode **node);

TreeNode *
fsexam_node_new (const gchar *filename, 
                 gboolean recursive, 
                 gboolean need_convert)
{
    TreeNode *node = NULL;
    gchar    *orig = NULL;
    
    if (NULL == filename)
        return NULL;

    node = g_new0 (TreeNode, 1);

    orig = g_strdup (filename);
    TREENODE_SET_ORIG (node, orig);
    TREENODE_SET_UTF8 (node, NULL);
    TREENODE_SET_TARGET (node, NULL);
    TREENODE_SET_ID (node, (short)-1);
    TREENODE_SET_RECURSIVE (node, recursive ? 1 : 0);
    TREENODE_SET_TRAVERSED (node, 0);
    TREENODE_SET_EXPAND (node, 0);
    TREENODE_SET_REVERSE (node, 0);
    TREENODE_SET_CONVERTED (node, 0);
    TREENODE_SET_SYMLINK (node, 0);
    TREENODE_SET_SPECIAL (node, 0);
    TREENODE_SET_FLAGS (node, 0);
    TREENODE_SET_NEED_CONVERT (node, need_convert ? 1 : 0);

    return node;
}

void
fsexam_node_destroy (TreeNode *node)
{
    if (NULL == node)
        return;

    g_free (TREENODE_GET_ORIG(node));
    g_free (TREENODE_GET_UTF8(node));
    g_free (node);

    return;
}
 /* ---------   API For Tree Struct ------------------ */


/*
 * Unlink one node from tree and destroy all its node
 */
static void
unlink_and_destroy (GNode *node, gpointer data)
{
    if (NULL == node)
        return;

    g_node_unlink (node);
    fsexam_tree_destroy (node);
}

/*
 * free one tree node's data
 */
static gboolean
fsexam_tree_node_data_destroy (GNode *node, gpointer  data)
{
    fsexam_node_destroy (node->data);

    return FALSE;
}

/*
 * Init the whole tree to have only one node for '/'
 */
GNode *
fsexam_tree_init ()
{
    TreeNode *node = fsexam_node_new ("/", FALSE, FALSE);

    return g_node_new (node);
}


/*
 *  Compare whether node's original name is filename or not 
 */
static gboolean 
compare_filename (GNode *node, gchar *filename)
{
    TreeNode *tnode = NULL;

    if ((NULL == node) || (NULL == filename))
        return FALSE;

    tnode = node->data;
    if (strcmp (filename, TREENODE_GET_ORIG (tnode)) != 0){
        return FALSE;
    } 
   
    return TRUE;
}

/*
 * Fill target field for one symlink node, If the target file
 * doesn't exist, set flag so handle it during conversion.
 *
 * called only when !follow && !no_check_symlink_content
 */
static gboolean
fsexam_tree_fill_target (GNode *node, gpointer root)
{
    if ((NULL == node) || (NULL == root))
        return FALSE;

    TreeNode *tnode = node->data;

    if (TREENODE_FLAG_TARGET_NOTEXIST (tnode)) {
        return FALSE;
    }

    if ((TREENODE_IS_SYMLINK (tnode)) 
            && (TREENODE_GET_TARGET (tnode) == NULL)) {
        /* Have not set target link */
        gchar   *abs_path = fsexam_tree_get_path (node, TRUE);
        gchar   *target = NULL;

        if (abs_path == NULL)
            return FALSE;
            
        target = get_abs_path_for_symlink_target (abs_path);

        if (target != NULL) {
            GNode *tmp = fsexam_tree_search_path ((GNode *)root, target);

            if (tmp != NULL) {
                TREENODE_SET_TARGET (tnode, tmp);
            }
        }else{
            TREENODE_SET_FLAGS (tnode, TREEFLAG_TARGET_NOTEXIST);
        }

        g_free (target);
        g_free (abs_path);
    }
    
    return FALSE;
}

/*
 *  Insert one file(with full path) into tree, don't handle flags
 *  if last=TRUE, then only convert basename
 *  Return the last node
 */
static GNode *
insert_single_file (GNode *root, const gchar *fullpath, gboolean last)
{
    GNode   *parent = root;
    gchar   subdir[PATH_MAX];
    gchar   *tmp = subdir;

    if ((NULL == root) || (NULL == fullpath))
        return NULL;

    if (*(++fullpath) == '\0') {    /* skip the leading '/' */
        return NULL;
    }

    while (TRUE) {
        *tmp++ = *fullpath++; 
        
        if ((*fullpath == '\0') || (*fullpath == '/')){
            GNode       *gnode = NULL;
            TreeNode    *tnode = NULL;
            
            *tmp = '\0';

            if (subdir == tmp)      /* empty subcomponent */
                continue;

            gnode = fsexam_tree_search_name (parent, subdir);

            if (gnode == NULL) {    /* new node */
                gboolean  need_convert = FALSE;

                if ((last) && (*fullpath == '\0')) {
                    need_convert = !str_isutf8 (subdir, DEFAULT_DETECTING_FLAG);
                }else if (!last){
                    need_convert = !str_isutf8 (subdir, DEFAULT_DETECTING_FLAG);
                }
                
                tnode = fsexam_node_new (subdir, FALSE, need_convert);
                parent = g_node_insert(parent, -1, g_node_new (tnode));
            } else {                /* existing node */
                parent = gnode;
            }

            tmp = subdir;           /* prepare for next subdirectory */
    
            if (*fullpath == '/'){
                fullpath++;
            } else {                /* (*fullpath == '\0')  */
                break;
            }
        }
    }

    return parent;
}

/*
 *  Insert single file and set flags.
 *  Return the new added last node 
 */
static GNode * 
fsexam_tree_insert_single_file (GNode *root, 
                                const gchar *fullpath, 
                                FSEXAM_setting *setting) 
{
    GNode       *last = NULL;
    TreeNode    *tnode = NULL;
    gchar       *bname = NULL;
    
    if ((NULL == root) || (NULL == fullpath) || (NULL == setting)) {     
        return NULL;
    } 
    
    if ((last = insert_single_file (root, fullpath, TRUE))  == NULL) {
        return NULL;
    }

    tnode = last->data;
    bname = TREENODE_GET_ORIG (tnode);

    if ((!setting->pref->force) 
            && (str_isutf8 (bname, DEFAULT_DETECTING_FLAG))) {
        TREENODE_SET_FLAGS (tnode, TREEFLAG_UTF8_ALREADY);  
        TREENODE_SET_NEED_CONVERT (tnode, 0);   /* restore don't use this */
    }if ((!setting->pref->remote) 
            && (is_remote_file (setting->remote_path, fullpath))) {
        TREENODE_SET_FLAGS (tnode, TREEFLAG_REMOTE_FILE);
    } else if ((!setting->pref->hidden) 
            && (('/' == *bname) || ('.' == *bname))) {
        TREENODE_SET_FLAGS (tnode, TREEFLAG_HIDDEN_FILE);
    } else if (setting->pref->special 
            && fsexam_special_is_special_for_name (fullpath, setting)) {
        TREENODE_SET_SPECIAL (tnode, 1);
    }

    return last;
}

/*
 *  Insert one file and set flags.
 *
 *  fullname:  the real name on the disk.
 *  from_id:   the encoding of fullname (real encoding on the disk)
 *  to_id:     the encoding we will convert to
 *
 *  Note:
 *      TreeNode->orig maybe UTF-8 encoded while TreeNode->utf8 may
 *      be not real UTF-8
 */
static GNode *
fsexam_tree_insert_single_file_full (GNode *root,
                                     const gchar *fullname,
                                     short from_id,
                                     short to_id,
                                     FSEXAM_setting *setting)
{
    GNode        *last = NULL;
    static short utf8_id;

    ++setting->total_num;
    utf8_id = encoding2id ("UTF-8");

    last = insert_single_file (root, fullname, TRUE);

    if (last != NULL) {
        TreeNode    *tnode = last->data;
        const gchar *from_encoding = id2encoding (from_id);
        const gchar *to_encoding = id2encoding (to_id);
        gchar       *orig_name = TREENODE_GET_ORIG (tnode); /* disk name */
        gchar       *new_name = NULL;

        new_name = g_convert (orig_name,
                              strlen (orig_name),
                              to_encoding,        //to_encoding
                              from_encoding,      //from_encoding
                              NULL,
                              NULL,
                              NULL);

        TREENODE_SET_UTF8 (tnode, new_name);    //free when don't use

        if (g_ascii_strcasecmp (from_encoding, "UTF-8") == 0) {
            TREENODE_SET_REVERSE (tnode, 1);
            TREENODE_SET_ID (tnode, to_id);
        }else if (g_ascii_strcasecmp (to_encoding, "UTF-8") == 0) {
            TREENODE_SET_ID (tnode, from_id);
        }

        if (setting->pref->special 
                && fsexam_special_is_special_for_name (fullname, setting)) {
            TREENODE_SET_SPECIAL (tnode, 1);
        }
    }

    return last;
}

/*
 * Insert one file and handle recursive and symlink.
 * Return TRUE when only insert the current file successfully.
 *
 * Note that the file may have been added into the tree already.
 */
static gboolean
fsexam_tree_insert_file (GNode *root, 
                        const gchar *filename, 
                        FSEXAM_setting *setting, 
                        GNode **node)
{
    struct      stat buf;
    GNode       *newnode = NULL;
    TreeNode    *tnode = NULL;
    gchar       *fullname = NULL;
    gboolean    ret = FALSE;

    if ((root == NULL) || (filename == NULL) || (setting == NULL)) {
           return FALSE;
    }
   
    fsexam_errno = ERR_OK;
    fullname = get_abs_path (filename);
   
    /* 
     * we need lstat for both new node and existing node for
     * following determination on S_ISLNK and S_ISREG
     */
    if (lstat (fullname, &buf) == -1) {
        fsexam_errno = ERR_FILE_NONEXIST;
    } else if (!(S_ISREG(buf.st_mode)) && !(S_ISDIR(buf.st_mode)) 
            && !(S_ISLNK(buf.st_mode))){
        fsexam_errno = ERR_FILE_TYPE_NOT_SUPPORT; 
    }
    
    if (fsexam_errno != ERR_OK) {
        fsexam_log_puts (setting->log_info, filename, NULL);
        if (setting->display_msg) {
            setting->display_msg (filename, fsexam_error_get_msg());
        }
        goto done;
    }

    /* Is this file added into the tree by other path? */
    newnode = fsexam_tree_search_path (root, fullname);

    if (newnode == NULL) {
        ++setting->total_num;

        /* Insert this file if not in tree already */
        if ((newnode = insert_single_file (root, fullname, TRUE)) == NULL)
            goto done;
    } 

    if (node != NULL)
        *node = newnode;
    tnode = (TreeNode *)newnode->data;
    ret = TRUE;

    {
        /* 
         * Code block to set flags for both new node and old node
         */
        gchar *bname = TREENODE_GET_ORIG (tnode);
        TREENODE_SET_NEED_CONVERT (tnode, 1);

        if ((!setting->pref->force) 
                && (str_isutf8 (bname, DEFAULT_DETECTING_FLAG))) {
            TREENODE_SET_FLAGS (tnode, TREEFLAG_UTF8_ALREADY);  
            TREENODE_SET_NEED_CONVERT (tnode, 0);   /* restore not use this */
            ++setting->ignore_num;
        } 
        if ((!setting->pref->remote) 
                && (is_remote_file (setting->remote_path, fullname))) {
            TREENODE_SET_FLAGS (tnode, TREEFLAG_REMOTE_FILE);
            ++setting->ignore_num;
        } 
        if ((!setting->pref->hidden) 
                && (('/' == *bname) || ('.' == *bname))) {
            TREENODE_SET_FLAGS (tnode, TREEFLAG_HIDDEN_FILE);
            ++setting->ignore_num;
        } 
        if (setting->pref->special 
            && fsexam_special_is_special_for_name (fullname, setting)) {
            TREENODE_SET_SPECIAL (tnode, 1);
        }
    }

    /* 
     * This node may has processed 'symlink' and 'recursive', such as if 
     * parent directory is processed after subdirectory has been handled,
     * or if one symlink link to its ancestor.
     */
    if (S_ISLNK (buf.st_mode) && ! TREENODE_IS_SYMLINK (tnode)) {
        TREENODE_SET_SYMLINK (tnode, 1);
        TREENODE_SET_TARGET  (tnode, NULL);
   
        /* Here we only handle 'follow', 'no_check...' is delayed to caller */
        if (setting->pref->follow) {
            gchar  *target = get_abs_path_for_symlink_target (fullname);
            GNode   *added_node = NULL;
    
            if (target != NULL) {
            //recursive call myself
            fsexam_tree_insert_file (root, target, setting, &added_node);
                        
            if (added_node != NULL) {
                TREENODE_SET_TARGET (tnode, added_node);
            }
            } else {
                    TREENODE_SET_FLAGS (tnode, TREEFLAG_TARGET_NOTEXIST);
            }
    
            g_free (target);
        } 
    } else if ((S_ISDIR (buf.st_mode)) && setting->pref->recursive) {
        if ((! setting->pref->follow 
                    && setting->pref->no_check_symlink_content)) {
            /* create -R node */
            TREENODE_SET_RECURSIVE (tnode, 1);
            g_node_children_foreach (newnode, 
                            G_TRAVERSE_ALL, 
                            unlink_and_destroy, 
                            NULL);
        } else if (! TREENODE_IS_EXPAND (tnode)) {
            /* expand this node only if we have not expanded it before */
            DIR *dp = opendir (fullname);

            /* We are in the process of expanding one node recursively */
            TREENODE_SET_EXPAND (tnode, 1);

            if (dp != NULL) {
                struct dirent *dirent = NULL;

                while ((dirent = readdir (dp)) != NULL) {
                    gchar   *childname = NULL;

                    if ((strcmp (dirent->d_name, ".") == 0)
                            || (strcmp (dirent->d_name, "..") == 0))
                        continue;

                    childname = g_strdup_printf ("%s/%s", 
                                                 fullname, 
                                                 dirent->d_name);
                    fsexam_tree_insert_file (root, childname, setting, NULL);
                    g_free (childname);
                }

                closedir (dp);
            }
        }
    }

    g_free (fullname);
    return ret;

done:
    g_free (fullname);
    ++setting->ignore_num;

    return ret;
}

static gboolean
fsexam_tree_insert_file_for_restore (GNode *root, 
                                     const gchar *filename, 
                                     FSEXAM_setting *setting, 
                                     GNode **node)
{
    struct      stat statbuf;
    GNode       *newnode = NULL;
    TreeNode    *tnode = NULL;
    gchar       *fullname = NULL;
    Hist_item   *item = NULL;
    gboolean    ret = FALSE;

    if ((NULL == root) || (NULL == filename) || (NULL == setting))
        return FALSE;

    ++setting->total_num;
    fullname = get_abs_path (filename);

    fsexam_errno = ERR_OK;
    if (lstat (fullname, &statbuf) == -1) {
        fsexam_errno = ERR_FILE_NONEXIST;
        ++setting->ignore_num;

        goto done;
    }

    item = fsexam_history_search (setting->hist_info, fullname, TRUE);

    if (item == NULL) {
        fsexam_errno = ERR_HIST_NO_ITEM;
        /* for special file only */
        if (setting->pref->special 
                && fsexam_special_is_special_for_name (fullname, setting)) {
            newnode = fsexam_tree_insert_single_file (root,
                                    fullname,   //real name on the disk
                                    setting);
        }else{
            ++setting->ignore_num;
        }
    } else if ((item != NULL) && (item->convtype == ConvName)) {
        if (((newnode = fsexam_tree_search_path (root, fullname)) != NULL) 
                || ((newnode = insert_single_file (root, fullname, TRUE)) != NULL)) {
            /* New added node or existing node */
            const gchar  *from_encoding = id2encoding (item->to_encoding);
            const gchar  *to_encoding = id2encoding (item->from_encoding);
            gchar        *orig_name = NULL;
            gchar        *new_name = NULL;

            tnode = (TreeNode *)newnode->data;

            orig_name = TREENODE_GET_ORIG (tnode);  //The real name on disk
            new_name = g_convert (orig_name,
                                strlen (orig_name),
                                to_encoding,        //to_encoding
                                from_encoding,      //from_encoding
                                NULL,
                                NULL,
                                NULL);

            g_free (TREENODE_GET_UTF8 (tnode));     //need this?
            TREENODE_SET_UTF8 (tnode, new_name);

            if (g_ascii_strcasecmp (from_encoding, "UTF-8") == 0) {
                TREENODE_SET_REVERSE (tnode, 1);
                TREENODE_SET_ID (tnode, item->from_encoding);   /* to_id */
            }else if (g_ascii_strcasecmp (to_encoding, "UTF-8") == 0) {
                TREENODE_SET_ID (tnode, item->to_encoding);     /* from_id */
            }

            if (setting->pref->special 
                    && fsexam_special_is_special_for_name (fullname, setting)) {
                TREENODE_SET_SPECIAL (tnode, 1);
            }

            if (node != NULL)
                *node = newnode;
            ret = TRUE;
        }
    }

    if (fsexam_errno != ERR_OK) {
        fsexam_log_puts (setting->log_info, filename, NULL);
        if (setting->display_msg != NULL) {
            setting->display_msg (filename, fsexam_error_get_msg());
        }
    }

    /* 
     * This node may has processed symlink and recursive, such as if parent
     * directory is processed after subdirectory has been handled, or if
     * one symlink link to its ancestor.
     */
    if (S_ISLNK (statbuf.st_mode)) {
        if (tnode == NULL) {    /* No history infor, but it is symlink */
            GNode *gnode = fsexam_tree_insert_single_file (root, 
                                                           fullname, 
                                                           setting);
            if (gnode != NULL)
                tnode = gnode->data;
        }

        if ((tnode == NULL) || TREENODE_IS_SYMLINK (tnode))
            goto done;

        TREENODE_SET_SYMLINK (tnode, 1);
        TREENODE_SET_TARGET  (tnode, NULL);

        if (setting->pref->follow) {
            gchar   *target = get_abs_path_for_symlink_target (fullname);
            GNode   *added_node = NULL;

            if (target != NULL) {
                fsexam_tree_insert_file_for_restore (root, 
                        target, setting, &added_node);

                if (added_node != NULL) {
                    TREENODE_SET_TARGET (tnode, added_node);
                }
            }else{
                TREENODE_SET_FLAGS (tnode, TREEFLAG_TARGET_NOTEXIST);
            }

            g_free (target);
        }
    } else if ((S_ISDIR (statbuf.st_mode)) && setting->pref->recursive) {
        DIR *dp = NULL;

        if (tnode == NULL) {
            /* 
             * No history infor for current file, but it is directory,
             * and have 'recursive' flag, so we add this file into tree
             * always.
             */
            GNode *gnode = fsexam_tree_insert_single_file (root, 
                                                           fullname, 
                                                           setting);
            if (gnode != NULL)
                tnode = gnode->data;
        }

        if ((tnode == NULL) || TREENODE_IS_EXPAND (tnode))
            goto done;

        /* expand this node only when we have not expaned it before */
        TREENODE_SET_EXPAND (tnode, 1);
        
        if (setting->flags & FSEXAM_SETTING_FLAGS_UNDO)
            goto done;  /* don't need further handling for UNDO */

        if ((dp = opendir (fullname)) != NULL) {
            struct dirent *dirent = NULL;

            while ((dirent = readdir (dp)) != NULL) {
                gchar    *childname = NULL;

                if ((strcmp (dirent->d_name, ".") == 0)
                        || (strcmp (dirent->d_name, "..") == 0))
                    continue;

                childname = g_strdup_printf ("%s/%s", fullname, dirent->d_name);
                fsexam_tree_insert_file_for_restore (root, 
                                                     childname, 
                                                     setting, 
                                                     NULL);
                g_free (childname);
            }

            closedir (dp);
        }
    }

done:
    g_free (fullname);
    
    return ret;
}

/* --- public API --- */

/*
 * Construct tree from single file, don't care flags at all
 */
GNode *
fsexam_tree_construct_from_single_file (const gchar *filename)
{
    GNode *root;
    gchar *abs_path = NULL;

    abs_path = get_abs_path (filename);

    if (NULL == abs_path)
        return NULL;

    if ((root = fsexam_tree_init ()) == NULL)
        return NULL;

    insert_single_file (root, abs_path, FALSE);

    if (fsexam_debug () & FSEXAM_DBG_TREE) {
        g_node_traverse (root, G_PRE_ORDER, 
                         G_TRAVERSE_ALL, -1, 
                         display_node, NULL);
    }

    g_free (abs_path);

    return root;
}

GNode *
fsexam_tree_construct_from_file (const gchar *filename, 
                                 FSEXAM_setting *setting)
{
    GList   *list = NULL;
    GNode   *root = NULL;

    if ((NULL == filename) || (NULL == setting))
        return NULL;

    list = g_list_prepend (list, (gpointer)filename);

    root = fsexam_tree_construct_from_list (list, setting);

    fsexam_list_free (list);

    return root;
}

GNode *
fsexam_tree_construct_from_list (GList *list, FSEXAM_setting *setting)
{
    GNode *root;

    if ((NULL == list) || (NULL == setting))
        return NULL;
    
    if ((root  = fsexam_tree_init ()) == NULL) {
        return NULL;
    }

    for (; list != NULL; list = g_list_next (list)) {
        gchar *filename = list->data;
        ++setting->passin_num;

        (void) fsexam_tree_insert_file (root, filename, setting, NULL);
    }

    if (! setting->pref->follow && ! setting->pref->no_check_symlink_content) {
        /* need separate traverse to mark symlink target */
        g_node_traverse (root,
                        G_PRE_ORDER,
                        G_TRAVERSE_ALL,
                        -1,
                        fsexam_tree_fill_target,
                        (gpointer)root);
    }

    if (fsexam_debug () & FSEXAM_DBG_TREE) {
        g_node_traverse (root, G_PRE_ORDER, 
                         G_TRAVERSE_ALL, -1, 
                         display_node, NULL);
    }

    return root;
}

/*
 * construct tree from list + history
 */
GNode *
fsexam_tree_construct_from_history (GList *list, 
                                    Hist_info *info, 
                                    FSEXAM_setting *setting)
{
    GNode *root;

    if ((NULL == list) || (NULL == setting) || (NULL == info))
        return NULL;
    
    if ((root  = fsexam_tree_init ()) == NULL) {
        return NULL;
    }

    for (; list; list = g_list_next (list)) {
        const gchar *filename = list->data;
        ++setting->passin_num;
        fsexam_tree_insert_file_for_restore (root, 
                filename, 
                setting, 
                NULL);
    }

    /* need separate traverse to mark symlink target */
    if (!setting->pref->follow && !setting->pref->no_check_symlink_content) {
        g_node_traverse (root,
                        G_PRE_ORDER,
                        G_TRAVERSE_ALL,
                        -1,
                        fsexam_tree_fill_target,
                        (gpointer)root);
    }

    if (fsexam_debug () & FSEXAM_DBG_TREE) {
        g_node_traverse (root, G_PRE_ORDER, 
                         G_TRAVERSE_ALL, -1, 
                         display_node, NULL);
    }

    return root;
}


/*
 * construct tree from dryrun
 */
GNode *
fsexam_tree_construct_from_dryrun (GSList *slist, FSEXAM_setting *setting)
{
    GNode *root = NULL;

    if ((NULL == slist) || (NULL == setting))
        return NULL;

    if ((root = fsexam_tree_init()) == NULL)
        return NULL;

    setting->passin_num = g_slist_length (slist);

    while (slist != NULL) {
        Dryrun_item *item = slist->data;
        short en_id = encoding2id (item->encoding);

        if (en_id != -1)
            fsexam_tree_insert_single_file_full (
                                root, 
                                item->path, 
                                en_id,
                                encoding2id ("UTF-8"), 
                                setting);

        slist = slist->next;
    }
 
    if (fsexam_debug () & FSEXAM_DBG_TREE) {
        g_node_traverse (root, G_PRE_ORDER, 
                         G_TRAVERSE_ALL, -1, 
                         display_node, NULL);
    }   

    return root;
}

GNode *
fsexam_tree_search_name (GNode *parent, const gchar *filename)
{
    GNode *current;

    if ((parent == NULL) || (filename == NULL)) {
        return NULL;
    }

    current = parent->children;
    while (current) {
        if (compare_filename (current, (gchar *)filename)){
            return current;
        }
        
        current = current->next;
    }
    
    return NULL;
}

GNode *
fsexam_tree_search_path (GNode *parent, const gchar *path)
{
    gchar *abs_path = NULL;
    gchar *p_abs_path = NULL;
    gchar *tmp = NULL;
    GNode *result = NULL;
    gchar subdir[PATH_MAX];

    if ((NULL == parent) || (NULL == path))
        return NULL;

    p_abs_path = abs_path = get_abs_path (path);
    
    if (NULL == abs_path) 
        return NULL;
    
    if (*(++abs_path) == '\0') {    /* skip the leading '/' */
        goto free;
    }

    tmp = subdir;
    while (TRUE) {
        *tmp++ = *abs_path++; 
        
        if ((*abs_path == '\0') || (*abs_path == '/')) {
            *tmp = '\0';            /* strlen(abs_path) < PATH_MAX */

            parent = fsexam_tree_search_name (parent, subdir);

            if (parent == NULL) {
                result = NULL;
                break;
            }else{
                result = parent;
            }
            
            tmp = subdir;           /* prepare for next subdirectory */
        
            if (*abs_path == '/') {
                abs_path++;
            } else {                /* (*abs_path == '\0') */
                break;
            }
        }
    }

free:
    g_free (p_abs_path);

    return result;
}

/* 
 * Get the real path for one node and including this node
 * return the new allocated memory which store the path Need free when no use 
 */
gchar *
fsexam_tree_get_path (GNode *node, gboolean old_path)
{
    TreeNode    *tnode = NULL;
    gchar       *result = NULL;
    gchar       *subdir = NULL;
    gchar       *tmp = NULL;

    /* traverse from node to root */
    while (node){
        tnode = node->data;

        if (!old_path && (TREENODE_IS_CONVERTED (tnode))){
            subdir = TREENODE_GET_UTF8 (tnode);
        }else{
            subdir = TREENODE_GET_ORIG (tnode);
        }

        if (strcmp (subdir, "/") == 0) {        /* root node */
            if (result == NULL){
                result = g_strdup (subdir);
            }else{
                tmp = g_strdup_printf ("%s%s", subdir, result);
                g_free (result);
                result = tmp;
                tmp = NULL;
            }
        } else {
            if (result == NULL){
                result = g_strdup (subdir);
            } else {
                tmp = g_strdup_printf ("%s/%s", subdir, result);
                g_free (result);
                result = tmp;
                tmp = NULL;
            }
        }

        node = node->parent;
    }

    return result;
}

void
fsexam_tree_destroy (GNode *root)
{
    if (NULL == root) 
        return;

    g_node_traverse (root,
                    G_IN_ORDER,
                    G_TRAVERSE_ALL,
                    -1,
                    fsexam_tree_node_data_destroy,
                    NULL);

    g_node_destroy (root);

    return;
}


/*
 *   Test function
 */
gboolean
display_node (GNode *node, gpointer data)
{
    g_return_val_if_fail (node != NULL, FALSE);

    TreeNode *tnode = node->data;
    //printf ("================== TreeNode ==============================\n");
    printf ("  recu = %d, cvted = %d, needconv = %d, trav = %d, sym = %d,"
            " special = %d, target = %p, utf8 = %s, name = %s \n", 
            TREENODE_IS_RECURSIVE (tnode), 
            TREENODE_IS_CONVERTED (tnode),
            TREENODE_IS_NEED_CONVERT (tnode),
            TREENODE_IS_TRAVERSED (tnode),
            TREENODE_IS_SYMLINK (tnode),
            TREENODE_IS_SPECIAL (tnode),
            TREENODE_GET_TARGET (tnode) ? TREENODE_GET_TARGET (tnode) 
                                        : (void *)0,
            TREENODE_GET_UTF8 (tnode) ? TREENODE_GET_UTF8 (tnode) : "NULL",
            fsexam_tree_get_path (node, TRUE));

    return FALSE;
}

gboolean
display_full_path (GNode *node, gpointer data)
{
    g_return_val_if_fail (node != NULL, FALSE);

    gchar *path = fsexam_tree_get_path (node, FALSE);

    printf ("\tfullpath = %s\n", path);

    g_free (path);

    return FALSE;
}

void
print_tree (GNode *root)
{
    g_node_traverse (root, 
                    G_PRE_ORDER, 
                    G_TRAVERSE_ALL, 
                    -1, 
                    display_node, 
                    NULL);

    return;
}
