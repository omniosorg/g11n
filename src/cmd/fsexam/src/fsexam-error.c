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

#include "fsexam-error.h"

#include <stdio.h>
#include <glib/gi18n.h>

char *errmsg[] = {
    N_("Success"),                              /* ERR_OK */
    N_("File name is UTF-8 already"),           /* ERR_NAME_UTF8_ALREADY */
    N_("File content is UTF-8 already"),        /* ERR_CONTENT_UTF8_ALREADY */
    N_("New name is the same as old name"),     /* ERR_NAME_SAME */

    /* Error */
    "holder for ERR_ERROR",
    N_("File doesn't exist"),                   /* ERR_FILE_NONEXIST */
    N_("No Memory"),                            /* ERR_NO_MEMORY */
    N_("Invalid encoding index"),               /* ERR_ENCODING_INDEX_INVALID */
    N_("Can't open file"),                      /* ERR_CANNOT_OPEN */
    N_("Can't rename file"),                    /* ERR_CANNOT_RENAME */
    N_("Can't write file"),                     /* ERR_CANNOT_WRITE */
    N_("Given encoding list is not suitable"),  /* ERR_NO_PROPER_ENCODING */
    N_("Is not regular file"),                  /* ERR_NOT_REG_FILE */
    N_("Symlink target don't exist"),           /* ERR_SYMLINK_TARGET_NOEXIST */
    N_("Can't convert given file"),             /* ERR_CANNOT_CONVERT */
    N_("Can't read file"),                      /* ERR_CANNOT_READ */
    N_("No proper permission"),                     /* ERR_NO_RIGHTS */
    /* ERR_CANNOT_RM_SYMLINK */
    N_("Can't relink symbolic link which will break symbolic link"), 
    /* ERR_LOST_SYMLINK_FILE */
    N_("Have deleted symbolic link file, but can't recreate it"), 
    N_("Dryrun file format is invalid"),        /* ERR_DRYRUN_FILE_INVALID */
    N_("Can't get current directory"),          /* ERR_GET_CWD */
    N_("Can't change directory"),               /* ERR_CHDIR */
   
    /* Warning */
   "HOLDER for ERR_WARNING", 
    N_("No history information for current file"),   /* ERR_HIST_NO_ITEM  */
    N_("No given files or all files are UTF-8 already"), /* ERR_TREE_IS_EMPTY */
    /* ERR_NAME_EXIST */
    N_("Same name file exists, will append 'utf-8' as suffix"),
    N_("Skip hidden file"),                     /* ERR_IGNORE_HIDDEN_FILE */
    N_("Skip non-local file"),                  /* ERR_IGNORE_REMOTE_FILE */
    N_("Empty file"),                           /* ERR_EMPTY_FILE */
    N_("No parameters"),                        /* ERR_NO_PARAMS */
    N_("Error occurred during filtering"),      /* ERR_FILTER */
    N_("Don't support this kind of file type"), /* ERR_FILE_TYPE_NOT_SUPPORT */
    N_("Buffer overflow"),                      /* ERR_BUFFER_OVERFLOW */
    N_("Canceled the conversion"),              /* ERR_CANCEL_CONVERSION */

    /* Won't Log */
    "HOLDER for won't log",
    N_("Can't write dryrun result file"),       /* ERR_CANNOT_WRITE_DRYRUN */
    N_("Can't open dryrun result file"),        /* ERR_CANNOT_OPEN_DYRRUN */
    N_("Can't convert filename to URI"),        /* ERR_CANNOT_CONVERT_TO_URI */

    /* MISC*/
    N_("Unknown error occurred"),                /* ERR_MISC */
};

//TODO: store the errno in the system.
ERROR_NO fsexam_errno = ERR_OK;

const char *
fsexam_error_get_msg ()
{
    return _(errmsg[fsexam_errno]);
}

void
_fsexam_perror (char *errnofile, int line, char *filename)
{
    if (filename != NULL){
        printf (_("ERROR ==> %s:%d (%s): %s\n"), errnofile, line, 
                filename, _(errmsg[fsexam_errno]));
        return;
    }

    printf (_("ERROR ==> %s:%d %s\n"), errnofile, line, _(errmsg[fsexam_errno]));

    return;
}
