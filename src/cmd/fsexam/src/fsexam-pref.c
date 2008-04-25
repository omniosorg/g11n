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
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include "fsexam.h"
#include "fsexam-header.h"

void cb_toolbar_style (GConfClient *, guint, GConfEntry *, gpointer);

static GSList *
remove_duplicated_encoding_name (GSList *list, GSList *standard)
{
    GSList *result = list;

    while (list != NULL) {
        GSList *second = standard;
        
        while (second != NULL) {
            if (encoding2id (list->data) == encoding2id (second->data))
                break;
            second = g_slist_next (second);
        }

        if (second != NULL) {       /* duplicated, remove it */
            GSList *next = NULL;

            next = g_slist_next (list);
            result = g_slist_delete_link (result, list);
            list = next;
        } else {
            list = g_slist_next (list);
        }
    }

    return result;
}

FSEXAM_pref *
fsexam_pref_init (gboolean cmd_mode)
{
    FSEXAM_pref *pref = NULL;
    GConfClient *gconf_client = NULL;

    pref = g_new0 (FSEXAM_pref, 1);

    gconf_client = pref->gconf_client = gconf_client_get_default ();
    pref->hist_len = gconf_client_get_int (gconf_client, HISTLEN, NULL);

    if (cmd_mode){
        pref->auto_detect = FALSE;
        pref->hidden = FALSE;
        pref->auto_conversion= FALSE;
        pref->recursive = FALSE;
        pref->remote = FALSE;
        pref->follow = FALSE;
        pref->no_check_symlink_content = FALSE;
        pref->use_log = FALSE;
        pref->log_file = NULL;
        pref->suffix_list = NULL;
        pref->special = 0;
    }else{
		gconf_client_notify_add (gconf_client,
				TOOLBAR_STYLE,
				cb_toolbar_style,
				NULL, NULL, NULL);

        pref->auto_detect = gconf_client_get_bool (gconf_client,
                                    AUTODETECTMODE,
                                    NULL);
        pref->hidden = gconf_client_get_bool (gconf_client,
                                    HIDDENMODE,
                                    NULL);
        pref->auto_conversion= gconf_client_get_bool (gconf_client,
                                    INTERACTIVEMODE,
                                    NULL);
        pref->recursive = gconf_client_get_bool (gconf_client,
                                    RECURSIVEMODE,
                                    NULL);
        pref->remote = gconf_client_get_bool (gconf_client,
                                    REMOTEMODE,
                                    NULL);
        pref->follow = gconf_client_get_bool (gconf_client,
                                    FOLLOWMODE,
                                    NULL);
        pref->use_log = gconf_client_get_bool (gconf_client,
                                    USELOG,
                                    NULL);
        pref->no_check_symlink_content = gconf_client_get_bool (gconf_client,
                                    CHECKSYMLINKTARGETMODE,
                                    NULL);
        pref->suffix_list = (GSList *) gconf_client_get_list (gconf_client,
                                    SUFFIXLIST,
                                    GCONF_VALUE_STRING,
                                    NULL);
        pref->log_file = gconf_client_get_string (gconf_client,
                                    LOGFILE,
                                    NULL);
        pref->special = gconf_client_get_int (gconf_client,
                                    SPECIAL,
                                    NULL);
    }

    pref->conv_content = FALSE;
    pref->dry_run = FALSE;
    pref->force = FALSE;

    pref->encode_list = NULL;
    pref->encode_name_list = NULL;

    return pref;
}

void
fsexam_pref_destroy (FSEXAM_pref *pref)
{
    if (NULL == pref)
        return;

    g_object_unref (pref->gconf_client);
    fsexam_encoding_destroy (pref->encode_list);

    fsexam_slist_free (pref->encode_name_list);
    fsexam_slist_free (pref->suffix_list);

    g_free (pref->log_file);
    g_free (pref);
}

/*
 *  Init encoding_list and encoding_name_list from gconf and CLI option
 */
void
fsexam_pref_set_encoding_list (FSEXAM_pref *pref,
                               gchar *encoding_string,
                               gboolean append,
                               gboolean prepend,
                               gboolean save)
{
    GConfClient     *client;

    if (NULL == pref)
        return;

    client = pref->gconf_client;

    if (encoding_string == NULL){   /* Use GConf Encoding list only */
        pref->encode_name_list = (GSList *)gconf_client_get_list (client,
                                                    ENCODINGLIST,
                                                    GCONF_VALUE_STRING,
                                                    NULL);
        pref->encode_list = fsexam_encoding_init (pref->encode_name_list);
    }else{
        GSList      *encode_text;

        /* encoding list from command line arguments */
        pref->encode_name_list = encoding_string_parse (encoding_string);

        /* encoding list from gconf database */
        encode_text = (GSList *)gconf_client_get_list (client,
                                                       ENCODINGLIST,
                                                       GCONF_VALUE_STRING,
                                                       NULL);
        
        if (append) {
            pref->encode_name_list = remove_duplicated_encoding_name (
                    pref->encode_name_list,
                    encode_text);
            pref->encode_name_list = g_slist_concat (encode_text, 
                                                     pref->encode_name_list);
        }else if (prepend){
            pref->encode_name_list = remove_duplicated_encoding_name (
                    pref->encode_name_list,
                    encode_text);
            pref->encode_name_list = g_slist_concat (pref->encode_name_list, 
                                                     encode_text);
        }else{
            fsexam_slist_free (encode_text);    /* free encode from gconf */
        }

        if (save){
            gconf_client_set_list (client,
                                   ENCODINGLIST,
                                   GCONF_VALUE_STRING,
                                   pref->encode_name_list,
                                   NULL);
        }

        pref->encode_list = fsexam_encoding_init (pref->encode_name_list);
    }

    return;
}

/* 
 * Update FSEAM_pref encoding name and encoding list, save to gconf
 * if client != NULL
 *
 * slist will be used directly, so caller don't free it 
 */
void
fsexam_pref_update_encoding (FSEXAM_pref *pref, 
                             GSList *slist, 
                             GConfClient *client)
{
    if (NULL == pref) {
		fsexam_slist_free (slist);
        return;
	}

    if (client != NULL) {   /* save to gconf */
        gconf_client_set_list (client,
                               ENCODINGLIST,
                               GCONF_VALUE_STRING,
                               slist,
                               NULL);
    }

    /* free the old data */
    fsexam_slist_free (pref->encode_name_list);
    fsexam_encoding_destroy (pref->encode_list);
   
    /* init with new data */
    pref->encode_name_list = slist;
    pref->encode_list = fsexam_encoding_init (pref->encode_name_list);

    return;
}

/*
 *  Save the user's preference setting into gconf database
 */
void
fsexam_pref_save_to_gconf (FSEXAM_pref *pref, 
                           GConfClient *client, 
                           gboolean save_encoding)
{
    if ((NULL == pref) || (NULL == client))
        return;

    gconf_client_set_bool (client, AUTODETECTMODE, pref->auto_detect, NULL);
    gconf_client_set_bool (client, HIDDENMODE, pref->hidden, NULL);
    gconf_client_set_bool (client, RECURSIVEMODE, pref->recursive, NULL);
    gconf_client_set_bool (client, REMOTEMODE, pref->remote, NULL);
    gconf_client_set_bool (client, FOLLOWMODE, pref->follow, NULL);
    gconf_client_set_bool (client, USELOG, pref->use_log, NULL);
    gconf_client_set_bool (client, INTERACTIVEMODE, 
                           pref->auto_conversion, NULL);
    gconf_client_set_bool (client, CHECKSYMLINKTARGETMODE, 
                           pref->no_check_symlink_content, NULL);

    if (pref->log_file != NULL)
    	gconf_client_set_string (client, LOGFILE, pref->log_file, NULL);
    gconf_client_set_int (client, SPECIAL, pref->special, NULL);
    gconf_client_set_int (client, HISTLEN, pref->hist_len, NULL);

    if (save_encoding)
        gconf_client_set_list (client, ENCODINGLIST,
                               GCONF_VALUE_STRING, pref->encode_name_list,
                               NULL);

    return;
}
