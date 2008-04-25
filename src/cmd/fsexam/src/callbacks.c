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

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <strings.h>
#include <string.h>

#include <glade/glade.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>

#include "fsexam.h"
#include "file-filter.h"
#include "fsexam-header.h"
#include "fsexam-ui.h"
#include "callbacks.h"
#include "fsexam-debug.h"
#include "fsexam-preference-dialog.h"

static gboolean cb_pre_handling (void);
static void     common_convert (ConvType type);
static void     print_hash (gpointer key, gpointer value, gpointer data);
static void     free_selection (gpointer row, gpointer data);
static void     fsexam_undo_insert (gint32 serial);
static gint32   fsexam_undo_remove (void);

/*
 *  pre-handling before real conversion.
 */
static gboolean
cb_pre_handling ()
{
    FSEXAM_setting  *setting = view->setting;

    while (TRUE) {
        if ((setting->pref->encode_list == NULL) 
                && !setting->pref->auto_detect) {
            fsexam_gui_show_dialog (
                            GTK_WINDOW (view->mainwin),
                            GTK_MESSAGE_ERROR, 
                            _("No encoding provided. Please check 'Auto Detect'"
                              " button in preferences dialog or add encoding"
                              " through encoding dialog.\n"));

            cb_create_pref_dialog ();
        }else{
            break;
        }
    }

    return TRUE;
}

static void
common_convert (ConvType type)
{
    GtkTreeModel        *model = NULL;
    GtkTreeSelection    *selection = NULL;
    GtkWidget           *treeview = NULL;
    GtkWidget           *notebook = NULL;
    GList               *selected_rows = NULL;
    GList               *tmp = NULL;
    GList               *files = NULL;
    gint32              old_serial, new_serial;
    gboolean            need_hash = FALSE;
    GtkWidget           *widget = NULL;
    gchar               *widget_name = NULL;

    widget = gtk_window_get_focus (GTK_WINDOW (view->mainwin));
    if (widget != NULL)
        widget_name = (gchar *)gtk_widget_get_name (widget);

    if (widget_name != NULL &&
            strcmp (widget_name, "treeview_search") == 0) {
        /* convert search result */
        if (view->basedir == NULL)
            return;

        view->focus_treeview = treeview = widget;
    }else{
        /* convert files in left pane */
        if (NULL == view->rootdir)  
            return;

        treeview = g_object_get_data (G_OBJECT (view->mainwin), 
                                      "treeview_file");
        view->focus_treeview = treeview;
    }


    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    if (selection == NULL) {
        fsexam_statusbar_update (_("No Selection"));
        goto free;
    }

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
    selected_rows = gtk_tree_selection_get_selected_rows (selection, &model);
    if (selected_rows == NULL) {
        fsexam_statusbar_update (_("No files need to handle"));
        goto free;
    }

    if (!view->setting->pref->dry_run 
            && ((type == ConvName) || (type == RestoreConvName)))
        need_hash = TRUE;   /* use path hash to update GUI */

    for (tmp = selected_rows; tmp; tmp = g_list_next (tmp) ) {
        GtkTreeIter  iter;
        GtkTreePath  *treepath = NULL;
        
        treepath = (GtkTreePath *) tmp->data;
        if (gtk_tree_model_get_iter (model, &iter, treepath)){
            gchar *filename = NULL;

            filename = fsexam_filename_get_fullname (model, &iter);
            files = g_list_prepend (files, filename);
        
            if (need_hash) {
                g_hash_table_insert (view->treepath_hash, 
                           g_strdup (filename), 
                           gtk_tree_path_to_string (treepath));
            }
        }
    }

    if (fsexam_debug () & FSEXAM_DBG_HASH)
        g_hash_table_foreach (view->treepath_hash, print_hash, NULL);

    /* show conversion log notebook page or dryrun result notebook page */
    notebook = g_object_get_data (G_OBJECT (view->mainwin), "notebook_report");
    if (view->setting->pref->dry_run)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 1);
    else
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);
        

    old_serial = fsexam_history_get_serial (view->setting->hist_info);

    /* call underneath conversion func */
    if (type == ConvName) {
        fsexam_convert_filename_batch (view->setting, files);
    }else if (type == ConvContent) {
        fsexam_convert_content_batch (view->setting, files);
    }else if ((type == RestoreConvName) || (type == RestoreConvContent)) {
        fsexam_restore (view->setting, files, type);
    }

    new_serial = fsexam_history_get_serial (view->setting->hist_info);
    if (new_serial != old_serial) {
        fsexam_undo_insert (new_serial);    /* insert only when has history */
    }
    
    if (need_hash) {
        // g_hash_table_remove_all (view->treepath_hash); //need glib-2.12
        fsexam_hash_remove_all (view->treepath_hash);
        // view->treepath_hash = NULL;  //still need this for late conversion
    }

free:
    g_list_foreach (selected_rows, free_selection, NULL);
    g_list_free (selected_rows);
    fsexam_list_free (files);
    view->focus_treeview = NULL;

    return;
}

static void 
print_hash (gpointer key, gpointer value, gpointer data)
{
    printf ("value = %s, key = %s\n", (gchar *)value, (gchar *)key);

    return;
}

static void 
free_selection (gpointer row, gpointer data)
{
    gtk_tree_path_free ((GtkTreePath *) row);

    return;
}

static void
fsexam_undo_insert (gint32 serial)
{
    view->undo_list = g_slist_prepend (view->undo_list, (gpointer)serial);

    gtk_widget_set_sensitive (
            g_object_get_data (G_OBJECT (view->mainwin), "menu_undo"),
            TRUE);
    gtk_widget_set_sensitive (
            g_object_get_data (G_OBJECT (view->mainwin), "toolbutton_undo"),
            TRUE);

}

/* caller need ensure view->undo_list != NULL */
static gint32
fsexam_undo_remove ()
{
    gint32  serial = (gint32)view->undo_list->data;   /* First In Last Out */

    view->undo_list = g_slist_remove (view->undo_list, (gpointer) serial);

    if (view->undo_list == NULL) {
        gtk_widget_set_sensitive (
                  g_object_get_data (G_OBJECT (view->mainwin), "menu_undo"),
                  FALSE);
        gtk_widget_set_sensitive (
                  g_object_get_data (G_OBJECT (view->mainwin), "toolbutton_undo"),
                  FALSE);
    }

    return serial;
}

static void
common_dryrun (ConvType type)
{
    FSEXAM_setting   *setting = view->setting;
    gboolean         old_convtype;

    old_convtype = setting->pref->conv_content;
    setting->pref->dry_run = TRUE;

    fsexam_dryrun_buffer_clear_buffer(
            FSEXAM_DRYRUN_BUFFER (setting->dryrun_info));
    fsexam_dryrun_write_convtype (setting->dryrun_info, type);
    setting->pref->conv_content = (type == ConvName) ? FALSE : TRUE;
    
    cb_convert ();

    setting->pref->conv_content = old_convtype;
    setting->pref->dry_run = FALSE;

    return;
}

/* --------------- Public functions ------------------------ */

void
fsexam_undo_removeall ()
{
  g_slist_free (view->undo_list);
  view->undo_list = NULL;

  gtk_widget_set_sensitive (
                g_object_get_data (G_OBJECT (view->mainwin), "menu_undo"),
                FALSE);
  gtk_widget_set_sensitive (
                g_object_get_data (G_OBJECT (view->mainwin), "toolbutton_undo"),
                FALSE);
}

/*
 *  Restore won't remove the history info in hist_file.
 *  But does undo remove it? The answer is yes, because it
 *  is the semantic of UNDO
 *
 *  Actually hist_info will store the lastest op into 
 *  hist_info->hist_array[info->current]. So we won't use
 *  serial at all here.
 */
void
cb_undo ()
{
    ConvType    type;
    GList       *file_list = NULL;
    GtkWidget   *widget = NULL;
    gchar       *widget_name = NULL;

    if (view->undo_list == NULL)   /* In fact 'undo' menu is disabled now */
        return;

    (void) fsexam_undo_remove ();   

    /* get the undo files from history's last record */
    file_list = fsexam_history_get_last_list (
                        view->setting->hist_info,
                        &type);

    if (file_list == NULL) {
        fsexam_statusbar_update (_("Nothing need to undo"));
        return;
    }

    widget = gtk_window_get_focus (GTK_WINDOW (view->mainwin));
    if (widget != NULL)
        widget_name = (gchar *)gtk_widget_get_name (widget);

    if (widget_name != NULL &&
            strcmp (widget_name, "treeview_search") != 0) {
        widget = g_object_get_data (G_OBJECT (view->mainwin), 
                                      "treeview_file");
    }

    view->focus_treeview = widget;
    view->setting->flags |= FSEXAM_SETTING_FLAGS_UNDO;

    if ((type == ConvName) || (type == ConvNameSpecial)) {
        fsexam_restore (view->setting, file_list, RestoreConvName);
    }else if ((type == ConvContent) || (type == ConvContentSpecial)) {
        fsexam_restore (view->setting, file_list, RestoreConvContent);
    }

    view->setting->flags &= ~FSEXAM_SETTING_FLAGS_UNDO;

    /* remove latest history infor after undo */
    fsexam_history_remove_last (view->setting->hist_info);
    fsexam_list_free (file_list);

    return; 
}

void
cb_dryrun ()
{
    common_dryrun (view->setting->pref->conv_content ? ConvContent : ConvName);

    if (view->setting->ignore_num == view->setting->total_num) {
        fsexam_dryrun_write_msg (view->setting->dryrun_info, 
                _("No results. Please see report pane for more information.\n"));
    }

    gtk_widget_set_sensitive (
            g_object_get_data (G_OBJECT (view->mainwin), "menu_scenario"), 
            TRUE);
    gtk_widget_set_sensitive (
            g_object_get_data (G_OBJECT (view->mainwin), "toolbutton_scenario"), 
            TRUE);

    return;
}


void 
cb_preview_content ()
{
    fsexam_content_peek (0, 0);

    return;
}

void
cb_force_convert ()
{
    view->setting->pref->force = TRUE;
    cb_convert ();
    view->setting->pref->force = FALSE;

    return; 
}

void
cb_scenario ()
{
    gint32      old_serial, new_serial;
    GtkWidget   *widget = NULL;
    gchar       *widget_name = NULL;

    widget = gtk_window_get_focus (GTK_WINDOW (view->mainwin));
    if (widget != NULL)
        widget_name = (gchar *)gtk_widget_get_name (widget);

    if (widget_name != NULL && 
            strcmp (widget_name, "treeview_search") != 0) {
        widget = g_object_get_data (G_OBJECT (view->mainwin), 
                                      "treeview_file");
    }

    view->focus_treeview = widget;

    /* show conversion log notebook page or dryrun result notebook page */
    gtk_notebook_set_current_page (
            GTK_NOTEBOOK (g_object_get_data (G_OBJECT (view->mainwin), "notebook_report")),
            0);

    old_serial = fsexam_history_get_serial (view->setting->hist_info);

    /* reset the dryrun result output position */
    fsexam_dryrun_buffer_set_current_line (
            (FsexamDryrunBuffer *)view->setting->dryrun_info,
            0);

    fsexam_convert_scenario (view->setting);

    new_serial = fsexam_history_get_serial (view->setting->hist_info);
    if (new_serial != old_serial) {
        fsexam_undo_insert (new_serial);    /* insert only when has history */
    }

    view->focus_treeview = NULL;
    gtk_widget_set_sensitive (
            g_object_get_data (G_OBJECT (view->mainwin), "menu_scenario"), 
            FALSE);
    gtk_widget_set_sensitive (
            g_object_get_data (G_OBJECT (view->mainwin), "toolbutton_scenario"), 
            FALSE);

    return;
}

void
cb_restore ()
{
    common_convert (view->setting->pref->conv_content ? RestoreConvContent 
                                                      : RestoreConvName);

    return;
}

void
cb_convert ()
{
    if (! cb_pre_handling ())
        return;

    common_convert (view->setting->pref->conv_content ? ConvContent : ConvName);

    return;
}

/*
 * callback for "Content mode" and "Name mode" menu
 */
void
cb_mode ()
{
    GtkWidget *w = NULL;

    w = g_object_get_data (G_OBJECT (view->mainwin), "menu_content_mode");
    view->setting->pref->conv_content = gtk_check_menu_item_get_active (
            GTK_CHECK_MENU_ITEM (w));

    return;
}

/*
 * callback for "Name mode" popup toggle menu
 */
void
cb_name_mode_popup (GtkCheckMenuItem *menuitem, gpointer data)
{
    gboolean  content_mode;
    GtkWidget *w = NULL;

    content_mode = ! gtk_check_menu_item_get_active (menuitem);
    view->setting->pref->conv_content = content_mode;

    w = g_object_get_data (G_OBJECT (view->mainwin), 
            content_mode ? "menu_content_mode" : "menu_name_mode");
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (w), TRUE);

    return;
}

/*
 * callback for "Content mode" popup toggle menu
 */
void
cb_content_mode_popup (GtkCheckMenuItem *menuitem, gpointer data)
{
    gboolean  content_mode;
    GtkWidget *w = NULL;

    content_mode = gtk_check_menu_item_get_active (menuitem);
    view->setting->pref->conv_content = content_mode;

    w = g_object_get_data (G_OBJECT (view->mainwin), 
            content_mode ? "menu_content_mode" : "menu_name_mode");
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (w), TRUE);

    return;
}

/*
 * callback for the GtkEntry 'entry_folder'
 */
void
cb_change_folder ()
{
    GtkWidget   *widget = NULL;
   
    widget = g_object_get_data (G_OBJECT (view->mainwin), "entry_folder");
    fsexam_change_dir (gtk_entry_get_text (GTK_ENTRY (widget)));
    
    return;
}

/* 
 * callback for toggle signal of "show dryrun result" menu
 * show responding page of GtkNoteBook
 */
void
cb_show_dryrun_result (GtkCheckMenuItem *item, gpointer data)
{
    GtkWidget *w = NULL;
    gint      page_num;

    if (! gtk_check_menu_item_get_active (item))
        return;

    w = g_object_get_data (G_OBJECT (view->mainwin), "notebook_report");
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (w));

    if (page_num == 1)
        return;

    gtk_notebook_set_current_page (GTK_NOTEBOOK (w), 1);

    return;
}

/*
 * callback for toggle singal of "show report pane" menu
 */
void
cb_show_report_pane (GtkCheckMenuItem *item, gpointer data)
{
    GtkWidget *w;
    gint      page_num;

    if (! gtk_check_menu_item_get_active (item))
        return;

    w = g_object_get_data (G_OBJECT (view->mainwin), "notebook_report");
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (w));

    if (page_num == 0)
        return;

    gtk_notebook_set_current_page (GTK_NOTEBOOK (w), 0);

    return;
}

/*
 * Set Radio Menu Item for 'Show report pane' or 'Show Dryrun result page'
 */
void
cb_change_current_page (GtkNotebook *notebook, gint arg1, gpointer user_data)
{
    GtkWidget *w;
    gint      page_num;

    w = g_object_get_data (G_OBJECT (view->mainwin), "notebook_report");
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (w));

    w = g_object_get_data (G_OBJECT (view->mainwin),
            page_num == 0 ? "menu_report" : "menu_dryrun_result");
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (w), TRUE);

    return; 
}

/*
 * Callback for searching dialog  response events
 */
static void
search_dialog_response_handler (GtkWidget *dialog, gint id, gpointer data)
{
    if (id == GTK_RESPONSE_OK) {            /* find button */
        GtkWidget   *w = NULL;
        gchar       *param = NULL;
        gchar       *tmp = NULL;
        gchar       *folder = NULL;
        const gchar *user = NULL;
        const gchar *group = NULL;
        const gchar *name = NULL;
        gint        data_date;
        gint        status_date;

        w = g_object_get_data (G_OBJECT (dialog), "filebtn_search_folder");
        folder = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (w));

        w = g_object_get_data (G_OBJECT (dialog), "entry_name");
        name = gtk_entry_get_text (GTK_ENTRY (w));
        
        w = g_object_get_data (G_OBJECT (dialog), "entry_user");
        user = gtk_entry_get_text (GTK_ENTRY (w));

        w = g_object_get_data (G_OBJECT (dialog), "entry_group");
        group = gtk_entry_get_text (GTK_ENTRY (w));

        w = g_object_get_data (G_OBJECT (dialog), "spinbutton_data");
        data_date = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (w));
        
        w = g_object_get_data (G_OBJECT (dialog), "spinbutton_status");
        status_date = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (w));

        param = g_strdup (folder);
        
        if ((name != NULL) && (*name != '\0')) {
            tmp = g_strdup_printf ("%s -name %s", param, name);
            g_free (param);
            param = tmp;
        }

        if ((user != NULL) && (*user != '\0')) {
            tmp = g_strdup_printf ("%s -user %s", param, user);
            g_free (param);
            param = tmp;
        }

        if ((group != NULL) && (*group != '\0')) {
            tmp = g_strdup_printf ("%s -group %s", param, group);
            g_free (param);
            param = tmp;
        }

        if (data_date != 0) {
            tmp = g_strdup_printf ("%s -mtime %d", param, data_date);
            g_free (param);
            param = tmp;
        }

        if (status_date != 0) {
            tmp = g_strdup_printf ("%s -ctime %d", param, status_date);
            g_free (param);
            param = tmp;
        }

        gtk_widget_destroy (GTK_WIDGET (dialog));
       
        filter_gui_run (folder, param);

        g_free (folder);
        g_free (param);

        return;
    }

    gtk_widget_destroy (GTK_WIDGET (dialog));

    return;
}

/*
 * callback for menu "Search for files..."
 */
void
cb_filter ()
{
    GladeXML    *xml = NULL;
    GtkWidget   *dialog = NULL;
    GtkWidget   *w = NULL;

    xml = fsexam_gui_load_glade_file (FSEXAM_GLADE_FILE,
            "dialog_search", GTK_WINDOW (view->mainwin));
    
    if (xml == NULL)
        return;

    dialog = glade_xml_get_widget (xml, "dialog_search");
    g_signal_connect (G_OBJECT (dialog), "response",
            G_CALLBACK (search_dialog_response_handler), NULL);
    
    w = glade_xml_get_widget (xml, "filebtn_search_folder");
    g_object_set_data (G_OBJECT (dialog), "filebtn_search_folder", w);
    
    w = glade_xml_get_widget (xml, "entry_name");
    g_object_set_data (G_OBJECT (dialog), "entry_name", w);

    w = glade_xml_get_widget (xml, "entry_group");
    g_object_set_data (G_OBJECT (dialog), "entry_group", w);

    w = glade_xml_get_widget (xml, "entry_user");
    g_object_set_data (G_OBJECT (dialog), "entry_user", w);

    w = glade_xml_get_widget (xml, "spinbutton_data");
    g_object_set_data (G_OBJECT (dialog), "spinbutton_data", w);
    
    w = glade_xml_get_widget (xml, "spinbutton_status");
    g_object_set_data (G_OBJECT (dialog), "spinbutton_status", w);

    w = glade_xml_get_widget (xml, "button_find");
    g_object_set_data (G_OBJECT (dialog), "button_find", w);
    
    w = glade_xml_get_widget (xml, "button_stop");
    g_object_set_data (G_OBJECT (dialog), "button_stop", w);

    g_object_unref (xml);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);
    gtk_dialog_run (GTK_DIALOG (dialog));

    return;
}

/*
 * callback for menu "stop searching"
 */
void
cb_stop_search ()
{
    GtkWidget *w = NULL;

    if (view->pid < 0)
        return;

    fsexam_statusbar_update (_("Stop the Searching..."));

    if (view->pid > 0) {
        kill (view->pid, SIGKILL);
        wait (NULL);
    }
    view->pid = -1;
    
    stop_search = TRUE;

    w = g_object_get_data (G_OBJECT (view->mainwin), "menu_stop_search");
    gtk_widget_hide (w);
    w = g_object_get_data (G_OBJECT (view->mainwin), "menu_search");
    gtk_widget_show (w);

    return;
}

/*
 *  Hide the search result pane after click the Close icon 'x'
 */  
void
cb_hide_search_result ()
{
    fsexam_search_treeview_hide ();

    return;
}

/*
 * callback for menu "search result": show or hide
 */
void
cb_menu_search_result () 
{
    GtkWidget *w = NULL;

    w = g_object_get_data (G_OBJECT (view->mainwin), "menu_search_result");
    
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (w))) {
        fsexam_search_treeview_show ();
    }else{
        fsexam_search_treeview_hide ();
    }

    return;
}

/*
 * callback for "clear search result" menu
 */
void
cb_menu_clear_search ()
{
    GtkWidget *w = NULL;

    fsexam_search_treeview_append_file (NULL, TRUE);

    w = g_object_get_data (G_OBJECT (view->mainwin), "menu_clear_search");
    gtk_widget_set_sensitive (w, FALSE);

    w = g_object_get_data (G_OBJECT (view->mainwin), "label_result");
    gtk_label_set_text (GTK_LABEL (w), _("Search Result has been cleared."));

    g_free (view->basedir);
    view->basedir = NULL;

    return;
}

/*
 * "changed" event handler for dryrun text view
 */
void
cb_text_buffer_changed (GtkTextBuffer *buffer, gpointer user_data)
{
    GtkWidget   *textview = NULL;
    GtkTextMark *mark = NULL;

    textview = g_object_get_data (G_OBJECT (view->mainwin), "textview_dryrun");
    mark = gtk_text_buffer_get_mark (buffer, "end_mark");
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (textview), 
            mark,
            0.0,
            TRUE,
            0.0,
            1.0);

    return;
}

/*
 * callback for toolbar style change 
 */
void
cb_toolbar_style (GConfClient *client, 
		guint cnxn_id,
		GConfEntry *entry,
		gpointer user_data)
{
	GtkWidget 	*toolbar = NULL;
	const gchar *style;

	if (entry->value == NULL)
		return;
	style = gconf_value_get_string (entry->value);
	toolbar = g_object_get_data (G_OBJECT (view->mainwin), "toolbar");

	if (strcmp (style, "both") == 0) {
		gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH);
	}else if (strcmp (style, "both-horiz") == 0) {
		gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH_HORIZ);
	}else if (strcmp (style, "icons") == 0) {
		gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
	}else if (strcmp (style, "text") == 0) {
		gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_TEXT);
	}

	return;
}
