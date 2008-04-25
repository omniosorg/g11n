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


#ifndef _FSEXAM_SPECIALFILE_H
#define _FSEXAM_SPECIALFILE_H

#define SPECIAL_NO          (0)
#define SPECIAL_COMPRESS    (1<<0)

/*
 *  API for special file content conversion
 */

gboolean fsexam_special_is_special_for_content (const gchar *fullpath, 
                        FSEXAM_setting *setting);
gboolean fsexam_special_convert_content (const gchar *fullpath, 
                        FSEXAM_setting *setting,
                        gboolean restore);

/*
 *  API for special file name conversion. 
 */

gboolean fsexam_special_is_special_for_name (const gchar *fullpath, 
                        FSEXAM_setting *setting);

gboolean fsexam_compress_convert_name (FSEXAM_setting *setting, 
                        const gchar *fullpath, 
                        const gchar *hist_search_path, 
                        gboolean restore);

#endif //_FSEXAM_SPECIALFILE_H
