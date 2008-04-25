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


#ifndef _FSEXAM_HEADER_H
#define _FSEXAM_HEADER_H

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>

#include "fsexam.h"
#include "fsexam-debug.h"
#include "auto-detect.h"
#include "fsexam-helper.h"
#include "encoding.h"
#include "fsexam-log.h"
#include "fsexam-history.h"
#include "fsexam-dryrun.h"
#include "fsexam-pref.h"
#include "fsexam-setting.h"
#include "fsexam-plaintext.h"
#include "fsexam-specialfile.h"
#include "fsexam-tree.h"
#include "fsexam-error.h"
#include "file-validate.h"
#include "fsexam-convname.h"
#include "fsexam-convcontent.h"
#include "fsexam-encoding-dialog.h"

#endif
