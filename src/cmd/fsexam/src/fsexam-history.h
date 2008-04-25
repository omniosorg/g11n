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

#ifndef _FSEXAM_HISTORY_
#define _FSEXAM_HISTORY_

#define HIST_LEN 100

typedef struct _History_item
{
    gint32    serial;           /* used by undo */
    ConvType  convtype;
    gint      from_encoding;
    gint      to_encoding;
    gchar     *uri;             /* always is uri */
}Hist_item;

/* 
 * defined to History_info in header file 
 *
 * 1) Empty state:
 *      head = 0;
 *      current = -1;
 *      last_serial = 0;
 *
 * 2) Only one element:
 *      head = 0;
 *      current = -1
 *
 * 3) Full
 *      current + 1 == head
 */
struct _History_info
{
    FILE      *fp;              /* FILE stream pointer for ~/.fsexam.history */
    GList     **hist_array;     /* the list of History_item */
    gint32    max_len;         /* The total length of history */
    gint32    head;             /* always point to NUL element as condition  */
    gint32    current;          /* the last used element */
    gint32    last_serial;
    gboolean  changed;
};

typedef struct _History_info Hist_info;

/* 
 * to make life easier, history use uri internally 
 * both in disk and in memory 
 */

Hist_info * fsexam_history_open (const gchar *filename, gint max_len);
gboolean    fsexam_history_close (Hist_info *);
void        fsexam_history_remove_last (Hist_info *);
gint32      fsexam_history_get_serial (Hist_info *info);
GList     * fsexam_history_get_last_list (Hist_info *info, ConvType *type);
Hist_item * fsexam_history_search (Hist_info *info, 
                                   const gchar *path, 
                                   gboolean convname);
void        fsexam_history_put (Hist_info *,
                                ConvType convtype,
                                const gchar *filename,
                                gint from_encoding,
                                gint to_encoding,
                                gboolean same_serial);

#endif
