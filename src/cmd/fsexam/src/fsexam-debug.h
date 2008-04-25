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

#ifndef _FSEXAM_DEBUG
#define _FSEXAM_DEBUG

#define FSEXAM_DBG_OPTION       (1 << 0)
#define FSEXAM_DBG_TREE         (1 << 1)
#define FSEXAM_DBG_ENCODING     (1 << 2)
#define FSEXAM_DBG_HASH         (1 << 3)
#define FSEXAM_DBG_FILELIST     (1 << 4)
#define FSEXAM_DBG_HISTORY      (1 << 5)
#define FSEXAM_DBG_PLAIN_TEXT   (1 << 6)
#define FSEXAM_DBG_ARCHIVE      (1 << 7)

gint fsexam_debug (void);

#endif  //_FSEXAM_DEBUG
