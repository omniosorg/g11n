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


#ifndef _FSEXAM_CONVCONTENT_H
#define _FSEXAM_CONVCONTENT_H

/*============================================================================
 *  Function Name:  fsexam_convert_content
 *
 *  Parameters:
 *      FSEXAM_setting *setting: struct contain all information
 *      gchar * filename:   the filename which will be converted
 *
 *  Desc:
 *      Convert one file's content to UTF-8
 *
 *  Return value:
 *      True for success otherwise fail
 *
 *  Author:     Yandong Yao 2006/08/22
 ============================================================================*/ 
gboolean fsexam_convert_content (FSEXAM_setting *setting, 
                                 const gchar *filename);
/*============================================================================
 *  Function Name:  fsexam_convert_filelist_for_content 
 *
 *  Parameters:
 *      FSEXAM_setting *setting: the all-in-one struct
 *      GList *list: The list contain file names
 *
 *  Desc:
 *      Convert the content of file list 
 *
 *  Return value:
 *      TRUE for success, otherwise fail
 *
 *  Author:     Yandong Yao 2006/08/22
 ============================================================================*/
gboolean fsexam_convert_content_batch (FSEXAM_setting *setting, GList *list);

gboolean fsexam_convert_scenario_for_content (FSEXAM_setting *setting);

gboolean fsexam_restore_content (FSEXAM_setting *setting, GList *list);

#endif //_FSEXAM_CONVCONTENT_H
