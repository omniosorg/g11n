/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/gnome-i18n.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "encode.h"
#include "fsexam-history.h"
#include "fsexam-preference.h"
#include "fsexam-conversion.h"
#include "fsexam-content.h"
#include "fsexam-accels.h"
#include "fsexam-dnd.h"
#include "fsexam.h"
#include "fsexam-accels.h"

#ifndef ICON_PATH
# define ICON_PATH "/usr/share/pixmaps/fsexam-icon.png"
#endif

typedef struct _TreeItem TreeItem;
struct _TreeItem
{
  GdkPixbuf *icon;  // file icon
  gchar *label;     // filename
  gboolean loaded;  // 0 if it is directory but those files under it hasn't been 
                    //   loaded into treeview yet.
  TreeItem *children;
};

GtkWidget *menu_action;

static gint
button_release_event (GtkWidget *widget,
		      GdkEventButton *event)
{
  if (view->peekwin && event->button == 2)
    {
      GtkWidget *peekwin = view->peekwin;

      view->peekwin = NULL;

      gdk_window_set_cursor (view->mainwin->window, NULL);
      gtk_object_destroy (GTK_OBJECT (peekwin));
    }

  return FALSE;
}

static gint
key_press_event (GtkWidget *widget,
		 GdkEventKey *event)
{
  switch (event->keyval)
    {
    case GDK_space:
      fsexam_filename_convert ();
      return TRUE;
      break;
    defaults:
      break;
    }

  return FALSE;
}

static gint 
tree_popup_handler(GtkWidget *widget, 
		   GdkEventButton *event)
{
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
    {
      fsexam_filename_convert ();

      return TRUE;
    }

  if (event->button == 2)
    {
      fsexam_content_peek (event->x_root, event->y_root);

      return TRUE;
    }

  if (event->button == 3)
    {
      gtk_menu_popup(menu_action, NULL, NULL, NULL, NULL, 
		     event->button, event->time);

      return TRUE;
    }

  return FALSE;
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
#define ICON_THEME_SOCKET           "gnome-fs-socket"
#define ICON_THEME_FIFO            "gnome-fs-fifo"
#define ICON_SIZE                  24

static GdkPixbuf *
get_file_pixbuf (gchar *file)
{
  GdkPixbuf *pixbuf = NULL;
  char *icon_name = NULL;
  char *mime_type;

  mime_type = gnome_vfs_get_file_mime_type (file, NULL, FALSE);

  if (file == NULL || mime_type == NULL) 
    {
      icon_name = g_strdup (ICON_THEME_REGULAR_ICON);
    } 
  else if ((g_file_test (file, G_FILE_TEST_IS_EXECUTABLE)) &&
	     !g_ascii_strcasecmp (mime_type, "application/x-executable-binary"))
    {
      icon_name = g_strdup (ICON_THEME_EXECUTABLE_ICON);
    }
  else if (!g_ascii_strcasecmp (mime_type, "x-special/device-char"))
    {
      icon_name = g_strdup (ICON_THEME_CHAR_DEVICE);
    }
  else if (!g_ascii_strcasecmp (mime_type, "x-special/device-block"))
    {
      icon_name = g_strdup (ICON_THEME_BLOCK_DEVICE);
    }
  else if (!g_ascii_strcasecmp (mime_type, "x-special/socket"))
    {
      icon_name = g_strdup (ICON_THEME_SOCKET);
    }
  else if (!g_ascii_strcasecmp (mime_type, "x-special/fifo"))
    {
      icon_name = g_strdup (ICON_THEME_FIFO);
    }
  else
    icon_name = gnome_icon_lookup (gtk_icon_theme_get_default (), NULL, file,
				   NULL, NULL, mime_type, 0, NULL);

  pixbuf = (GdkPixbuf *)g_hash_table_lookup (view->pixbuf_hash, icon_name);

  if (!pixbuf)
    {
      pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), 
					 icon_name,
					 ICON_SIZE, 0, NULL);
      g_hash_table_insert (view->pixbuf_hash, g_strdup (icon_name), pixbuf);
    }

  g_free (icon_name);

  return pixbuf;

  return NULL;
}

static void
fsexam_treeitem_free (TreeItem *item)
{
  TreeItem *item2 = item;

  // As one place holder, demo_toplevel isn't dynamically
  // generated, we can't free it
  if (!item || item == demo_toplevel) return;

  while (1)
    {
      if (!item->label) break;

      g_free (item->label);

      if (item->children)
	fsexam_treeitem_free (item->children);

      ++item;
    }

  g_free (item2);
}

static gint
get_dir_elements (DIR *dirp)
{
  struct dirent *dp;
  int num_elements = 0;

  while (dirp)
    {
      if ((dp = readdir (dirp)) != NULL)
	++num_elements;
      else
	break;
    }

  rewinddir (dirp);

  return num_elements;
}

/*
 * Change the title of treeview to indicate the current root directory
 * that treeview is based on.
 */
static void
fsexam_treeview_set_title (char *title)
{
  GtkTreeViewColumn *column;
  gchar *title2 = fsexam_validate_with_newline (title, FALSE);
  gchar *location = g_strdup_printf ("%s - %s", _("Location:"), title2);

  column = gtk_tree_view_get_column (GTK_TREE_VIEW (view->treeview), 0);
  gtk_tree_view_column_set_title (column, location);

  g_free (title2);
  g_free (location);
}

static gint
set_item_collation (gconstpointer a,
		    gconstpointer b)
{
  TreeItem *item_a = (TreeItem *)a;
  TreeItem *item_b = (TreeItem *)b;
  gint retval;

  if (!item_a->label || !item_b->label) return -1;

  // put directory ahead
  if (item_a->children && !item_b->children)
    return -1;
  if (!item_a->children && item_b->children)
    return 1;

  retval = g_strcasecmp (item_a->label, item_b->label);

  return (retval == 0 ? (strcmp (item_a->label, item_b->label) > 0 ? -1 : 1) : retval);
}

// FIXME - ugly implementation, how to output error message?
static TreeItem *
fsexam_treeitem_create (char *dir)
{
  TreeItem *tree;
  DIR *dirp;
  struct stat stat_buf;
  int count = 0;
  char *msg = NULL;
  int filenum = 0;

  if (!dir || stat (dir, &stat_buf) == -1) goto _ERR;

  if (!S_ISDIR (stat_buf.st_mode))
    {
      gchar *ptmp;

      tree = g_new0 (TreeItem, 2);
      tree->icon = get_file_pixbuf (dir);
      tree->label = g_strdup (g_path_get_basename (dir));
      tree->loaded = 1;

      // treepath shouldn't contain file name.
      // Actually 'dir' points to view->rootdir, we can't free view->rootdir
      // before accessing 'dir'.
      ptmp = g_strdup (g_path_get_dirname (dir));
      g_free (view->rootdir);
      view->rootdir = ptmp;

      return tree;
    }

  dirp = opendir (dir);
  if (!dirp) goto _ERR;

  filenum = get_dir_elements (dirp);
  tree = g_new0 (TreeItem, filenum);

  while (dirp)
    {
      struct dirent *dp;
      TreeItem *item;
      
      if ((dp = readdir (dirp)) != NULL)
	{
	  //char name[256];
	  char *name;
	  int name_len = 0;

	  // don't display hidden files
	  if (dp->d_name[0] == '.') continue;
	  
	  item = tree + count;
	  item->label = g_new0 (gchar, strlen (dp->d_name) + 1);
	  item->loaded = 1;
	  strcpy (item->label, dp->d_name);
	 
	  name_len = strlen(dir) + strlen(dp->d_name) + 10;
	  name = malloc(name_len);	
	  //memset (name, 0, 256);
	  memset(name, 0, name_len);

	  sprintf (name, "%s/%s", dir, dp->d_name);

	  item->icon = get_file_pixbuf (name);

	  if (stat (name, &stat_buf) == 0 && S_ISDIR (stat_buf.st_mode))
	    {
	      // mark it is unloaded yet and put demo_toplevel as its children
	      // to act as one placeholder
	      item->loaded = 0;
	      item->children = demo_toplevel;
	    }
	
	  free(name);  
	  ++count;
	}
      else
	{
	  closedir (dirp);
	  break;
	}
    }

  qsort (tree, count, sizeof (TreeItem), set_item_collation);

  return tree;

 _ERR:
     switch (errno)
       {
       case EACCES:
	 msg = g_strdup_printf (_("No read permission"));
	 break;
       case ENOENT:
	 msg = g_strdup_printf (_("Folder doesn't exist yet"));
	 break;
       }

     if (!dir) msg = g_strdup_printf (_("No folder specified"));

     if (msg)
       {
	 fsexam_statusbar_update (msg);

	 g_free (msg);
       }

     if (view->rootdir)
       {
	 g_free (view->rootdir);
	 view->rootdir = NULL;
       }

     return demo_toplevel;
}

static void
treemodel_create_with_treeitem (GtkTreeStore *model,
		       TreeItem *item,
		       GtkTreeIter parent_iter,
		       gboolean is_root)
{
  GtkTreeIter iter;

  while (item->label)
    {
      TreeItem *subitem = item->children;

      if (is_root)
	gtk_tree_store_append(model, &iter, NULL);
      else
	gtk_tree_store_append(model, &iter, &parent_iter);

      gtk_tree_store_set(model, &iter,
			 ICON_COLUMN, item->icon,
			 FILENAME_COLUMN, item->label,
			 LOADED_COLUMN, item->loaded,
			 -1);

      if (subitem)
	treemodel_create_with_treeitem(model, subitem, iter, 0);

      item++;
    }
}

/*
 * Check if it is one directory which hasn't been loaded yet,
 * if yes, load its files and add into model.
 * If permission issue exists, collapse the directory row and
 * write error message to statusbar. 
 */
static gboolean
fsexam_treeview_expand (GtkWidget *widget,
			GtkTreeIter *iter,
			GtkTreePath *path,
			gpointer user_data)
{
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));
  gboolean loaded;
  char *filename;

  gtk_tree_model_get (model, iter,		      
		      FILENAME_COLUMN, &filename,
		      LOADED_COLUMN, &loaded,
		      -1);

  if (!loaded)
    {
      TreeItem *item;
      GString *dir = fsexam_filename_get_path (model, *iter, view->rootdir);
      //char name[256];
      char *name;
      int  name_len = 0;
      GtkTreeIter dummy_iter;

      name_len = strlen(dir->str) + strlen(filename) + 10;
      name = malloc(name_len);
      memset (name, 0, name_len);
      g_sprintf (name, "%s/%s", dir->str, filename);

      item = fsexam_treeitem_create (name);

      if (item == demo_toplevel)
	{
	  // write the error message to statusbar
	  fsexam_statusbar_update (_("No read permission"));
	  // row expand disabled
	  gtk_tree_view_collapse_row (GTK_TREE_VIEW (widget), path);
	}
      else if (!item->label)
	{
	  // the directory doesn't contain any files
	  fsexam_statusbar_update (_("Null folder"));
	  // row expand disabled
	  gtk_tree_view_collapse_row (GTK_TREE_VIEW (widget), path);
	}
      else 
	{
	  // load the files under the directory and add into model
	  treemodel_create_with_treeitem (GTK_TREE_STORE (model), 
					  item,
					  *iter, 
					  FALSE);

	  // change it's status
	  gtk_tree_store_set (GTK_TREE_STORE (model), iter,
			      LOADED_COLUMN, TRUE,
			      -1);

	  // the dummy iter is the first child of 'iter', delete it
	  // from 'model' in order not to show it 
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

static GtkTreeModel *
fsexam_treemodel_create_with_treeitem (TreeItem *root)
{
  GtkTreeStore *model;
  GtkTreeIter iter;
  
  if (!root) return NULL;
 
  model = gtk_tree_store_new (NUM_COLUMNS,
			      GDK_TYPE_PIXBUF,
			      G_TYPE_STRING,
			      G_TYPE_BOOLEAN);

  treemodel_create_with_treeitem (model, root, iter, TRUE);

  return GTK_TREE_MODEL (model);
}

void
fsexam_treeview_construct (gchar *dir)
{
  TreeItem *toplevel;
  GtkTreeModel *model;

  view->rootdir = g_strdup (dir);

  toplevel = fsexam_treeitem_create (dir);

  model = fsexam_treemodel_create_with_treeitem (toplevel);

  gtk_tree_view_set_model (GTK_TREE_VIEW (view->treeview), model);

  fsexam_treeitem_free (toplevel);

  if (view->rootdir) fsexam_treeview_set_title (view->rootdir);
}

void
fsexam_statusbar_update (gchar *msg)
{
  gtk_statusbar_pop (GTK_STATUSBAR (view->statusbar), 0);
  gtk_statusbar_push (GTK_STATUSBAR (view->statusbar), 0, msg);
}

/*
 * Clear statusbar
 */
static gboolean 
fsexam_statusbar_fresh ()
{
  fsexam_statusbar_update ("");

  return TRUE;
}

//FIXME, it isn't able to select both folder and file in the dialog
static void
fsexam_chooser_dir ()
{
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new (_("Folder Selection"),
					GTK_WINDOW (view->mainwin),
					GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename;

      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      fsexam_treeview_construct (filename);
      fsexam_undo_removeall ();
    }

  gtk_widget_destroy (dialog);
}

static void
show_report_window ()
{
  static gboolean show_flag = FALSE;

  if (show_flag)
    {
      gtk_widget_hide (view->reportwin);
      show_flag = FALSE;
    }
  else
    {
      gtk_widget_show (view->reportwin);
      show_flag = TRUE;
    }
}

static void show_help()
{
  GError *err = NULL;

  gnome_help_display ("fsexam.xml", "fsexam-intro", &err);

  if (err)
    {
      GtkWidget *dialog;
      dialog = gtk_message_dialog_new (GTK_WINDOW (view->mainwin),
                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_CLOSE,
                                       _("There was an error displaying help: %s"),
                                       err->message);

      g_signal_connect (G_OBJECT (dialog),
                        "response",
                        G_CALLBACK (gtk_widget_destroy),
                        NULL);

      gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
      gtk_widget_show (dialog);
      g_error_free (err);
    }
}

static void
fsexam_about ()
{
  static GtkWidget *about = NULL;

  gchar *authors [] = {
    "Federic Zhang <Federic.Zhang@Sun.COM>",
    "Yong Sun <Yong.Sun@Sun.COM>",
    NULL
  };

  if (about != NULL)
    {
      gtk_window_present (GTK_WINDOW (about));
      return;
    }

  about = gnome_about_new (_("fsexam"), VERSION,
			   "Copyright 2003-2004",
			   _("fsexam is to help migrate file name and file content from legacy encoding to UTF8"),
			   (const char **)authors,
			   NULL, NULL, view->icon);

  gtk_window_set_destroy_with_parent (GTK_WINDOW (about), TRUE);

  g_signal_connect (G_OBJECT (about), "destroy",
		    G_CALLBACK (gtk_widget_destroyed), &about);

  gtk_widget_show (about);
}

static GtkWidget *
append_menuitem (GtkWidget *menu,
		 const char *text,
		 const char *accel_path,
		 gpointer callback,
		 gpointer data)
{
  GtkWidget *menu_item;

  menu_item = gtk_menu_item_new_with_mnemonic (text);

  if (accel_path)
    gtk_menu_item_set_accel_path (GTK_MENU_ITEM (menu_item),
				  accel_path);

  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu),
			 menu_item);

  if (callback)
    g_signal_connect (G_OBJECT (menu_item),
		      "activate",
		      callback, data);

  return menu_item;
}

static GtkWidget *
append_stock_menuitem (GtkWidget *menu,
		       const char *text,
		       const char *accel_path,
		       GtkAccelGroup *accel_group,
		       gpointer callback,
		       gpointer data)
{
  GtkWidget *menu_item;
  
  menu_item = gtk_image_menu_item_new_from_stock (text, accel_group);

  if (accel_path)
    gtk_menu_item_set_accel_path (GTK_MENU_ITEM (menu_item),
				  accel_path);

  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu),
			 menu_item);

  if (callback)
    g_signal_connect (G_OBJECT (menu_item),
		      "activate",
		      callback, data);

  return menu_item;
}

static GtkWidget *
append_check_menuitem (GtkWidget *menu,
		       const char *text,
		       const char *accel_path,
		       gboolean active,
		       gpointer callback,
		       gpointer user_data)
{
  GtkWidget *menu_item;

  menu_item = gtk_check_menu_item_new_with_mnemonic (text);

  if (accel_path)
    gtk_menu_item_set_accel_path (GTK_MENU_ITEM (menu_item),
				  accel_path);

  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item),
				  active);

  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL(menu), menu_item);

  if (callback)
    g_signal_connect (G_OBJECT (menu_item), "toggled", callback, user_data);

  return menu_item;
}

static void
set_menuitem_text (GtkWidget *menu_item,
		   const char *text)
{
  GtkWidget *child;

  child = gtk_bin_get_child (GTK_BIN (menu_item));

  if (child && GTK_IS_LABEL (child))
    gtk_label_set_text_with_mnemonic (GTK_LABEL (child), text);
}

static void
set_menuitem_accel (GtkWidget *menu_item,
		gchar *accel_path,
		guint accel_key,
		GdkModifierType accel_mods)
{
  gtk_accel_map_add_entry (accel_path, accel_key, accel_mods);
  gtk_menu_item_set_accel_path (GTK_MENU_ITEM (menu_item), accel_path);
}

static GtkWidget *
fsexam_construct_menu (GtkAccelGroup *accel_group)
{
  GtkWidget *menu;
  GtkWidget *menu_item;
  GtkWidget *menu_bar;

  menu_bar = gtk_menu_bar_new();

  menu_item = append_menuitem (menu_bar,
			       _("_File"), NULL,
			       NULL, NULL);
  menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (menu),
			    accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);

  menu_item = append_stock_menuitem (menu, 
				     GTK_STOCK_OPEN, 
				     ACCEL_PATH_OPEN,
				     accel_group,
				     G_CALLBACK (fsexam_chooser_dir),
				     NULL);
  set_menuitem_text (menu_item, _("_Open..."));

  menu_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  append_stock_menuitem (menu,
			 GTK_STOCK_QUIT,
			 ACCEL_PATH_EXIT,
			 accel_group,
			 G_CALLBACK (gtk_main_quit),
			 NULL);

  menu_item = append_menuitem (menu_bar,
			       _("_Edit"), NULL,
			       NULL, NULL);
  menu_action = menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (menu),
			    accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);

  view->undo_menuitem = menu_item = append_stock_menuitem (menu,
				     GTK_STOCK_UNDO,
				     ACCEL_PATH_UNDO,
				     accel_group,
				     G_CALLBACK (fsexam_undo),
				     NULL);
  set_menuitem_accel (menu_item, ACCEL_PATH_UNDO, GDK_Z, GDK_CONTROL_MASK);
  gtk_widget_set_sensitive (menu_item, FALSE);

  menu_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = append_stock_menuitem (menu,
				     GNOME_STOCK_PIXMAP_REVERT,
				     ACCEL_PATH_REVERT,
				     accel_group,
				     G_CALLBACK (fsexam_reverse),
				     NULL);
  set_menuitem_text (menu_item, _("_Restore Original File"));
  set_menuitem_accel (menu_item, ACCEL_PATH_REVERT, GDK_R, GDK_CONTROL_MASK);

  menu_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = append_stock_menuitem (menu,
				     GTK_STOCK_APPLY,
				     ACCEL_PATH_CONVERT_NAME,
				     accel_group,
				     G_CALLBACK (fsexam_filename_convert),
				     NULL);
  set_menuitem_text (menu_item, _("Convert _Filename"));
  set_menuitem_accel (menu_item, ACCEL_PATH_CONVERT_NAME, GDK_F, GDK_CONTROL_MASK);

  menu_item = append_stock_menuitem (menu,
				     GTK_STOCK_CONVERT,
				     ACCEL_PATH_CONVERT_CONTENT,
				     accel_group,
				     G_CALLBACK (fsexam_content_convert),
				     NULL);
  set_menuitem_text (menu_item, _("_Convert Content"));
  set_menuitem_accel (menu_item, ACCEL_PATH_CONVERT_CONTENT, GDK_C, GDK_CONTROL_MASK);

  menu_item = append_menuitem (menu,
			       _("Pre_view Content"),
			       ACCEL_PATH_PEEK_CONTENT,
			       G_CALLBACK (fsexam_content_peek),
			       NULL);
  set_menuitem_accel (menu_item, ACCEL_PATH_PEEK_CONTENT, GDK_V, GDK_CONTROL_MASK);

  menu_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

  menu_item = append_stock_menuitem (menu,
				     GTK_STOCK_PREFERENCES,
				     NULL,
				     NULL,
				     G_CALLBACK (create_pref_dialog),
				     NULL);
  set_menuitem_text (menu_item, _("_Preferences"));

  menu_item = append_menuitem (menu_bar,
			       _("_View"), NULL,
			       NULL, NULL);

  menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (menu),
			    accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);

  append_check_menuitem (menu,
			 _("_Report Pane"),
			 ACCEL_PATH_REPORT_PANE,
			 FALSE,
			 G_CALLBACK (show_report_window),
			 NULL);
			 
  menu_item = append_menuitem (menu_bar,
			       _("_Help"), NULL,
			       NULL, NULL);
  menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (menu),
			    accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);

  menu_item = append_stock_menuitem (menu,
				     GTK_STOCK_HELP,
				     ACCEL_PATH_HELP,
				     NULL,
				     G_CALLBACK (show_help),
				     NULL);
  set_menuitem_text (menu_item, _("_Contents"));
  set_menuitem_accel (menu_item, ACCEL_PATH_HELP, GDK_F1, 0);
  
  menu_item = append_stock_menuitem (menu,
				     GNOME_STOCK_ABOUT,
				     NULL,
				     NULL,
				     G_CALLBACK (fsexam_about),
				     NULL);
  set_menuitem_text (menu_item, _("_About"));

  return menu_bar;
}

static void
load_icon ()
{
  GError *error = NULL;

  view->icon = gdk_pixbuf_new_from_file (ICON_PATH, &error);

  if (error)
    {
      g_assert (view->icon == NULL);
      g_warning (_("Error loading icon %s\n"), error->message);
      g_error_free (error);
    }
  else
    gtk_window_set_icon (GTK_WINDOW (view->mainwin), view->icon);
}

GtkWidget *
fsexam_construct_ui (char *dir)
{
  GtkWidget *table;
  GtkAccelGroup *accel_group;
  GtkWidget *sw;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkWidget *pane;
  PangoFontDescription *fontdesc;
  GtkWidget *mainwin, *treeview, *reportwin;
  GtkWidget *menu_bar;
  
  view->mainwin = mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  fsexam_dnd_set (mainwin);

  load_icon ();
  gtk_window_set_title (GTK_WINDOW (mainwin), _("File System Examiner"));
  g_signal_connect (mainwin, "destroy",
		    G_CALLBACK (gtk_main_quit),
		    &mainwin);

  table = gtk_table_new (1, 3, FALSE);

  gtk_container_add (GTK_CONTAINER (mainwin), table);

  accel_group = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (mainwin), accel_group);
  menu_bar = fsexam_construct_menu(accel_group);
  gtk_table_attach(GTK_TABLE(table), menu_bar, 0, 1, 0, 1,  GTK_EXPAND | GTK_FILL, 0, 0, 0); 

  pane = gtk_hpaned_new ();

  gtk_table_attach (GTK_TABLE (table),
		    pane,
		    0, 1,                  1, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		    0,                     0);

  view->statusbar = gtk_statusbar_new ();
  gtk_statusbar_push (GTK_STATUSBAR (view->statusbar), 0, _("Welcome"));
  gtk_table_attach (GTK_TABLE (table),
		    view->statusbar,
		    /* X direction */      /* Y direction */
		    0, 1,                  2, 3,
		    GTK_EXPAND | GTK_FILL, 0,
		    0,                     0);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_ETCHED_IN);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  view->treeview = treeview = gtk_tree_view_new ();
  g_signal_connect (GTK_TREE_VIEW (treeview),
		    "cursor-changed",
		    G_CALLBACK (fsexam_statusbar_fresh),
		    NULL);
  g_signal_connect (GTK_TREE_VIEW (treeview),
		    "row-expanded",
		    G_CALLBACK (fsexam_treeview_expand),
		    NULL);

  gtk_widget_add_events(treeview, GDK_BUTTON_PRESS_MASK);
  g_signal_connect (GTK_TREE_VIEW (treeview), 
		    "button_press_event", 
		    G_CALLBACK (tree_popup_handler),
		    NULL); 
  g_signal_connect (GTK_TREE_VIEW (treeview),
		    "button_release_event",
		    G_CALLBACK (button_release_event),
		    NULL);
  g_signal_connect (GTK_TREE_VIEW (treeview),
		    "key-press-event",
		    G_CALLBACK (key_press_event),
		    NULL);

  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
			       GTK_SELECTION_MULTIPLE);

  /* create the file column */
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, _("Location:"));

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer,
				       "pixbuf", ICON_COLUMN,
				       NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_set_attributes (column, renderer,
				       "text", FILENAME_COLUMN,
				       NULL);

  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, TRUE);
  gtk_tree_view_column_set_sort_column_id (column, FILENAME_COLUMN);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  fsexam_treeview_construct (dir);
							    
  gtk_container_add (GTK_CONTAINER (sw), treeview);

  gtk_paned_pack1 (GTK_PANED (pane), sw, TRUE, FALSE);
  gtk_widget_set_size_request (sw, 50, -1);

  view->reportwin = reportwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (reportwin),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (reportwin),
				       GTK_SHADOW_IN);

  view->contents = gtk_text_view_new ();

  gtk_text_view_set_editable (GTK_TEXT_VIEW (view->contents), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view->contents), FALSE);

  // use small font to display text in text buffer
  fontdesc = pango_font_description_from_string ("Serif 10");
  gtk_widget_modify_font (view->contents, fontdesc);
  pango_font_description_free (fontdesc);

  gtk_container_add (GTK_CONTAINER (reportwin),
		     view->contents);

  gtk_paned_pack2 (GTK_PANED (pane), reportwin, TRUE, FALSE);
  gtk_widget_set_size_request (reportwin, 50, -1);

  gtk_window_set_default_size (GTK_WINDOW (mainwin), 450, 300);

  gtk_widget_show_all (mainwin);
  gtk_widget_hide (reportwin);

  return mainwin;
}
