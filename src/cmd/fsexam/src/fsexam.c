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

#include <gconf/gconf-client.h>
#include <gnome.h>
#include <gtk/gtk.h>
#include <libgnome/gnome-i18n.h>
#include <stdlib.h>
#include "encode.h"
#include "fsexam-history.h"
#include "fsexam-preference.h"
#include "fsexam.h"

FSEXAM_view *view;

int
main(int argc, char **argv)
{
  GnomeClient *client;
  char *pwd, *home;
  char history[100], document[256];

  // GnomeProgram *program;
  // CORBA_Object factory;


  bindtextdomain (GETTEXT_PACKAGE, FSEXAM_LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  gnome_program_init ("fsexam", VERSION,
				LIBGNOMEUI_MODULE, argc, argv,
				GNOME_PARAM_HUMAN_READABLE_NAME,
				_("File System Examiner"),
				GNOME_PROGRAM_STANDARD_PROPERTIES,
				GNOME_PARAM_APP_DATADIR, DATADIR,
				NULL);

  client = gnome_master_client();
  g_signal_connect (G_OBJECT (client), "die",
		    G_CALLBACK (exit),
		    NULL);

  view = g_new0 (FSEXAM_view, 1);
  view->pixbuf_hash = g_hash_table_new (g_str_hash, g_str_equal);

  pwd = getenv ("PWD");
  home = getenv ("HOME");

  memset (history, 0, 100);
  g_sprintf (history, "%s/.fsexam-history", home);
  view->histinfo = fsexam_history_open (history);

  view->pref = create_fsexam_pref ();

#if 0
  factory = bonobo_activation_activate_from_id
    ("OAFIID:GNOME_FSAXEM_Factory",

     Bonobo_ACTIVATION_FLAG_EXISTING_ONLY,
     NULL, NULL);

  if (factory != NULL)
    {
      exit (0);
    }
#endif
     
  gtk_init(&argc, &argv);

  memset (document, 0, 256);
  if (argc > 1)
    strcpy (document, argv[1]);
  else
    {
      if (pwd && strcmp (pwd, home))
	strcpy (document, pwd);
      else
	g_sprintf (document, "%s/Documents", home);
    }
  fsexam_construct_ui (document);

  gtk_quit_add (0, fsexam_history_close, (gpointer)view->histinfo);
  gtk_quit_add (0, fsexam_pref_free, (gpointer)view->pref);
  // fsexam_history_update will be invoked at the interval of 
  // one minute.
  gtk_timeout_add (60000, fsexam_history_update, (gpointer)view->histinfo);

  gtk_main();
  return 0;
}
