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

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <strings.h>
#include <string.h>

#ifdef sun
#include <sys/mnttab.h>
#else
#include <mntent.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>

#include "fsexam-header.h"

/*
 * Determine whether 'filename' is one file under directory 'path'
 * 
 * Params:
 *      filename: abs path of file (both path has no '.' or '..')
 */
static gboolean
compare_path (const gchar *path, const gchar *filename)
{
    if ((path == NULL) || (filename == NULL))
        return FALSE;

    while ((*path == *filename) && (*path != '\0') && (*filename != '\0')) {
        path++;
        filename++;
    }

    if ((*path == '\0') && ((*filename == '\0') || (*filename == '/')))
        return TRUE;
    else 
        return FALSE;
}

/*
 * Given file is remote file (NFS) or not
 */
gboolean
is_remote_file (GList *remote_paths, const gchar *filename)
{
    if (filename == NULL)
        return TRUE;    //The return value has no meaning

    while (remote_paths) {
        const gchar *mountp = remote_paths->data;

        if (compare_path (mountp, filename))
            return TRUE;

        remote_paths = g_list_next (remote_paths);
    }

    return FALSE;
}

/*
 * Whether need handle this file or not
 */
gboolean
file_validate_for_contentconv (const gchar *filename, FSEXAM_setting *setting)
{
    struct stat     buf;
    gboolean        ret = FALSE;
    gchar           *ptr = NULL;
    gchar           *abs_path = NULL;

    if ((NULL == filename) || (NULL == setting))
        return FALSE;

    if (lstat(filename, &buf) < 0){
        fsexam_errno = ERR_CANNOT_OPEN;
        goto done;
    }

    if (! S_ISREG(buf.st_mode)) { 
        fsexam_errno = ERR_NOT_REG_FILE;
        goto done;
    }

    if (buf.st_size == 0) {
        fsexam_errno = ERR_EMPTY_FILE;
        goto done;
    }

    abs_path = get_abs_path (filename); /* for is_remote_file () */
    ptr = basename (abs_path);          /* return "/" if only contain '/' */
    
    if (!setting->pref->hidden){
        if (('/' == *ptr) || ('.' == *ptr)){
            fsexam_errno = ERR_IGNORE_HIDDEN_FILE;
            goto done;
        }
    }
    
    if ((!setting->pref->remote)
            && (is_remote_file (setting->remote_path, abs_path))){
        fsexam_errno = ERR_IGNORE_REMOTE_FILE;
        goto done;
    }

    if ((!setting->pref->force) 
            && (file_isutf8 (filename, DEFAULT_DETECTING_FLAG))) {
        fsexam_errno = ERR_CONTENT_UTF8_ALREADY;
        goto done;
    }

    ret = TRUE;

done:
    g_free (abs_path);

    return ret;
}

#ifdef sun
#define MNTFILENAME     "/etc/mnttab"

GList *
get_remote_paths ()
{
    struct  mnttab mt;
    FILE    *fp = NULL;
    GList   *list = NULL;

    fp = fopen (MNTFILENAME, "r");

    if (fp == NULL) {
        g_print (_("Can't open file %s.\n"), MNTFILENAME);
        return NULL;
    }

    while (getmntent (fp, &mt) == 0) {
        if ((strcmp (mt.mnt_fstype, "nfs") == 0)
                || (strcmp (mt.mnt_fstype, "autofs") == 0)) {
            list = g_list_prepend (list, g_strdup (mt.mnt_mountp));
        }
    }

    return list;
}

#else

#define MNTFILENAME     "/etc/mtab"

GList *
get_remote_paths ()
{
    struct  mntent *mt;
    FILE    *fp = NULL;
    GList   *list = NULL;

    fp = setmntent (MNTFILENAME, "r");

    if (fp == NULL) {
        g_print (_("Can't open file %s.\n"), MNTFILENAME);
        return NULL;
    }

    while ((mt = getmntent (fp)) == 0) {
        if ((strcmp (mt->mnt_type, "nfs") == 0)
                || (strcmp (mt->mnt_type, "autofs") == 0)) {
            list = g_list_prepend (list, g_strdup (mt->mnt_dir));
        }
    }

    return list;
}
#endif

