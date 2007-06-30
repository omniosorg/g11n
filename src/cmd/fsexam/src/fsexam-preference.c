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
#include <libgnome/libgnome.h>
#include <string.h>
#include "encode.h"
#include "fsexam-history.h"
#include "fsexam-preference.h"
#include "fsexam.h"

#define  PATH           "/apps/fsexam"
#define  AUTOMATICMODE  "/apps/fsexam/auto_mode"
#define  RECURSIVEMODE  "/apps/fsexam/recur_mode"
#define  ENCODINGLIST   "/apps/fsexam/encoding"
#define  SUFFIXLIST     "/apps/fsexam/suffix"

#define  PREF_AUTO_MODE 0x1
#define  PREF_RECUR_MODE 0x2
#define  PREF_ENCODING_LIST 0x4

enum {
  COLUMN_ENCODING_NAME = 0,
  COLUMN_ENCODING_INDEX,
  ENCODING_NUM_COLS
};

void
fsexam_pref_free (FSEXAM_pref *pref)
{
  destroy_encode (pref->encode_list);
  g_free (pref);
}

static GtkTreeModel *
create_encodings_treeview_model (FSEXAM_pref *pref)
{
  GtkListStore *store;
  GtkTreeIter  iter;
  GList        *list = pref->encode_list;
  gint         i = 0;
  
  store = gtk_list_store_new (ENCODING_NUM_COLS,
			      G_TYPE_STRING,
			      G_TYPE_INT);

  while (list)
    {
      Encoding *en = (Encoding *)list->data;
      
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
			  COLUMN_ENCODING_NAME, en->codename,
			  COLUMN_ENCODING_INDEX, i,
			  -1);
      ++i;
      list = g_list_next (list);
    }

  return GTK_TREE_MODEL (store);
}

static gboolean
set_button_sensitive (GtkTreeView *treeview,
		      gpointer user_data)
{
  FSEXAM_pref *pref = view->pref;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint num_encode_list = g_list_length (pref->encode_list);

  if (num_encode_list == 1) return TRUE;

  selection = gtk_tree_view_get_selection (treeview);
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gint index;

      gtk_tree_model_get (model, &iter, COLUMN_ENCODING_INDEX, &index, -1);

      if (index == 0)
	{
	  gtk_widget_set_sensitive (pref->up_button, FALSE);
	  gtk_widget_set_sensitive (pref->down_button, TRUE);
	}
      else if (index == num_encode_list - 1)
	{
	  gtk_widget_set_sensitive (pref->up_button, TRUE);
	  gtk_widget_set_sensitive (pref->down_button, FALSE);
	}
      else
	{
	  gtk_widget_set_sensitive (pref->up_button, TRUE);
	  gtk_widget_set_sensitive (pref->down_button, TRUE);
	}
    }

  return TRUE;
}

static GtkWidget *
create_encodings_treeview (FSEXAM_pref *pref)
{
  GtkTreeModel *model;
  GtkWidget *encode_view;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  model = create_encodings_treeview_model (pref);
  encode_view = gtk_tree_view_new_with_model (model);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (encode_view), FALSE);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (encode_view)),
			       GTK_SELECTION_SINGLE);
  renderer = gtk_cell_renderer_text_new ();

  column = gtk_tree_view_column_new_with_attributes (NULL,
						     renderer, 
						     "text",
						     COLUMN_ENCODING_NAME,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (encode_view),
			       column);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (encode_view),
				   COLUMN_ENCODING_NAME);

  return encode_view;
}

static gint
compEncodeList (gconstpointer a,
		gconstpointer b,
		gpointer user_data)
{
  gchar *aText = ((Encoding *)a)->codename;
  gchar *bText = ((Encoding *)b)->codename;
  GSList *slist = (GSList *)user_data;
  gint ai = 0, bi = 0, i = 0;

  while (slist)
    {
      if (!strcmp ((gchar *)slist->data, aText)) ai = i;
      else if (!strcmp ((gchar *)slist->data, bText)) bi = i;

      ++i;
      slist = g_slist_next (slist);
    }

  return (ai - bi);
}

static void
fsexam_pref_encoding_sync (FSEXAM_pref *pref, GSList *slist)
{
  pref->encode_list = g_list_sort_with_data (pref->encode_list, compEncodeList, slist);
}

static GSList *
adjust_encoding_set (GtkTreeModel *model)
{
  GtkTreeIter iter;
  gchar *codename;
  GSList *list = NULL;

  gtk_tree_model_get_iter_first (model, &iter);

  do 
    {
      gtk_tree_model_get (model, &iter,
			  COLUMN_ENCODING_NAME, &codename,
			  -1);
      list = g_slist_append (list, (gpointer)codename);
    } 
  while (gtk_tree_model_iter_next (model, &iter));

  return list;
}

static void
fsexam_tree_iter_swap (GtkTreeModel *model, GtkTreeIter a, GtkTreeIter b)
{
  gchar *aText, *bText;

  gtk_tree_model_get (model, &a,
		      COLUMN_ENCODING_NAME, &aText,
		      -1);

  gtk_tree_model_get (model, &b,
		      COLUMN_ENCODING_NAME, &bText,
		      -1);

  gtk_list_store_set (GTK_LIST_STORE (model), &a,
		      COLUMN_ENCODING_NAME, bText,
		      -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &b,
		      COLUMN_ENCODING_NAME, aText,
		      -1);

  g_free (aText); g_free (bText);
}

static void
fsexam_pref_set (GtkWidget *button,
		 gpointer  user_data)
{
  FSEXAM_pref  *pref = (FSEXAM_pref *)user_data;
  GtkTreeSelection *selection;
  gint num_encode_list = g_list_length (pref->encode_list);
  
  if (GTK_BUTTON (button) == GTK_BUTTON (pref->recur_button))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
	pref->recur_mode = TRUE;
      else
	pref->recur_mode = FALSE;

      pref->changed |= PREF_RECUR_MODE;
    }
  else if (GTK_BUTTON (button) == GTK_BUTTON (pref->auto_button))
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
	pref->auto_mode = TRUE;
      else
	pref->auto_mode = FALSE;

      pref->changed |= PREF_AUTO_MODE;
    }
  else if (GTK_BUTTON (button) == GTK_BUTTON (pref->up_button))
    {
      GtkTreeModel *model;
      GtkTreeIter iter, prev;

      selection = gtk_tree_view_get_selection ((GtkTreeView *) pref->encode_view);

      if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
	  gint index;
	  
	  gtk_tree_model_get (model, &iter, COLUMN_ENCODING_INDEX, &index, -1);
	  if (index)
	    {
	      gchar *path;

	      if (index == num_encode_list - 1)
		gtk_widget_set_sensitive (pref->down_button, TRUE);
		
	      path = gtk_tree_path_to_string (gtk_tree_path_new_from_indices (--index, -1));
	      gtk_tree_model_get_iter_from_string (model, &prev, path);
	      gtk_tree_selection_select_iter (selection, &prev);

	      fsexam_tree_iter_swap (model, iter, prev);

	      if (index == 0)
		gtk_widget_set_sensitive (button, FALSE);

	      g_free (path);
	    }
	}

      pref->changed |= PREF_ENCODING_LIST;
    }
  else if (GTK_BUTTON (button) == GTK_BUTTON (pref->down_button))
    {
      GtkTreeModel *model;
      GtkTreeIter iter, next;

      selection = gtk_tree_view_get_selection ((GtkTreeView *) pref->encode_view);

      if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
	  gint index;

	  gtk_tree_model_get (model, &iter, COLUMN_ENCODING_INDEX, &index, -1);
	  if (index < num_encode_list - 1)
	    {
	      gchar *path;

	      if (index == 0)
		gtk_widget_set_sensitive (pref->up_button, TRUE);

	      path = gtk_tree_path_to_string (gtk_tree_path_new_from_indices (++index, -1));
	      gtk_tree_model_get_iter_from_string (model, &next, path);
	      gtk_tree_selection_select_iter (selection, &next);

	      fsexam_tree_iter_swap (model, iter, next);

	      if (index == num_encode_list - 1)
		gtk_widget_set_sensitive (button, FALSE);

	      g_free (path);
	    }

	  pref->changed |= PREF_ENCODING_LIST;
	}
    }
  else if (GTK_BUTTON (button) == GTK_BUTTON (pref->add_button))
    {
      // FIXME
    }
  else if (GTK_BUTTON (button) == GTK_BUTTON (pref->del_button))
    {
      // FIXME
    }
}

FSEXAM_pref *
create_fsexam_pref ()
{
  FSEXAM_pref *pref;
  GSList *encode_text;

  pref = g_new0 (FSEXAM_pref, 1);
  
  pref->client = gconf_client_get_default ();
  pref->recur_mode = gconf_client_get_bool (pref->client,
					    RECURSIVEMODE,
					    NULL);
  pref->auto_mode = gconf_client_get_bool (pref->client,
					   AUTOMATICMODE,
					   NULL);
  pref->suffix_list = (GSList *)gconf_client_get_list (pref->client,
						       SUFFIXLIST,
						       GCONF_VALUE_STRING,
						       NULL);
  encode_text = (GSList *)gconf_client_get_list (pref->client,
					    ENCODINGLIST,
					    GCONF_VALUE_STRING,
					    NULL);
  pref->encode_list = init_encode (encode_text);

  g_slist_free (encode_text);

  return pref;
}

static void
show_help ()
{
  GError *err = NULL;
  
  gnome_help_display ("fsexam.xml", "fsexam-prefs", &err);

  if (err)
    {
      GtkWidget *dialog;

      dialog = gtk_message_dialog_new (GTK_WINDOW (view->mainwin),
				       GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_MESSAGE_ERROR,
				       GTK_BUTTONS_CLOSE,
				       _("There was an error displaying help: %s"),
				       err->message);

      gtk_dialog_run (GTK_DIALOG (dialog));

      gtk_widget_destroy (dialog);

      g_error_free (err);
    }
}

void
create_pref_dialog (gpointer data,
		    gpointer action,
		    GtkWidget *widget)
{
  FSEXAM_pref *pref = view->pref;
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *button;
  gchar *markup;
  gint response;
  gint num_encode_list = g_list_length (pref->encode_list);

  dialog = gtk_dialog_new_with_buttons (_("File System Examiner preferences"),
					GTK_WINDOW (view->mainwin),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					NULL);
  
  hbox = gtk_hbox_new ( FALSE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox),
		      hbox,
		      FALSE,
		      FALSE,
		      0);

  table = gtk_table_new (1, 4, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);

  label = gtk_label_new (NULL);
  markup = g_strdup_printf ("<b>%s</b>", _("Sub-folder:"));
  gtk_label_set_markup (GTK_LABEL (label), markup);
  g_free (markup);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     label,
			     0, 1, 0, 1);

  pref->recur_button = gtk_check_button_new_with_mnemonic (_("_Recursive Mode"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pref->recur_button), 
				pref->recur_mode);
  g_signal_connect (G_OBJECT (pref->recur_button),
		    "toggled",
		    G_CALLBACK (fsexam_pref_set),
		    pref);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     pref->recur_button,
			     0, 1, 1, 2);

  label = gtk_label_new (NULL);
  markup = g_strdup_printf ("<b>%s</b>", _("User intervention:"));
  gtk_label_set_markup (GTK_LABEL (label), markup);
  g_free (markup);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     label,
			     0, 1, 2, 3);

  pref->auto_button = gtk_check_button_new_with_mnemonic (_("_Automatic Conversion"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pref->auto_button), 
				pref->auto_mode);
  g_signal_connect (G_OBJECT (pref->auto_button),
		    "toggled",
		    G_CALLBACK (fsexam_pref_set),
		    pref);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     pref->auto_button,
			     0, 1, 3, 4);

  table = gtk_table_new (1, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);

  label = gtk_label_new (NULL);
  markup = g_strdup_printf ("<b>%s</b>", _("Encode list:"));
  gtk_label_set_markup (GTK_LABEL (label), markup);
  g_free (markup);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  gtk_table_attach (GTK_TABLE (table),
		    label,
		    0, 1, 0, 1,
		    GTK_EXPAND | GTK_FILL, 0,
		    0, 0);

  pref->encode_view = create_encodings_treeview (pref);
  g_signal_connect (G_OBJECT (pref->encode_view),
		    "cursor-changed",
		    G_CALLBACK (set_button_sensitive),
		    NULL);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     pref->encode_view,
			     0, 1, 1, 2);

  table = gtk_table_new (4, 1, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);
  
  pref->add_button = gtk_button_new_from_stock (GTK_STOCK_ADD);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     pref->add_button,
			     0, 1, 0, 1);
  g_signal_connect (G_OBJECT (pref->add_button),
		    "clicked",
		    G_CALLBACK (fsexam_pref_set),
		    view->pref);

  view->pref->del_button = gtk_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     pref->del_button,
			     0, 1, 1, 2);
  g_signal_connect (G_OBJECT (pref->del_button),
		    "clicked",
		    G_CALLBACK (fsexam_pref_set),
		    pref);

  pref->up_button = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     pref->up_button,
			     0, 1, 2, 3);
  g_signal_connect (G_OBJECT (pref->up_button),
		    "clicked",
		    G_CALLBACK (fsexam_pref_set),
		    pref);

  if (num_encode_list == 1)
    gtk_widget_set_sensitive (pref->up_button, FALSE);

  pref->down_button = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     pref->down_button,
			     0, 1, 3, 4);
  g_signal_connect (G_OBJECT (pref->down_button),
		    "clicked",
		    G_CALLBACK (fsexam_pref_set),
		    pref);

  if (num_encode_list == 1)
    gtk_widget_set_sensitive (pref->down_button, FALSE);

  gtk_button_box_set_layout (GTK_BUTTON_BOX (GTK_DIALOG (dialog)->action_area),
			     GTK_BUTTONBOX_EDGE);

  button = gtk_button_new_from_stock (GTK_STOCK_HELP);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area),
		      button, TRUE, FALSE, 0);
  g_signal_connect (G_OBJECT (button),
		    "clicked",
		    G_CALLBACK (show_help),
		    NULL);
  gtk_widget_show (button);

  gtk_dialog_add_button (GTK_DIALOG (dialog),
			 GTK_STOCK_CLOSE,
			 GTK_RESPONSE_CLOSE);

  gtk_widget_show_all (hbox);

  // FIXME
  gtk_widget_hide (pref->add_button);
  gtk_widget_hide (pref->del_button);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_CLOSE)
    {
      if (pref->changed & PREF_ENCODING_LIST)
	{
	  GSList *slist;

	  slist = adjust_encoding_set (gtk_tree_view_get_model (GTK_TREE_VIEW (pref->encode_view)));

	  gconf_client_set_list (pref->client,
				 ENCODINGLIST,
				 GCONF_VALUE_STRING,
				 slist,
				 NULL);

	  fsexam_pref_encoding_sync (pref, slist);

	  g_slist_free (slist);
	}

      if (pref->changed & PREF_AUTO_MODE)
	gconf_client_set_bool (pref->client,
			       AUTOMATICMODE,
			       pref->auto_mode,
			       NULL);

      if (pref->changed & PREF_RECUR_MODE)
	gconf_client_set_bool (pref->client,
			       RECURSIVEMODE,
			       pref->recur_mode,
			       NULL);
    }

  pref->changed = 0x0;
  gtk_widget_destroy (dialog);
}
