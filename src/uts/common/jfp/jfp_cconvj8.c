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
 * Copyright (c) 1991-1997,2004 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma ident  "@(#)jfp_cconvj8.c 1.19     04/08/10 SMI"

/*
 * JFP Code Converter for 8bit JIS terminal.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/errno.h>
#include <sys/ioccom.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>
#ifdef DEBUG
#include <sys/cmn_err.h>
#endif
#include <sys/eucioctl.h>
#include <sys/kmem.h>
#include "jaio.h"
#include "japanese.h"

/*
 * This is the loadable module wrapper.
 */
#include <sys/conf.h>
#include <sys/modctl.h>

extern struct streamtab jconv8info;

static struct fmodsw fsw = {
	"jconv8",
	&jconv8info,
	D_NEW|D_MTQPAIR|D_MP
};

/*
 * module linkage information for kernel
 */
extern struct mod_ops mod_strmodops;

static struct modlstrmod modlstrmod = {
	&mod_strmodops,
	"str mod for JIS8 terminal",
	&fsw
};

static struct modlinkage modlinkage = {
	MODREV_1,
	(void *)&modlstrmod,
	NULL
};

/*
 * This is the module initialization routine.
 */
int
_init(void)
{
	return (mod_install(&modlinkage));
}

int
_fini(void)
{
	return (mod_remove(&modlinkage));
}

int
_info(struct modinfo *modinfop)
{
	return (mod_info(&modlinkage, modinfop));
}

extern int jfpbappend(mblk_t **bpp, int ch);

/*
 * These open, close, put, srv functions return int, but
 * return values are always ignored. See put(9E)/srv(9E)
 */
static int jconv8open(queue_t *, dev_t *, int, int, cred_t *);
static int jconv8close(queue_t *, int, cred_t *);
static int jconv8rput(queue_t *, mblk_t *);
static int jconv8wput(queue_t *, mblk_t *);
static void do_ioctl(queue_t *, mblk_t *);
static void do_mctl(queue_t *, mblk_t *);
static void putesc(void *);
static void putls0(void *);
static void putls1(void *);

static struct module_info jconv8miinfo = {
	111,
	"jconv8",
	0,
	INFPSZ,
	128,
	64
};

static struct qinit jconv8rinit = {
	jconv8rput,
	NULL,
	jconv8open,
	jconv8close,
	NULL,
	&jconv8miinfo,
	NULL
};

static struct module_info jconv8moinfo = {
	111,
	"jconv8",
	0,
	INFPSZ,
	300,
	0
};

static struct qinit jconv8winit = {
	jconv8wput,
	NULL,
	jconv8open,
	jconv8close,
	NULL,
	&jconv8moinfo,
	NULL
};

struct streamtab jconv8info = {
	&jconv8rinit,
	&jconv8winit,
	NULL,
	NULL
};

/*
 * JIS special characters.
 */
#define	ESC	033	/* 1st. char of KI/KO (constant) */
#define	KI2	'$'	/* 2nd. char of KI (constant) */
#define	KI3	'B'	/* 3rd. char of KI (VARIABLE) */
#define	KO2	'('	/* 2nd. char of KO (constant) */
#define	KO3	'J'	/* 3rd. char of KO (VARIABLE) */

#define	LS1	016	/* Locking Shift 1, go G1 set (same as SO) */
#define	LS0	017	/* Locking Shift 0, come back G0 set (same as SI) */

/*
 * 'LS0' calls G0 set.
 * 'LS1' calls G1 set.
 *
 * "KI" puts KANJI character set into G0 set.
 * "KO" puts ASCII character set into G0 set.
 *
 * Halfsize Katakana is always in G1 set.
 */
#define	G0SET		0	/* G0 set */
#define	G1SET		1	/* G1 set */

#define	SET_ASCII	0	/* ASCII */
#define	SET_KANJI	1	/* Kanji */
#define	SET_KANA	2	/* Katakana */

/*
 * Input/Output status
 */
#define	GOT_ASCII	0	/* ASCII char */
#define	GOT_FIRST_BYTE	1	/* 1st. byte of Kanji char */

/* Input status */
#define	GOT_ESC		2	/* ESC */
#define	GOT_KI2		3	/* '$' */
#define	GOT_KO2		4	/* '(' */
#define	GOT_LS0		5	/* LS0 */
#define	GOT_LS1		6	/* LS1 */

/* Output status */
#define	GOT_SS2		2	/* SS2 */
#define	GOT_SS3		3	/* SS3, NOT USED NOW! */

/* Previous mode */
#define	M_ASCII		0
#define	M_KANJI		1
#define	M_KANA		2

/*
 * timeout clicks
 */
#define	NCLICK	8

typedef struct {
	int do_conv;		/* indicate doing conversion or not */
	char ki;		/* 3rd char of KI */
	char ko;		/* 3rd char of KO */
/* read side */
	int r_charset;		/* current charcter set, G0 or G1 */
	int r_saveset;		/* previous character set */
	int r_G0set;		/* current G0 character set is */
	int r_G1set;		/* current G1 character set is */
	int r_state;		/* previous JIS character category */
	unsigned char j1;	/* previous character for JIS to EUC */
/* write side */
	int w_pstate;		/* previous state ASCII/KANJI/KANA */
	int w_state;		/* previous EUC character category */
	unsigned char e1;	/* previous character for EUC to JIS */
	timeout_id_t vtid[GOT_LS1+1];	/* vtime timer id */
} jis8_state_t;

/*ARGSUSED*/
static int
jconv8open(queue_t *q, dev_t *dev, int oflag, int sflag, cred_t *cred_p)
{
	register jis8_state_t *jp;

	if (sflag != MODOPEN) {
		return (OPENFAIL);
	}
	if (q->q_ptr != NULL) {
		return (0);	/* already attached */
	}
	qprocson(q);
	jp = (jis8_state_t *)kmem_zalloc(sizeof (jis8_state_t), KM_NOSLEEP);
	if (jp == (jis8_state_t *)NULL)
		return (ENOMEM);

	jp->do_conv = ON;	/* do code conversion */

	jp->ki = KI3;	/* 'B' */
	jp->ko = KO3;	/* 'J' */

	/* read side */
	jp->r_charset = G0SET;
	jp->r_saveset = G0SET;
	jp->r_G0set = SET_ASCII;
	jp->r_G1set = SET_KANA;
	jp->r_state = GOT_ASCII;
	jp->j1 = '\0';

	/* write side */
	jp->w_pstate = M_ASCII;
	jp->w_state = GOT_ASCII;
	jp->e1 = '\0';

	q->q_ptr = (void *)jp;
	WR(q)->q_ptr = (void *)jp;
	return (0);
}

/* ARGSUSED */
static int
jconv8close(queue_t *q, int flag, cred_t *cred_p)
{
	register jis8_state_t *jp = (jis8_state_t *)q->q_ptr;

	quntimeout(q, jp->vtid[GOT_ESC]); /* Clear callback func. */
	quntimeout(q, jp->vtid[GOT_LS0]); /* Clear callback func. */
	quntimeout(q, jp->vtid[GOT_LS1]); /* Clear callback func. */
	qprocsoff(q);
	kmem_free(jp, sizeof (jis8_state_t));
	q->q_ptr = NULL;
	return (0);
}

#define	APPEND_CHAR(c) \
	while ((ret_val = jfpbappend(&nbp, (c))) <= 0) { \
		if (ret_val == 0) { \
			if (nmp == NULL) \
				nmp = nbp; \
			else \
				linkb(nmp, nbp); \
			nbp = NULL; \
			continue; \
		} else {	/* allocb() failed. */ \
			return (ENOMEM); \
		} \
	}
/*
 * Read side code conversion.
 *	From JIS8 to EUC.
 */
static int
jconv8rput(queue_t *q, mblk_t *mp)
{
	register jis8_state_t *jp = (jis8_state_t *)q->q_ptr;
	register mblk_t *bp;
	register unsigned char c;
	mblk_t *nmp, *nbp;
	int ret_val;

	switch (mp->b_datap->db_type) {
	default:
		putnext(q, mp);
		return (0);

	case M_DATA:
		break;
	}

	if (jp->do_conv == OFF) {	/* no conversion mode */
		putnext(q, mp);
		return (0);
	}

	nmp = NULL;
	nbp = NULL;
	for (bp = mp; bp != NULL; bp = bp->b_cont) {
		while (bp->b_rptr < bp->b_wptr) {

			c = *bp->b_rptr;

			if (jp->r_state == GOT_LS0)
				quntimeout(q, jp->vtid[GOT_LS0]);
			else if (jp->r_state == GOT_LS1)
				quntimeout(q, jp->vtid[GOT_LS1]);

			if (jp->r_state == GOT_FIRST_BYTE) {
				APPEND_CHAR(jp->j1);
				APPEND_CHAR((c | 0200));
				jp->r_state = GOT_ASCII;
			} else if (jp->r_state == GOT_KI2) {
				if (c == jp->ki) {
					jp->r_G0set = SET_KANJI;
				} else {
					APPEND_CHAR(ESC);
					APPEND_CHAR(KI2);
					APPEND_CHAR(c);
				}
				jp->r_state = GOT_ASCII;
			} else if (jp->r_state == GOT_KO2) {
				if (c == jp->ko) {
					jp->r_G0set = SET_ASCII;
				} else {
					APPEND_CHAR(ESC);
					APPEND_CHAR(KO2);
					APPEND_CHAR(c);
				}
				jp->r_state = GOT_ASCII;
			} else if (jp->r_state == GOT_ESC) {
				quntimeout(q, jp->vtid[GOT_ESC]);
				if (c == KI2)
					jp->r_state = GOT_KI2;
				else if (c == KO2)
					jp->r_state = GOT_KO2;
				else {
					APPEND_CHAR(ESC);
					APPEND_CHAR(c);
					jp->r_state = GOT_ASCII;
				}
			} else if (c == ESC) {
				jp->r_state = GOT_ESC;
				jp->vtid[GOT_ESC] =
				qtimeout(q, putesc, (void *)q, NCLICK);
			} else if (c == LS0) {
				jp->r_state = GOT_LS0;
				jp->r_saveset = jp->r_charset;
				jp->r_charset = G0SET;
				jp->vtid[GOT_LS0] =
				qtimeout(q, putls0, (void *)q, NCLICK);
			} else if (c == LS1) {
				jp->r_state = GOT_LS1;
				jp->r_saveset = jp->r_charset;
				jp->r_charset = G1SET;
				jp->vtid[GOT_LS1] =
				qtimeout(q, putls1, (void *)q, NCLICK);
			} else if (c <= 040 || c == 0177) {
				APPEND_CHAR(c);
			} else if (c < 0200) {
				if (jp->r_charset == G0SET) {
					if (jp->r_G0set == SET_ASCII) {
						APPEND_CHAR(c);
					} else {
						jp->j1 = c | 0200;
						jp->r_state = GOT_FIRST_BYTE;
					}
				} else {
					APPEND_CHAR(SS2);
					APPEND_CHAR(c);
				}
			} else {
				APPEND_CHAR(SS2);
				APPEND_CHAR(c);
			}
			bp->b_rptr++;
		}
	}
	if (nmp == NULL)
		nmp = nbp;
	else
		linkb(nmp, nbp);
	freemsg(mp);
	if (nmp)
		putnext(q, nmp);
	return (0);
}

static void
putesc(void *q)
{
	jis8_state_t *jp = (jis8_state_t *)(((queue_t *)q)->q_ptr);
	mblk_t *bp;

	if ((bp = allocb(4, BPRI_HI)) == NULL) {
		cmn_err(CE_WARN, "putesc: allocb failed.\n");
	}
	*bp->b_wptr++ = ESC;
	jp->r_state = GOT_ASCII;
	putnext(q, bp);
}

static void
putls0(void *q)
{
	jis8_state_t *jp = (jis8_state_t *)(((queue_t *)q)->q_ptr);
	mblk_t *bp;

	if ((bp = allocb(4, BPRI_HI)) == NULL) {
		cmn_err(CE_WARN, "putls0: allocb failed.\n");
	}
	*bp->b_wptr++ = LS0;
	jp->r_charset = jp->r_saveset;
	jp->r_state = GOT_ASCII;
	putnext(q, bp);
}

static void
putls1(void *q)
{
	jis8_state_t *jp = (jis8_state_t *)(((queue_t *)q)->q_ptr);
	mblk_t *bp;

	if ((bp = allocb(4, BPRI_HI)) == NULL) {
		cmn_err(CE_WARN, "putls1: allocb failed.\n");
	}
	*bp->b_wptr++ = LS1;
	jp->r_charset = jp->r_saveset;
	jp->r_state = GOT_ASCII;
	putnext(q, bp);
}

/*
 * Write side code conversion.
 *	From EUC to JIS8.
 */
static int
jconv8wput(queue_t *q, mblk_t *mp)
{
	register jis8_state_t *jp = (jis8_state_t *)q->q_ptr;
	register mblk_t *bp;
	register unsigned char c;
	mblk_t *nmp, *nbp;
	int ret_val;

	switch (mp->b_datap->db_type) {

	default:
		putnext(q, mp);
		return (0);

	case M_CTL:
		do_mctl(q, mp);
		return (0);
	case M_IOCTL:
		do_ioctl(q, mp);
		return (0);

	case M_DATA:
		break;
	}

	if (jp->do_conv == OFF) {	/* no conversion mode */
		putnext(q, mp);
		return (0);
	}

	nmp = NULL;
	nbp = NULL;
	for (bp = mp; bp != NULL; bp = bp->b_cont) {
		while (bp->b_rptr < bp->b_wptr) {

			c = *bp->b_rptr;

			if (jp->w_state == GOT_FIRST_BYTE) {
				APPEND_CHAR(jp->e1);
				APPEND_CHAR((c & 0177));
				jp->w_state = GOT_ASCII;
			} else if (jp->w_state == GOT_SS2) {
				APPEND_CHAR(c);
				jp->w_state = GOT_ASCII;
			} else if (c == SS2) {
				if (jp->w_pstate == M_KANJI) {
					APPEND_CHAR(ESC);
					APPEND_CHAR(KO2);
					APPEND_CHAR(jp->ko);
				}
				jp->w_state = GOT_SS2;
			} else if (c >= 0200) {
				APPEND_CHAR(ESC);
				APPEND_CHAR(KI2);
				APPEND_CHAR(jp->ki);
				jp->w_state = GOT_FIRST_BYTE;
				jp->w_pstate = M_KANJI;
				jp->e1 = c & 0177;
			} else {
				if (jp->w_pstate == M_KANJI) {
					APPEND_CHAR(ESC);
					APPEND_CHAR(KO2);
					APPEND_CHAR(jp->ko);
				}
				APPEND_CHAR((c & 0177));
				jp->w_state = GOT_ASCII;
				jp->w_pstate = M_ASCII;
			}
			bp->b_rptr++;
		}
	}
	if (nmp == NULL)
		nmp = nbp;
	else
		linkb(nmp, nbp);
	freemsg(mp);
	if (nmp)
		putnext(q, nmp);
	return (0);
}

static void
do_ioctl(queue_t *q, mblk_t *mp)
{
	register jis8_state_t *jp;
	register struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	jp = (jis8_state_t *)q->q_ptr;

	switch (iocp->ioc_cmd) {

	case JA_SKIOC: {
		register kioc_t *kio = (kioc_t *)mp->b_cont->b_rptr;

		if (kio == NULL) {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_count = sizeof (kioc_t);
			iocp->ioc_error = EPROTO;
			iocp->ioc_rval = (-1);
			qreply(q, mp);
			break;
		}
		jp->ki = kio->ki;
		jp->ko = kio->ko;

		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = sizeof (kioc_t);
		iocp->ioc_rval = 0;
		qreply(q, mp);
		break;
	}

	case JA_GKIOC: {
		register kioc_t *kio = (kioc_t *)mp->b_cont->b_rptr;

		if (kio == NULL) {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_count = sizeof (kioc_t);
			iocp->ioc_error = EPROTO;
			iocp->ioc_rval = (-1);
			qreply(q, mp);
			break;
		}
		kio->ki = jp->ki;
		kio->ko = jp->ko;

		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = sizeof (kioc_t);
		iocp->ioc_rval = 0;
		qreply(q, mp);
		break;
	}

	case EUC_OXLON:
		jp->do_conv = ON;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		qreply(q, mp);
		break;

	case EUC_OXLOFF:
		jp->do_conv = OFF;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		qreply(q, mp);
		break;

	default:
		putnext(q, mp);
		break;
	}
}

/*
 * function to communicate with upper module
 * while being used in ja_JP.PCK locale.
 * [06/10/96(JST) inukai]
 */
static void
do_mctl(queue_t *q, mblk_t *mp)
{
	register jis8_state_t *jp;
	register mc_m_ctl_msg_t *mctlp;

	mctlp = (mc_m_ctl_msg_t *)mp->b_rptr;
	jp = (jis8_state_t *)q->q_ptr;

	switch (mctlp->mc_m_ctl_cmd) {

	case MC_PEERQUERY:	/* Just echo back to notify its presence */
		qreply(q, mp);
		break;

	case MC_DO_CONVERT:
		jp->do_conv = ON;
		qreply(q, mp);
		break;

	case MC_NO_CONVERT:
		jp->do_conv = OFF;
		qreply(q, mp);
		break;

	default:
		putnext(q, mp);
		break;
	}
}
