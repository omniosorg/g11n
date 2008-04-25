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

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glade/glade.h>

#include "fsexam.h"
#include "fsexam-header.h"
#include "encoding.h"
#include "fsexam-history.h"
#include "fsexam-ui.h"
#include "callbacks.h"
#include "fsexam-dialog.h"
#include "fsexam-dnd.h"

FSEXAM_view *view;      // global variable 

typedef struct _TreeItem TreeItem;
struct _TreeItem
{
    GdkPixbuf   *icon;  // file icon
    gchar       *label;     // filename
    gboolean    loaded;  // meaningful only for directory
    TreeItem    *children;
};

static gchar    *get_root_dir_name (const gchar *dir);
static void     set_root_dir (const gchar *dir);
static void     fsexam_treeitem_free (TreeItem *);
static gint     get_dir_elements (DIR *dirp);
static void     fsexam_treeview_set_title (const gchar *);
static gint     set_item_collation (gconstpointer a, gconstpointer b);
static void     treemodel_create_with_treeitem (
                            GtkTreeStore *, 
                            TreeItem *, 
                            GtkTreeIter, 
                            gboolean);
static gchar    *get_last_path (GtkTextBuffer *, GtkTextIter *);
static void     common_construct_ui ();
static GString  *fsexam_filename_get_path (GtkTreeModel *, GtkTreeIter);

static GdkPixbuf    *get_file_pixbuf (const gchar *);
static TreeItem     *fsexam_treeitem_create (const gchar *);
static TreeItem     *fsexam_treeitem_create_from_list (GList *);
static GtkTreeModel *fsexam_treemodel_create_with_treeitem (TreeItem *root);

static void
mainwin_destroy (GtkObject *object, gpointer user_data)
{
    force_quit = TRUE;
    fsexam_cleanup_all ();

    gtk_main_quit ();

    return;
}

static gboolean
mainwin_delete (GtkObject *object, gpointer user_data)
{
    return FALSE;   /* return TRUE will not send 'destroy' signal */
}

/*
 *  dir can be NULL, ""(for file list), and abs_path
 */
static void
set_root_dir (const gchar *path)
{
    gchar    *uri = NULL;
    gchar    *dir = (gchar *)path;

    g_free (view->rootdir);     //free old rootdir first

    if (dir == NULL) {
        view->rootdir = NULL;
        return;
    }

    if (! g_file_test (path, G_FILE_TEST_IS_DIR)) {
        dir = g_path_get_dirname (path);

        if ((*dir == '\0') || (g_utf8_validate (dir, -1, NULL))) {
            view->rootdir = g_strdup (dir);
            return;
        }

        uri = g_filename_to_uri (dir, NULL, NULL);
        view->rootdir = uri;

        g_free (dir);
    }else{
        if ((*dir == '\0') || (g_utf8_validate (dir, -1, NULL))) {
            view->rootdir = g_strdup (dir);
            return;
        }

        uri = g_filename_to_uri (dir, NULL, NULL);
        view->rootdir = uri;
    }

    return;
} 

/*
 * Get the root dirname, may need convert from uri.
 * free when don't need again.
 */
static gchar *
get_root_dir_name (const gchar *rootdir)
{
    if (NULL == rootdir)
        return NULL;

    if (*rootdir == 'f')  /* file:///... */
        return g_filename_from_uri (rootdir, NULL, NULL);
    else
        return g_strdup (rootdir);
}

static gboolean
button_press (GtkWidget      *widget,
              GdkEventButton *button,
              gpointer       data)
{
    g_return_val_if_fail (button != NULL, FALSE);

    if (button->button == 1 && button->type == GDK_2BUTTON_PRESS) {
        cb_convert ();

        return TRUE;
    }else if (button->button == 2) {
        fsexam_content_peek (button->x_root, button->y_root);

        return TRUE;
    }else if (button->button == 3) {
        GtkWidget *menu = view->popup_menu;
        GtkWidget *w = NULL;
        gboolean  sensitive;

        /* undo menu */
        w = g_object_get_data (G_OBJECT (menu), "popup_undo");
        gtk_widget_set_sensitive (w, view->undo_list ? TRUE : FALSE);

        /* conversion mode menu */
        w = g_object_get_data (G_OBJECT (menu), 
                view->setting->pref->conv_content ? "popup_content_mode"
                                                  : "popup_name_mode");
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (w), TRUE);

        /* scenario menu */
        w = g_object_get_data (G_OBJECT (view->mainwin), "menu_scenario");
        sensitive = GTK_WIDGET_IS_SENSITIVE (w);
        w = g_object_get_data (G_OBJECT (menu), "popup_scenario");
        gtk_widget_set_sensitive (w, sensitive);
        //gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (w), sensitive);

        /* display the popup menu */
        gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 
                button->button, button->time);

        return TRUE;
    }

        return FALSE;
}

static gboolean
button_release (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    if (view->peekwin && event->button == 2) {
        gtk_object_destroy (GTK_OBJECT (view->peekwin));
        view->peekwin = NULL;
    }

    return FALSE;
}

static gboolean
key_press (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    switch (event->keyval) {
        case GDK_space:
            cb_convert ();
            return TRUE;
       /* case GDK_F12:
            if (event->state == GDK_SHIFT_MASK) {
                printf ("shitf+F12\n");
            }
            printf ("F12\n");
            return TRUE; */
        default:
            break;
    }

    return FALSE;
}

/*
 * Search give string at the same level with given iter
 */
static GtkTreeIter *
search_model_at_same_level (GtkTreeModel *model,
                            GtkTreeIter *iter,
                            const gchar *string)
{
    if ((NULL == string) || (NULL == iter))
        return NULL;
    
    do {
        gchar *name = NULL;
        gchar *internal_name = NULL;    /* escaped name */

        /* get the escaped name */
        gtk_tree_model_get (model, iter, FILENAME_COLUMN, &internal_name, -1);

        /* unescaped the escaped name */
        name = g_strcompress (internal_name);
        g_free (internal_name);

        if (strcmp (name, string) == 0) {
            g_free (name);      /* found */

            return gtk_tree_iter_copy (iter);
        }

        g_free (name);
    } while (gtk_tree_model_iter_next (model, iter));

    return NULL;
}

/*
 * Update GtkTreeView which constructed from file list.
 * The underlying store is GtkListStore
 */
static void 
update_gui_for_filelist (GtkTreeModel *model, 
                        const gchar *oldname, 
                        const gchar *newname,
                        GtkTreePath *treepath,
                        gboolean    is_myself)
{
    GtkTreeIter iter;
    gint        old_name_len;

    if ((NULL == model) || (NULL == oldname) 
            || (NULL == newname) || (NULL == view->basedir))
        return;

    if (! fsexam_is_subpath (oldname, view->basedir))
        return;

    if (! gtk_tree_model_get_iter_first (model, &iter)) {
        return;
    }

    old_name_len = strlen (oldname);

    do {
        gchar *name = NULL;
        gchar *internal_name = NULL;
        gchar *tmp = NULL;
        
        /* get the filename from TreeView */
        gtk_tree_model_get (model, &iter, FILENAME_COLUMN, &internal_name, -1);
        name = g_strcompress (internal_name);
        g_free (internal_name);

        /* oldname is prefix path of name ? */
        if (g_str_has_prefix (name, oldname)) {
            tmp = name + old_name_len;

            /* construct new name */
            if (*tmp == '\0') {
                gchar *display_name = NULL;
                gchar *internal_name = NULL;

                internal_name = g_strescape (newname, "");
                display_name = fsexam_filename_display_basename (newname);

                gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        FILENAME_COLUMN, internal_name,
                        DISPLAYNAME_COLUMN, display_name,
                        -1);

                g_free (display_name);
                g_free (internal_name);
            }else if (*tmp == '/') {
                gchar *new_full_name;
                gchar *display_name;
                gchar *internal_name;

                new_full_name = g_strdup_printf ("%s/%s", newname, tmp + 1);
                display_name = fsexam_filename_display_basename (new_full_name);
                internal_name = g_strescape (new_full_name, "");

                gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        FILENAME_COLUMN, internal_name,
                        DISPLAYNAME_COLUMN, display_name,
                        -1);

                g_free (new_full_name);
                g_free (display_name);
                g_free (internal_name);
            }
        }

        g_free (name);
    } while (gtk_tree_model_iter_next (model, &iter));

    return;
}

/*
 *  Update treeview_file
 *
 *  is_myself is TRUE if conversion are caused by dir/file TreeView
 *  is_myself is FALSE if conversion are caused by search result Treeview 
 */
static void
update_gui_for_dir (GtkTreeModel *model,
                    const gchar *path,
                    const gchar *oldname,
                    const gchar *newname,
                    GtkTreePath *treepath,
                    gboolean    is_myself)
{
    GtkTreeIter iter;
    GtkTreeIter *match_iter = NULL;
    gchar       *rootdir = NULL;
    gchar       *remaining = NULL;
    gchar       *display_name = NULL;
    gchar       *internal_name = NULL;
    gchar       *path_clone = NULL;
    gint        len;

    display_name = fsexam_filename_display_name (newname);
    internal_name = g_strescape (newname, "");

    if (is_myself && treepath != NULL) {
        gtk_tree_model_get_iter (model, &iter, treepath);
        gtk_tree_store_set (GTK_TREE_STORE (model),
                            &iter,
                            FILENAME_COLUMN, internal_name,
                            DISPLAYNAME_COLUMN, display_name,
                            -1);

        goto CLEAN_UP;
    }

    rootdir = get_root_dir_name (view->rootdir);

    if (! g_str_has_prefix (path, rootdir)) 
        goto CLEAN_UP;

    path_clone = g_strdup (path);
    len = strlen (rootdir);
    remaining = path_clone + len;

    if (! gtk_tree_model_get_iter_first (model, &iter)) {
        goto CLEAN_UP;     /* empty tree */
    }

    if (*remaining == '\0') {   /* top level */
        match_iter = search_model_at_same_level (model,
                                                 &iter,
                                                 oldname);
    
        if (match_iter == NULL)
            goto CLEAN_UP;
    
        gtk_tree_store_set (GTK_TREE_STORE (model), match_iter,
                            FILENAME_COLUMN, internal_name,
                            DISPLAYNAME_COLUMN, display_name,
                            -1);
    
        gtk_tree_iter_free (match_iter);
    }else if (*remaining == '/') {  /* iterate each directory level */
        gchar       *cur = ++remaining; //skip the '/'
        gboolean    last_element;
        gboolean    loaded;

        while (TRUE) {
            if ((*remaining != '\0') && (*remaining != '/'))  {
                ++remaining;
                continue;
            }
               
            last_element = (*remaining == '\0') ? TRUE : FALSE;
            *remaining = '\0';
            match_iter = search_model_at_same_level (model,
                                                     &iter,
                                                     cur);
            if (match_iter == NULL)
                break;

            if (! gtk_tree_model_iter_children (model, &iter, match_iter)) {
                gtk_tree_iter_free (match_iter);    /* no children */
                break;
            }

            /* now iter is the first child of match_iter */
            gtk_tree_model_get (model, match_iter,
                                LOADED_COLUMN, &loaded,
                                -1);

            gtk_tree_iter_free (match_iter);

            if (! loaded) {
                break;
            }else if (last_element) {
                match_iter = search_model_at_same_level (model, 
                                            &iter, oldname);

                if (match_iter != NULL) {
                    gchar *display_name;

                    display_name = fsexam_filename_display_name (newname);
                    gtk_tree_store_set (
                            GTK_TREE_STORE (model), match_iter,
                            FILENAME_COLUMN, internal_name,
                            DISPLAYNAME_COLUMN, display_name,
                            -1);

                    gtk_tree_iter_free (match_iter);
                    g_free (display_name);
                }

                break;
            }

            cur = ++remaining;
        }
    }

CLEAN_UP:
    g_free (rootdir);
    g_free (display_name);
    g_free (internal_name);
    g_free (path_clone);

    return;
}

/*
 *  Get the last path of GtkTextBuffer of report pane
 */
static gchar *
get_last_path (GtkTextBuffer *buffer, GtkTextIter *iter)
{
    gchar   *text = NULL;
    gint    line;

    line = gtk_text_iter_get_line (iter) - 1;

    while (line >= 0) {
        GtkTextIter start, end;
        gtk_text_buffer_get_iter_at_line_offset (buffer, &start, line, 0);
        gtk_text_buffer_get_iter_at_line_offset (buffer, &end, line + 1, 0);
        text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

        if (*text != '\n' && *text != '\t') /* info line */
            break;

        --line;
        g_free (text);
    }

    if (text == NULL)
        return NULL;

    *(text + strlen (text) - 1) = '\0';     /* remove '\n' */

    if (strncmp (text, "file:///", 8) == 0) {
        gchar *tmp = g_filename_from_uri (text, NULL, NULL);
        g_free (text);
        text = tmp;
    }

    return text;
}

// dummy TreeItem
static TreeItem demo_toplevel[] =
{
    { NULL, "", TRUE, NULL},
    { NULL }
};

// copy from gsearchtool to load the pixbuf

#define ICON_THEME_EXECUTABLE_ICON "gnome-fs-executable"
#define ICON_THEME_REGULAR_ICON    "gnome-fs-regular"
#define ICON_THEME_CHAR_DEVICE     "gnome-fs-chardev"
#define ICON_THEME_BLOCK_DEVICE    "gnome-fs-blockdev"
#define ICON_THEME_SOCKET          "gnome-fs-socket"
#define ICON_THEME_FIFO            "gnome-fs-fifo"
#define ICON_THEME_SYMLINK         "emblem-symbolic-link"
#define ICON_SIZE                  24

#if 0
static GdkPixbuf *
get_file_pixbuf (const gchar *file)
{
    GdkPixbuf *pixbuf = NULL;
    gchar      *icon_name = NULL;
    gchar      *escape_path = NULL;
    GnomeVFSFileInfo    *vfs_file_info = NULL;

    vfs_file_info = gnome_vfs_file_info_new ();
    escape_path = gnome_vfs_escape_path_string (file);

    gnome_vfs_get_file_info (escape_path, vfs_file_info,
                            GNOME_VFS_FILE_INFO_DEFAULT | 
                            GNOME_VFS_FILE_INFO_GET_MIME_TYPE |
                            GNOME_VFS_FILE_INFO_FORCE_SLOW_MIME_TYPE);

#endif 

static GdkPixbuf *
get_file_pixbuf (const gchar *file)
{
    GdkPixbuf   *pixbuf = NULL;
    gchar       *icon_name = NULL;
    const gchar *mime_type;

    mime_type = gnome_vfs_get_file_mime_type (file, NULL, FALSE);

    if (g_file_test (file, G_FILE_TEST_IS_SYMLINK)) {
        icon_name = g_strdup (ICON_THEME_SYMLINK);
    } else if (file == NULL || mime_type == NULL) {
        icon_name = g_strdup (ICON_THEME_REGULAR_ICON);
    } else if ((g_file_test (file, G_FILE_TEST_IS_EXECUTABLE)) &&
           !g_ascii_strcasecmp (mime_type, "application/x-executable-binary")) {
        icon_name = g_strdup (ICON_THEME_EXECUTABLE_ICON);
    } else if (!g_ascii_strcasecmp (mime_type, "x-special/device-char")) {
        icon_name = g_strdup (ICON_THEME_CHAR_DEVICE);
    } else if (!g_ascii_strcasecmp (mime_type, "x-special/device-block")) {
        icon_name = g_strdup (ICON_THEME_BLOCK_DEVICE);
    } else if (!g_ascii_strcasecmp (mime_type, "x-special/socket")) {
        icon_name = g_strdup (ICON_THEME_SOCKET);
    } else if (!g_ascii_strcasecmp (mime_type, "x-special/fifo")) {
        icon_name = g_strdup (ICON_THEME_FIFO);
    } else {
      icon_name = gnome_icon_lookup (
                        gtk_icon_theme_get_default (), 
                        NULL, file,
                        NULL, NULL, 
                        mime_type, 0, 
                        NULL);
    }

    if (icon_name == NULL)
        return NULL;

    pixbuf = (GdkPixbuf *)g_hash_table_lookup (view->pixbuf_hash, icon_name);

    if (pixbuf == NULL) {
        pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), 
                                           icon_name,
                                           ICON_SIZE, 0, NULL);

        /* free icon_name when free view->pixbuf_hash */
        if (pixbuf != NULL)
            g_hash_table_insert (view->pixbuf_hash,
                    g_strdup (icon_name),
                    pixbuf);
    }
        
    g_free (icon_name);

    return pixbuf;
}

/*
 * free TreeItem and the memory it points to
 */
static void
fsexam_treeitem_free (TreeItem *item)
{
    TreeItem *item2 = item;

    // As one place holder, demo_toplevel isn't dynamically
    // generated, we can't free it
    if (NULL == item || item == demo_toplevel) return;

    while (TRUE) {
        if (item->label == NULL) /* all remaining TreeItem contain no data */
            break;
        
        g_free (item->label);

        if (item->icon != NULL)
            g_object_unref (item->icon);

        if (item->children)
            fsexam_treeitem_free (item->children);

        ++item;
    }

    g_free (item2);
}

/*
 * Get the number of elements in given directory
 */
static gint
get_dir_elements (DIR *dirp)
{
    struct dirent *dp = NULL;
    gint   num_elements = 0;

    if (dirp == NULL)
        return 0;

    while ((dp = readdir (dirp)) != NULL) {
        ++num_elements;
    }

    rewinddir (dirp);

    return num_elements;
}

/*
 * Change the title of treeview to indicate the current root directory
 * that treeview is based on.
 */
static void
fsexam_treeview_set_title (const gchar *title)
{
    GtkWidget         *w;
    
    w = g_object_get_data (G_OBJECT (view->mainwin), "entry_folder");
    gtk_entry_set_text (GTK_ENTRY (w), title);
    
    return;
}

/*
 * TODO: collation for non-utf8
 */
static gint
set_item_collation (gconstpointer a, gconstpointer b)
{
    TreeItem *item_a = (TreeItem *)a;
    TreeItem *item_b = (TreeItem *)b;
    gint     retval;

    if (!item_a->label || !item_b->label) return -1;

    // put directory ahead
    if (item_a->children && !item_b->children)
      return -1;
    if (!item_a->children && item_b->children)
      return 1;

    retval = g_strcasecmp (item_a->label, item_b->label);

    return (retval == 0 ?  (strcmp (item_a->label, item_b->label) > 0 ? -1 : 1)
                        : retval);
}

/*
 *  Create TreeItem list from given directory
 */
static TreeItem *
fsexam_treeitem_create (const gchar *dir)
{
    struct stat stat_buf;
    struct dirent *dp = NULL;
    TreeItem     *tree = NULL;
    TreeItem     *item = NULL;
    DIR          *dirp;
    gint         count;
    gchar        *msg = NULL;
    gint         filenum = 0;
    
    if (NULL == dir || lstat (dir, &stat_buf) == -1) 
        goto _ERR;

    if (! S_ISDIR (stat_buf.st_mode)) {
        tree = g_new0 (TreeItem, 2);
        tree->icon = get_file_pixbuf (dir);
        tree->label = g_path_get_basename (dir);
        tree->loaded = 1;

        return tree;
    }

    dirp = opendir (dir);
    if (dirp == NULL) 
        goto _ERR;

    filenum = get_dir_elements (dirp);  //contain '.' and '..'
    tree = g_new0 (TreeItem, filenum);
    count = 0;

    while (((dp = readdir (dirp)) != NULL)) {
        gchar    *name = NULL;
        
        if ((strcmp (dp->d_name, ".") == 0) 
                || (strcmp (dp->d_name, "..") == 0)) 
            continue;

        name = g_strdup_printf ("%s/%s", dir, dp->d_name); 

        item = tree + count;
        item->label = g_strdup (dp->d_name);
        item->loaded = 1;
        item->icon = get_file_pixbuf (name);

        if (lstat (name, &stat_buf) == 0 && S_ISDIR (stat_buf.st_mode)) {
             // mark it is unloaded yet and put demo_toplevel as its children
             // to act as one placeholder
             item->loaded = 0;
             item->children = demo_toplevel;
        }
      
        g_free (name);  
        ++count;
    }

    closedir (dirp);

    qsort (tree, count, sizeof (TreeItem), set_item_collation);

    return tree;

 _ERR:
    switch (errno) {
        case EACCES:
            msg = g_strdup_printf (_("No read permission"));
            break;
        case ENOENT:
            msg = g_strdup_printf (_("File doesn't exist"));
            break;
    }
    
    if (!dir) 
        msg = g_strdup_printf (_("No folder specified"));

    if (msg) {
        GtkWidget *notebook = NULL;

        /* show conversion log notebook page */
        notebook = g_object_get_data (G_OBJECT (view->mainwin), 
                "notebook_report");
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);

        fsexam_statusbar_update (msg);
        fsexam_gui_display_msg (dir, msg);
        g_free (msg);
    }

    return demo_toplevel;
}

/*
 * Create TreeItem list from file list
 */
static TreeItem *
fsexam_treeitem_create_from_list (GList *files)
{
    TreeItem        *tree = NULL;
    TreeItem        *item = NULL;
    struct stat     buf;
    GList           *current;

    tree = g_new0 (TreeItem, g_list_length (files) + 1);
    item = tree;
    current = files;

    while (current != NULL) {
        gchar *filename = current->data;

        current = g_list_next (current);

        if ((lstat (filename, &buf) < 0)) {
            fsexam_gui_display_msg (filename, 
                        _("File does not exist or no access permission."));

            continue;
        }else if (!(S_ISREG(buf.st_mode)) 
                && !(S_ISDIR(buf.st_mode)) 
                && !(S_ISLNK(buf.st_mode))) {
            fsexam_gui_display_msg (filename, _("File type is not supported."));
            continue;
        }
        
        item->label = get_abs_path (filename);
        item->loaded = 1;
        item->icon = get_file_pixbuf (filename);

        item ++;
    }

    return tree;
}

/* create GtkTreeModel from TreeItem list */
static void
treemodel_create_with_treeitem (GtkTreeStore *model,
                                TreeItem *item,
                                GtkTreeIter parent_iter,
                                gboolean is_root)
{
    GtkTreeIter iter;
    gchar       *display_name;
    gchar       *internal_name;

    if (NULL == item)
        return;

    while (item->label != NULL) {
        TreeItem *subitem = item->children;
        display_name = fsexam_filename_display_name (item->label);
        internal_name = g_strescape (item->label, "");

        if (is_root)
            gtk_tree_store_append(model, &iter, NULL);
        else
            gtk_tree_store_append(model, &iter, &parent_iter);

        gtk_tree_store_set (model, &iter,
                            ICON_COLUMN, item->icon,
                            FILENAME_COLUMN, internal_name,
                            DISPLAYNAME_COLUMN, display_name,
                            LOADED_COLUMN, item->loaded,
                            -1);

        if (subitem)
            treemodel_create_with_treeitem(model, subitem, iter, FALSE);

        g_free (display_name);
        g_free (internal_name);
        item++;
    }

    return;
}

/*
 * Concatenate each part to generate its full path name
 * The returned GString should be freed with g_string_free ()
 *
 * please be sure that the third param is view->rootdir
 */
static GString *
fsexam_filename_get_path (GtkTreeModel *model, GtkTreeIter iter)
{
    GtkTreeIter parent;
    GString     *dir;
    gchar       *fullpath = get_root_dir_name (view->rootdir);

    dir = g_string_new (NULL);

    while (gtk_tree_model_iter_parent (model, &parent, &iter)) {
        gchar *filename = NULL;
        gchar *internal_name = NULL;

        gtk_tree_model_get (model, &parent,
                            FILENAME_COLUMN, &internal_name,
                            -1);
       
        filename = g_strcompress (internal_name);
        dir = g_string_prepend (dir, filename);
        dir = g_string_prepend (dir, "/");

        g_free (filename);
        g_free (internal_name);
        iter = parent; 
    }

    if (fullpath) {
        // if it ends with '/', modify it to avoid two consecutive slashes
        gchar *ch = fullpath + strlen (fullpath) - 1;

        if (*ch == '/') 
            *ch = 0;

        dir = g_string_prepend (dir, fullpath);
    }

    g_free (fullpath);

    return dir;
}

/*
 * Concatenate each part to generate its full file name
 * The returned GString should be freed with g_string_free ()
 */
gchar *
fsexam_filename_get_fullname (GtkTreeModel *model, GtkTreeIter *iter)
{
    GString     *fullname = NULL;
    gchar       *filename = NULL;
    gchar       *internal_name = NULL;

    gtk_tree_model_get (model, iter,              
                        FILENAME_COLUMN, &internal_name,
                        -1);

    filename = g_strcompress (internal_name);
    g_free (internal_name);

    if (*filename == '/') {     /* search result treeview */
        return filename;
    }

    /* upper-left corner treeview, not search result treeview */
    fullname = fsexam_filename_get_path (model, *iter);
    fullname = g_string_append (fullname, "/");
    fullname = g_string_append (fullname, filename);

    g_free (filename);
    filename = g_string_free (fullname, FALSE);

    return filename;
}



/*
 * Check if it is one directory which hasn't been loaded yet,
 * if yes, load its files and add into model.
 * If permission issue exists, collapse the directory row and
 * write error message to statusbar. 
 */
gboolean
fsexam_treeview_expand (GtkWidget *widget,
                        GtkTreeIter *iter,
                        GtkTreePath *path,
                        gpointer user_data)
{
    GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));
    gboolean    loaded;
    gchar       *filename = NULL;
    gchar       *internal_name = NULL;

    gtk_tree_model_get (model, iter,              
                        FILENAME_COLUMN, &internal_name,
                        LOADED_COLUMN, &loaded,
                        -1);

    filename = g_strcompress (internal_name);
    g_free (internal_name);

    if (!loaded) {
        GtkTreeIter dummy_iter;
        TreeItem    *item = NULL;
        GString     *dir = NULL;
        gchar       *name = NULL;
        
        dir = fsexam_filename_get_path (model, *iter);
        name = g_strdup_printf ("%s/%s", dir->str, filename);

        item = fsexam_treeitem_create (name);

        if (item == demo_toplevel) {
            fsexam_statusbar_update (_("No read permission"));
            gtk_tree_view_collapse_row (GTK_TREE_VIEW (widget), path);
        } else if (!item->label) {
            fsexam_statusbar_update (_("Empty folder"));
            gtk_tree_view_collapse_row (GTK_TREE_VIEW (widget), path);
        } else {
            treemodel_create_with_treeitem (GTK_TREE_STORE (model), 
                                            item,
                                            *iter, 
                                            FALSE);

            // change it's status
            gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                                LOADED_COLUMN, TRUE,
                                -1);

            // dummy_iter will be the first child of iter.
            // it is demo_toplevel, need remove it
            gtk_tree_model_iter_children (model, &dummy_iter, iter);
            gtk_tree_store_remove (GTK_TREE_STORE (model), &dummy_iter);
        }

        g_string_free (dir, TRUE);
        free(name);  
        fsexam_treeitem_free (item); 
    }

    g_free (filename);

    return TRUE;
}

/*
 * Create GtkTreeModel using given TreeItem list
 */
static GtkTreeModel *
fsexam_treemodel_create_with_treeitem (TreeItem *root)
{
    GtkTreeStore     *model;
    GtkTreeIter      iter;
    
    if (root == NULL) 
        return NULL;
 
    model = gtk_tree_store_new (NUM_COLUMNS,
                                GDK_TYPE_PIXBUF,
                                G_TYPE_STRING,
                                G_TYPE_STRING,
                                G_TYPE_BOOLEAN);

    treemodel_create_with_treeitem (model, root, iter, TRUE);

    return GTK_TREE_MODEL (model);
}

/* 
 * create GtkTreeView from single directory 
 *
 * dir need to be absolute path
 */
gboolean
fsexam_treeview_construct (const gchar *dir)
{
    TreeItem         *toplevel;
    GtkWidget        *widget;
    GtkTreeModel     *model;
    GtkTreeSelection *selection;
    GtkTreeIter      iter;
    gchar            *abs_path = NULL;

    if (dir == NULL) 
        return FALSE;

    if (g_file_test (dir, G_FILE_TEST_IS_SYMLINK)) {
        abs_path = get_abs_path_for_symlink_target (dir);
    } else {
        abs_path = get_abs_path (dir);
    }

    if (abs_path == NULL) {
        fsexam_gui_display_msg (dir,
                g_file_test (dir, G_FILE_TEST_IS_SYMLINK) ? 
                _("Symlink target doesn't exist") : 
                _("doesn't exist"));
        fsexam_gui_display_stats (view->setting);
   
        return FALSE;
    }

    /* Create TreeItem from directory name */
    if ((toplevel = fsexam_treeitem_create (abs_path)) == demo_toplevel) {
        g_free (abs_path);
        return FALSE;
    }

    set_root_dir (abs_path);

    widget = g_object_get_data (G_OBJECT (view->mainwin), "treeview_file");
    
    /* Create GtkTreeModel from TreeItem list */
    model = fsexam_treemodel_create_with_treeitem (toplevel);
    gtk_tree_view_set_model (GTK_TREE_VIEW (widget), model);
    fsexam_treeitem_free (toplevel);

    /* select the first row by default */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
    if (gtk_tree_model_get_iter_first (model, &iter)) 
        gtk_tree_selection_select_iter (selection, &iter);

    if (view->rootdir) 
        fsexam_treeview_set_title (view->rootdir);

    g_free (abs_path);

    return TRUE;
}

/* create GtkTreeView from multiple files */
void
fsexam_treeview_construct_from_list (GList *list)
{
    TreeItem        *toplevel;
    GtkWidget       *widget;
    GtkTreeModel    *model;
    GtkTreeIter     iter;
    GtkTreeSelection *selection;

    if (NULL == list)
        return;

    g_free (view->rootdir);
    view->rootdir = g_strdup (FSEXAM_ROOT_DIR);  
    fsexam_treeview_set_title (view->rootdir);

    widget = g_object_get_data (G_OBJECT (view->mainwin), "treeview_file");
    toplevel = fsexam_treeitem_create_from_list (list);
    model = fsexam_treemodel_create_with_treeitem (toplevel);
    gtk_tree_view_set_model (GTK_TREE_VIEW (widget), model);
    fsexam_treeitem_free (toplevel);

    /* select the first row by default */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
    gtk_tree_model_get_iter_first (model, &iter);
    gtk_tree_selection_select_iter (selection, &iter);

    return;
}

void
fsexam_statusbar_update (gchar *msg)
{
    GtkWidget *statusbar;

    statusbar = g_object_get_data (G_OBJECT (view->mainwin), "appbar");
    gnome_appbar_pop (GNOME_APPBAR (statusbar));
    gnome_appbar_push (GNOME_APPBAR(statusbar), msg);

    return;
}

/*
 * Clear statusbar
 */
gboolean 
fsexam_statusbar_fresh ()
{
    fsexam_statusbar_update ("");

    return TRUE;
}

/*
 * Get the filename from GtkFileChooser
 *
 * Ask to convert to UTF-8 if filename is not UTF-8 encoded.
 */
gchar *
fsexam_file_chooser_get_name (const gchar *title, 
                              GtkFileChooserAction action, 
                              gboolean no_check, 
                              gboolean ensure_utf8)
{
    GtkWidget   *dialog;
    gchar       *filename = NULL;
    const gchar *emp_msg = N_("The selected file is not empty, do you want to override it?");
    const gchar *nonutf8_msg = N_("The selected file is not UTF-8 encoded, we strongly suggest you to use UTF-8 filename.\n\n Do you want to convert it to UTF8?");
    const gchar *converr_msg = N_("Can't convert %s to UTF8: %s.\n\nPlease Note that there may exist partial conversion.\n So please Refresh you directory and select file again.");


    dialog = gtk_file_chooser_dialog_new (title,
                            GTK_WINDOW (view->mainwin),
                            action,   //GTK_FILE_CHOOSER_ACTION_OPEN,
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                            NULL);

    while (TRUE) {
        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
            filename = gtk_file_chooser_get_filename (
                            GTK_FILE_CHOOSER (dialog));

            if (filename == NULL)
                continue;
            if (no_check)   /* don't need any check so break */
                break;

            if (action == GTK_FILE_CHOOSER_ACTION_OPEN) { /* Open REG file */
                struct stat statbuf;
                
                stat (filename, &statbuf);
                if (statbuf.st_size != 0) {
                    if (! fsexam_gui_show_yesno_dialog (GTK_WINDOW (dialog), 
                                _(emp_msg))) {
                        g_free (filename);
                        filename = NULL;
                        continue;
                    }
                }
            }

            if ((ensure_utf8) && ! g_utf8_validate (filename, -1, NULL)) {
                if (fsexam_gui_show_yesno_dialog (
                            GTK_WINDOW (dialog), _(nonutf8_msg))) {
                    gboolean ret;
                    gchar    *result = NULL;

                    ret = fsexam_convert_single_filename (view->setting,
                                                          filename,
                                                          &result);
                    if (ret) {  /* convert success */
                        g_free (filename);
                        filename = result;
                        break;
                    }else{
                        gchar *uri; 
                        GtkWidget *dlg;

                        if (fsexam_errno == ERR_CANCEL_CONVERSION) {
                            g_free (filename);
                            filename = NULL;
                            break;
                        }

                        uri = g_filename_to_uri (filename, NULL, NULL);
                        dlg = gtk_message_dialog_new (
                                            GTK_WINDOW (dialog),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_CLOSE,
                                            _(converr_msg),
                                            uri,
                                            fsexam_error_get_msg ());
                        g_signal_connect (G_OBJECT (dlg), "response",
                                          G_CALLBACK (gtk_widget_destroy),
                                          NULL);

                        gtk_dialog_run (GTK_DIALOG (dlg));

                        gtk_widget_destroy (dlg);
                        g_free (filename);
                        g_free (uri);
                        filename = NULL;
                        continue;
                    }
                }else{  /* fsexam_gui_show_yesno_dialog (...) */
                    break;
                }
            }else{  /* if  ((ensure_utf8) && !g_utf8_validate (...) */
                break;
            }
        }else{  /* if (GTK_DIALOG_RUN (dialog) == GTK_RESPONSE_ACCEPT) */
            g_free (filename);
            filename = NULL;
            break;
        }
    }

    gtk_widget_destroy (dialog);

    return filename;
}

/*
 * If dir is symlink or under symlink directory, convert it
 * to absolute path firstly.
 */
void 
fsexam_change_dir (const gchar *dir)
{
    if (dir == NULL)
        return;

    if (view->rootdir != NULL && strcmp (dir, view->rootdir) == 0) 
        return;

    if (! fsexam_treeview_construct (dir))
        return;

    fsexam_undo_removeall ();       /* clear undo stack */
    fsexam_statusbar_fresh ();      /* refresh the status bar msg stack */

    return;
}

/* callback for open menu */
void
fsexam_choose_dir ()
{
    gchar *filename = NULL;

    filename = fsexam_file_chooser_get_name (_("Folder Selection"),
                                GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, 
                                FALSE,
                                TRUE);
    fsexam_change_dir (filename);
    g_free (filename);

    return;
}

void 
show_help()
{
    GError *err = NULL;

    gnome_help_display ("fsexam.xml", "fsexam-intro", &err);

    if (err) {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (
                        GTK_WINDOW (view->mainwin),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_ERROR,
                        GTK_BUTTONS_CLOSE,
                        _("There was an error when displaying help: %s"),
                        err->message);

        g_signal_connect (G_OBJECT (dialog),
                          "response",
                          G_CALLBACK (gtk_widget_destroy),
                          NULL);

        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
        gtk_widget_show (dialog);
        g_error_free (err);
    }

    return;
}

void
fsexam_about ()
{
    static GtkWidget *about = NULL;

    gchar *authors [] = {
        "Yandong Yao <Yandong.Yao@Sun.COM>",
        "Federic Zhang <Federic.Zhang@Sun.COM>",
        "Yong Sun <Yong.Sun@Sun.COM>",
        NULL
    };

    if (about != NULL) {
        gtk_window_present (GTK_WINDOW (about));
        return;
    }

    about = gnome_about_new (("fsexam"), VERSION,
               "Copyright 2003-2008",
               _("fsexam is to help migrate file name and file content from"
                 " legacy encoding to UTF8"),
               (const gchar **)authors,
               NULL, NULL, NULL);

    gtk_window_set_destroy_with_parent (GTK_WINDOW (about), TRUE);

    g_signal_connect (G_OBJECT (about), "destroy",
                      G_CALLBACK (gtk_widget_destroyed), &about);

    gtk_widget_show (about);

    return;
}

/* construct GtkTreeView column and renderer */
static void
common_construct_ui ()
{
    static gboolean     initialized = FALSE;
    GtkCellRenderer     *renderer;
    GtkTreeViewColumn   *column;
    GtkWidget           *treeview;

    if (initialized)
        return;

    initialized = TRUE;

    /* construct tree view */
    treeview = g_object_get_data (G_OBJECT (view->mainwin), "treeview_file");

    /* ICON_COLUMN + DISPLAYNAME_COLUMN */
    column = gtk_tree_view_column_new ();
 
    /* Icon renderer */
    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "pixbuf", ICON_COLUMN,
                                         NULL);
 
    /* text renderer */
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", DISPLAYNAME_COLUMN,
                                         NULL);
  
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (treeview), TRUE);

//    gtk_tree_view_column_set_sort_column_id (column, FILENAME_COLUMN);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    /* FILENAME_COLUMN */
    column = gtk_tree_view_column_new ();
    
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", FILENAME_COLUMN,
                                         NULL);
  
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_visible (column, FALSE);
    gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (treeview), TRUE);

    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    /* set GtkTreeView selection mode */
    gtk_tree_selection_set_mode (
          gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)), 
          GTK_SELECTION_MULTIPLE);

    return;
}

/*
 * Construct file list in the left pane and display dirname in location 
 * field.
 */
void
fsexam_construct_ui (const gchar *dir)
{
    if (NULL == dir)
        return;

    common_construct_ui ();             /* create GtkTreeView */
    fsexam_treeview_construct (dir);    /* create GtkTreeModel */
                                
    return;
}

/*
 * Create and show search pane 
 */
static void
create_search_treeview_column (GtkTreeView *treeview)
{
    GtkTreeViewColumn   *column = NULL;
    GtkCellRenderer     *renderer = NULL;
    GtkListStore        *model = NULL;

    /* Create empty list store */
    model = gtk_list_store_new (3, 
            GDK_TYPE_PIXBUF, 
            G_TYPE_STRING, 
            G_TYPE_STRING);
    gtk_tree_view_set_model (treeview, GTK_TREE_MODEL (model));
    g_object_unref (model);

    //gtk_tree_view_set_headers_visible (treeview, TRUE);
    gtk_tree_selection_set_mode (
            gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)), 
            GTK_SELECTION_MULTIPLE);

    /* display name column */
    column = gtk_tree_view_column_new ();
    //gtk_tree_view_column_set_title (column, _("Name"));

    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, 
            renderer, "pixbuf", 
            ICON_COLUMN, NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, 
            renderer, "text", 
            DISPLAYNAME_COLUMN, 
            NULL);

    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (treeview), TRUE);
    //gtk_tree_view_column_set_sort_column_id (column, DISPLAYNAME_COLUMN);

    gtk_tree_view_append_column (treeview, column);

    /* real name column */
    column = gtk_tree_view_column_new ();

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, 
            renderer, "text", 
            FILENAME_COLUMN, 
            NULL);

    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_visible (column, FALSE);
    gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (treeview), TRUE);

    gtk_tree_view_append_column (treeview, column);

    /* connect signals */
    gtk_widget_add_events (GTK_WIDGET (treeview), GDK_BUTTON_PRESS_MASK);
    g_signal_connect (G_OBJECT (treeview), "button_press_event",
                      G_CALLBACK (button_press), NULL);
    g_signal_connect (G_OBJECT (treeview), "button_release_event",
                      G_CALLBACK (button_release), NULL);
    g_signal_connect (G_OBJECT (treeview), "key_press_event",
                      G_CALLBACK (key_press), NULL);

    return;
}

void 
fsexam_search_treeview_show ()
{
    GtkWidget *treeview = NULL;
    GtkWidget *widget = NULL;

    treeview = g_object_get_data (G_OBJECT (view->mainwin), "treeview_search");

    if (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)) == NULL)
        create_search_treeview_column (GTK_TREE_VIEW (treeview));

    /* show the search result pane for the first time */
    widget = g_object_get_data (G_OBJECT (view->mainwin), "vpaned_main");
    gtk_widget_show (gtk_paned_get_child2 (GTK_PANED (widget)));
    gtk_paned_set_position (GTK_PANED (widget), 
                            2 * widget->allocation.height / 3);

    /* check search result menu */
    widget = g_object_get_data (G_OBJECT (view->mainwin), "menu_search_result");
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);

    return;
}

void 
fsexam_search_treeview_hide ()
{
    GtkWidget *vpaned_main = NULL;
    GtkWidget *widget = NULL;

    vpaned_main = g_object_get_data (G_OBJECT (view->mainwin), "vpaned_main");
    gtk_paned_set_position (GTK_PANED (vpaned_main), 
                            vpaned_main->allocation.height);

    /* check search result menu */
    widget = g_object_get_data (G_OBJECT (view->mainwin), "menu_search_result");
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), FALSE);

    return;
}

/*
 * Append search result into search result listview 
 *
 * If filename == NULL, then show result pane and clear old result only
 */
void
fsexam_search_treeview_append_file (const gchar *filename, gboolean first_file)
{
    GtkTreeView     *treeview = NULL;
    GtkListStore    *model = NULL;
    GtkTreeIter     iter;
    gchar           *abs_path = NULL;
    gchar           *display_name = NULL;
    gchar           *internal_name = NULL;
    gboolean        showned;

    showned = gtk_check_menu_item_get_active (
            g_object_get_data (G_OBJECT (view->mainwin), "menu_search_result"));

    if (!showned)   /* show search result treeview if not */
        fsexam_search_treeview_show ();

    treeview = GTK_TREE_VIEW (g_object_get_data (G_OBJECT (view->mainwin), 
                              "treeview_search"));
    model = GTK_LIST_STORE (gtk_tree_view_get_model (treeview));

    if (first_file) {
        gtk_list_store_clear (model);
    }
   
    if (filename == NULL)
        return;

    abs_path = get_abs_path (filename);

    if (abs_path == NULL) {
        fsexam_gui_display_msg (filename,
                g_file_test (filename, G_FILE_TEST_IS_SYMLINK) ? 
                _("Symlink target doesn't exist") : 
                _("doesn't exist"));
        return;
    }
   
    /* add search result into search tree view */
    display_name = fsexam_filename_display_name (abs_path);
    internal_name = g_strescape (abs_path, "");
    gtk_list_store_append(model, &iter);
    gtk_list_store_set (model, &iter,
                       ICON_COLUMN, get_file_pixbuf (abs_path),
                       DISPLAYNAME_COLUMN, display_name,
                       FILENAME_COLUMN, internal_name,
                       -1);

    g_free (abs_path);
    g_free (display_name);
    g_free (internal_name);

    return;
}

void
fsexam_search_treeview_append_list (GList *list)
{
    GtkWidget *notebook = NULL;
    gboolean  first_file = TRUE;

    if (list == NULL)
        return;

    while (list) {
        fsexam_search_treeview_append_file (list->data, first_file);

        first_file = FALSE;
        list = list->next;
    }

    /* show conversion log notebook page */
    notebook = g_object_get_data (G_OBJECT (view->mainwin), "notebook_report");
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);

    return;
}

gboolean
fsexam_gui_show_yesno_dialog (GtkWindow *parent, const gchar *msg_format, ...)
{
    GtkWidget   *dialog = NULL;
    gchar       *msg = NULL;
    va_list     args;
    gboolean    ret = FALSE;

    if (msg_format) {
        va_start (args, msg_format);
        msg = g_strdup_vprintf (msg_format, args);
        va_end (args);
    }

    dialog = gtk_message_dialog_new (
                            parent,
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_QUESTION,
                            GTK_BUTTONS_YES_NO,
                            msg);
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES) {
        ret = TRUE;
    }

    g_free (msg);
    
    gtk_widget_destroy (dialog);

    return ret;
}

void
fsexam_gui_show_dialog (GtkWindow *parent, 
                        GtkMessageType type, 
                        const gchar *message_format, 
                        ...)
{
    GtkWidget   *dialog = NULL;
    gchar       *msg = NULL;
    va_list     args;

    if (message_format) {
        va_start (args, message_format);
        msg = g_strdup_vprintf (message_format, args);
        va_end (args);
    }

    dialog = gtk_message_dialog_new (
                            parent, 
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            type,
                            GTK_BUTTONS_CLOSE,
                            msg);

    g_signal_connect (G_OBJECT (dialog), "response", 
                      G_CALLBACK (gtk_widget_destroy), NULL);

    gtk_dialog_run (GTK_DIALOG (dialog));

    g_free (msg);

    return;
}

/*
 * Display msg into report pane.
 *  filename: maybe non-utf8
 *  msg: must be utf8
 */
void
fsexam_gui_display_msg (const gchar *filename, const gchar *msg)
{
    GtkWidget       *textview = NULL;
    GtkTextBuffer   *buffer = NULL;
    GtkTextMark     *mark = NULL;
    GtkTextIter     iter;
    gchar           *messages = NULL;
    static gboolean created_mark = FALSE;
    static gboolean tag_created = FALSE;
    
    textview = g_object_get_data (G_OBJECT (view->mainwin), 
                                  "textview_report");
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    gtk_text_buffer_get_end_iter (buffer, &iter);
   
    /* creat mark based on iter */
    if (!created_mark) {
        mark = gtk_text_buffer_create_mark (buffer, "end_mark", &iter, FALSE);
        created_mark = TRUE;
    }else{
        mark = gtk_text_buffer_get_mark (buffer, "end_mark");
    }
   
    /* create BOLD tag */
    if (!tag_created) {
        gtk_text_buffer_create_tag (buffer,
                "BOLD_TAG",
                "weight", PANGO_WEIGHT_BOLD,
                NULL);
        tag_created = TRUE;
    }

    if (filename) {
        gchar *dname = g_path_get_dirname (filename);
        gchar *bname = g_path_get_basename (filename);
        gchar *prev_line = NULL;

        /* Under the same directory as the previous report? */
        prev_line = get_last_path (buffer, &iter);
        if (prev_line == NULL || strcmp (dname, prev_line) != 0) {
            gtk_text_buffer_get_end_iter (buffer, &iter);

            if (g_utf8_validate (dname, -1, NULL)) {
                messages = g_strdup_printf ("%s\n", dname);
            }else{
                gchar *uri = g_filename_to_uri (dname, NULL, NULL);
                messages = g_strdup_printf ("%s\n", uri);
                g_free (uri);
            }

            gtk_text_buffer_insert (buffer, &iter, messages, -1);
            g_free (messages);
        }

        /* print basename and conversion message */
        gtk_text_buffer_get_end_iter (buffer, &iter);
        gtk_text_buffer_insert (buffer, &iter, "\t\t", -1);

        gtk_text_buffer_get_end_iter (buffer, &iter);
        if (g_utf8_validate (bname, -1, NULL)) {
            gtk_text_buffer_insert_with_tags_by_name (buffer,
                    &iter,
                    bname,
                    -1,
                    "BOLD_TAG",
                    NULL);
        }else{
            gchar *escape = fsexam_string_escape (bname);
            gtk_text_buffer_insert_with_tags_by_name (buffer,
                    &iter,
                    escape,
                    -1,
                    "BOLD_TAG",
                    NULL);

            g_free (escape);
        }

        gtk_text_buffer_get_end_iter (buffer, &iter);
        messages = g_strdup_printf (":\t%s\n", msg ? msg : "");
        gtk_text_buffer_insert (buffer, &iter, messages, -1);
        g_free (messages);
        
        g_free (dname);
        g_free (bname);
        g_free (prev_line);
    }else{
        gtk_text_buffer_insert (buffer, &iter, msg, -1);
        gtk_text_buffer_get_end_iter (buffer, &iter);
        gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    }

    /* scroll text view so that the newest line is visible */
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (textview),
            mark,
            0.0,
            TRUE,
            0.0,
            1.0);

    return;
}

/* 
 * callbacks for FSEXAM_setting functions 
 *
 * update TreeView after convert file name successfully.
 */
void
fsexam_gui_update ( FSEXAM_setting *setting, 
                    const gchar *path,
                    const gchar *oldname,
                    const gchar *newname)
{
    GtkWidget       *tree_view = NULL;
    GtkTreeModel    *tree_model = NULL;
    GtkTreeModel    *another_model = NULL;
    GtkTreePath     *tree_path = NULL;
    gchar           *tree_path_str = NULL;
    gchar           *old_full_name = NULL;
    gchar           *new_full_name = NULL;
    gboolean        on_treeview_search = FALSE;

    if (setting->pref->conv_content)
        return;

    if ((NULL == path) || (NULL == oldname) 
            || (NULL == newname) || (view->focus_treeview == NULL))
        return;

    tree_view = view->focus_treeview;
    tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    
    if (strcmp (gtk_widget_get_name (tree_view), "treeview_search") == 0) 
        on_treeview_search = TRUE;

    tree_view = g_object_get_data (G_OBJECT (view->mainwin), 
                    on_treeview_search ? "treeview_file" : "treeview_search");
    another_model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    old_full_name = g_strdup_printf ("%s/%s", path, oldname);
    new_full_name = g_strdup_printf ("%s/%s", path, newname);

    /* lookup hash to get the GtkTreePath string */
    if (view->treepath_hash != NULL) {
        tree_path_str = (gchar *)g_hash_table_lookup (view->treepath_hash, 
                                                      old_full_name);
    }

    if (tree_path_str != NULL)
        tree_path = gtk_tree_path_new_from_string (tree_path_str);

    /* update search result treeview */
    update_gui_for_filelist (on_treeview_search ? tree_model : another_model,
                             old_full_name,
                             new_full_name,
                             tree_path,
                             on_treeview_search ? TRUE : FALSE);

    /* update folder treeview */
    update_gui_for_dir (on_treeview_search ? another_model : tree_model,
                        path,
                        oldname,
                        newname,
                        tree_path,
                        on_treeview_search ? FALSE : TRUE);

    g_free (new_full_name);
    g_free (old_full_name);
    gtk_tree_path_free (tree_path);

    return;
}

extern gint indexg;

gint
fsexam_gui_get_index (GList *encoding_list, gboolean forname)
{
    GtkWidget   *dialog;
    gint        response;
    gboolean    donnot_ask;

    indexg = -1;    /* init indexg to override previous value */

    dialog = fsexam_dialog_candidate (encoding_list, forname);
    response = gtk_dialog_run (GTK_DIALOG (dialog));

    donnot_ask = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
                g_object_get_data (G_OBJECT (dialog), "chkbtn_ask")));

    /* now indexg should contain the real index */
    if (response == GTK_RESPONSE_OK){
        gint index = 0;

        if (indexg == -1){
            /* use the first available encode */
            fsexam_encoding_iterate_with_func (encoding_list,
                        fsexam_encoding_translate_index,
                        &index,
                        &indexg);
        }

        /* indexg is the index in encoding_list, so it is safe */
        if (donnot_ask)
            view->setting->gold_index = indexg;
    }else{
        indexg = -1;
    }

    /* indexg == -1 means cancel conversion by user */
    if (indexg == -1 && donnot_ask)
        view->setting->flags |= FSEXAM_SETTING_FLAGS_STOP;

    gtk_widget_destroy (dialog);

    return indexg;
}

#define END_LINE "==================================================\n"
void
fsexam_gui_display_stats (FSEXAM_setting *setting)
{
    gchar           *stats_info = NULL;
    GtkTextView     *textview = NULL;
    GtkTextBuffer   *buffer = NULL;
    GtkTextMark     *mark = NULL;
    GtkTextIter     iter, start, end;
    gchar           *text;
    gint            line;
    static gboolean tag_created = FALSE;

    stats_info = g_strdup_printf (
            _("Rough summary: %d given, %d total, %d ignore, %d fail, %d succeed\n"),
            setting->passin_num,
            setting->total_num,
            setting->ignore_num,
            setting->fail_num,
            setting->succ_num);

    /* display statastics information in statusbar */
    *(stats_info + strlen (stats_info) - 1) = '\0';
    fsexam_statusbar_update (stats_info);
    g_free (stats_info);

    /* display something in report pane to make it clear to see */
    textview = g_object_get_data (G_OBJECT (view->mainwin), 
                                  "textview_report");
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    gtk_text_buffer_get_end_iter (buffer, &iter);

    line = gtk_text_iter_get_line (&iter) - 1;
    gtk_text_buffer_get_iter_at_line_offset (buffer, &start, line, 0);
    gtk_text_buffer_get_iter_at_line_offset (buffer, &end, line + 1, 0);
    text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

    if (text == NULL || *text == '=') {		/* No new data, return */
        g_free (text);
        return;
    }

    if (!tag_created) {
        gtk_text_buffer_create_tag (buffer, 
                "SUMMARY_TAG",      /* tag name */
                "foreground", "blue", 
                //"weight", PANGO_WEIGHT_BOLD, 
                NULL);
        tag_created = TRUE;
    }

    mark = gtk_text_buffer_get_mark (buffer, "end_mark");

    if (mark == NULL) {
        mark = gtk_text_buffer_create_mark (buffer, "end_mark", &iter, FALSE);
    }

    gtk_text_buffer_insert_with_tags_by_name (buffer, 
            &iter, 
            END_LINE,
            -1,
            "SUMMARY_TAG",
            NULL);

    gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (textview), mark);

    g_free (text);
    
    return;
}

/* 
 * Load Glade xml file
 */
GladeXML *
fsexam_gui_load_glade_file (const gchar *filename,
                            const gchar *widget_root,
                            GtkWindow  *error_dialog_parent)
{
    gchar       *path = NULL;
    GladeXML    *xml = NULL;

    path = g_strconcat ("./", filename, NULL);  /* for test in local dir */
    if (g_file_test (path, G_FILE_TEST_EXISTS)) {
        xml = glade_xml_new (path, widget_root, GETTEXT_PACKAGE);
    }

    if (xml == NULL) {
        g_free (path);
        path = g_build_filename (FSEXAM_GLADE_DIR, filename, NULL);

        xml = glade_xml_new (path, widget_root, GETTEXT_PACKAGE);
    }

    if (xml == NULL) {
        fsexam_gui_show_dialog (error_dialog_parent,    
                                GTK_MESSAGE_ERROR,
                                _("The file \"%s\" is missing."), 
                                path);
    }else{
        glade_xml_signal_autoconnect (xml);
    }

    g_free (path);

    return xml;
}

FSEXAM_view *
fsexam_view_new ()
{
    FSEXAM_view *view = NULL;
    GladeXML    *xml = NULL;
    GtkWidget   *mainwin = NULL;
    GtkWidget   *w = NULL;
    gchar       *icon_path = NULL;

    view = g_new0 (FSEXAM_view, 1);

    view->pixbuf_hash = g_hash_table_new_full (
                                        g_str_hash, 
                                        g_str_equal,
                                        (GDestroyNotify) g_free,
                                        (GDestroyNotify) g_object_unref);
    view->treepath_hash = g_hash_table_new_full (
                                        g_str_hash, 
                                        g_str_equal,
                                        (GDestroyNotify) g_free,
                                        (GDestroyNotify) g_free);
    view->undo_list = NULL;
    view->rootdir = NULL;
    view->basedir = NULL;
    view->lineoffset = 0;
    view->pid = -1;

    view->peekwin = NULL;
    view->focus_treeview = NULL;

    /* set the default icon for all window */
    icon_path = g_strdup_printf ("%s/%s", FSEXAM_ICON_DIR, FSEXAM_ICON_FILE);
	gtk_window_set_default_icon_from_file (icon_path, NULL);
    g_free (icon_path);

    /* Main Window */
    xml = fsexam_gui_load_glade_file (
                        FSEXAM_GLADE_FILE,
                        "app_window",
                        NULL); 
    if (xml == NULL)
        return view;

    mainwin = view->mainwin = glade_xml_get_widget (xml, "app_window");
    g_signal_connect (G_OBJECT (mainwin), "destroy",
                      G_CALLBACK (mainwin_destroy), NULL);
    g_signal_connect (G_OBJECT (mainwin), "delete-event",
                      G_CALLBACK (mainwin_delete), NULL);

    fsexam_dnd_set (mainwin);       /* set DnD */

    /* main menu */
    w = glade_xml_get_widget(xml, "menu_quit");
    g_signal_connect (G_OBJECT (w), "activate",
		      G_CALLBACK (mainwin_destroy), NULL);

    /* toolbar */
    w = glade_xml_get_widget (xml, "toolbar_main");
    g_object_set_data (G_OBJECT (mainwin), "toolbar", w);

    /* Treeview and its mouse and button events */
    w = glade_xml_get_widget (xml, "treeview_file");
    g_object_set_data (G_OBJECT (mainwin), "treeview_file", w);
    gtk_widget_add_events (w, GDK_BUTTON_PRESS_MASK);
    g_signal_connect (GTK_TREE_VIEW (w), "button_press_event",
                      G_CALLBACK (button_press), NULL);
    g_signal_connect (GTK_TREE_VIEW (w), "button_release_event",
                      G_CALLBACK (button_release), NULL);
    g_signal_connect (GTK_TREE_VIEW (w), "key_press_event",
                      G_CALLBACK (key_press), NULL);
    
    /* status bar */
    w = glade_xml_get_widget (xml, "appbar1");
    g_object_set_data (G_OBJECT (mainwin), "appbar", w);

    /* notebook */
    w = glade_xml_get_widget (xml, "notebook_report");
    g_object_set_data (G_OBJECT (mainwin), "notebook_report", w);

    w = glade_xml_get_widget (xml, "scrollwin_report");
    g_object_set_data (G_OBJECT (mainwin), "scrollwin_report", w);

    w = glade_xml_get_widget (xml, "textview_report");
    g_object_set_data (G_OBJECT (mainwin), "textview_report", w);

    w = glade_xml_get_widget (xml, "scrollwin_dryrun");
    g_object_set_data (G_OBJECT (mainwin), "scrollwin_dryrun", w);

    w = glade_xml_get_widget (xml, "textview_dryrun");
    g_object_set_data (G_OBJECT (mainwin), "textview_dryrun", w);

    /* folder entry */
    w = glade_xml_get_widget (xml, "entry_folder");
    g_object_set_data (G_OBJECT (mainwin), "entry_folder", w);

    /* submenu */
    w = glade_xml_get_widget (xml, "menu_search");
    g_object_set_data (G_OBJECT (mainwin), "menu_search", w);

    w = glade_xml_get_widget (xml, "menu_stop_search");
    g_object_set_data (G_OBJECT (mainwin), "menu_stop_search", w);

    w = glade_xml_get_widget (xml, "undo_menu");
    g_object_set_data (G_OBJECT (mainwin), "menu_undo", w);
    gtk_widget_set_sensitive (w, FALSE);

    w = glade_xml_get_widget (xml, "convert_menu");
    g_object_set_data (G_OBJECT (mainwin), "menu_convert", w);

    w = glade_xml_get_widget (xml, "restore_menu");
    g_object_set_data (G_OBJECT (mainwin), "menu_restore", w);

    w = glade_xml_get_widget (xml, "dryrun_menu");
    g_object_set_data (G_OBJECT (mainwin), "menu_dryrun", w);

    w = glade_xml_get_widget (xml, "menu_scenario");
    g_object_set_data (G_OBJECT (mainwin), "menu_scenario", w);

    w = glade_xml_get_widget (xml, "force_convert_menu");
    g_object_set_data (G_OBJECT (mainwin), "menu_force_convert", w);
    
    w = glade_xml_get_widget (xml, "menu_clear_search");
    g_object_set_data (G_OBJECT (mainwin), "menu_clear_search", w);

    w = glade_xml_get_widget (xml, "name_mode_menu");
    g_object_set_data (G_OBJECT (mainwin), "menu_name_mode", w);

    w = glade_xml_get_widget (xml, "content_mode_menu");
    g_object_set_data (G_OBJECT (mainwin), "menu_content_mode", w);

    w = glade_xml_get_widget (xml, "report_pane_menu");
    g_object_set_data (G_OBJECT (mainwin), "menu_report", w);

    w = glade_xml_get_widget (xml, "dryrun_result_menu");       /* dryrun result under view */
    g_object_set_data (G_OBJECT (mainwin), "menu_dryrun_result", w);

    w = glade_xml_get_widget (xml, "menu_search_result");
    g_object_set_data (G_OBJECT (mainwin), "menu_search_result", w);

    /* toolbar button */
    w = glade_xml_get_widget (xml, "toolbutton_convert");
    g_object_set_data (G_OBJECT (mainwin), "toolbutton_convert", w);

    w = glade_xml_get_widget (xml, "toolbutton_restore");
    g_object_set_data (G_OBJECT (mainwin), "toolbutton_restore", w);
    
    w = glade_xml_get_widget (xml, "toolbutton_dryrun");
    g_object_set_data (G_OBJECT (mainwin), "toolbutton_dryrun", w);
    
    w = glade_xml_get_widget (xml, "toolbutton_force");
    g_object_set_data (G_OBJECT (mainwin), "toolbutton_force", w);
    
    w = glade_xml_get_widget (xml, "toolbutton_undo");
    g_object_set_data (G_OBJECT (mainwin), "toolbutton_undo", w);

    w = glade_xml_get_widget (xml, "toolbutton_scenario");
    g_object_set_data (G_OBJECT (mainwin), "toolbutton_scenario", w);

    /* search result pane */
    w = glade_xml_get_widget (xml, "vpaned_main");
    g_object_set_data (G_OBJECT (mainwin), "vpaned_main", w);

    w = glade_xml_get_widget (xml, "label_result");
    g_object_set_data (G_OBJECT (mainwin), "label_result", w);

    w = glade_xml_get_widget (xml, "treeview_search");
    g_object_set_data (G_OBJECT (mainwin), "treeview_search", w);

    g_object_unref (G_OBJECT (xml));

    /* popup menu */
    xml = fsexam_gui_load_glade_file (FSEXAM_GLADE_FILE, 
                                      "menu_popup",
                                      GTK_WINDOW (view->mainwin));
   if (xml == NULL)
       return view;

    view->popup_menu = glade_xml_get_widget (xml, "menu_popup");
#ifdef HAVE_NO_GLIB_2_8
    g_object_ref (view->popup_menu);
#else
    g_object_ref_sink (view->popup_menu);   /* we own this object */
#endif

    w = glade_xml_get_widget (xml, "popup_undo");
    g_object_set_data (G_OBJECT (view->popup_menu), "popup_undo", w);

    w = glade_xml_get_widget (xml, "popup_convert");
    g_object_set_data (G_OBJECT (view->popup_menu), "popup_convert", w);

    w = glade_xml_get_widget (xml, "popup_restore");
    g_object_set_data (G_OBJECT (view->popup_menu), "popup_restore", w);
    
    w = glade_xml_get_widget (xml, "popup_dryrun");
    g_object_set_data (G_OBJECT (view->popup_menu), "popup_dryrun", w);
    
    w = glade_xml_get_widget (xml, "popup_scenario");
    g_object_set_data (G_OBJECT (view->popup_menu), "popup_scenario", w);

    w = glade_xml_get_widget (xml, "popup_forceful");
    g_object_set_data (G_OBJECT (view->popup_menu), "popup_forceful", w);
    
    w = glade_xml_get_widget (xml, "popup_name_mode");
    g_object_set_data (G_OBJECT (view->popup_menu), "popup_name_mode", w);
    
    w = glade_xml_get_widget (xml, "popup_content_mode");
    g_object_set_data (G_OBJECT (view->popup_menu), "popup_content_mode", w);

    g_object_unref (G_OBJECT (xml));

    return view;
}

void
fsexam_view_destroy (FSEXAM_view *view)
{
    /* hash table */
    g_hash_table_destroy (view->pixbuf_hash);
    g_hash_table_destroy (view->treepath_hash);
    view->treepath_hash = NULL;
    view->pixbuf_hash = NULL;

    /* FSEXAM_setting */
    if (view->setting) 
        fsexam_setting_destroy (view->setting);
    view->setting = NULL;

    /* undo list */
    if (view->undo_list) {
        g_slist_free (view->undo_list);
    }
    view->undo_list = NULL;

    /* popup menu object */
    g_object_unref (G_OBJECT (view->popup_menu));
    view->popup_menu = NULL;

    /* dirname */
    g_free (view->rootdir);
    view->rootdir = NULL;
    g_free (view->basedir);
    view->basedir = NULL;

    g_free (view);

    return;
}

/* ---------------------  Content peek funcs -------------------- */

GdkPixmap *content_pixmap;

static void
get_upper_left_xy (GtkWidget *peekwin,
                   gint width, gint height,
                   gint x_root, gint y_root,
                   gint *x, gint *y)
{
    *x = x_root;
    *y = y_root;

    *x -= width;
    *y -= height;

    return;
}


static void
set_window_background (GtkWidget *window, GdkPixmap *pixmap)
{
    gdk_window_set_back_pixmap (window->window, pixmap, FALSE);
}

/*
 * Create a pixmap from contents 
 */
static GdkPixmap *
create_content_pixmap (GtkWidget *peekwin, gchar *content)
{
    PangoLayout     *pango_layout;
    PangoLayout     *main_pango_layout = NULL;
    PangoRectangle  rect;
    gint            pixmap_width, pixmap_height;
    GtkStyle        *style;
    GdkPixmap       *pixmap;
    PangoFontDescription *font_desc;

    enum { PADDING = 8 };

    /* create PangoLayout */
    main_pango_layout = gtk_widget_create_pango_layout (view->mainwin, NULL);
    font_desc = pango_font_description_copy (
                        gtk_widget_get_style (view->mainwin)->font_desc);

    pango_layout = pango_layout_new (
                        pango_layout_get_context (main_pango_layout));

    pango_layout_set_font_description (pango_layout, font_desc);
    
    if (str_isutf8 (content, -1)) {
        pango_layout_set_text (pango_layout, content, -1);
    } else {
        gchar   *display_content;

        /* Use below function to replace non-utf8 char with '?' */
        display_content = fsexam_filename_display_name (content);
        pango_layout_set_text (pango_layout, display_content, -1);

        g_free (display_content);
    }

    pango_layout_get_pixel_extents (pango_layout, &rect, NULL);

    pixmap_width = rect.width + 2 * PADDING;
    pixmap_height = rect.height + 2 * PADDING;

    style = gtk_widget_get_style (view->mainwin);
   
    /* create pixmap */
    pixmap = gdk_pixmap_new (view->mainwin->window, 
                             pixmap_width, 
                             pixmap_height, 
                             -1);

    gdk_draw_rectangle (pixmap, 
                        style->base_gc[GTK_STATE_NORMAL], 
                        TRUE, 0, 0, 
                        pixmap_width, pixmap_height);

    gdk_draw_rectangle (pixmap, 
                        style->fg_gc[GTK_STATE_INSENSITIVE], 
                        FALSE, 1, 1, 
                        pixmap_width - 3, pixmap_height -3);

    gdk_draw_layout (pixmap, 
                     style->text_gc[GTK_STATE_NORMAL], 
                     -rect.x + PADDING, -rect.y + PADDING, 
                     pango_layout);

    pango_font_description_free (font_desc);
    g_object_unref (pango_layout);

    return pixmap;
}

static void 
free_selection (gpointer row, gpointer data)
{
    GtkTreePath *path = (GtkTreePath *)row;
    gtk_tree_path_free (path);

    return;
}

#define MAX_NUM_MULTIBYTE_CHARACTERS 400
#define MAX_NUM_CHARACTERS 2000

static gchar *
fsexam_content_get_sample (char *file)
{
    gchar   *p = NULL, *sample = NULL;
    gsize   length;
    gint    multi_num = 0, char_num = 0;
    GError  *err = NULL; 

    if (NULL == file)
        return NULL;

    if (! g_file_get_contents (file, &p, &length, &err)) {
        fsexam_statusbar_update (err->message);
        g_error_free (err);

        return NULL;
    }

    sample = p;

    while (TRUE) {
        gunichar wc;

        /* get the next Unicode character */
        wc = g_utf8_get_char_validated (p, -1);
        if (wc == 0x0 || multi_num == MAX_NUM_MULTIBYTE_CHARACTERS 
                || char_num == MAX_NUM_CHARACTERS) {
            *p = 0x0;
            break;
        }
        
        if (wc == (gunichar)-1 || wc == (gunichar)-2) { /* invalid UTF8 */
            ++multi_num;
            ++p;
        } else {                                        /* valid UTF8 */
            if (wc >= 0x7e)
                ++multi_num;

            p = g_utf8_next_char (p);
        }
        ++char_num;
    }

    return sample;
}

/* callback for realize signal */
static void
peek_window_realize (GtkWidget *peekwin, gpointer user_data)
{
    gint width, height;

    set_window_background (peekwin, content_pixmap);
    gdk_window_clear (peekwin->window);

    gdk_drawable_get_size (GDK_DRAWABLE (content_pixmap),
                           &width, &height);

    gtk_widget_set_size_request (peekwin, width, height);
    gtk_window_resize (GTK_WINDOW (peekwin), width, height);

    return;
}

/*
 *  Update peek window using new content
 */
static void
update_peek_window (GtkWidget *peekwin, gchar *content)
{
    gint width, height;

    if (NULL == peekwin)
        return;

    if (content_pixmap != NULL)
        g_object_unref (content_pixmap);

    /* create pixmap from content */
    content_pixmap = create_content_pixmap (peekwin, content);

    if (GTK_WIDGET_REALIZED (peekwin)) {
        set_window_background (peekwin, content_pixmap);
        gdk_window_clear (peekwin->window);
    }

    gdk_drawable_get_size (GDK_DRAWABLE (content_pixmap),
                           &width, &height);

    gtk_widget_set_size_request (peekwin, width, height);
    gtk_window_resize (GTK_WINDOW (peekwin), width, height);

    return;
}

static void
place_peek_window (GtkWidget *peekwin, gint x_root, gint y_root)
{
    gint width, height;
    gint x, y;

    if (NULL == peekwin)
        return;

    gtk_widget_get_size_request (peekwin, &width, &height);

    get_upper_left_xy (peekwin, width, height,
                       x_root, y_root, &x, &y);

    gtk_window_move (GTK_WINDOW (peekwin), x, y);

    return;
}

/*
 * Create a toplevel peek window 
 */
static GtkWidget *
make_peek_window ()
{
    GtkWidget *peekwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    g_signal_connect (peekwin, "realize", 
                      G_CALLBACK (peek_window_realize), NULL);

    gtk_window_set_type_hint (GTK_WINDOW (peekwin),
                              GDK_WINDOW_TYPE_HINT_UTILITY);
    gtk_window_set_decorated (GTK_WINDOW (peekwin), FALSE);
    gtk_window_set_screen (GTK_WINDOW (peekwin),
                           gtk_widget_get_screen (view->mainwin));
    gtk_widget_set_app_paintable (peekwin, TRUE);

    return peekwin;
}

static gboolean
fsexam_content_get_selection (gchar **path, gchar **filename)
{
    GtkTreeSelection    *selection;
    GtkTreeIter         iter, subiter;
    GtkTreeModel        *model;
    GtkWidget           *treeview;
    GList               *row;
    GString             *dir;
    gchar               *internal_name;

    treeview = view->focus_treeview;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    row = gtk_tree_selection_get_selected_rows (selection, &model);

    if (row == NULL) {
        fsexam_statusbar_update (_("No selection"));

        return FALSE;
    }

    if (g_list_length (row) > 1) {
        g_list_foreach (row, free_selection, NULL);
        g_list_free (row);

        fsexam_statusbar_update (_("Cannot preview multiple selections"));

        return FALSE;
    }

    gtk_tree_model_get_iter (model, &iter, row->data);

    g_list_foreach (row, free_selection, NULL);
    g_list_free (row);

    if (gtk_tree_model_iter_children (model, &subiter, &iter)) {
        fsexam_statusbar_update (_("Cannot preview contents of folder"));

        return FALSE; 
    }

    gtk_tree_model_get (model, &iter,
                        FILENAME_COLUMN, &internal_name,
                        -1);

    *filename = g_strcompress (internal_name);
    g_free (internal_name);

    if (**filename == '/') { /* we are on search result treeview */
        *path = NULL;
        return TRUE;
    }

    dir = fsexam_filename_get_path (model, iter);

    *path = g_string_free (dir, FALSE);

    return TRUE;
}

static void
peekwin_destroy (GtkWidget *widget, gpointer user_data)
{
    gtk_object_destroy (GTK_OBJECT (view->peekwin));
    view->peekwin = NULL;

    return;
}

void
fsexam_content_peek (gint x_root, gint y_root)
{
    gchar          *path = NULL, *filename = NULL, *name = NULL;
    gchar          *sample_text = NULL;
    gchar          *statusmsg = NULL;
    gint           y_root2 = y_root;
    struct stat    statbuf;
    GtkWidget      *widget = NULL;
    const gchar    *widget_name;

    widget = gtk_window_get_focus (GTK_WINDOW (view->mainwin));
    widget_name = gtk_widget_get_name (widget);

    if (strcmp (widget_name, "treeview_search") == 0) {
        /* convert search result */
        if (view->basedir == NULL)
            return;

        view->focus_treeview = widget;
    }else{
        /* convert files in left pane */
        if (NULL == view->rootdir)  
            return;

        widget = g_object_get_data (G_OBJECT (view->mainwin), 
                                      "treeview_file");
        view->focus_treeview = widget;

    }

    if (!fsexam_content_get_selection (&path, &filename))
       return;

    if (y_root == 0) {
        gint x, y, height, width;

        gtk_window_get_position (GTK_WINDOW (view->mainwin), &x, &y);
        gtk_window_get_size (GTK_WINDOW (view->mainwin), &height, &width);

        x_root = x+width/2;
        y_root = y+height/2;
    }

    if (path == NULL)
        name = g_strdup (filename);
    else
        name = g_strdup_printf ("%s/%s", path, filename);

    if (lstat (name, &statbuf) == -1) {
        statusmsg = g_strdup_printf ("%s: %s", _("Can't access"), filename);
        goto ERR;
    }

    if (! S_ISREG (statbuf.st_mode)) {
        statusmsg = g_strdup_printf ("%s: %s", 
                                     _("Not regular file"), filename);
        goto ERR;
    }

    if (statbuf.st_size == 0) {
        statusmsg = g_strdup_printf ("%s: %s", _("Empty file"), filename);
        goto ERR;
    }

    if (!fsexam_is_plain_text (name, view->setting)) { 
        statusmsg = g_strdup_printf ("%s: %s", _("Not plain text"), filename);
        goto ERR;
    }

    if (!view->peekwin)
        view->peekwin = make_peek_window ();

    place_peek_window (view->peekwin, x_root, y_root);

    if ((sample_text = fsexam_content_get_sample (name)) == NULL) 
        goto ERR;

    update_peek_window (view->peekwin, sample_text);
    gtk_widget_show (view->peekwin);

    set_window_background (view->peekwin, content_pixmap);
    gdk_window_clear (view->peekwin->window);

    if (y_root2 == 0)
       g_signal_connect (G_OBJECT (view->peekwin), "focus-in-event",
                         G_CALLBACK (peekwin_destroy), view->peekwin);

    g_free (sample_text);

ERR:
    if (statusmsg != NULL) {
        fsexam_statusbar_update (statusmsg);
        g_free (statusmsg);
    }

    g_free (name);
    g_free (filename);
    g_free (path);

    return;
}


GtkWidget *
fsexam_gui_get_focused_treeview ()
{
    GtkWidget   *widget = NULL;
    const gchar *name;

    widget = gtk_window_get_focus (GTK_WINDOW (view->mainwin));
    name = gtk_widget_get_name (widget);

    if (strcmp (name, "treeview_search") != 0) {
        widget = g_object_get_data (G_OBJECT (view->mainwin), 
                                    "treeview_file");
    }
        
    return widget;
}

/* Set the initial toolbar style */
void
fsexam_gui_set_initial_state (void) 
{
    GtkWidget   *w = NULL;
    gchar       *toolbar_style = NULL;

    /* set toolbar style */
    toolbar_style = gconf_client_get_string (view->setting->pref->gconf_client,
            TOOLBAR_STYLE,
            NULL);

    if (toolbar_style != NULL) {
        w = g_object_get_data (G_OBJECT (view->mainwin), "toolbar");

        if (strcmp (toolbar_style, "both") == 0) {
	    	gtk_toolbar_set_style (GTK_TOOLBAR (w), GTK_TOOLBAR_BOTH);
	    }else if (strcmp (toolbar_style, "both-horiz") == 0) {
	    	gtk_toolbar_set_style (GTK_TOOLBAR (w), GTK_TOOLBAR_BOTH_HORIZ);
	    }else if (strcmp (toolbar_style, "icons") == 0) {
	    	gtk_toolbar_set_style (GTK_TOOLBAR (w), GTK_TOOLBAR_ICONS);
	    }else if (strcmp (toolbar_style, "text") == 0) {
	    	gtk_toolbar_set_style (GTK_TOOLBAR (w), GTK_TOOLBAR_TEXT);
	    }

        g_free (toolbar_style);
    }

    /* update name/content conversion mode accordting to command line option */
    if (view->setting->pref->conv_content) {
        w = g_object_get_data (G_OBJECT (view->mainwin), "menu_content_mode");
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (w), TRUE);
    }

    return;
}
