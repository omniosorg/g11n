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
#include <eel/eel-vfs-extensions.h>
#include <stdio.h>
#include "encode.h"
#include "fsexam-history.h"
#include "fsexam-preference.h"
#include "fsexam.h"

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
			     int x, int y,
			     GtkSelectionData *data,
			     guint info,
			     guint32 time,
			     gpointer user_data)
{
  char *canonical_uri, *local_path, *p;

  canonical_uri = eel_make_uri_canonical (data->data);

  local_path = gnome_vfs_get_local_path_from_uri (canonical_uri);

  if (local_path)
    {
      p = local_path;
      while (*p != '\r' && *p != '\n') ++p;
      *p = 0x0;

      fsexam_treeview_construct (local_path);

      g_free (local_path);
    }
  else
    {
      if (eel_uri_is_desktop (canonical_uri) || eel_uri_is_trash (canonical_uri))
	{
	  char *uri = eel_str_get_after_prefix (canonical_uri, ":");

	  p = uri + 4; // to skip ":///"

	  if (!strncmp (p, "home", 4) || !strncmp (p, "trash", 5))
	    {
	      if (*p == 'h')
		local_path = g_strdup_printf ("%s/%s", g_getenv ("HOME"), "Documents");
	      else
		local_path = g_strdup_printf ("%s/%s", g_getenv ("HOME"), ".Trash");

	      fsexam_treeview_construct (local_path);

	      g_free (local_path);
	    }

	  g_free (uri);
	}
    }

  gtk_drag_finish (context, TRUE, FALSE, time);
  g_free (canonical_uri);
  fsexam_statusbar_update("");
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
}
