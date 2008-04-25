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


#ifndef _FSEXAM_SETTING_H
#define _FSEXAM_SETTING_H

/* Use same serial for history item or not */
#define FSEXAM_SETTING_FLAGS_DIFF_SERIAL    (1<<0)  
/* Don't write history information during UNDO */
#define FSEXAM_SETTING_FLAGS_UNDO           (1<<1)
#define FSEXAM_SETTING_FLAGS_SCENARIO       (1<<2)
#define FSEXAM_SETTING_FLAGS_STOP           (1<<3)

typedef struct _FSEXAM_setting FSEXAM_setting;

struct _FSEXAM_setting {
    FSEXAM_pref     *pref;          /* Config preferences */
    guint           flags;          /* Flags for one pass of conversion */
    /* encoding index for current pass, -1 means nop */
    gint            gold_index;
    gchar           *utf8_locale;   /* UTF-8 locale name */
   
    Hist_info       *hist_info;     /* For history */
    Log_info        *log_info;      /* For log */
    FsexamDryrun    *dryrun_info;   /* For dryrun */
    GList           *remote_path;   /* For NFS/Local file */

    /* The number of files passed in by user */
    guint           passin_num;     
    /* The total number of files processed */
    guint           total_num;     
    /* The number of files are ignored, including no hist, doesn't exist ...*/
    guint           ignore_num;     
    /* The number of files which are converted successfully */
    guint           succ_num;       
    /* The No. of files which met error during converting: such as no rights */
    guint           fail_num;       

    /* Function pointer to handle GUI/CMD diff */

    /* get the candidate index */
    gint (*get_index)  (GList *encoding_list, gboolean forname);    
    /* update GUI after conversion */
    void (*update_gui) (FSEXAM_setting *setting,                    
                        const gchar *path, 
                        const gchar *oldname, 
                        const gchar *newname);
    /* display information */
    void (*display_msg) (const gchar *fname, const gchar *msg);     
    /* display statistics information */
    void (*display_stats) (FSEXAM_setting *setting);
};

FSEXAM_setting *fsexam_setting_init (gboolean cmd_mode);
void            fsexam_setting_destroy (FSEXAM_setting *setting);

void            fsexam_setting_reset_stats (FSEXAM_setting *setting);
void            fsexam_setting_display_stats (FSEXAM_setting *setting);
void            fsexam_setting_print (FSEXAM_setting *setting);

#endif //_FSEXAM_SETTING_H
