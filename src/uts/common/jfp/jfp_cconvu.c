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
 * Copyright (c) 1997, by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma ident  "@(#)jfp_cconvu.c	1.2	97/10/28 SMI"

/*
 * JFP Code Converter for UTF-8 terminal
 * (to be pushed between ldterm and ptem)
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/open.h>
#include <sys/cred.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>
#include <sys/cmn_err.h>
#include <sys/eucioctl.h>
#include <sys/kmem.h>
#include <sys/termio.h>
#include <sys/eucioctl.h>
#include "jfp_euc16.h"
#include "jfp_ucsutf.h"
#include "japanese.h"

/*
 * This is the loadable module wrapper.
 */
#include <sys/conf.h>
#include <sys/modctl.h>

extern struct streamtab jconvuinfo;

static struct fmodsw fsw = {
	"jconvu",
	&jconvuinfo,
	D_NEW|D_MTQPAIR|D_MP
};

/*
 * module linkage information for kernel
 */
extern struct mod_ops mod_strmodops;

static struct modlstrmod modlstrmod = {
	&mod_strmodops,
	"strmod for UTF-8 terminal (driver side)",
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
_init()
{
	return (mod_install(&modlinkage));
}

int
_fini()
{
	return (mod_remove(&modlinkage));
}

int
_info(struct modinfo *modinfop)
{
	return (mod_info(&modlinkage, modinfop));
}

/*
 * These open, close, put, srv functions return int, but
 * return values are always ignored. See put(9E)/srv(9E)
 */
static int jconvuopen(queue_t *, dev_t *, int, int, cred_t *);
static int jconvuclose(queue_t *, int, cred_t *);
static int jconvurput(queue_t *, mblk_t *);
static int jconvuwput(queue_t *, mblk_t *);
static void do_ioctl(queue_t *, mblk_t *);
static void do_mctl(queue_t *, mblk_t *);

extern int jfp_utf8_to_euc(mblk_t *, mblk_t **, unsigned char *, int *);
extern int jfp_euc_to_utf8(mblk_t *, mblk_t **, unsigned char *, int *);
extern void jfptrimmsg(mblk_t **);
extern mblk_t *jfpallocb(void);

static struct module_info jconvumiinfo = {
	133,
	"jconvu",
	0,
	INFPSZ,
	128,
	64
};

static struct qinit jconvurinit = {
	jconvurput,
	NULL,
	jconvuopen,
	jconvuclose,
	NULL,
	&jconvumiinfo,
	NULL
};

static struct module_info jconvumoinfo = {
	133,
	"jconvu",
	0,
	INFPSZ,
	300,
	0
};

static struct qinit jconvuwinit = {
	jconvuwput,
	NULL,
	jconvuopen,
	jconvuclose,
	NULL,
	&jconvumoinfo,
	NULL
};

struct streamtab jconvuinfo = {
	&jconvurinit,
	&jconvuwinit,
	NULL,
	NULL
};

typedef struct {
	int		do_conv;	/* indicate doing conversion or not */
	unsigned char	r_utf8[UTF8_LEN_MAX];	/* read side buffer */
	int		r_state;	/* read side state */
	unsigned char	ioc_utf8[UTF8_LEN_MAX];	/* (TIOCSTI) buffder */
	int		ioc_state;	/* (TIOCSTI) state */
	unsigned char	w_euc[EUC_LEN_MAX];	/* write side buffer */
	int		w_state;	/* write side state */
} utf8_state_t;

/*ARGSUSED*/
static int
jconvuopen(queue_t *q, dev_t *dev, int oflag, int sflag, cred_t *cred_p)
{
	register utf8_state_t *sp;

	if (sflag != MODOPEN) {
		return (OPENFAIL);
	}

	if (q->q_ptr != NULL) {
		return (0);	/* already attached */
	}
	qprocson(q);
	sp = (utf8_state_t *)kmem_zalloc(sizeof (utf8_state_t), KM_NOSLEEP);
	if (sp == (utf8_state_t *)NULL)
		return (ENOMEM);

	sp->do_conv = ON;	/* do code conversion */
	sp->r_state = 0;
	sp->ioc_state = 0;
	sp->w_state = 0;
	q->q_ptr = (void *)sp;
	WR(q)->q_ptr = (void *)sp;
	return (0);
}

/* ARGSUSED */
static int
jconvuclose(queue_t *q, int flag, cred_t *cred_p)
{
	register utf8_state_t *sp = (utf8_state_t *)q->q_ptr;

	qprocsoff(q);
	kmem_free(sp, sizeof (utf8_state_t));
	q->q_ptr = NULL;
	return (0);
}

/*
 * Read side code conversion.
 *	From UTF-8 to EUC.
 */
static int
jconvurput(queue_t *q, mblk_t *mp)
{
	register utf8_state_t *sp = (utf8_state_t *)q->q_ptr;
	mblk_t *nmp = NULL;
	int ret_val;

	switch (mp->b_datap->db_type) {

	default:
		putnext(q, mp);
		return (0);

	case M_DATA:
		if (mp->b_rptr == mp->b_wptr) { /* May be EOF */
			putnext(q, mp);
			return (0);
		}
		break;
	}
	if (sp->do_conv == OFF) {
		putnext(q, mp);
		return (0);
	}
	if ((ret_val = jfp_utf8_to_euc(mp, &nmp, sp->r_utf8, &(sp->r_state)))
		!= 0)
		return (ret_val);
	freemsg(mp);
	if (nmp)
		putnext(q, nmp);
	return (0);
}

/*
 * Write side code conversion.
 *	From EUC to UTF-8
 */
static int
jconvuwput(queue_t *q, mblk_t *mp)
{
	register utf8_state_t *sp = (utf8_state_t *)q->q_ptr;
	mblk_t		*nmp = NULL;
	int		ret_val;

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
		if (mp->b_rptr == mp->b_wptr) { /* May be EOF */
			putnext(q, mp);
			return (0);
		}
		break;
	}
	if (sp->do_conv == OFF) {
		putnext(q, mp);
		return (0);
	}
	if ((ret_val = jfp_euc_to_utf8(mp, &nmp, sp->w_euc, &(sp->w_state)))
		!= 0)
		return (ret_val);
	freemsg(mp);
	if (nmp)
		putnext(q, nmp);
	return (0);
}

static void
do_ioctl(queue_t *q, mblk_t *mp)
{
	utf8_state_t	*sp;
	struct iocblk	*iocp;
	mblk_t		*nmp = NULL;
	mblk_t		*bp, *nbp;
	unsigned char	*cp;
	int		ret_val = 0;

	iocp = (struct iocblk *)mp->b_rptr;
	sp = (utf8_state_t *)q->q_ptr;

	switch (iocp->ioc_cmd) {
	case TIOCSTI:
		ret_val = jfp_euc_to_utf8(mp->b_cont,
			&nmp, sp->ioc_utf8, &(sp->ioc_state));
		if (ret_val != 0) {
			/* Memory problem. putnext() anyway */
			if (nmp)
				freeb(nmp);
			putnext(q, mp);
		} else {
			freeb(mp->b_cont);
			if (nmp) {
				for (bp = nmp; bp; bp = bp->b_cont) {
					cp = bp->b_rptr;
					while (cp < bp->b_wptr) {
						nbp = copyb(mp);
						nbp->b_cont = jfpallocb();
						*nbp->b_cont->b_wptr++ =
							*cp++;
						putnext(q, nbp);
					}
				}
				freeb(nmp);
				freeb(mp);
			} else {
				/* Char split. qreply() */
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_count = 0;
				iocp->ioc_error = 0;
				iocp->ioc_rval = 0;
				mp->b_cont = NULL;
				qreply(q, mp);
			}
		}
		break;
	case EUC_OXLON:
		sp->do_conv = ON;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		qreply(q, mp);
		break;

	case EUC_OXLOFF:
		sp->do_conv = OFF;
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
	register utf8_state_t *sp;
	register mc_m_ctl_msg_t *mctlp;

	mctlp = (mc_m_ctl_msg_t *)mp->b_rptr;
	sp = (utf8_state_t *)q->q_ptr;

	switch (mctlp->mc_m_ctl_cmd) {

	case MC_PEERQUERY:	/* Just echo back to notify its presence */
		qreply(q, mp);
		break;

	case MC_DO_CONVERT:
		sp->do_conv = ON;
		qreply(q, mp);
		break;

	case MC_NO_CONVERT:
		sp->do_conv = OFF;
		qreply(q, mp);
		break;

	default:
		putnext(q, mp);
		break;
	}
}
