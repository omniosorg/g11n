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
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "fsexam.h"
#include "fsexam-error.h"
#include "fsexam-helper.h"
#include "file-filter.h"
#include "fsexam-header.h"
#include "fsexam-ui.h"

#define CMD_NAME                "/usr/bin/find"
#define FSEXAM_REFRESH_TIMEOUT  50000

static gchar **
filter_compose_argv (const gchar *params)
{
    gchar *full_cmd = NULL;
    gchar **argv = NULL;

    if (params == NULL) {
        return NULL;
    }

    full_cmd = g_strdup_printf ("%s %s", CMD_NAME, params);
    argv = g_strsplit (full_cmd, " ", -1);
    g_free (full_cmd);

    return argv;
}

/*
 * Run find(1) with given params, and return the result
 */
GList *
filter_cmd_run (const gchar *params)
{
    FILE    *fp = NULL;
    gchar   **argv = NULL;
    gchar   *buf = NULL;
    GList   *list = NULL;
    GError  *error = NULL;
    gint    child_stdout;

    argv = filter_compose_argv (params);
    if (argv == NULL) {
        g_print (_("Parameters of find(1) is empty.\n"));
        return NULL;
    }

    /* create pipe */
    if (!g_spawn_async_with_pipes (NULL, argv,
                NULL,       /* envp */
                0,      /* GSpawnFlags */
                NULL, NULL,
                NULL,   /* pid */
                NULL,
                &child_stdout,
                NULL, 
                &error)) {
        g_print (error->message);
        g_error_free (error);
        g_strfreev (argv);

        return NULL;
    }

    /* create FILE pointer from subprocess's stdout */
    if ((fp = fdopen (child_stdout, "r")) == NULL) {
        g_print (_("Can't open pipe file descriptor.\n"));
        g_strfreev (argv);
        return NULL;
    }
        
    buf = (gchar *) g_malloc (PATH_MAX);
    g_print (_("Searching..."));
    g_print ("\n");

    while (fgets (buf, PATH_MAX, fp) != NULL) {
        gint  len = strlen (buf);

        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';

        list = g_list_prepend (list, g_strdup (buf));
    }

    g_print (_("Search finished"));
    g_print ("\n");

    g_free (buf);
    g_strfreev (argv);
    fclose (fp);

    return list;
}

/*
 * Run file(1) and append searched files into search result treeview 
 */
void
filter_gui_run (const gchar *folder, const gchar *params)
{
    FILE    *fp = NULL;
    gchar   **argv = NULL;
    GError  *error = NULL;
    gint    child_stdout;
    gint    file_count = 0;
    gchar   *buf = NULL;
    gchar   *msg = NULL;
    GTimer  *timer = NULL;
    gulong  duration;
    GtkWidget *menu_stop = NULL;
    GtkWidget *menu_search = NULL;
    GtkWidget *menu_clear = NULL;
    GtkWidget *label_result = NULL;

    argv = filter_compose_argv (params);
    if (argv == NULL) {
        g_print (_("Parameters of find(1) is empty.\n"));
        return;
    }

    /* create pipe */
    if (!g_spawn_async_with_pipes (NULL, argv, 
                NULL,       /* envp */
                0,          /* GSpawnFlags */
                NULL, NULL, 
                &view->pid, 
                NULL,
                &child_stdout,
                NULL,
                &error)) {
        GtkWidget *dialog = NULL;

        dialog = gtk_message_dialog_new (GTK_WINDOW (view->mainwin),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                _("Error occurs during executing the search command."));
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                error->message);

        gtk_dialog_run (GTK_DIALOG (dialog));
        
        gtk_widget_destroy (dialog);
        g_error_free (error);
        g_strfreev (argv);
        
        return;
    }

    /* create FILE pointer from subprocess's stdout */
    if ((fp = fdopen (child_stdout, "r")) == NULL) {
        GtkWidget *dialog = NULL;

        dialog = gtk_message_dialog_new (GTK_WINDOW (view->mainwin),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                _("Error occurs when open fd."));
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                g_strerror (errno));

        gtk_dialog_run (GTK_DIALOG (dialog));
        
        gtk_widget_destroy (dialog);
        g_strfreev (argv);

        return;
    }

    /*
     * Prepare for searching 
     */
    menu_clear = g_object_get_data (G_OBJECT (view->mainwin), 
                                    "menu_clear_search");
    menu_search = g_object_get_data (G_OBJECT (view->mainwin), "menu_search");
    menu_stop = g_object_get_data (G_OBJECT (view->mainwin), 
                                    "menu_stop_search");
    gtk_widget_set_sensitive (menu_clear, FALSE);
    gtk_widget_hide (menu_search);
    gtk_widget_show (menu_stop);
    
    label_result = g_object_get_data (G_OBJECT (view->mainwin), "label_result");
    fsexam_statusbar_update (_("Searching..."));

    /* show result pane and clear previous result */
    fsexam_search_treeview_append_file (NULL, TRUE);
    g_free (view->basedir);

    /* resolve possible symlink in folder path */
    if (g_file_test (folder, G_FILE_TEST_IS_SYMLINK))
        view->basedir = get_abs_path_for_symlink_target (folder);
    else
        view->basedir = get_abs_path (folder);

    stop_search = FALSE;
    buf = (gchar *) g_malloc (PATH_MAX);
    memset (buf, 0, PATH_MAX);

    timer = g_timer_new ();
    g_timer_start (timer);

    /* Read data from child's stdout async */
    while (fgets (buf, PATH_MAX, fp) != NULL) {
        gint  len = strlen (buf);

        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';

        fsexam_search_treeview_append_file (buf, FALSE);
        file_count ++;

        g_timer_elapsed (timer, &duration);

        if (duration > FSEXAM_REFRESH_TIMEOUT) {
            msg = g_strdup_printf (_("%d files found"), file_count);
            gtk_label_set_text (GTK_LABEL (label_result), msg);
            g_free (msg);

            while (gtk_events_pending ()) {
                gtk_main_iteration ();
            }

            g_timer_reset (timer);
        }

        if (force_quit) /* user quit application */
            break;
    }

    g_timer_destroy (timer);

    if (!force_quit) {  /* Quit the whole app, mainwin has been destroyed */
        g_spawn_close_pid (view->pid);
        view->pid = -1;

        msg = g_strdup_printf (_("%d files found"), file_count);
        gtk_label_set_text (GTK_LABEL (label_result), msg);
        g_free (msg);

        if (file_count != 0)
            gtk_widget_set_sensitive (menu_clear, TRUE);

        gtk_widget_hide (menu_stop);
        gtk_widget_show (menu_search);
    
        fsexam_statusbar_update (stop_search ? _("Search stopped") 
                                             : _("Search finished"));
    }

    fclose (fp);
    g_free (buf);
    g_strfreev (argv);

    return;
}
