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
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>

#include "fsexam-header.h"
#include "fsexam-dialog.h"
#include "fsexam-ui.h"

extern gint indexg;

/*
 *  Callback for 'toggled' signal of GtkRadioButton
 */
void
fsexam_convert_candidate_set (GtkWidget *widget,
                              gpointer user_data)
{
    indexg = (gint)user_data;  /* the user_data is the real index in GList */
}

/*
 *  Iterate function for Encoding List
 *
 *  Add one element of Encoding list into candidate dialog as one candidate.
 */
static gboolean
fsexam_construct_index_ui (Encoding *encode,
                           gint index,     /* the index in encode_list */ 
                           va_list args)
{
    GtkWidget *table = NULL;
    GtkWidget *entry = NULL;
    GtkWidget *radio = NULL;
    GtkWidget **radio_group = NULL;
    gboolean  name_conversion;

    table = va_arg (args, GtkWidget *);
    radio_group = va_arg (args, GtkWidget **);
    name_conversion = va_arg (args, gboolean);

    /* Radio button */
    if (*radio_group == NULL) { /* is NULL for the first element */
        radio = gtk_radio_button_new_with_label (NULL,
                                        id2encoding (encode->encodingID));
        *radio_group = radio;
    } else {
        radio = gtk_radio_button_new_with_label_from_widget (
                                        GTK_RADIO_BUTTON (*radio_group),
                                        id2encoding (encode->encodingID));
    }

    gtk_table_attach (GTK_TABLE (table),
                      radio, 
                      0, 1, 
                      index, index+1,
                      GTK_FILL | GTK_SHRINK, /* GtkAttachOptions xoption */
                      GTK_SHRINK,            /* GtkAttachOptions yoption */
                      0,                     /* xpadding */
                      0);                    /* ypadding */

    /* Text Entry */
    entry = gtk_entry_new ();

    if (name_conversion) {
        gtk_entry_set_text (GTK_ENTRY (entry), encode->u.converted_text);
    }else{
        gtk_entry_set_text (GTK_ENTRY (entry), encode->u.contents);
    }

    gtk_table_attach (GTK_TABLE (table),
                      entry,
                      1, 2, 
                      index, index+1,
                      GTK_EXPAND | GTK_FILL,
                      GTK_SHRINK | GTK_FILL,
                      0,
                      0);


    g_signal_connect (G_OBJECT (radio),
                      "toggled",
                      G_CALLBACK (fsexam_convert_candidate_set),
                      (gpointer)index); /* this is the index in encode list */

    return TRUE;
}

#define BOX_SPACE  8

/*
 * Create candidates list dialog for user to select
 */
GtkWidget *
fsexam_dialog_candidate (GList *encoding, gboolean forname)
{
    GtkWidget   *dialog = NULL;
    GtkWidget   *hbox = NULL;
    GtkWidget   *vbox = NULL;
    GtkWidget   *stock = NULL;
    GtkWidget   *table = NULL;
    GtkWidget   *button_ask = NULL;
    GtkWidget   *radio_group = NULL;
    gint        num_encoding;

    num_encoding = fsexam_encoding_get_length (encoding);

    dialog = gtk_dialog_new_with_buttons (
                            _("Please select candidate"),
                            GTK_WINDOW (view->mainwin),
                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_STOCK_CANCEL,
                            GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,
                            GTK_RESPONSE_OK,
                            NULL);

    /* The main container for Dialog content */
    hbox = gtk_hbox_new (FALSE, BOX_SPACE);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), BOX_SPACE);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                        hbox,
                        TRUE,   /* expand */
                        TRUE,   /* fill */
                        0);     /* padding */

    /* Left part */
    vbox = gtk_vbox_new (FALSE, BOX_SPACE);
    gtk_box_pack_start (GTK_BOX (hbox), 
                        vbox, 
                        FALSE, 
                        FALSE, 
                        0);


    /* Add Quesiton stock ICON into HBox */
    stock = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
                                      GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (vbox), 
                        stock, 
                        TRUE, 
                        TRUE, 
                        0);
    /* Ask me button */
    button_ask = gtk_check_button_new_with_label (_("Don't ask me again"));
    g_object_set_data (G_OBJECT (dialog), "chkbtn_ask", button_ask);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_ask), FALSE);
    gtk_box_pack_start (GTK_BOX (vbox),
                        button_ask,
                        FALSE,
                        FALSE,
                        0);

    /* Create table */
    table = gtk_table_new (num_encoding,
                           2, 
                           FALSE);     /* homogouse */
    gtk_table_set_row_spacings (GTK_TABLE (table), 4);
    gtk_table_set_col_spacings (GTK_TABLE (table), 4);

    if (num_encoding > 10) {
        /* add scrollwindow */
        GtkWidget   *scroll_window = NULL;
        GtkWidget   *view_port = NULL;

        view_port = gtk_viewport_new (NULL, NULL);
        gtk_viewport_set_shadow_type (GTK_VIEWPORT (view_port),
                                       GTK_SHADOW_NONE);
        gtk_container_add (GTK_CONTAINER (view_port), table);

        scroll_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (
                            GTK_SCROLLED_WINDOW (scroll_window),
                            GTK_SHADOW_NONE);
        gtk_container_add (GTK_CONTAINER (scroll_window), view_port);
        
        gtk_box_pack_start (GTK_BOX (hbox), 
                            scroll_window, 
                            TRUE, 
                            TRUE, 
                            5);

        gtk_window_resize (GTK_WINDOW (dialog), 550, 400);
    }else{
        gint height;

        gtk_box_pack_start (GTK_BOX (hbox), 
                        table, 
                        TRUE, 
                        TRUE, 
                        0);

        gtk_window_get_size (GTK_WINDOW (dialog), NULL, &height);
        gtk_window_resize (GTK_WINDOW (dialog), 450, height);
    }

    fsexam_encoding_iterate_with_func (encoding, 
                                       fsexam_construct_index_ui,
                                       table, 
                                       &radio_group, 
                                       forname);

    gtk_widget_show_all (dialog);

    return dialog;
}
