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


/****************************************************************
 *                                                                  *
 *      Will check file use file validate module                    *
 *      This module won't check whether file is ready               *
 *                                                                  *
 *****************************************************************/

#ifndef _FSEXAM_TREE_H_
#define _FSEXAM_TREE_H_

typedef struct _TreeNode
{
    gchar *orig;                    //original name from disk
    gchar *utf8;                    //utf8 name
    GNode *target;                  //symlink target

    short           id;             //encoding ID, -1 for illegal value
    unsigned short  flags;          //error flags

    unsigned short  need_convert    : 1; //need convert this node
    unsigned short  reverse         : 1; //for restore, indicating direction
    unsigned short  special         : 1; //Special node
    unsigned short  recursive       : 1; //recursive node
    unsigned short  symlink         : 1; //current node file is symlink
    unsigned short  expand          : 1; //expanded or expanding node
    unsigned short  traversed       : 1; //been traversed? for symlink
    unsigned short  converted       : 1; //converted succ, write back to disk
}TreeNode;

#define TREEFLAG_TARGET_NOTEXIST        (1<<0)
#define TREEFLAG_UTF8_ALREADY           (1<<1)
#define TREEFLAG_REMOTE_FILE            (1<<2)
#define TREEFLAG_HIDDEN_FILE            (1<<3)

/* Getter */
#define TREENODE_GET_ORIG(tnode)        ((tnode)->orig)
#define TREENODE_GET_UTF8(tnode)        ((tnode)->utf8)
#define TREENODE_GET_ID(tnode)          ((tnode)->id)
#define TREENODE_GET_TARGET(tnode)      ((tnode)->target)
#define TREENODE_IS_RECURSIVE(tnode)    ((tnode)->recursive == 1)
#define TREENODE_IS_NEED_CONVERT(tnode) ((tnode)->need_convert == 1)
#define TREENODE_IS_TRAVERSED(tnode)    ((tnode)->traversed == 1)
#define TREENODE_IS_EXPAND(tnode)       ((tnode)->expand == 1)
#define TREENODE_IS_CONVERTED(tnode)    ((tnode)->converted == 1)
#define TREENODE_IS_REVERSE(tnode)      ((tnode)->reverse == 1)
#define TREENODE_IS_SYMLINK(tnode)      ((tnode)->symlink == 1)
#define TREENODE_IS_SPECIAL(tnode)      ((tnode)->special == 1)

/* Setter */
#define TREENODE_SET_ORIG(tnode, _orig)         ((tnode)->orig = (_orig))
#define TREENODE_SET_UTF8(tnode, _utf8)         ((tnode)->utf8 = (_utf8))
#define TREENODE_SET_ID(tnode, _id)             ((tnode)->id = (short)(_id))
#define TREENODE_SET_TARGET(tnode, _target)     ((tnode)->target = (_target))
#define TREENODE_SET_RECURSIVE(tnode, _recur)   ((tnode)->recursive = (_recur))
#define TREENODE_SET_NEED_CONVERT(tnode, _need_convert) \
                            ((tnode)->need_convert = (_need_convert))
#define TREENODE_SET_TRAVERSED(tnode, _traversed)   \
                            ((tnode)->traversed = (_traversed))
#define TREENODE_SET_EXPAND(tnode, _expand)   \
                            ((tnode)->expand = (_expand))
#define TREENODE_SET_CONVERTED(tnode, _converted)   \
                            ((tnode)->converted = (_converted))
#define TREENODE_SET_REVERSE(tnode, _reverse)       \
                            ((tnode)->reverse = (_reverse))
#define TREENODE_SET_SYMLINK(tnode, _symlink)   ((tnode)->symlink = (_symlink))
#define TREENODE_SET_SPECIAL(tnode, _special)   ((tnode)->special = (_special))

/* Flags macro */
#define TREENODE_SET_FLAGS(tnode, _flag)        ((tnode)->flags |= (_flag))
#define TREENODE_HAS_FLAGS(tnode)               ((tnode)->flags != 0)
#define TREENODE_FLAG_TARGET_NOTEXIST(tnode)    \
                            ((tnode)->flags & TREEFLAG_TARGET_NOTEXIST)
#define TREENODE_FLAG_UTF8_ALREADY(tnode)       \
                            ((tnode)->flags & TREEFLAG_UTF8_ALREADY)
#define TREENODE_FLAG_REMOTE_FILE(tnode)        \
                            ((tnode)->flags & TREEFLAG_REMOTE_FILE)
#define TREENODE_FLAG_HIDDEN_FILE(tnode)        \
                            ((tnode)->flags & TREEFLAG_HIDDEN_FILE)

/*==================================================================
 *  Function Name:  fsexam_node_new
 *
 *  Parameters:
 *      const gchar *filename: contain only basename, no subpath. 
 *      gboolean: recursive:  recursive handle directory or not
 *      goolean: need_convert: whether this node need convert or not
 *
 *  Desc:
 *      Create one new TreeNode according to filename and flag.
 *      This function won't check whether filename exist or not. caller do this.
 *
 *  Return value:
 *      TreeNode pointer when succeed. Need call fsexam_node_destroy() to 
 *      free mem.  Otherwise return NULL.
 *
 *  Author:     Yandong Yao 2006/08/24
 =================================================================*/
TreeNode *fsexam_node_new           (const gchar *filename, 
                                     gboolean recursive, 
                                     gboolean need_convert);
void     fsexam_node_destroy (TreeNode *node);


/*
 * Init the whole tree with only one node for "/"
 */
GNode *fsexam_tree_init ();
void fsexam_tree_destroy (GNode *root);

GNode *fsexam_tree_construct_from_list (GList *list, FSEXAM_setting *setting);
GNode *fsexam_tree_construct_from_single_file (const gchar *filename);
GNode *fsexam_tree_construct_from_file (const gchar *filename, 
                                        FSEXAM_setting *setting);
GNode *fsexam_tree_construct_from_dryrun (GSList *slist, 
                                        FSEXAM_setting *setting);
GNode *fsexam_tree_construct_from_history (GList *list, 
                                        Hist_info *info, 
                                        FSEXAM_setting *setting);

/* 
 * Search whether parent has child with filename or not
 */
GNode *fsexam_tree_search_name (GNode *parent,  const gchar *filename);

/*
 * Search whether path has been added into tree
 */
GNode *fsexam_tree_search_path (GNode *root, const gchar *path);

/*
 * Get the full path of one node
 */
gchar *fsexam_tree_get_path (GNode *node, gboolean old_path);


/*==================================================================
 *      Function for test purpose
 ==================================================================*/
gboolean display_node (GNode *node, gpointer data);
gboolean display_full_path (GNode *node, gpointer data);
void     print_tree (GNode *root);

#endif //_FSEXAM_TREE_H_
