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


#ifndef _FSEXAM_PREFERENCE_

#define _FSEXAM_PREFERENCE_

typedef struct _fsexam_pref FSEXAM_pref;

struct _fsexam_pref
{
  GConfClient * client;
  gboolean recur_mode;
  gboolean auto_mode;

  GList * encode_list;
  GSList * suffix_list;
  char  changed;

  // dynamic widget
  GtkWidget * auto_button;
  GtkWidget * recur_button;
  GtkWidget * encode_view;
  GtkWidget * add_button;
  GtkWidget * del_button;
  GtkWidget * up_button;
  GtkWidget * down_button;
};

// public application
void fsexam_pref_free (FSEXAM_pref *);
FSEXAM_pref *create_fsexam_pref ();
void  create_pref_dialog (gpointer, gpointer, GtkWidget *);

#endif
