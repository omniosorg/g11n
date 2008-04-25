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

#include <glib.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include <gtk/gtk.h>

#include "fsexam-header.h"
#include "fsexam-encoding-dialog.h"
#include "fsexam-preference-dialog.h"
#include "fsexam-ui.h"

/* the encoding has been selected, will used to update encoding treeview */
static GSList *selected_encodings = NULL;

/*
 *  - convert encoding to locale for file(1)
 *  - convert encoding to cononical name
 *  - convert encoding to id
 *  - auto_ef encoding
 */
static FsexamEncoding encodings[] = {
    /* Index                    Charset        Cononical Name   Name                        locale */
    { FSEXAM_ENCODING_BIG5,     "BIG5",        "BIG5",          N_("Chinese Traditional"),  "zh_TW.BIG5" },
    { FSEXAM_ENCODING_HKSCS,    "BIG5-HKSCS",  "BIG5HKSCS",     N_("Chinese Traditional(Hong Kong)"),"zh_HK.BIG5HK" },
    { FSEXAM_ENCODING_CP1250,   "CP1250",      "CP1250",        N_("Central European"),     NULL }, 
    { FSEXAM_ENCODING_CP1251,   "CP1251",      "CP1251",        N_("Cyrillic"),             NULL },
    { FSEXAM_ENCODING_CP1252,   "CP1252",      "CP1252",        N_("Western"),              NULL },
    { FSEXAM_ENCODING_CP1253,   "CP1253",      "CP1253",        N_("Greek"),                NULL },
    { FSEXAM_ENCODING_CP1254,   "CP1254",      "CP1254",        N_("Turkish"),              NULL },
    { FSEXAM_ENCODING_CP1255,   "CP1255",      "CP1255",        N_("Hebrew"),               NULL },
    { FSEXAM_ENCODING_CP1256,   "CP1256",      "CP1256",        N_("Arabic"),               NULL },
    { FSEXAM_ENCODING_CP1257,   "CP1257",      "CP1257",        N_("Baltic"),               NULL },
    { FSEXAM_ENCODING_CP1258,   "CP1258",      "CP1258",        N_("Vietnamese"),           NULL },
    { FSEXAM_ENCODING_CP437,    "CP437",       "CP437",         N_("Western Europe"),       NULL },
    { FSEXAM_ENCODING_CP737,    "CP737",       "CP737",         N_("Greek"),                NULL },
    { FSEXAM_ENCODING_CP850,    "CP850",       "CP850",         N_("Western Europe"),       NULL },
    { FSEXAM_ENCODING_CP852,    "CP852",       "CP852",         N_("Central Europe"),       NULL },
    { FSEXAM_ENCODING_CP855,    "CP855",       "CP855",         N_("Russian"),              NULL },
    { FSEXAM_ENCODING_CP857,    "CP857",       "CP857",         N_("Turkish"),              NULL },
    { FSEXAM_ENCODING_CP860,    "CP860",       "CP860",         N_("Portuguese"),           NULL },
    { FSEXAM_ENCODING_CP861,    "CP861",       "CP861",         N_("Icelandic"),            NULL },
    { FSEXAM_ENCODING_CP863,    "CP863",       "CP863",         N_("French Canadian"),      NULL },
    { FSEXAM_ENCODING_CP865,    "CP865",       "CP865",         N_("Nordic"),               NULL },
    { FSEXAM_ENCODING_CP866,    "CP866",       "CP866",         N_("Cyrillic/Russian"),     NULL },
    { FSEXAM_ENCODING_CP869,    "CP869",       "CP869",         N_("Greek"),                NULL },
    { FSEXAM_ENCODING_CP949,    "CP874",       "CP874",         N_("Thai"),                 NULL },
    { FSEXAM_ENCODING_CP949,    "CP932",       "CP932",         N_("Japanese"),             NULL },
//  { FSEXAM_ENCODING_CP949,    "CP936",       "CP936",         N_("Simplified Chinese"),   NULL },
    { FSEXAM_ENCODING_CP949,    "CP949",       "CP949",         N_("Korean"),               NULL },
//  { FSEXAM_ENCODING_CP949,    "CP950",       "CP950",         N_("Traditional Chinese"),  NULL },
    { FSEXAM_ENCODING_eucJP,    "eucJP",       "EUCJP",         N_("Japanese"),             "ja_JP.eucJP" },
    { FSEXAM_ENCODING_EUC_KR,   "EUC-KR",      "EUCKR",         N_("Korean"),               "ko_KR.EUC" },
    { FSEXAM_ENCODING_EUC_TH,   "EUC-TH",      "EUCTH",         N_("Thai"),                 NULL },
    { FSEXAM_ENCODING_EUC_TW,   "EUC-TW",      "EUCTW",         N_("Chinese Traditional"),  "zh_TW.EUC" },
    { FSEXAM_ENCODING_GB18030,  "GB18030",     "GB18030",       N_("Chinese Simplified"),   "zh_CN.GB18030" },
    { FSEXAM_ENCODING_GB2312,   "GB2312",      "GB2312",        N_("Chinese Simplified"),   "zh" },
    { FSEXAM_ENCODING_GBK,      "GBK",         "GBK",           N_("Chinese Simplified"),   "zh_CN.GBK" },
    { FSEXAM_ENCODING_ISO_2022_CN,  "ISO-2022-CN",   "ISO2022CN",  N_("Chinese Simplified"), "zh" },
    { FSEXAM_ENCODING_ISO_2022_JP,  "ISO-2022-JP",  "ISO2022JP",  N_("Japanese"),           "ja" },
    { FSEXAM_ENCODING_ISO_2022_KR,  "ISO-2022-KR",  "ISO2022KR",  N_("Korean"),             "ko" },
    { FSEXAM_ENCODING_8859_1,   "ISO-8859-1",   "ISO88591",     N_("Western"),              "de_AT.ISO8859-1" },    
    { FSEXAM_ENCODING_8859_10,  "ISO-8859-10",  "ISO885910",    N_("Nordic"),               NULL },
    { FSEXAM_ENCODING_8859_11,  "ISO-8859-11",  "ISO885911",    N_("Thai"),                 "th_TH.ISO8859-11"},
    { FSEXAM_ENCODING_8859_13,  "ISO-8859-13",  "ISO885913",    N_("Baltic"),               "lv_LV.ISO8859-13"},
    { FSEXAM_ENCODING_8859_14,  "ISO-8859-14",  "ISO885914",    N_("Celtic"),               NULL },
    { FSEXAM_ENCODING_8859_15,  "ISO-8859-15",  "ISO885915",    N_("Western"),              "de_AT.ISO8859-15"},
    { FSEXAM_ENCODING_8859_16,  "ISO-8859-16",  "ISO885916",    N_("Romanian"),             NULL },
    { FSEXAM_ENCODING_8859_2,   "ISO-8859-2",   "ISO88592",     N_("Central European"),     "pl_PL.ISO8859-2" },
    { FSEXAM_ENCODING_8859_3,   "ISO-8859-3",   "ISO88593",     N_("South European"),       NULL },
    { FSEXAM_ENCODING_8859_4,   "ISO-8859-4",   "ISO88594",     N_("Baltic"),               NULL },
    { FSEXAM_ENCODING_8859_5,   "ISO-8859-5",   "ISO88595",     N_("Cyrillic"),             "bg_BG.ISO8859-5" },
    { FSEXAM_ENCODING_8859_6,   "ISO-8859-6",   "ISO88596",     N_("Arabic"),               NULL },
    { FSEXAM_ENCODING_8859_7,   "ISO-8859-7",   "ISO88597",     N_("Greek"),                NULL },
    { FSEXAM_ENCODING_8859_8,   "ISO-8859-8",   "ISO88598",     N_("Hebrew"),               NULL },
    { FSEXAM_ENCODING_8859_9,   "ISO-8859-9",   "ISO88599",     N_("Turkish"),              "tr_TR.ISO8859-9"},
    { FSEXAM_ENCODING_JOHAB,    "JOHAB",        "JOHAB",        N_("Korean"),               NULL },
    { FSEXAM_ENCODING_SJIS,     "SJIS",         "SJIS",         N_("Japanese"),             "ja_JP.PCK" },
/*   { FSEXAM_ENCODING_UTF_16,   "UTF-16",       "UTF16",        N_("Unicode"),              NULL },
    { FSEXAM_ENCODING_UTF_16BE, "UTF-16BE",     "UTF16BE",      N_("Unicode"),              NULL },
    { FSEXAM_ENCODING_UTF_16LE, "UTF-16LE",     "UTF16LE",      N_("Unicode"),              NULL },
*/
    { FSEXAM_ENCODING_UTF_8,    "UTF-8",        "UTF8",         N_("Unicode"),              NULL },
};

static short max_length = sizeof (encodings) / sizeof (encodings[0]);

gchar *
id2encoding (short id)
{
    if ((id < max_length) && (id >= 0))
        return encodings[id].charset;
    else
        g_print (_("Warning: The encoding id %d is greater than max length %d.\n"),
                 id, 
                 max_length);

    return NULL;
}

#define MAX_CODENAME_LEN    63

/* The following characters do not exist in the normalized code names. */
#define	SKIPPABLE_CHAR(c)	\
	((c) == '-' || (c) == '_' || (c) == '.' || (c) == '@')

short 
encoding2id (const gchar *encoding_name)
{
    char  s[MAX_CODENAME_LEN + 1];
    char  *tmp;
	int   i;
    short low = 0;
    short high = max_length - 1;
    short mid;

    if (encoding_name == NULL)
        return (short)-1;

    /*
	 * Normalize a code name, n, by not including skippable characters
	 * and folding uppercase letters to corresponding lowercase letters.
	 * We only fold 7-bit ASCII uppercase characters.
	 */
	for (i = 0, tmp = (char *)encoding_name; *tmp; tmp++) {
		if (SKIPPABLE_CHAR(*tmp))
			continue;

		/* If unreasonably lengthy, we don't support such names. */
		if (i >= MAX_CODENAME_LEN)
			return (short)-1;

		s[i++] = (*tmp >= 'a' && *tmp <= 'z') ? 
                 *tmp - 'a' + 'A' : *tmp;
	}
	s[i] = '\0';

    /* binsearch normalized name */
    while (low <= high) {
        gint   result;
    
        mid = (low + high) / 2;
        result = g_ascii_strcasecmp (s, encodings[mid].normalized_name);

        if (result == 0)
            return mid;
        else if (result > 0)
            low = mid + 1;
        else
            high = mid - 1;
    }

    g_print (_("Encoding '%s' is not supported.\n"), encoding_name);

    return (short)-1;
}

gchar *
encoding_to_locale (const gchar *encoding)
{
    gint id = -1;

    id = encoding2id (encoding);

    if (id != -1)
        return encodings[id].locale;
    else
        return NULL;
}

void
show_avail_encoding ()
{
    gint i;

    g_print (_("The supported encoding list:\n"));
    for (i = 0; i < max_length; i ++) {
        g_print ("\t%-15s\t%s\n", encodings[i].charset, _(encodings[i].name));
    }

    return;
}

/* Encoding manipulate dialog */

enum {
    ENCODING_COLUMN_NAME,
    ENCODING_COLUMN_CHARSET,
    ENCODING_N_COLUMNS
};

/*
 * whether given encoding has in list already or not
 */
static gboolean
encoding_in_selected_list (GSList *list, const gchar *name)
{
    GSList  *tmp = list;

    if ((NULL == list) || (NULL == name))
        return FALSE;

    while (tmp != NULL) {
        if (g_ascii_strcasecmp (name, (gchar *)tmp->data) == 0)
            return TRUE;

        tmp = g_slist_next (tmp);
    }

    return FALSE;
}

/*
 *  Update selected encoding TreeView according to selected_encoding
 */
static void
update_selected_tree_model (GtkListStore *store)
{
    GSList      *tmp = selected_encodings;
    GtkTreeIter iter;

    if (NULL == store)
        return;

    gtk_list_store_clear (store);

    while (tmp != NULL) {
        short encoding_id = encoding2id (tmp->data);
        tmp = tmp->next;

        if (encoding_id == -1)
            continue;

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (
                store, &iter,
                ENCODING_COLUMN_CHARSET, encodings[encoding_id].charset,
                ENCODING_COLUMN_NAME, _(encodings[encoding_id].name),
               -1);
    }

    return;
}

/*
 *  Remove the selected encoding after click 'Remove' button
 *  Just remove encoding from selected_encodings list.
 */
static void
remove_encoding_of_cur_row (GtkTreeModel   *model,
                            GtkTreePath    *path,
                            GtkTreeIter    *iter,
                            gpointer       data)
{
    gchar   *charset = NULL;
    gchar   *name = NULL;
    
    gtk_tree_model_get (model, 
                        iter,
                        ENCODING_COLUMN_CHARSET, &charset,
                        ENCODING_COLUMN_NAME, &name,
                        -1);

    selected_encodings = remove_string_from_slist (selected_encodings, charset);

    g_free (charset);
    g_free (name);

    return;
}

/*
 *  Add current selected encoding 
 */
static void
add_encoding_of_cur_row (GtkTreeModel   *model,
                         GtkTreePath    *path,
                         GtkTreeIter    *iter,
                         gpointer       data)
{
    GtkWidget *selected_treeview = data;
    gchar     *charset = NULL;
    gchar     *name = NULL;

    gtk_tree_model_get (model, 
                        iter,
                        ENCODING_COLUMN_CHARSET, &charset,
                        ENCODING_COLUMN_NAME, &name,
                        -1);

    if (! encoding_in_selected_list (selected_encodings, charset)) {
        GtkTreeModel    *selected_model;
        GtkTreeIter     selected_iter;

        /* Add new encoding to selected tree view */
        selected_model = gtk_tree_view_get_model (
                                    GTK_TREE_VIEW (selected_treeview));
        gtk_list_store_append (GTK_LIST_STORE (selected_model), &selected_iter);
        gtk_list_store_set (GTK_LIST_STORE (selected_model), &selected_iter,
                            ENCODING_COLUMN_CHARSET, charset,
                            ENCODING_COLUMN_NAME, name,
                            -1);
        
        /* Add new encoding to static gchar *selected_encodings */
        selected_encodings = g_slist_append (selected_encodings, 
                                             g_strdup (charset));
    }

    g_free (charset);
    g_free (name);

    return;
}

/*
 * count the number of selected encoding. used for button sensitive
 */
static void
count_selected_items_func (GtkTreeModel *model,
                           GtkTreePath  *path,
                           GtkTreeIter  *iter,
                           gpointer     data)
{
    gint *count = data;

    *count += 1;

    return;
}

/*
 *  callback for 'Add' button
 */
static void
add_button_clicked_callback (GtkWidget *button, gpointer data)
{
    GtkWidget           *dialog = data;
    GtkWidget           *treeview = NULL;
    GtkTreeSelection    *selection = NULL;

    treeview = g_object_get_data (G_OBJECT (dialog), 
                                "encoding-dialog-available-treeview");
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

    treeview = g_object_get_data (G_OBJECT (dialog), 
                                "encoding-dialog-displayed-treeview");

    gtk_tree_selection_selected_foreach (selection,
                                         add_encoding_of_cur_row,
                                         treeview);
    
    return;
}

/*
 *  callback for 'Remove' button
 */
static void
remove_button_clicked_callback (GtkWidget *button, gpointer data)
{
    GtkWidget           *dialog = data;
    GtkWidget           *treeview = NULL;
    GtkTreeSelection    *selection = NULL;
    GtkListStore        *store = NULL;

    treeview = g_object_get_data (G_OBJECT (dialog), 
                                  "encoding-dialog-displayed-treeview");
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

    /* Remove from selected_encoding list first */
    gtk_tree_selection_selected_foreach (selection,
                                         remove_encoding_of_cur_row,
                                         NULL);

    /* Then get the reference of store and update it */
    store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
    update_selected_tree_model (store);

    return;
}

/*
 *  Set the button's sensitivity
 */
static void
available_selection_changed_callback (GtkTreeSelection *selection, 
                                      gpointer data)
{
    GtkWidget   *dialog = data;
    GtkWidget   *add_button = NULL;
    gint        count = 0;

    gtk_tree_selection_selected_foreach (selection,
                                         count_selected_items_func,
                                         &count);

    add_button = g_object_get_data (G_OBJECT (dialog), "encoding-dialog-add");
    gtk_widget_set_sensitive (add_button, count > 0);

    return;
}

/*
 * Set button's sensitivity.
 */
static void
selected_selection_changed_callback (GtkTreeSelection *selection, gpointer data)
{
    GtkWidget   *dialog = data;
    GtkWidget   *remove_button = NULL;
    int         count = 0;

    gtk_tree_selection_selected_foreach (selection,
                                         count_selected_items_func,
                                         &count);

    remove_button = g_object_get_data (G_OBJECT (dialog), "encoding-dialog-remove");

    gtk_widget_set_sensitive (remove_button, count > 0);

    return;
}

/*
 *  Create selected encoding GtkTreeModel
 */
static void
register_selected_encoding_tree_model (GtkListStore *store, 
                                       GSList *selected_list)
{
    GSList      *list = selected_list;
    GtkTreeIter iter;

    if ((NULL == store) || (NULL == selected_list))
        return;

    while (list) {
        short   encoding_id = encoding2id ((gchar *)list->data);

        if (encoding_id != -1) {
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter,
                    ENCODING_COLUMN_CHARSET, id2encoding (encoding_id),
                    ENCODING_COLUMN_NAME, _(encodings[encoding_id].name),
                    -1);
        }

        list = g_slist_next (list);
    }

    return;
}

/* 
 * Response call back for Encoding dialog
 */
static void
dialog_response_callback (GtkWidget *window, gint id, gpointer data)
{
    GtkWidget *dialog = data;

    if (id == GTK_RESPONSE_OK) {
        /* 
         * update encoding list in Preferences dialog 
         * according to selected_encodings 
         */
        fsexam_prefdialog_update_encoding_model (dialog, selected_encodings);
    }

    /* free selected_encodings */
    fsexam_slist_free (selected_encodings);
    selected_encodings = NULL;

    gtk_widget_destroy (GTK_WIDGET (window));

    return;
}

/* 
 *  callback for encoding Add event 
 *      GtkWindow *parent: The preference dialog, used to generate 
 *                         selected encoding list
 */ 
void
cb_create_encoding_dialog (GtkWidget *parent)
{
    gint            i;
    GladeXML        *xml =  NULL;
    GtkWidget       *dialog = NULL;
    GtkWidget       *widget = NULL;
    GtkListStore    *tree = NULL;
    GtkTreeIter     iter;
    GtkTreeViewColumn *column = NULL;
    GtkCellRenderer   *renderer = NULL;
    GtkTreeSelection  *selection = NULL;

    if (parent == NULL)
        return;

    xml = fsexam_gui_load_glade_file (FSEXAM_GLADE_FILE, 
                                      "encodings-dialog",
                                      GTK_WINDOW (view->mainwin));
    if (xml == NULL)
        return;

    dialog = glade_xml_get_widget (xml, "encodings-dialog");
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
    g_signal_connect (G_OBJECT (dialog), "response",
                      G_CALLBACK (dialog_response_callback),
                      parent);

    /* buttons */
    widget = glade_xml_get_widget (xml, "add-button");
    g_object_set_data (G_OBJECT (dialog),
                       "encoding-dialog-add",
                       widget);
    g_signal_connect (G_OBJECT (widget), "clicked",
                      G_CALLBACK (add_button_clicked_callback),
                      dialog);

    widget = glade_xml_get_widget (xml, "remove-button");
    g_object_set_data (G_OBJECT (dialog),
                       "encoding-dialog-remove",
                       widget);
    g_signal_connect (G_OBJECT (widget), "clicked",
                      G_CALLBACK (remove_button_clicked_callback),
                      dialog);

    /* Create List Store (Model) */
    tree = gtk_list_store_new (ENCODING_N_COLUMNS, 
                               G_TYPE_STRING, 
                               G_TYPE_STRING);

    /* Add the data into Model */
    i = 0;
    while (i < (gint) G_N_ELEMENTS (encodings) - 1) { /* skip 'UTF-8' by -1 */
        gtk_list_store_append (tree, &iter);
        gtk_list_store_set (tree, &iter,
                    ENCODING_COLUMN_CHARSET, encodings[i].charset,
                    ENCODING_COLUMN_NAME, _(encodings[i].name),
                    -1);
        ++i;
    }

    /* Tree view of avaiable encodings */
    widget = glade_xml_get_widget (xml, "available-treeview");
    g_object_set_data (G_OBJECT (dialog),
                       "encoding-dialog-available-treeview",
                       widget);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
                    _("_Description"), renderer,
                    "text", ENCODING_COLUMN_NAME,
                    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (widget), column);
    gtk_tree_view_column_set_sort_column_id (column, ENCODING_COLUMN_NAME);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
                    _("_Encoding"), renderer,
                    "text", ENCODING_COLUMN_CHARSET,
                    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (widget), column);
    gtk_tree_view_column_set_sort_column_id (column, ENCODING_COLUMN_CHARSET);

    gtk_tree_view_set_model (GTK_TREE_VIEW (widget), GTK_TREE_MODEL (tree));
    g_object_unref (G_OBJECT (tree));

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
                                 GTK_SELECTION_MULTIPLE);

    available_selection_changed_callback (selection, dialog);
    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (available_selection_changed_callback),
                      dialog);

    /* Tree view of selected encodings */
    widget = glade_xml_get_widget (xml, "displayed-treeview");
    g_object_set_data (G_OBJECT (dialog),
                       "encoding-dialog-displayed-treeview",
                       widget);
    tree = gtk_list_store_new (ENCODING_N_COLUMNS, 
                               G_TYPE_STRING, 
                               G_TYPE_STRING);
    
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
                    _("_Description"), renderer,
                    "text", ENCODING_COLUMN_NAME,
                    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (widget), column);
    gtk_tree_view_column_set_sort_column_id (column, ENCODING_COLUMN_NAME);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
                    _("_Encoding"), renderer,
                    "text", ENCODING_COLUMN_CHARSET,
                    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (widget), column);
    gtk_tree_view_column_set_sort_column_id (column, ENCODING_COLUMN_CHARSET);

    /* Add the data */
    selected_encodings = fsexam_prefdialog_get_encoding_list (parent);
    register_selected_encoding_tree_model (tree, selected_encodings);

    gtk_tree_view_set_model (GTK_TREE_VIEW (widget), GTK_TREE_MODEL (tree));
    g_object_unref (G_OBJECT (tree));
    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
                                 GTK_SELECTION_MULTIPLE);

    selected_selection_changed_callback (selection, dialog);
    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (selected_selection_changed_callback),
                      dialog);

    gtk_dialog_run (GTK_DIALOG (dialog));
    g_object_unref (G_OBJECT (xml));

    return;
}
