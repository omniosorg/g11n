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

#include <stdlib.h>
#include <libgen.h>
#include <strings.h>
#include <string.h>

#include <glade/glade.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>

#include "fsexam-header.h"
#include "fsexam-debug.h"
#include "fsexam-preference-dialog.h"
#include "fsexam-ui.h"
#include "callbacks.h"

enum {
    COLUMN_ENCODING_NAME = 0,
    ENCODING_NUM_COLS
};

typedef enum {
    ActionInvalid,
    ActionAutoDetect,
    ActionAutoConversion,
    ActionRecursive,
    ActionHidden,
    ActionFollow,
    ActionRemote,
    ActionNoCheckSymlink,
    ActionSpecial,
    ActionCompress,
    ActionUseLog,
    ActionLogFile,
    ActionHistLen,
    ENCODING_UP,
    ENCODING_DOWN,
    ENCODING_ADD,
    ENCODING_DELETE,
    ENCODING_SAVE,
    ENCODING_RESET
} EncodingActionType;

static gboolean get_encoding_name_of_row (GtkTreeModel *,
                                          GtkTreePath *,
                                          GtkTreeIter *,
                                          gpointer);
static void selection_changed_callback (GtkTreeSelection *, gpointer);
static void handle_encoding_action (EncodingActionType, GtkWidget *);
static void cb_set_sensitive (GtkWidget *, gpointer);
static void cb_pref_reset (GtkDialog *, FSEXAM_pref *);
static void cb_encoding_manipulate (GtkWidget *, gpointer);
static void selection_changed_callback (GtkTreeSelection *, gpointer);
static void pref_dialog_response_callback (GtkWidget *, int, gpointer);
static void create_encodings_treeview (GtkTreeView *, GtkWidget *);
static void show_pref_help(void);

/*
 * Get the current rows encoding name, and append it to the user
 * provided list
 */
static gboolean
get_encoding_name_of_row (GtkTreeModel *model,
                          GtkTreePath  *path,
                          GtkTreeIter  *iter,
                          gpointer     user_data)
{
    GSList  **list = user_data;
    gchar   *name = NULL;

    gtk_tree_model_get (model, iter, COLUMN_ENCODING_NAME, &name, -1);
    *list = g_slist_prepend (*list, name);  /* free name when free list */

    return FALSE;
}

/*
 *  Event handler for Up/Down/Add/Remove/Save button of encoding treeview
 */
static void
handle_encoding_action (EncodingActionType type, GtkWidget *dialog)
{
    GtkTreeSelection *selection = NULL;
    GtkTreeModel     *model = NULL;
    GtkWidget        *view = NULL;
    GtkTreeIter      iter_cur, iter_other;

    if (type == ActionInvalid)
        return;

    view = g_object_get_data (G_OBJECT (dialog), "treeview_encoding_list");
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

    if (gtk_tree_selection_get_selected (selection, &model, &iter_cur)) {
        GtkTreePath *treepath = gtk_tree_model_get_path (model, &iter_cur);
        iter_other = iter_cur;

        switch (type) {
            case ENCODING_UP:
                if ((gtk_tree_path_prev (treepath)) 
                    && (gtk_tree_model_get_iter (model, &iter_other, treepath)))
                {
                        gtk_list_store_swap (GTK_LIST_STORE (model), 
                                             &iter_cur, 
                                             &iter_other);
                }

                gtk_widget_set_sensitive (
                        g_object_get_data (G_OBJECT (dialog), "button_down"),
                        TRUE);
                if (! gtk_tree_path_prev (treepath)) {
                    gtk_widget_set_sensitive (
                            g_object_get_data (G_OBJECT (dialog), "button_up"),
                            FALSE);
                }
                
                break;
            case ENCODING_DOWN:
                if (gtk_tree_model_iter_next (model, &iter_other)) {
                    gtk_list_store_swap (GTK_LIST_STORE (model), 
                                         &iter_cur, 
                                         &iter_other);
                }

                gtk_widget_set_sensitive (
                        g_object_get_data (G_OBJECT (dialog), "button_up"),
                        TRUE);
                if (! gtk_tree_model_iter_next (model, &iter_cur)) {
                    gtk_widget_set_sensitive (
                        g_object_get_data (G_OBJECT (dialog), "button_down"),
                        FALSE);
                }
            
                break;
            case ENCODING_DELETE:
                gtk_list_store_remove (GTK_LIST_STORE (model), &iter_cur);

                break;

            default:
                break;
        }

        if (treepath != NULL)
            gtk_tree_path_free (treepath);
    }

    return;
}

/*
 * Make related button sensitive or not,
 * according to 'Button Specail' and 'Button Use Log"
 */
static void
cb_set_sensitive (GtkWidget *widget, gpointer data)
{
    GtkWidget   *dialog = data;
    gboolean    active; 

    active = gtk_toggle_button_get_active (
                g_object_get_data (G_OBJECT (dialog), "special"));
    gtk_widget_set_sensitive(
                g_object_get_data (G_OBJECT (dialog), "compress"), 
                active);

    active = gtk_toggle_button_get_active (
                g_object_get_data (G_OBJECT (dialog), "use_log"));
    gtk_widget_set_sensitive (
                g_object_get_data (G_OBJECT (dialog), "log_file"), 
                active);
    gtk_widget_set_sensitive (
                g_object_get_data (G_OBJECT (dialog), "label_log_file"), 
                active);

    return;
}

/*
 * Refresh FSEXAM_pref data structure after clicked preference dialog's
 * 'OK' Button
 *
 * TODO: Use flags to indicate what we have changed.
 */
static void
cb_pref_reset (GtkDialog *dialog, FSEXAM_pref *pref)
{
    gboolean    active;
    GtkWidget   *w = NULL;

    w = g_object_get_data (G_OBJECT (dialog), "auto_detect");
    pref->auto_detect = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));

    w = g_object_get_data (G_OBJECT (dialog), "auto_convert");
    pref->auto_conversion = gtk_toggle_button_get_active (
                                    GTK_TOGGLE_BUTTON (w));

    w = g_object_get_data (G_OBJECT (dialog), "recursive");
    pref->recursive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));

    w = g_object_get_data (G_OBJECT (dialog), "hidden");
    pref->hidden = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));

    w = g_object_get_data (G_OBJECT (dialog), "follow");
    pref->follow = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));

    w = g_object_get_data (G_OBJECT (dialog), "remote");
    pref->remote = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));

    w = g_object_get_data (G_OBJECT (dialog), "nocheck_symlink");
    pref->no_check_symlink_content = gtk_toggle_button_get_active (
                                    GTK_TOGGLE_BUTTON (w));

    /* Special type handling setting */
    w = g_object_get_data (G_OBJECT (dialog), "special");
    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));

    if (active) {		/* enable special handling */
        w = g_object_get_data (G_OBJECT (dialog), "compress");
        active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));
        if (active) {
            pref->special |= SPECIAL_COMPRESS;
        }else{
            pref->special &= ~SPECIAL_COMPRESS;
        }
    }else{
        pref->special = SPECIAL_NO;
    }

    /* Log setting */
    w = g_object_get_data (G_OBJECT (dialog), "use_log");
    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));
    
    if (active) {
        gchar *new_log_name = NULL;

        pref->use_log = TRUE;

        w = g_object_get_data (G_OBJECT (dialog), "log_file");
        new_log_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (w));

        if ((pref->log_file == NULL) || ((new_log_name != NULL) 
			&& (strcmp (new_log_name, pref->log_file) != 0))) {
            Log_info  *new_log_info = fsexam_log_open (new_log_name);

            if (new_log_info != NULL) { /* change to new log struct */
                fsexam_log_close (view->setting->log_info);
                g_free (pref->log_file);

                pref->log_file = g_strdup (new_log_name);
                view->setting->log_info = new_log_info;
            }
        }

        g_free (new_log_name);
    }else{
        pref->use_log = FALSE;

        if (view->setting->log_info != NULL) {
            /*
             * We won't gfree(pref->log_file), result in it will remember the
             * last setting: will be written to gconf before exit.
             */
            fsexam_log_close (view->setting->log_info);
            view->setting->log_info = NULL;
        }
    }

    /* history length */
    w = g_object_get_data (G_OBJECT (dialog), "spinbutton_hist_len");
    pref->hist_len = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (w));

    /* update encoding */
    fsexam_pref_update_encoding (
                    view->setting->pref,
                    fsexam_prefdialog_get_encoding_list (GTK_WIDGET (dialog)),
                    NULL);

    return;
}

/* wrapper for cb_create_encoding_dialog func in other source file */
static void
create_encoding_dialog (GtkWidget *widget, gpointer data)
{
    GtkWidget   *parent = data;

    cb_create_encoding_dialog (parent);

    return;
}

/*
 *  Wrapper for handle_encoding_action: handle actions on encoding
 */
static void
cb_encoding_manipulate (GtkWidget *widget, gpointer user_data)
{
    GtkWidget   *dialog = user_data;

    EncodingActionType type = ActionInvalid;

    if (g_object_get_data (G_OBJECT (dialog), "button_up") == widget)
        type = ENCODING_UP;
    else if (g_object_get_data (G_OBJECT (dialog), "button_down") == widget)
        type = ENCODING_DOWN;
    else if (g_object_get_data (G_OBJECT (dialog), "button_delete") == widget)
        type = ENCODING_DELETE;
    else if (g_object_get_data (G_OBJECT (dialog), "button_reset") == widget)
        type = ENCODING_RESET;
    
    handle_encoding_action (type, dialog);

    if (GTK_WIDGET_CAN_FOCUS (widget))
        gtk_widget_grab_focus (widget);

    return;
}

/* 
 * callback for 'Save' button:
 * save encoding into gconf and sync with Encoding list
 */
static void
save_encoding_list (GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog = data;
    GtkWidget *confirm_dlg = NULL;
    gint      response;

    confirm_dlg = gtk_message_dialog_new (
                    GTK_WINDOW (view->mainwin),
                    GTK_DIALOG_DESTROY_WITH_PARENT,
                    GTK_MESSAGE_WARNING,
                    GTK_BUTTONS_YES_NO,
                    _("Do you want to save this encoding list permanently?"));

    response = gtk_dialog_run (GTK_DIALOG (confirm_dlg));

    if (response == GTK_RESPONSE_YES) {
        GSList *list;

        list = fsexam_prefdialog_get_encoding_list (dialog);
        fsexam_pref_update_encoding (view->setting->pref, 
                                     list, 
                                     view->setting->pref->gconf_client);
    }

    gtk_widget_destroy (confirm_dlg);

    return;
}

/*
 *  Fill in Model data according to Encoding List
 */
static void
set_encodings_treeview_model (GtkTreeView *view, GSList *encoding_list)
{
    GtkListStore  *store = NULL;
    GtkTreeIter   iter;
    GSList        *list = encoding_list;

    store = (GTK_LIST_STORE (gtk_tree_view_get_model (view)));
	gtk_list_store_clear (store);

    while (list) {
        short encoding_id = encoding2id ((gchar *)list->data);
        list = g_slist_next (list);

        if (encoding_id == -1)
            continue;

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COLUMN_ENCODING_NAME, id2encoding (encoding_id),
                            -1);
    }

	return;
}

/*
 * Create GtkTreeView and empty TreeModel
 */
static void
create_encodings_treeview (GtkTreeView *encode_view, 
                           GtkWidget *dialog)
{
    GtkCellRenderer     *renderer = NULL;
    GtkTreeViewColumn   *column = NULL;
    GtkTreeSelection    *selection = NULL;
	GtkListStore		*store = NULL;
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (NULL,
                                    renderer,
                                    "text", COLUMN_ENCODING_NAME,
                                    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (encode_view), column);
    gtk_tree_view_set_search_column (GTK_TREE_VIEW (encode_view),
                                    COLUMN_ENCODING_NAME);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (encode_view)),
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

    /* selection event handler */
    selection_changed_callback (selection, dialog);
    g_signal_connect (G_OBJECT (selection), 
                      "changed",
                      G_CALLBACK (selection_changed_callback), 
                      dialog);

	/* create empty model */
	store = gtk_list_store_new (ENCODING_NUM_COLS, G_TYPE_STRING);
    gtk_tree_view_set_model (GTK_TREE_VIEW (encode_view), 
							 GTK_TREE_MODEL (store));
    g_object_unref (store);

    return;

}

/*
 * callback to handle sensitive/insensitive of buttons
 */
static void
selection_changed_callback (GtkTreeSelection *selection, gpointer data)
{
    GtkWidget       *dialog = data;
    GtkWidget       *up_button = NULL;
    GtkWidget       *down_button = NULL;
    GtkWidget       *delete_button = NULL;
    GtkTreeModel    *model = NULL;
    GtkTreeIter     iter;
    gboolean        has_selection;

    up_button = g_object_get_data (G_OBJECT (dialog), "button_up");
    down_button = g_object_get_data (G_OBJECT (dialog), "button_down");
    delete_button = g_object_get_data (G_OBJECT (dialog), "button_delete");

    has_selection = gtk_tree_selection_get_selected (
                                        selection,
                                        &model,
                                        &iter);

    if (has_selection) {
        GtkTreePath *treepath = NULL;
    
        treepath = gtk_tree_model_get_path (model, &iter);

        gtk_widget_set_sensitive (delete_button, TRUE);

        if (! gtk_tree_path_prev (treepath))
            gtk_widget_set_sensitive (up_button, FALSE);
        else
            gtk_widget_set_sensitive (up_button, TRUE);
        
        if (! gtk_tree_model_iter_next (model, &iter))
            gtk_widget_set_sensitive (down_button, FALSE);
        else
            gtk_widget_set_sensitive (down_button, TRUE);

        gtk_tree_path_free (treepath);
    }else{
        gtk_widget_set_sensitive (up_button, FALSE);
        gtk_widget_set_sensitive (down_button, FALSE);
        gtk_widget_set_sensitive (delete_button, FALSE);
    }

    return;
}

static void 
show_pref_help(void)
{
    GError *err = NULL;

    gnome_help_display ("fsexam.xml", "fsexam-prefs", &err);

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

/*
 * Dialog response event hander 
 */
static void
pref_dialog_response_callback (GtkWidget *window, gint id, gpointer data)
{
    FSEXAM_pref     *pref = data;

    if (id == GTK_RESPONSE_OK) {
        cb_pref_reset (GTK_DIALOG (window), pref);

        if (fsexam_debug() & FSEXAM_DBG_OPTION) {
            fsexam_setting_print (view->setting);
        }
    } else if (id == GTK_RESPONSE_HELP) {
        show_pref_help ();
        return;
    }

    gtk_widget_destroy (window);

    return;
}

/*
 *  callback for 'Preferences' menu: create preference dialog 
 */
void
cb_create_pref_dialog ()
{
    FSEXAM_pref  *pref = view->setting->pref;
    GtkWidget    *dialog = NULL;
    GtkWidget    *w = NULL;
    GladeXML     *xml = NULL;
		
    xml = fsexam_gui_load_glade_file (FSEXAM_GLADE_FILE, 
                                      "pref_dialog",
                                      GTK_WINDOW (view->mainwin));
    if (xml == NULL)
        return;

    /* main dialog */
    dialog = glade_xml_get_widget (xml, "pref_dialog");
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), 
    		GTK_RESPONSE_CANCEL);
    g_signal_connect (G_OBJECT (dialog), "response",
    		G_CALLBACK (pref_dialog_response_callback),
    		pref);

    /* buttons */
    w = glade_xml_get_widget (xml, "chkbtn_auto_detect");
    g_object_set_data (G_OBJECT (dialog), "auto_detect", w);

    w = glade_xml_get_widget (xml, "chkbtn_auto_convert");
    g_object_set_data (G_OBJECT (dialog), "auto_convert", w);
    
    w = glade_xml_get_widget (xml, "chkbtn_recursive");
    g_object_set_data (G_OBJECT (dialog), "recursive", w);

    w = glade_xml_get_widget (xml, "chkbtn_hidden");
    g_object_set_data (G_OBJECT (dialog), "hidden", w);

    w = glade_xml_get_widget (xml, "chkbtn_follow");
    g_object_set_data (G_OBJECT (dialog), "follow", w);

    w = glade_xml_get_widget (xml, "chkbtn_remote");
    g_object_set_data (G_OBJECT (dialog), "remote", w);

    w = glade_xml_get_widget (xml, "chkbtn_nocheck_symlink");
    g_object_set_data (G_OBJECT (dialog), "nocheck_symlink", w);

    /* Special file type */
    w = glade_xml_get_widget (xml, "chkbtn_special");
    g_object_set_data (G_OBJECT (dialog), "special", w);
    g_signal_connect (G_OBJECT (w), "toggled",
                      G_CALLBACK (cb_set_sensitive), dialog);

    w = glade_xml_get_widget (xml, "chkbtn_compress");
    g_object_set_data (G_OBJECT (dialog), "compress", w);

    /* Use log or not */
    w = glade_xml_get_widget (xml, "chkbtn_use_log");
    g_object_set_data (G_OBJECT (dialog), "use_log", w);
    g_signal_connect (G_OBJECT (w), "toggled", 
                      G_CALLBACK (cb_set_sensitive), dialog);

    w = glade_xml_get_widget (xml, "label_log_file");
    g_object_set_data (G_OBJECT (dialog), "label_log_file", w);

    w = glade_xml_get_widget (xml, "filebtn_log_file");
    g_object_set_data (G_OBJECT (dialog), "log_file", w);

    w = glade_xml_get_widget (xml, "spinbutton_hist_len");
    g_object_set_data (G_OBJECT (dialog), "spinbutton_hist_len", w);

    /* Up/Down/Add/Delete buttons */
    w = glade_xml_get_widget (xml, "chkbtn_up");
    g_object_set_data (G_OBJECT (dialog), "button_up", w);
    g_signal_connect (G_OBJECT (w), 
                      "clicked",
                      G_CALLBACK (cb_encoding_manipulate),
                      dialog);

    w = glade_xml_get_widget (xml, "chkbtn_down");
    g_object_set_data (G_OBJECT (dialog), "button_down", w);
    g_signal_connect (G_OBJECT (w), 
                      "clicked",
                      G_CALLBACK (cb_encoding_manipulate),
                      dialog);

    w = glade_xml_get_widget (xml, "chkbtn_delete");
    g_object_set_data (G_OBJECT (dialog), "button_delete", w);
    g_signal_connect (G_OBJECT (w), 
                      "clicked",
                      G_CALLBACK (cb_encoding_manipulate),
                      dialog);

    w = glade_xml_get_widget (xml, "chkbtn_save");
    g_object_set_data (G_OBJECT (dialog), "button_save", w);
    g_signal_connect (G_OBJECT (w), 
                      "clicked",
                      G_CALLBACK (save_encoding_list),
                      dialog);

    w = glade_xml_get_widget (xml, "chkbtn_add");
    g_signal_connect (G_OBJECT (w), 
                      "clicked",
                      G_CALLBACK (create_encoding_dialog),
                      dialog);

    /* Init encoding list from FSEXAM_setting->pref->encoding_list */
    w  = glade_xml_get_widget (xml, "treeview_encoding_list");
    g_object_set_data (G_OBJECT (dialog), "treeview_encoding_list", w);

    /* create tree view and empty tree model */
    create_encodings_treeview (GTK_TREE_VIEW (w), dialog); 
   
    g_object_unref (xml);       /* destroy xml */

    /* buttons */
    w = g_object_get_data (G_OBJECT (dialog), "auto_detect");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), pref->auto_detect);

    w = g_object_get_data (G_OBJECT (dialog), "auto_convert");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), pref->auto_conversion);
    
    w = g_object_get_data (G_OBJECT (dialog), "recursive");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), pref->recursive);

    w = g_object_get_data (G_OBJECT (dialog), "hidden");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), pref->hidden);

    w = g_object_get_data (G_OBJECT (dialog), "follow");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), pref->follow);

    w = g_object_get_data (G_OBJECT (dialog), "remote");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w),  pref->remote);

    w = g_object_get_data (G_OBJECT (dialog), "nocheck_symlink");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), 
                                  pref->no_check_symlink_content);

    /* Special file type */
    w = g_object_get_data (G_OBJECT (dialog), "special");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), 
                                  pref->special == 0 ? FALSE : TRUE);

    w = g_object_get_data (G_OBJECT (dialog), "compress");

    gtk_widget_set_sensitive (w, pref->special == 0 ? FALSE : TRUE);
    if ((pref->special & SPECIAL_COMPRESS) != 0) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
    }else{
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), FALSE);
    }

    /* Use log or not */
    w = g_object_get_data (G_OBJECT (dialog), "use_log");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), pref->use_log);

    w = g_object_get_data (G_OBJECT (dialog), "log_file");

    if (pref->log_file != NULL) {
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (w), pref->log_file);
    }

    /* Use log or not */
    if (pref->use_log) {
        gtk_widget_set_sensitive (w, TRUE);
        gtk_widget_set_sensitive (
                g_object_get_data (G_OBJECT (dialog), "label_log_file"),
                TRUE);
    }else{
        gtk_widget_set_sensitive (w, FALSE);
        gtk_widget_set_sensitive (
                g_object_get_data (G_OBJECT (dialog), "label_log_file"),
                FALSE);
    }

    w = g_object_get_data (G_OBJECT (dialog), "spinbutton_hist_len");
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), (gdouble)pref->hist_len);

    /* Init encoding list from FSEXAM_setting->pref->encoding_list */
    w  = g_object_get_data (G_OBJECT (dialog), "treeview_encoding_list");
    set_encodings_treeview_model (GTK_TREE_VIEW (w), pref->encode_name_list);
   
    gtk_dialog_run (GTK_DIALOG (dialog));

    return;
}

/*
 *  Return a new list containing encoding names.
 *  Need free it when no one use it any more.
 */
GSList *
fsexam_prefdialog_get_encoding_list (GtkWidget *window)
{
    GtkWidget       *treeview = NULL;
    GtkTreeModel    *treemodel = NULL;
    GSList          *result = NULL;

    treeview  = g_object_get_data (G_OBJECT (window), "treeview_encoding_list");
    treemodel = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));

    gtk_tree_model_foreach (treemodel, 
                            get_encoding_name_of_row, 
                            (gpointer) &result);

    result = g_slist_reverse (result);
    
    return result;
}

/*
 *  Update encoding treeview according to new encoding list
 */
void
fsexam_prefdialog_update_encoding_model (GtkWidget *window, 
                                         GSList *new_encoding_list)
{
    GtkWidget       *treeview = NULL;
    GtkListStore    *store = NULL;
    GtkTreeIter     iter;
    GSList          *tmp = new_encoding_list;

    treeview = g_object_get_data (G_OBJECT (window), "treeview_encoding_list");
    store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));

    gtk_list_store_clear (store);

    while (tmp != NULL) {
        gchar *name = tmp->data;
        short encoding_id = encoding2id (name);

        tmp = tmp->next;

        if (encoding_id == -1)
            continue;

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COLUMN_ENCODING_NAME, name,
                            -1);
    }

    return;
}
