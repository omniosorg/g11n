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


#ifndef _FSEXAM_CONVERSION_
#define _FSEXAM_CONVERSION_

void fsexam_undo_insert (unsigned int);
void fsexam_undo_removeall ();
void fsexam_filename_convert();
void fsexam_undo ();
void fsexam_reverse();
GString *fsexam_filename_get_path (GtkTreeModel *, GtkTreeIter, char *);
char *fsexam_validate_with_newline (char *, gboolean);
int write_to_report_pane (FSEXAM_pref *, ConvType, gint, char *, char *, int);

#endif
