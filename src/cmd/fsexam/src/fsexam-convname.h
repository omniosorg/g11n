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


#ifndef _FSEXAM_CONVNAME_H
#define _FSEXAM_CONVNAME_H

/*===================================================================
 *  Function Name:  fsexam_convert_single_filename
 *
 *  Parameters:
 *
 *  Desc:   
 *      Convert single full file path, don't care any other flags, such
 *      as symlink, recursive etc.
 *
 *  Return value:
 *      TRUE if success, FALSE if error occured
 *
 *  Author:     Yandong Yao 2007/02/06
 =====================================================================*/
gboolean fsexam_convert_single_filename (FSEXAM_setting *setting,
                                         const gchar *filename,
                                         gchar **result_name);

/*============================================================================
 *  Function Name:  fsexam_convert_filename
 *
 *  Parameters:
 *      FSEXAM_setting *setting: struct contain all information
 *      gchar * filename:   the filename which will be converted
 *
 *  Desc:
 *      Convert one filename, but maybe convert more than one file, if 
 *      has follow or recursive options.
 *
 *  Return value:
 *      True for success otherwise fail
 *
 *  Author:     Yandong Yao 2006/08/22
 ===========================================================================*/ 
gboolean fsexam_convert_filename (FSEXAM_setting *setting, 
                                  const gchar *filename);

/*===========================================================================
 *  Function Name:  fsexam_convert_filelist 
 *
 *  Parameters:
 *      FSEXAM_setting *setting: the all-in-one struct
 *      GList *list: The list contain file names
 *
 *  Desc:
 *      Convert the whole tree structure for list.
 *
 *  Return value:
 *      TRUE for success, otherwise fail
 *
 *  Author:     Yandong Yao 2006/08/22
 ============================================================================*/
gboolean fsexam_convert_filename_batch (FSEXAM_setting *setting, GList *list);

/*====================================================================
 *  Function Name:  fsexam_convert_scenario
 *
 *  Parameters:
 *      FSEXAM_setting *setting: contain the setting 
 *
 *  Desc:
 *      Scenario base conversion.  fsexam_dryrun_convert_filename() will convert
 *      based on dryrun result.
 *
 *  Return value:
 *      TRUE if success, otherwise fail
 *
 *  Author:     Yandong Yao 2006/08/31
 ========================================================================*/ 
gboolean fsexam_convert_scenario (FSEXAM_setting *);

gboolean fsexam_restore (FSEXAM_setting *, GList *list, ConvType restore_type);

gchar   *find_non_exist_name (const gchar *name);

/* --------- Default handler ----------------- */
gint get_index_default (GList *encoding_list, gboolean forname);
void display_msg_default (const gchar *filename, const gchar *msg);


#endif //_FSEXAM_CONVNAME_H
