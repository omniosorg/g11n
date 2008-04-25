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

#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>

#include "encoding.h"
#include "fsexam-log.h"
#include "fsexam-error.h"
#include "fsexam-helper.h"

static gboolean log_puts (Log_info *info, 
                          const gchar *category, 
                          const gchar *filename, 
                          const gchar *msg);

Log_info *
fsexam_log_open (const gchar *logfile)
{
    Log_info *log_info = NULL;

    if (NULL == logfile)
        return NULL;

    log_info = g_new0 (Log_info, 1);

    log_info->fp = fopen (logfile, "a");
    if (log_info->fp == NULL){
        fsexam_errno = ERR_CANNOT_OPEN;
    };

    return log_info;
}

static gboolean
log_puts (Log_info *info, 
          const gchar *category, 
          const gchar *filename, 
          const gchar *msg)
{
    gint ret;

    if (filename) {
        if (g_utf8_validate (filename, -1, NULL)) {
            ret = fprintf (info->fp, "%s %s: %s\n", 
                           category, 
                           filename, 
                           msg ? msg : "");
        }else{
            gchar *uri = g_filename_to_uri (filename, NULL, NULL);
            ret = fprintf (info->fp, "%s %s: %s\n", 
                           category, 
                           uri, 
                           msg ? msg : "");
            g_free (uri);
        }
    }else{
        ret = fprintf (info->fp, "%s %s\n", category, msg ? msg : "");
    }

    if (ret < 0) {
        fsexam_errno = ERR_CANNOT_WRITE;
        return FALSE;
    }

    return TRUE;
}

gboolean 
fsexam_log_puts (Log_info *info, const gchar *filename, const gchar *msg)
{
    gboolean    ret = TRUE;
    const char  *messages = NULL;

    if ((info == NULL) || (info->fp == NULL)) {
        return FALSE;
    }

    if (msg == NULL)
        messages = fsexam_error_get_msg ();
    else
        messages = msg;

    if (fsexam_errno < ERR_ERROR) {
        /* Information */
        ret = log_puts (info, LOG_INFO, filename, messages);
    }else if (fsexam_errno < ERR_WARNING) {
        /* ERROR */
        ret = log_puts (info, LOG_ERROR, filename, messages);
    }else if (fsexam_errno < ERR_WONNOT_LOG) {
        /* WARNING */
        ret = log_puts (info, LOG_WARNING, filename, messages);
    }else if (fsexam_errno < ERR_MISC) {
        /* won't log */
    }else{
        /* unknow error */
        ret = log_puts (info, LOG_ERROR, filename, _("Unknown error occurred"));
    }

    return ret;
}

gboolean
fsexam_log_puts_folder_and_name (Log_info *info, 
                                 const gchar *dirname, 
                                 const gchar *filename, 
                                 const gchar *msg)
{
    gchar     *fullpath = NULL;
    gboolean  ret;

    if ((NULL == info) || (NULL == info->fp))
        return FALSE;

    fullpath = g_strdup_printf ("%s/%s", dirname, filename);
    if (fullpath == NULL)
        return FALSE;

    ret = fsexam_log_puts (info, fullpath, msg);

    g_free (fullpath);

    return ret;
}

void
fsexam_log_flush (Log_info *info)
{
    if (info == NULL || info->fp == NULL)
        return;

    fflush (info->fp);

    return;
}

void 
fsexam_log_close (Log_info *info)
{
    if ((info == NULL) || (info->fp == NULL))
        return;

    fclose (info->fp);

    return;
}
