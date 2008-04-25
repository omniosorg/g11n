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


#ifndef _FSEXAM_UI_H_
#define _FSEXAM_UI_H_

#define FSEXAM_ROOT_DIR "/"

enum
{
    ICON_COLUMN = 0,
    DISPLAYNAME_COLUMN,
    FILENAME_COLUMN,
    LOADED_COLUMN,
    NUM_COLUMNS
};

typedef struct _FSEXAM_view FSEXAM_view;
struct _FSEXAM_view
{
    FSEXAM_setting  *setting;

    GHashTable      *pixbuf_hash;
    GHashTable      *treepath_hash;     /* update GUI after conversion */
    GSList          *undo_list;

    /* gui pointer */
    GtkWidget       *mainwin;
    GtkWidget       *peekwin;
    GtkWidget       *focus_treeview;    /* The focused one of 2 file treeview */
    GtkWidget       *popup_menu;

    /* rootdir maybe "", "/tmp/..", "file:///tmp/..." */
    gchar           *rootdir;           /* rootdir of file Treeview*/
    gint            lineoffset;         /* for report pane */

    gchar           *basedir;           /* basedir for searching */
    GPid            pid;                /* for searching pid */
};

extern FSEXAM_view *view;

FSEXAM_view *fsexam_view_new ();
void        fsexam_view_destroy (FSEXAM_view *);

void        fsexam_change_dir (const gchar *dir);
gchar       *fsexam_file_chooser_get_name (const gchar *, 
                        GtkFileChooserAction, 
                        gboolean, 
                        gboolean);

void        fsexam_construct_ui (const gchar *);
gboolean    fsexam_treeview_construct (const gchar *);
void        fsexam_statusbar_update (gchar *);

gchar       *fsexam_filename_get_fullname (GtkTreeModel *model, 
                                           GtkTreeIter *iter);

gboolean    fsexam_gui_show_yesno_dialog (GtkWindow *parent, 
                                       const gchar *msg_format, ...);
void        fsexam_gui_show_dialog (GtkWindow *, 
                                GtkMessageType type, 
                                const gchar *messages, ...);

void        fsexam_gui_set_initial_state (void);
void        fsexam_gui_display_msg (const gchar *filename, const gchar *msg);
void        fsexam_gui_display_stats (FSEXAM_setting *setting);
int         fsexam_gui_get_index (GList *encoding_list, gboolean forname);
void        fsexam_gui_update (FSEXAM_setting *setting, 
                               const gchar *path,
                               const gchar *oldname, 
                               const gchar *newname);

GladeXML    *fsexam_gui_load_glade_file (const gchar *filename, 
                                          const gchar *widget_root, 
                                          GtkWindow *error_dialog_parent);

GtkWidget   *fsexam_gui_get_focused_treeview ();
void        fsexam_content_peek (gint x, gint y);


/* --- search result pane --- */
void        fsexam_search_treeview_append_file (const gchar *filename, 
                                                gboolean first_file);
void        fsexam_search_treeview_append_list (GList *list);
void        fsexam_search_treeview_show (void);
void        fsexam_search_treeview_hide (void);

#endif  //_FSEXAM_UI_H_
