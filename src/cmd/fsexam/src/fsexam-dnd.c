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

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>
#include <eel/eel-vfs-extensions.h>
#include <eel/eel-string.h>
#include <stdio.h>

#include "fsexam-header.h"
#include "fsexam-ui.h"
#include "fsexam-dnd.h"

enum {
  FSEXAM_ICON_DND_GNOME_ICON_LIST,
  FSEXAM_ICON_DND_URI_LIST,
  FSEXAM_ICON_DND_TEXT
};

static GtkTargetEntry drop_types[] = {
            {"text/uri-list", 0, FSEXAM_ICON_DND_URI_LIST},
            {"text/plain", 0, FSEXAM_ICON_DND_TEXT},
            {"x-special/gnome-icon-list", 0, FSEXAM_ICON_DND_GNOME_ICON_LIST}
};

static void
drag_data_received_callback (GtkWidget *widget,
                             GdkDragContext *context,
                             gint x, 
                             gint y,
                             GtkSelectionData *data,
                             guint info,
                             guint32 time,
                             gpointer user_data)
{
    gchar    *canonical_uri = NULL, *local_path, *p;
    gchar    **uris = NULL;
    gint     i;

    uris = g_uri_list_extract_uris ((gchar *) data->data);

    for (i = 0; uris[i] != NULL; i++) {
        gchar *uri;

        uri = gnome_vfs_make_uri_from_shell_arg (uris[i]);
        //TODO: is this valid?
        canonical_uri = gnome_vfs_make_uri_canonical (uri);

        g_free (uri);

        if (canonical_uri != NULL)
            break;
    }

    g_strfreev (uris);

    if (canonical_uri == NULL)
        return;

    local_path = gnome_vfs_get_local_path_from_uri (canonical_uri);

    if (local_path) {
        fsexam_change_dir (local_path);
    } else {
        if (eel_uri_is_desktop (canonical_uri) 
                || eel_uri_is_trash (canonical_uri)) {
            gchar *uri = NULL;
            p = strstr(canonical_uri, ":");
            if (p != NULL) {
                uri = g_strdup(p);
            }

            p = uri + 4; // to skip ":///"

            if (!strncmp (p, "home", 4) || !strncmp (p, "trash", 5) 
                    || !strncmp (p, "documents", 9)) {
                if ((*p == 'h') || (*p == 'd'))
                    local_path = g_strdup_printf ("%s/%s", 
                                                   g_getenv ("HOME"), 
                                                   "Documents");
                else
                    local_path = g_strdup_printf ("%s/%s", 
                                                   g_getenv ("HOME"), 
                                                   ".Trash");

                fsexam_change_dir (local_path);
            }

            g_free (uri);
        }
    }

    g_free (local_path);

    gtk_drag_finish (context, TRUE, FALSE, time);
    g_free (canonical_uri);

    fsexam_statusbar_update("");

    return;
}

void
fsexam_dnd_set (GtkWidget *widget)
{
    gtk_drag_dest_set (widget,
                 GTK_DEST_DEFAULT_ALL,
                 drop_types, G_N_ELEMENTS (drop_types),
                 GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_ASK);
    
    g_signal_connect (widget,
                "drag_data_received",
                G_CALLBACK (drag_data_received_callback),
                NULL);

    return;
}
