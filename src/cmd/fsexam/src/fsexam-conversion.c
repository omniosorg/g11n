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
#include <libgnome/gnome-i18n.h>
#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "encode.h"
#include "fsexam-history.h"
#include "fsexam-preference.h"
#include "fsexam-content.h"
#include "fsexam.h"

void
fsexam_undo_insert (guint serial)
{
  view->undo_list = g_slist_insert (view->undo_list, (gpointer)serial, -1);

  gtk_widget_set_sensitive (view->undo_menuitem, TRUE);
}

static guint
fsexam_undo_remove ()
{
  GSList *last = g_slist_last (view->undo_list);
  guint serial = (guint)last->data;

  view->undo_list = g_slist_remove (view->undo_list, (gpointer) last->data);

  if (view->undo_list == NULL)
    gtk_widget_set_sensitive (view->undo_menuitem, FALSE);

  return serial;
}

void
fsexam_undo_removeall ()
{
  g_slist_free (view->undo_list);

  view->undo_list = NULL;

  gtk_widget_set_sensitive (view->undo_menuitem, FALSE);
}


int indexg;
static void
fsexam_convert_candidate_set (GtkWidget *widget,
			      gpointer user_data)
{
  indexg = (int)user_data;
}

static gboolean
fsexam_convert_construct_ui (Encoding *encode,
			     gint index,
			     va_list args)
{
  GtkWidget *table;
  GtkWidget *entry;
  GtkWidget *radio;
  GtkWidget **radio_group;

  table = va_arg (args, GtkWidget *);
  radio_group = va_arg (args, GtkWidget **);

  if (!*radio_group)
    radio = *radio_group = gtk_radio_button_new_with_label (NULL,
					     encode->codename);
  else
    radio = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (*radio_group),
					    encode->codename);

  gtk_table_attach_defaults (GTK_TABLE (table),
			     radio,
			     0, 1, index, index+1);

  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (entry),
		      encode->u.converted_text);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     entry,
			     1, 2, index, index+1);
  g_signal_connect (G_OBJECT (radio),
			      "toggled",
			      G_CALLBACK (fsexam_convert_candidate_set),
			      (gpointer)index);

  return TRUE;
}

/*
 * Create the candidate dialog which lists candidates for user
 * to select which one is the best one.
 * Return the index number of the toggled button, if no button
 * is toggled, return 0, the index number of the first button.
 */
static gint
fsexam_convert_with_candidates (FSEXAM_pref *pref)
{
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *stock;
  GtkWidget *table;
  GtkWidget *radio_group = NULL;
  gint response;

  dialog = gtk_dialog_new_with_buttons (_("File System Examiner name conversion"),
					GTK_WINDOW (view->mainwin),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL,
					GTK_STOCK_OK,
					GTK_RESPONSE_OK,
					NULL);

  hbox = gtk_hbox_new (FALSE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
		      hbox,
		      FALSE,
		      FALSE,
		      0);

  stock = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
				    GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox),
		      stock,
		      FALSE, FALSE, 0);

  table = gtk_table_new (g_list_length (pref->encode_list), 3, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);

  // initialize the global variable
  indexg = -1;
  iterate_encode_with_func (pref->encode_list, (void *)fsexam_convert_construct_ui, 
			    table, &radio_group);

  gtk_widget_show_all (hbox);
  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      int index = 0;
      // translate into the index in encode list
      if (indexg == -1) 
	iterate_encode_with_func (pref->encode_list,
				  (void *)translate_encode_index,
				  &index,
				  &indexg);
    }
  else
    indexg = -1;

  gtk_widget_destroy (dialog);

  return indexg;
}

/*
 * Execute the 'rename' action with the new filename
 * return TRUE if it works, otherwise, return FALSE
 * If failed, update the status bar with one specific error message 
 */
static gboolean
fsexam_filename_rename (gchar *dir,
			gchar *oldname,
			gchar *newname)
{
  gchar *original, *new;
  gint val = -1;

  original = g_new0 (gchar, strlen (dir) + strlen (oldname) + 2);
  g_sprintf (original, "%s/%s", dir, oldname);

  new = g_new0 (gchar, strlen (dir) + strlen (newname) + 2);
  g_sprintf (new, "%s/%s", dir, newname);

  if (g_file_test (new, G_FILE_TEST_IS_DIR))
    {
      fsexam_statusbar_update (_("Folder already exists"));
      goto _ERR;
    }
  else if (g_file_test (new, G_FILE_TEST_EXISTS))
    {
      fsexam_statusbar_update (_("File already exists"));
      goto _ERR;
    }

  if ((val = rename (original, new)) == -1)
    {
      // failed, update the status bar with error message
      GFileError err;
      char *msg = NULL;

      // FIXME add more error message
      err = g_file_error_from_errno (errno);
      switch (errno)
	{
	case EACCES:
	  msg = g_strdup_printf (_("No permission to rename"));
	  break;
	case ENOENT:
	  msg = g_strdup_printf (_("File doesn't exist yet"));
	  break;
	}

      if (msg) 
	{
	  fsexam_statusbar_update (msg);
	  g_free (msg);
	}
    }

 _ERR:
  g_free (original);
  g_free (new);

  return (val == 0);
}

/*
 * Concatenate each part to generate its full path name
 * The returned GString should be freed with g_string_free ()
 */
GString *
fsexam_filename_get_path (GtkTreeModel *model,
			  GtkTreeIter iter,
			  gchar *fullpath)
{
  GtkTreeIter parent;
  GString *dir;

  dir = g_string_new (NULL);

  while (gtk_tree_model_iter_parent (model, &parent, &iter))
    {
      gchar *filename;

      gtk_tree_model_get (model, &parent,
			  FILENAME_COLUMN, &filename,
			  -1);
      iter = parent;
      
      dir = g_string_prepend (dir, filename);
      dir = g_string_prepend (dir, "/");

      g_free (filename);
    }

  if (fullpath)
    {
      // if it ends with '/', modify it to avoid two consecutive slashes
      char *ch = fullpath + strlen (fullpath) - 1;

      if (*ch == '/') *ch = 0;
      dir = g_string_prepend (dir, fullpath);
    }

  return dir;
}

/*
 * Look backward in text buffer to get the above directory name
 * Return the directory name if found, otherwise, return NULL
 */
static char *
fsexam_buffer_search_backward (GtkTextBuffer *buffer,
			       gint lineoffset)
{
  char *text = NULL;
  GtkTextIter start, end;

  while (lineoffset>=0)
    {
      gtk_text_buffer_get_iter_at_line_offset (buffer, &start, lineoffset, 0);
      gtk_text_buffer_get_iter_at_line_offset (buffer, &end, lineoffset+1, 0);
      text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

      // test if the line in text buffer contains directory name
      if (*text != '\n' && *text != '\t') break;

      --lineoffset;
      g_free (text);
      text = NULL;
    }

  return text;
}

static gboolean
write_to_buffer_in_report_pane (Encoding *encode,
				gint index,
				va_list args)
{
  gchar text[256];
  ConvType convtype;
  GtkTextBuffer *buffer;
  GtkTextIter *iter;
  GtkTextTag *colortag;
  char *filename;
  gint idx;

  memset (text, 0, 256);

  convtype = va_arg (args, ConvType);
  buffer = va_arg (args, GtkTextBuffer *);
  iter = va_arg (args, GtkTextIter *);
  colortag = va_arg (args, GtkTextTag *);
  filename = va_arg (args, char *);
  idx = va_arg (args, int);

  if (idx != index) return TRUE;

  switch (convtype)
    {
    case ConvName:
      g_sprintf (text, "\t[F] %s -- %s -> UTF8\n", encode->u.converted_text, encode->codename); 
      break;
    case ConvContent:
      g_sprintf (text, "\t[C] %s -- %s -> UTF8\n", filename, encode->codename);
      break;
    case ConvContentWithCRLF:
      g_sprintf (text, "\t[C] %s -- %s -> UTF8: %s\n", filename, encode->codename, _("delete Carriage Return"));
      break;
    }

  gtk_text_buffer_get_iter_at_line (buffer, iter, view->lineoffset);

  // use blue as background to indicate which encoding is used to convert
  // gtk_text_buffer_insert_with_tags (buffer, iter, text, -1, colortag, NULL);
  gtk_text_buffer_insert (buffer, iter, text, -1);
  ++view->lineoffset;

  return TRUE;
}

/*
 * Write the conversion result to report pane.
 * Use bold font to render directory name and blue as background in text buffer
 * to indicate which encoding is used to convert
 */
int
write_to_report_pane (FSEXAM_pref *pref, 
		      ConvType convtype,
		      gint index,
		      gchar *path,
		      gchar *filename,
		      gint lineoffset)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view->contents));
  GtkTextIter iter;
  char *text;
  static GtkTextTag *boldtag = NULL, *colortag = NULL;

  if (!boldtag)
    boldtag = gtk_text_buffer_create_tag (buffer, "bold_tag",
					  "font", "bold",
					  "size_points", 12.0,
					  NULL);
  if (!colortag)
    colortag = gtk_text_buffer_create_tag (buffer, "blue_foreground",
					   "foreground", "blue",
					   NULL);

  if (lineoffset && (text = fsexam_buffer_search_backward (buffer, lineoffset -1)) &&
      !strcmp (text, path))
    {
      // the current file is under the same directory with the above file
      gtk_text_buffer_get_iter_at_line (buffer, &iter, lineoffset);
      g_free (text);
    }
  else
    {
      // different directory, write the new directory name to text buffer
      gtk_text_buffer_get_iter_at_line (buffer, &iter, lineoffset);
      gtk_text_buffer_insert_with_tags (buffer, &iter, path, -1, boldtag, NULL);

      ++view->lineoffset;
    }

  if (index == -1 || 
      (convtype != ConvName && convtype != ConvContent && convtype != ConvContentWithCRLF))
    {
      gchar text[256];
      gchar *codename = "native";

      memset (text, 0, 256);
      switch (convtype)
	{
	case ConvContentCRLFOnly:
	  g_sprintf (text, "\t[C] %s: %s\n", filename, _("delete Carriage Return only"));
	  break;
	case ConvNameReverse:
	  g_sprintf (text, "\t[UF] %s -- UTF8 -> %s\n", filename, codename);
	  break;
	case ConvContentReverse:
	  g_sprintf (text, "\t[UC] %s -- UTF8 -> %s\n", filename, codename);
	  break;
	case ConvContentWithCRLFReverse:
	  g_sprintf (text, "\t[UC] %s -- UTF8 -> %s: %s\n", filename, codename, _("add Carriage Return"));
	  break;
	case ConvContentCRLFOnlyReverse:
	  g_sprintf (text, "\t[UC] %s: %s\n", filename, _("add Carriage Return only"));
	  break;
	}

      gtk_text_buffer_insert (buffer, &iter, text, -1);
      ++view->lineoffset;
    }
  else
    iterate_encode_with_func (pref->encode_list, (void *)write_to_buffer_in_report_pane,
			    convtype, buffer, &iter, colortag, filename, index);

  return 0;
}

/*
 * Given name, check if it is UTF8, use the sequece of question mark
 * to replace the non-utf8 part and append the ":\n" to the new string 
 * for convenience if newline_flag is set to TRUE.
 */
char *
fsexam_validate_with_newline (char *name,
			      gboolean newline_flag)
{
  char *text, *end;
  int length;

  if (newline_flag)
    {
      text = g_new0 (char, strlen (name) + 3);
      g_sprintf (text, "%s:\n", name);
    }
  else
    text = g_strdup (name);

  length = strlen (text);

  while (!g_utf8_validate (text, length, (const char **)&end)) *end = 0x3f;

  return text;
}

static void
fsexam_filename_convert_file (GtkTreeModel *model,
			      GtkTreeIter *iter,
			      gboolean same_serial)
{
  gchar *filename;
  FSEXAM_pref *pref = view->pref;
  Score score;

  gtk_tree_model_get (model, iter,
		      FILENAME_COLUMN, &filename,
		      -1);

  // if it is UTF8 encoding already, do nothing
  score = decode_analyzer (pref->encode_list, ConvName, filename, strlen (filename));
  if (score == FAIL)
    {
      fsexam_statusbar_update (_("File name - conversion failure"));
      goto done;
    }

  if (score == ORIGINAL)
    fsexam_statusbar_update (_("File name - UTF8 already"));
  else
    {
      GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->treeview));
      gint index = 0;
      Encoding *en;
      GString *dir;

      // Need reselection
      if (same_serial)
	gtk_tree_selection_select_iter (selection, iter);

      if (!pref->auto_mode)
	index = fsexam_convert_with_candidates (pref);
      else
	index = get_first_encode_index (pref->encode_list); 

      //conversion isn't needed any more
      if (index == -1) goto done;

      en = (Encoding *)g_list_nth_data (pref->encode_list, index);

      dir = fsexam_filename_get_path (model, *iter, view->rootdir);

      if (fsexam_filename_rename (dir->str, filename,
				  en->u.converted_text))
	{
	  char *text;
	  guint serial;

	  // write the pair of filenames into history file
	  serial = fsexam_history_put (view->histinfo, 
				       ConvName, 
				       filename, 
				       en->u.converted_text, 
				       dir->str,
				       same_serial);

	  if (!same_serial)
	    fsexam_undo_insert (serial);

	  gtk_tree_store_set (GTK_TREE_STORE (model), iter,
			      FILENAME_COLUMN, en->u.converted_text,
			      -1);

	  // indicate which encoding is used in the status bar
	  fsexam_statusbar_update (en->codename);

	  // to make sure that directory name is UTF8 encoding
	  text = fsexam_validate_with_newline (dir->str, TRUE);

	  // FIXME should we delete the conversion information due to undo?
	  //
	  // lineoffset records the current line in the text buffer
	  view->lineoffset += write_to_report_pane (pref, ConvName, index, text, 
				filename, view->lineoffset);

	  g_free (text);
	}

      g_string_free (dir, TRUE);
    }

done:
  g_free (filename);
}

static gboolean
is_dummy_iter(GtkTreeModel *model,
	      GtkTreeIter *iter)
{
  gchar *filename;
  int retval;

  gtk_tree_model_get (model, iter,
		      FILENAME_COLUMN, &filename,
		      -1);

  retval = strcmp (filename, "");

  g_free (filename);

  return (retval == 0);
}

static void
fsexam_filename_convert_dir (GtkTreeModel *model,
			     GtkTreeIter *child,
			     GtkTreeIter *iter)
{
  GtkTreePath *path = gtk_tree_model_get_path (model, iter);
  GtkTreeIter subiter;
  gboolean loaded = TRUE;

  gtk_tree_model_get (model, iter,
		      LOADED_COLUMN, &loaded,
		      -1);

  // if the row is collapsed and its subiters have been loaded already, 
  // expand it!
  if (loaded && !gtk_tree_view_row_expanded (GTK_TREE_VIEW (view->treeview), path))
    gtk_tree_view_expand_row (GTK_TREE_VIEW (view->treeview), path, TRUE);

  gtk_tree_path_free (path);

  do
    {
      fsexam_filename_convert_file (model, child, TRUE);

      if (gtk_tree_model_iter_children (model, &subiter, child) &&
	  !is_dummy_iter (model, &subiter))
	fsexam_filename_convert_dir (model, &subiter, child);
    }
  while (gtk_tree_model_iter_next (model, child));

  return;
}

static void
fsexam_filename_convert_single_selection (GtkTreeModel *model,
					  GtkTreePath  *path,
					  GtkTreeIter  *iter,
					  gpointer     user_data)
{
  GtkTreeIter subiter;
  gint *indicator = user_data;

  fsexam_filename_convert_file (model, iter, *indicator != 0);

  // check if the selected node is one directory and if recursive mode
  // is set, wall through the whole directory to convert with
  // fsexam_filename_convert_file ()
  if (view->pref->recur_mode &&
      gtk_tree_model_iter_children (model, &subiter, iter) &&
      !is_dummy_iter (model, &subiter))
    fsexam_filename_convert_dir (model, &subiter, iter);

  ++*indicator;
}

/*
 * convert the specified file/directory
 */
void
fsexam_filename_convert ()
{
  GtkTreeSelection *selection;
  gint same_serial_indicator = 0;

  // Do nothing if view->rootdir isn't set yet
  if (!view->rootdir) return;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->treeview));

  gtk_tree_selection_selected_foreach (selection,
				       fsexam_filename_convert_single_selection,
				       (gpointer) &same_serial_indicator);
}

static gboolean
fsexam_do_reverse (GtkTreeModel *model,
		   GtkTreeIter *iter,
		   ConvType convtype,
		   gchar *path,
		   gchar *filename,
		   gchar *oldname)
{
  char *target_encode = oldname;
  char name[256];
  char *text;
  gboolean retval = FALSE;

  memset (name, 0, 256);
  g_sprintf (name, "%s/%s", path, filename);
  text = fsexam_validate_with_newline (path, TRUE);

  switch (convtype)
    {
    case ConvName:
      if (fsexam_filename_rename (path, filename, oldname))
	{
	  gchar *str = fsexam_validate_with_newline (oldname, FALSE);

	  gtk_tree_store_set (GTK_TREE_STORE (model), iter,
			    FILENAME_COLUMN, oldname,
			    -1);
	  write_to_report_pane (view->pref, GetConvTypeReverse (convtype), -1, text,
				str, view->lineoffset);

	  retval = TRUE;
	  g_free (str);
	}
      break;
    case ConvContent:
    case ConvContentWithCRLF:
    case ConvContentCRLFOnly:
      {
	gchar *str = fsexam_validate_with_newline (filename, FALSE);

	if (fsexam_content_undo (name, target_encode, convtype))
	  write_to_report_pane (view->pref, GetConvTypeReverse (convtype), -1, text,
			      str, view->lineoffset);
	retval = TRUE;
	g_free (str);
      }
      break;
    defaults:
      break;
    }

  g_free (text);

  return retval;
}

static void 
fsexam_reverse_single_selection (GtkTreeModel *model,
				 GtkTreePath  *path,
				 GtkTreeIter  *iter,
				 gpointer     user_data)
{
  char *oldname, *filename, *text, *value;
  GString *dir;
  ConvType convtype;
  gint *indicator = (gint *)user_data;
  guint serial;

  gtk_tree_model_get (model, iter, FILENAME_COLUMN, &filename, -1);

  dir = fsexam_filename_get_path (model, *iter, view->rootdir);

  if (oldname = fsexam_history_get_reverse_by_value(view->histinfo,
						     &convtype,
						     filename,
						     dir->str))
    {
      if (convtype == ConvName)
	{
	  text = filename;
	  value = oldname;
	}
      else
	{
	  text = oldname;
	  value = filename;
	}

      if (fsexam_do_reverse (model, iter, convtype,
			     dir->str, filename, oldname))
	{
	  serial = fsexam_history_put (view->histinfo,
				       GetConvTypeReverse (convtype),
				       text,
				       value,
				       dir->str,
				       *indicator != 0);
	  if (*indicator == 0)
	    fsexam_undo_insert (serial);

	  ++*indicator;
	}
    }
  else
    goto Err;

  // Need look backward further to check whether the other conversion
  // exists, if yes, reverse it.
  switch (convtype)
    {
    case ConvName:
      convtype = ConvContent;
      g_free (filename);
      filename = g_strdup (oldname);
      break;
    case ConvContent:
    case ConvContentWithCRLF:
    case ConvContentCRLFOnly:
      convtype = ConvName;
      break;
    }

  if (!oldname) g_free (oldname);

  if (oldname = fsexam_history_get_reverse_by_value2 (view->histinfo,
						      &convtype,
						      filename,
						      dir->str))
    {
      if (convtype == ConvName)
	{
	  text = filename;
	  value = oldname;
	}
      else
	{
	  text = oldname;
	  value = filename;
	}
      
      if (fsexam_do_reverse (model, iter, convtype,
			     dir->str, filename, oldname))
	fsexam_history_put (view->histinfo,
			  GetConvTypeReverse (convtype),
			  text,
			  value,
			  dir->str,
			  TRUE);

      g_free (oldname);
    }

 Err:
  g_string_free (dir, TRUE);
  g_free (filename);
}

/*
 * Reverse all possible changes that have been made:
 *             file name
 *             file content
 */
void
fsexam_reverse ()
{
  GtkTreeSelection *selection;
  gint same_serial_indicator = 0;

  // Do nothing if view->rootdir isn't set yet
  if (!view->rootdir) return;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->treeview));
  
  gtk_tree_selection_selected_foreach (selection,
				       fsexam_reverse_single_selection,
				       (gpointer) &same_serial_indicator);
}

/*
 * Walk through the iter and and make recursive call to get the node
 * by comparing its content with the 'name', it is gurenteered that
 * the 'name' wouldn't start with '/'.
 * Return TRUE if node is found, otherwise, return FALSE
 */ 
static gboolean
get_iter (GtkTreeModel *model,
          GtkTreeIter *iter,
          GtkTreeIter *node,
          char *name)
{
  char *ch = name, *subname = NULL;

  // the name wouldn't start with '/'
  while (*ch != 0x0 && *ch != '/') ++ch;
  if (*ch == '/') { *ch = 0x0; subname = ++ch; }

  do
    {
      char *filename;

      gtk_tree_model_get (model, iter,
			  FILENAME_COLUMN, &filename,
			  -1);

      if (!strcmp (name, filename))
	{
	  GtkTreeIter subiter;

	  g_free (filename);

	  // the innest node, got it
	  if (*ch == 0x0)
	    {
	      *node = *iter;
	      return TRUE;
	    }

	  if (gtk_tree_model_iter_children (model, &subiter, iter) &&
	      !is_dummy_iter (model, &subiter))
	    {
	      if (get_iter (model, &subiter, node, subname))
		return TRUE;
	      else
		return FALSE;
	    }
	}
      else
	g_free (filename);
    }
  while (gtk_tree_model_iter_next (model, iter));

  return FALSE;
}

/*
 * Get the relative name by concatenate 'path' and 'filename'
 * and comparing it with 'root' and return 'subname',
 * if don't match, return NULL
 */
static char *
get_relative_name (char *path, 
		   char *filename, 
		   char *root)
{
  char *name, *subname, *pName, *pRoot;

  name = g_new0 (char, strlen (path) + strlen (filename) + 2);
  if (path[strlen (path) -1] == '/')
    g_sprintf (name, "%s%s", path, filename);
  else
    g_sprintf (name, "%s/%s", path, filename);

  pName = name; pRoot = root;
  while (*pName && *pRoot && *pName == *pRoot)
    { ++pName; ++pRoot; }

  if (*pRoot)
    {
      g_free (name);
      return NULL;
    }

  // make sure the subname wouldn't start with '/'
  if (*pName == '/') ++pName;

  subname = g_strdup (pName);

  g_free (name);

  return subname;
}

void
fsexam_undo ()
{
  char *oldname, *newname, *pathname;
  ConvType convtype;
  GtkTreeIter iter, node;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (view->treeview));
  unsigned int serial;

  if (view->undo_list == NULL) return;
  serial = fsexam_undo_remove ();

  while (fsexam_history_undo (view->histinfo, serial, &convtype,
			       &oldname, &newname, &pathname))
    {
      char *subname;
      char *native_encode = oldname;
      char name[256];
      char *text;

      memset (name, 0, 256);
      g_sprintf (name, "%s/%s", pathname, newname);
      text = fsexam_validate_with_newline (pathname, TRUE);

      if (!g_file_test (name, G_FILE_TEST_EXISTS)) continue;

      subname = get_relative_name (pathname, newname, view->rootdir);

      switch (convtype)
	{
	case ConvName:
	  // fsexam filename undo
	  gtk_tree_model_get_iter_first (model, &iter);
	  if (get_iter (model, &iter, &node, subname))
	    {
	      gchar *str = fsexam_validate_with_newline (oldname, FALSE);

	      gtk_tree_store_set (GTK_TREE_STORE (model), &node,
				  FILENAME_COLUMN, oldname,
				  -1);
	      fsexam_filename_rename (pathname, newname, oldname);
	      write_to_report_pane (view->pref, GetConvTypeReverse (convtype), -1, text,
				    str, view->lineoffset);
	      g_free (str);
	    }
	  break;
	case ConvNameReverse:
	  gtk_tree_model_get_iter_first (model, &iter);
	  if (get_iter (model, &iter, &node, subname))
	    {
	      gtk_tree_store_set (GTK_TREE_STORE (model), &node,
				  FILENAME_COLUMN, oldname,
				  -1);

	      fsexam_filename_rename (pathname, newname, oldname);

	      // FIXME add write_to_report_pane
	    }
	  break;
	case ConvContent:
	case ConvContentWithCRLF:
	case ConvContentCRLFOnly:
	  if (fsexam_content_undo (name, native_encode, convtype))
	    {
	      gchar *str = fsexam_validate_with_newline (newname, FALSE);

	      write_to_report_pane (view->pref, GetConvTypeReverse (convtype), -1, text,
				str, view->lineoffset);
	      g_free (str);
	    }
	  break;
	case ConvContentReverse:
	case ConvContentWithCRLFReverse:
	case ConvContentCRLFOnlyReverse:
	  // FIXME add write_to_report_pane
	  fsexam_content_undo (name, native_encode, convtype);
	  break;
	}

      g_free (newname), g_free (oldname);
      g_free (pathname), g_free (subname);
      g_free (text);
    }
}
