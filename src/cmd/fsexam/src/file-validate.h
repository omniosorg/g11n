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

#ifndef _FILE_VALIDATE_H
#define _FILE_VALIDATE_H

/*
 * file-validate.h
 *
 * Given a filename, determine whether it pass some criteria.
 *
 * Options:
 *      - '--follow': handle at preprocessing stage
 *      - '--symlink-target': handle at tree constructing stage
 * 
 * For filename conversion:
 *      - Filter only directory/reg/symbolic file out
 *      - Hidden flags
 *      - Forceful conversion
 *      - follow: this will be handled in fsexam-preprocessing.c
 *      - remote file
 *      - no-check-symlink-target
 *
 * For file content conversion:
 *      - Filter only dir/regular/symbolic file out
 *      - Hidden flags
 *      - Forceful conversion
 *      - Is plain text
 *      - Special file such as .tar, .zip
 *      - follow: this will be handled in fsexam-preprocessing.c
 *      - remote file
 */

/*============================================================================
 *  Function Name:  file_validate_for_contentconv
 *
 *  Parameters:
 *      gchar *filename: the filename we will validate
 *      VALIDATE_flag *flag: struct contain flags used to validate file
 *
 *  Desc:
 *      file_validate_for_contentconv() determine whether fsexam will handling 
 *      its content or not.
 *
 *  Return value:
 *      True if fsexam will handle this file.
 *      Otherwise False.
 *
 *  Author:     Yandong Yao 2006/08/24
 ===========================================================================*/
gboolean file_validate_for_contentconv(
                            const gchar *filename, 
                            FSEXAM_setting *setting);

/*============================================================================
 *  Function Name:  file_validate_for_nameconv
 *
 *  Parameters:
 *      gchar *filename: the filename we will validate
 *      VALIDATE_flag *flag: struct contain flags used to validate file
 *
 *  Desc:
 *      file_validate_for_nameconv() will determine whether fsexam will 
 *      convert filename or not.
 *
 *  Return value:
 *      True if fsexam will handle this file's name, otherwise False
 *
 *  Author:     Yandong Yao 2006/08/24
 ==========================================================================*/
gboolean file_validate_for_nameconv(const gchar *filename, 
                                    struct stat *statbuf, 
                                    FSEXAM_setting *setting);

GList    *get_remote_paths ();
gboolean is_remote_file (GList *remote_path, const gchar *filename);

#endif //_FSEXAM_VALIDATE_H
