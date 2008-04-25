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

#include "fsexam-header.h"

/*
 * Init FSEXAM_setting structure
 */
FSEXAM_setting *
fsexam_setting_init (gboolean cmd_mode)
{
    FSEXAM_setting *setting = NULL;

    setting = g_new0 (FSEXAM_setting, 1);

    setting->pref = fsexam_pref_init (cmd_mode);
    setting->gold_index = -1;
    setting->utf8_locale = NULL;
    setting->flags = 0;
    setting->hist_info = NULL;
    setting->remote_path = NULL;
    
    if (cmd_mode){
        setting->dryrun_info = NULL;
        setting->log_info = NULL;
    }else{
        if (setting->pref->use_log && (setting->pref->log_file != NULL)){
            setting->log_info = fsexam_log_open (setting->pref->log_file);
        }

        /* Init dryrun at the beginning for GUI dryrun */
        setting->dryrun_info = fsexam_dryrun_buffer_new ();
    }

    fsexam_setting_reset_stats (setting);
    
    return setting;
}

/*
 * Reset the statistics variable for FSEXAM_setting
 */
void
fsexam_setting_reset_stats (FSEXAM_setting *setting)
{
    g_return_if_fail (setting != NULL);

    setting->passin_num = 0;
    setting->total_num = 0;
    setting->succ_num = 0;
    setting->fail_num = 0;
    setting->ignore_num = 0;

    /* reset gold_index also */
    setting->gold_index = -1;

    return;
}

/*
 * Display the statistics information
 */
void
fsexam_setting_display_stats (FSEXAM_setting *setting)
{
    g_return_if_fail (setting != NULL);

    g_print (_("Rough summary: %d given, %d total, %d ignore, %d fail, %d succeed\n"),
            setting->passin_num,
            setting->total_num,
            setting->ignore_num,
            setting->fail_num,
            setting->succ_num);

    return;
}

/*
 * Destroy FSEXAM_setting structure 
 */
void 
fsexam_setting_destroy (FSEXAM_setting *setting)
{
    if (NULL == setting)
        return;

    if (setting->pref != NULL){
        fsexam_pref_destroy (setting->pref);
    }

    g_free (setting->utf8_locale);

    if (setting->hist_info)
        fsexam_history_close (setting->hist_info);
    if (setting->log_info)
        fsexam_log_close (setting->log_info);
    if (setting->dryrun_info)
        g_object_unref (setting->dryrun_info);
    if (setting->remote_path)
        fsexam_list_free (setting->remote_path);
    
    g_free (setting);

    return;
}

/*
 *  Print FSEXAM_setting structure
 */
void
fsexam_setting_print (FSEXAM_setting *setting)
{
    if ((setting == NULL) || (setting->pref == NULL)) {
        g_print ("seting or setting->pref is NULL\n");
        return;
    }

    g_print ("===========================Setting=================\n");
    g_print ("\tPreference\n");
    g_print ("\t\tconvcontent = %s\n", 
             setting->pref->conv_content ? "TRUE" : "False");
    g_print ("\t\tautodetect = %s\n", 
             setting->pref->auto_detect ? "TRUE" : "False");
    g_print ("\t\tdryrun= %s\n", setting->pref->dry_run ? "TRUE" : "False");
    g_print ("\t\tforce = %s\n", setting->pref->force ? "TRUE" : "False");
    g_print ("\t\thidden = %s\n", setting->pref->hidden ? "TRUE" : "False");
    g_print ("\t\tauto_conversion= %s\n", 
             setting->pref->auto_conversion? "TRUE" : "False");
    g_print ("\t\trecursive = %s\n", 
             setting->pref->recursive ? "TRUE" : "False");
    g_print ("\t\tno-check-symlink-content= %s\n", 
             setting->pref->no_check_symlink_content ? "TRUE" : "FALSE");
    g_print ("\t\tfollow = %s\n", setting->pref->follow ? "TRUE" : "False");
    g_print ("\t\tremote = %s\n", setting->pref->remote ? "TRUE" : "False");
    g_print ("\t\tuse_log = %s\n", setting->pref->use_log ? "TRUE" : "False");
    g_print ("\t\thist_len = %d\n", setting->pref->hist_len);

    g_print ("\t\tlog_file = %s\n", 
             setting->pref->log_file ? setting->pref->log_file : "NULL");
    if (setting->remote_path) {
        g_print ("\tNFS mount point\n");
        g_list_foreach (setting->remote_path, list_print, "\t");
    }
    g_print ("======================== End of Setting ============\n");

    return;
}

