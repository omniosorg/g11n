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


#ifndef _FSEXAM_PREF_H
#define _FSEXAM_PREF_H

#define AUTODETECTMODE  "/apps/fsexam/auto_detect"
#define INTERACTIVEMODE "/apps/fsexam/auto_mode"
#define RECURSIVEMODE   "/apps/fsexam/recur_mode"
#define HIDDENMODE      "/apps/fsexam/hidden"
#define REMOTEMODE      "/apps/fsexam/remote"
#define FOLLOWMODE      "/apps/fsexam/follow"
#define SUFFIXLIST      "/apps/fsexam/suffix"
#define CHECKSYMLINKTARGETMODE  "/apps/fsexam/no_check_symlink_content"
#define USELOG          "/apps/fsexam/uselog"
#define LOGFILE         "/apps/fsexam/logfile"
#define HISTLEN         "/apps/fsexam/hist_len"
#define SPECIAL         "/apps/fsexam/special"
#define ENCODINGLIST    "/apps/fsexam/encoding"

#define TOOLBAR_STYLE	"/desktop/gnome/interface/toolbar_style"

typedef struct _FSEXAM_pref FSEXAM_pref;

struct _FSEXAM_pref {
    GConfClient *gconf_client;

    gboolean    auto_detect;
    gboolean    auto_conversion;
    gboolean    follow;
    gboolean    hidden;
    gboolean    recursive;
    gboolean    remote;
    gboolean    no_check_symlink_content;

    gboolean    force;
    gboolean    dry_run;
    gboolean    conv_content;
    gboolean    use_log;

    gint        hist_len;   /* history info len */
    gint        special;    /* special file type */

    GList       *encode_list;       /* Encoding data structure list */
    GSList      *encode_name_list;  /* contains only encoding name */
    GSList      *suffix_list;   /* not use now */

    gchar       *log_file;
};

FSEXAM_pref *   fsexam_pref_init (gboolean cmd_mode);

void            fsexam_pref_set_encoding_list (FSEXAM_pref *pref,
                                               gchar *encoding_string, 
                                               gboolean append, 
                                               gboolean prepend, 
                                               gboolean save);

void            fsexam_pref_update_encoding (FSEXAM_pref *pref, 
                                             GSList *slist, 
                                             GConfClient *client);

void            fsexam_pref_save_to_gconf (FSEXAM_pref *pref, 
                                           GConfClient *client,
                                           gboolean save_encoding);

void            fsexam_pref_destroy (FSEXAM_pref *pref);

#endif //_FSEXAM_PREPROCESS_H_
