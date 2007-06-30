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


#ifndef _FSEXAM_H_
#define _FSEXAM_H_

#define GETTEXT_PACKAGE "fsexam"
#define FSEXAM_LOCALEDIR "/usr/share/locale/"
#define VERSION "0.3"

enum
{
  ICON_COLUMN = 0,
  FILENAME_COLUMN,
  LOADED_COLUMN,
  NUM_COLUMNS
};

typedef struct _FSEXAM_view FSEXAM_view;
struct _FSEXAM_view 
{
  GtkWidget *mainwin;
  GtkWidget *treeview;
  GtkWidget *contents;
  GtkWidget *reportwin;
  GtkWidget *statusbar;
  GtkWidget *peekwin;
  GtkWidget *undo_menuitem;
  GdkPixbuf *icon;
  GHashTable *pixbuf_hash;
  
  gint lineoffset; // for report pane

  gchar *rootdir;

  FSEXAM_pref *pref;
  History_info *histinfo;

  GSList *undo_list;
};

extern FSEXAM_view *view;

GtkWidget *fsexam_construct_ui (char *);
void fsexam_treeview_construct (char *);
void fsexam_statusbar_update (char *);

#endif
