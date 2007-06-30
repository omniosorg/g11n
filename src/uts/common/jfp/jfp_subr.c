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
 * Copyright (c) 1991-1997, by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma ident  "@(#)jfp_subr.c 1.13     97/10/08 SMI"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <sys/ddi.h>
#include <sys/cmn_err.h>

/*
 * Return a pointer to a fixed memory slot,
 * or NULL if failed to allocate.
 */

#define	MODBLKSZ	16	/* size of message blocks */

mblk_t *
jfpallocb(void)
{
	mblk_t *bp;
	bp = allocb(MODBLKSZ, BPRI_MED);
	return (bp);
}

/*
 * Append a character to a message block.
 * If (*bpp) is null, it will allocate a new block.
 * Return 1 when succeeded in appending, 0 when the current
 * message block is full, and -1 when memory allocation failed.
 */

int
jfpbappend(mblk_t **bpp, int ch)
{
	mblk_t	*bp;

	if ((bp = *bpp) != NULL) {
		if (bp->b_wptr >= bp->b_datap->db_lim)
			return (0);
	} else
		if ((*bpp = bp = jfpallocb()) == NULL) {
			cmn_err(CE_WARN, "jfpbappend(): jfpallocb() failed.\n");
			return (-1);
		}
	*bp->b_wptr++ = (unsigned char)ch;
	return (1);
}

/*
 * detach mblk in mp if it has no contents
 */
void
jfptrimmsg(mblk_t **mp)
{
	mblk_t *bp;
	mblk_t *nmp = (mblk_t *)NULL;
	mblk_t *delp = (mblk_t *)NULL;
	mblk_t *tmp = (mblk_t *)NULL;
#if	defined(DBG_TRIM_LINK) && defined(DBG_TRIM_DATA)
	unsigned char *cp;
#endif	/* defined(DBG_TRIM_LINK) && defined(DBG_TRIM_DATA) */

#ifdef DBG_TRIM_LINK
	for (bp = *mp; bp; bp = bp->b_cont) {
		cmn_err(CE_NOTE, "trim(): Orig. list has 0x%x", bp);
#ifdef DBG_TRIM_DATA
		cp = bp->b_rptr;
		while (cp < bp->b_wptr)
			cmn_err(CE_NOTE,
			"trim(): 0x%x has data 0x%x", bp, *cp++);
#endif
	}
#endif
	if (!(*mp))
		return;
	for (bp = *mp; bp; ) {
		tmp = bp->b_cont;
		bp->b_cont = (mblk_t *)NULL;
		if (bp->b_rptr != bp->b_wptr) {
			if (nmp)
				linkb(nmp, bp);
			else
				nmp = bp;
		} else {
#ifdef DBG_TRIM_LINK
			cmn_err(CE_NOTE, "0x%x will be detached.", bp);
#endif
			if (delp)
				linkb(delp, bp);
			else
				delp = bp;
		}
		bp = tmp;
	}
	*mp = nmp;
#ifdef DBG_TRIM_LINK
	for (bp = *mp; bp; bp = bp->b_cont) {
		cmn_err(CE_NOTE, "trim(): New list has 0x%x", bp);
#ifdef DBG_TRIM_DATA
		cp = bp->b_rptr;
		while (cp < bp->b_wptr)
			cmn_err(CE_NOTE,
			"trim(): 0x%x has data 0x%x", bp, *cp++);
#endif
	}
#endif
	if (delp)
		freemsg(delp);
}
