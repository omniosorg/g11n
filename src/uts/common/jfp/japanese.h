/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").  
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright (c) 1991, 1992, 1993, 1994, 1995, 1996 Sun Microsystems, Inc.
 * Copyright (c) 1991, 1992, 1993, 1994, 1995, 1996 Nihon Sun Microsystems K.K.
 * All rights reserved.
 */

#ident	"@(#)japanese.h	1.3	96/09/19	SMI"

/*
 * M_CTL IDs and data structures are assigned after
 * talking with STREAMS engineers (Mike.Tracy@Eng)
 */
#define	MC_MCTL_ID	(('J' << 24) | ('C' << 16))
#define	MC_PEERQUERY	(MC_MCTL_ID | 1)
#define	MC_DO_CONVERT	(MC_MCTL_ID | 2)
#define	MC_NO_CONVERT	(MC_MCTL_ID | 3)

typedef struct mc_m_ctl_msg {
	int mc_m_ctl_cmd;
} mc_m_ctl_msg_t;

#define	SS2	0x8e	/* Single Shift 2 */
#define	SS3	0x8f	/* Single Shift 3 */
#ifndef ON
#define	ON	1
#endif
#ifndef OFF
#define	OFF	0
#endif
