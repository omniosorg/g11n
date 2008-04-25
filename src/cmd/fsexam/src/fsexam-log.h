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


#ifndef _FSEXAM_LOG_H
#define _FSEXAM_LOG_H

/*
 * What is need log and what is not.
 *
 * The problem is that when we want to write log/mes info to the
 * terminal, we need the fullpath, while the subcoponent of fullpath
 * may contain NON-UTF8 characters.
 *
 * We can always met with such problem if we have no rights to 
 * modify filename on the disk, but we can always reduce this kind
 * of problem as far as possible.
 *
 * So we defer the error msg during constructing tree and handle them
 * during convertsion to ensure the path name conatin no non-utf8 name
 * as far as possible.
 *
 *  - File doesn't exist
 *
 *  - File Type Not Supported
 * 
 *  - No History Item:
 *      Display in log file when there is no history info.
 *
 *  -----------------------------------------------------
 *  - Symlink target doesn't exist: 
 *      the target file does't exist: get_abs_path_for_symlink_target == NULL
 *      log when traverse tree and after we convert symlink's filename
 *      Restore: 
 *
 *  - UTF-8 already:    
 *      Restore: Don't display such kind of error info, cause it is proper 
 *      behavior
 *
 *  - Remote file
 *
 *  - Hidden file
 *
 */

typedef struct _Log_info Log_info;
struct _Log_info {
    FILE *fp;
};

#define LOG_INFO    "(II)"
#define LOG_WARNING "(WW)"
#define LOG_ERROR   "(EE)"

Log_info *  fsexam_log_open (const gchar *logfile);
gboolean    fsexam_log_puts (Log_info *info, 
                             const gchar *filename, 
                             const gchar *msg);
gboolean    fsexam_log_puts_folder_and_name (Log_info *info, 
                             const gchar *dirname, 
                             const gchar *filename, 
                             const gchar *msg);

void        fsexam_log_flush (Log_info *info);
void        fsexam_log_close (Log_info *info);

#endif //_FSEXAM_LOG_H
