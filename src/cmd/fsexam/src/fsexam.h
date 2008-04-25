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

#ifndef _FSEXAM_H_
#define _FSEXAM_H_

#define FSEXAM_VERSION      "0.4"

#define HISTORY_LENGTH      100

#define FSEXAM_GUI_NAME     "fsexam"
#define FSEXAM_CLI_NAME     "fsexamc"

#define FEEXIT_SUCCESS      0
#define FEEXIT_FAILURE      1

#define FSEXAM_HIDDEN       ".fsexam"
#define HISTORY_FILE        "fsexam-hist.txt"

#ifndef PATH_MAX
#define PATH_MAX	2048
#endif

extern gboolean cli_mode;
extern gboolean force_quit;
extern gboolean stop_search;

void   fsexam_cleanup_all ();

#endif  //_FSEXAM_H_
