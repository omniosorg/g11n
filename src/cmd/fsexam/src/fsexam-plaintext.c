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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>

#include <glib.h>

#include "fsexam-header.h"
#include "fsexam-plaintext.h"

#define FILE_CMD    "/usr/bin/file"

static gchar * 
get_utf8_locale (void)
{
    gchar       buf[50];
    FILE        *fp = NULL;
    gchar       **argv = NULL;
    GError      *error = NULL;
    gint        child_stdout;
    gboolean    find = FALSE;

    memset (buf, 0, sizeof (buf));

    argv = g_new0 (gchar *, 3);
    argv[0] = "/usr/bin/locale"; 
    argv[1] = "-a";
    argv[2] = NULL;

    if (!g_spawn_async_with_pipes (NULL, argv, NULL,
                0, 	/* GSpawnFlags */
                NULL,   /* child setup func */
                NULL,   /* user data */
                NULL,   /* pid */
                NULL,
                &child_stdout,
                NULL,
                &error)) {
        g_print (error->message);
        goto done;
    }

    if ((fp = fdopen (child_stdout, "r")) == NULL) {
        g_print (_("Can't open pipe file descriptor.\n"));
        goto done;
    }

    while (fgets (buf, sizeof (buf), fp) != NULL) {
        if (buf[strlen (buf) - 1] == '\n')
            buf[strlen (buf) -1] = '\0';

        if (g_str_has_suffix (buf, "UTF-8")) {
            find = TRUE;
            break;
        }
    }

done:
    if (error != NULL)
        g_error_free (error);

    if (fp != NULL)
        fclose (fp);

    g_free (argv);

    if (! find) {
        g_print (_("The system has no UTF-8 locale\n"));
        return NULL;
    }

    return g_strdup (buf);
}

static gboolean
is_plain_text_by_locale (const gchar *name, const gchar *locale)
{
    gchar       buf[256];
    FILE        *fp = NULL;
    gchar       **argv = NULL;
    gchar       **envp = NULL;
    GError      *error = NULL;
    gint        child_stdout;
    gboolean    retval = FALSE;

    argv = g_new0 (gchar *, 3);
    argv[0] = FILE_CMD;
    argv[1] = (gchar *) name;
    argv[2] = NULL;

    envp = g_new0 (gchar *, 4);
    envp[0] = "LC_ALL=";
    envp[1] = "LC_MESSAGES=C";
    if (locale) {
        envp[2] = g_strdup_printf ("LC_CTYPE=%s", locale);
    }else{
        envp[2] = NULL;
    }
    envp[3] = NULL;

    if (!g_spawn_async_with_pipes (NULL, argv, envp,
                0, 	    /* GSpawnFlags */
                NULL,   /* child setup func */
                NULL,   /* user data */
                NULL,   /* pid */
                NULL,
                &child_stdout,
                NULL,
                &error)) {
        g_print (error->message);
        goto done;
    }

    if ((fp = fdopen (child_stdout, "r")) == NULL) {
        g_print (_("Can't open pipe file descriptor.\n"));
        goto done;
    }

    memset (buf, 0, sizeof (buf));
    fgets (buf, sizeof (buf), fp);

    if (fsexam_debug () & FSEXAM_DBG_PLAIN_TEXT) {
        gchar *lc_all = NULL;
        gchar *lc_ctype = NULL;

        lc_all = getenv ("LC_ALL");
        setenv ("LC_ALL", "", TRUE);    /* unset LC_ALL to get LC_CTYPE */

        if (locale == NULL)
            lc_ctype = setlocale (LC_CTYPE, NULL);
        else
            lc_ctype = (char *)locale;
        g_print ("LC_CTYPE=%s, file type: %s\n", 
                lc_ctype ? lc_ctype : "NULL",
                buf ? buf : "Unknown");
    
        if (lc_all != NULL)
            setenv ("LC_ALL", lc_all, TRUE);    /* reset LC_ALL */
    }

    if (*buf != '\0') {
        gchar *p = buf;
        while (*p != ':') 
            ++p;
        ++p;
        while (isspace (*p)) 
            ++p;

        if (strstr (p, "text"))
            retval = TRUE;
    }

done:
    if (error != NULL)
        g_error_free (error);
    if (fp != NULL)
        fclose (fp);

    g_free (argv);
    g_free (envp[2]);
    g_free (envp);

    return retval;
}

gboolean
fsexam_is_plain_text (const gchar *name, FSEXAM_setting *setting)
{
    static gboolean first = TRUE;
    gboolean    retval = FALSE;
    GList       *p = NULL;
    GList       *encode_list = NULL;

    if (setting->pref->force)
        return TRUE;

    /* fast check in current locale */
    if (is_plain_text_by_locale (name, NULL))
        return TRUE;

    /* check on UTF-8 locale */
    if (first) {
        setting->utf8_locale = get_utf8_locale ();
        first = FALSE;
    }

    if (setting->utf8_locale != NULL &&
            is_plain_text_by_locale (name, setting->utf8_locale))
        return TRUE;

    encode_list = setting->pref->encode_list;
    for (p = encode_list; p; p = p->next)
    {
        const gchar *locale = encoding_to_locale (id2encoding (
                                    ((Encoding*) p->data)->encodingID));

        if (locale == NULL)
            continue;

        if (is_plain_text_by_locale (name, locale)) {
            retval = TRUE;
            break;
        }
    }

    return retval;
}
