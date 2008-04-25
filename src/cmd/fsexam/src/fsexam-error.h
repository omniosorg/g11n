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


#ifndef _FSEXAM_ERROR_H
#define _FSEXAM_ERROR_H

/*
 * If change any ERR_*, exec 'make clean' before 'make'
 */
typedef enum {
    /* INFO */
    ERR_OK =                    0,
    ERR_NAME_UTF8_ALREADY, 
    ERR_CONTENT_UTF8_ALREADY,
    ERR_NAME_SAME,
    /* ERROR */
    ERR_ERROR,                  /* 1 */ 
    ERR_FILE_NONEXIST, 
    ERR_NO_MEMORY, 
    ERR_ENCODING_INDEX_INVALID,
    ERR_CANNOT_OPEN,    
    ERR_CANNOT_RENAME, 
    ERR_CANNOT_WRITE,
    ERR_NO_PROPER_ENCODING,
    ERR_NOT_REG_FILE,
    ERR_SYMLINK_TARGET_NOEXIST,
    ERR_CANNOT_CONVERT, 
    ERR_CANNOT_READ, 
    ERR_NO_RIGHTS, 
    ERR_CANNOT_RM_SYMLINK,
    ERR_LOST_SYMLINK_FILE,
    ERR_DRYRUN_FILE_INVALID,
    ERR_GET_CWD,
    ERR_CHDIR,
    /* WARNING */
    ERR_WARNING, 
    ERR_HIST_NO_ITEM,
    ERR_TREE_IS_EMPTY, 
    ERR_NAME_EXIST, 
    ERR_IGNORE_HIDDEN_FILE,
    ERR_IGNORE_REMOTE_FILE,
    ERR_EMPTY_FILE, 
    ERR_NO_PARAMS, 
    ERR_FILTER, 
    ERR_FILE_TYPE_NOT_SUPPORT, 
    ERR_BUFFER_OVERFLOW,
    ERR_CANCEL_CONVERSION,
    /* Won't write log */
    ERR_WONNOT_LOG, 
    ERR_CANNOT_WRITE_DRYRUN,
    ERR_CANNOT_OPEN_DRYRUN,
    ERR_CANNOT_CONVERT_TO_URI,
    /* Misc error */
    ERR_MISC, 
} ERROR_NO;

extern ERROR_NO fsexam_errno;

const char  *fsexam_error_get_msg (void);

void        _fsexam_perror (char *errnofile, int line, char *filename);

#define fsexam_perror(filename) \
        do {        \
            _fsexam_perror (__FILE__, __LINE__, filename);  \
        }while (0) 

/*
 * How to make one elegant error handling machanism?
 *
 *      - system error: errno need remember
 *      - custermized error: fsexam_errno
 *      - error context: such as 
 *          - filename, may be more than one file
 *          - Sometime there is error occured, but we still have
 *            get some result, such as get_abs_path, we have get
 *            the abs path, but can't rechange to old_cwd
 *
 *      - buffer overflow: do we need check and report?
 *          - strlcpy, strlcat
 *          - fgets
 *
 *      - Who need to check the input is NULL or not?
 *          - caller?
 *          - callee?
 */

#endif //_FSEXAM_ERROR_H
