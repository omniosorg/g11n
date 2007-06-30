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

#pragma ident  "@(#)jfp_cconvru.c	1.3	97/10/28 SMI"

/*
 * JFP Code Converter for ja_JP.UTF-8 locale
 * (to be pushed between ldterm and ttcompat)
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
#ifdef DEBUG
#include <sys/cmn_err.h>
#endif
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

extern struct streamtab jconvruinfo;

static struct fmodsw fsw = {
	"jconvru",
	&jconvruinfo,
	D_NEW|D_MTQPAIR|D_MP
};

/*
 * module linkage information for kernel
 */
extern struct mod_ops mod_strmodops;

static struct modlstrmod modlstrmod = {
	&mod_strmodops,
	"strmod for ja_JP.UTF-8 locale (head side)",
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

/*
 * These open, close, put, srv functions return int, but
 * return values are always ignored. See put(9E)/srv(9E)
 */
static int jconvruopen(queue_t *, dev_t *, int, int, cred_t *);
static int jconvruclose(queue_t *, int, cred_t *);
static int jconvruwput(queue_t *, mblk_t *);
static int jconvrurput(queue_t *, mblk_t *);
static void do_ioctl(queue_t *, mblk_t *);
static int check_param(struct termios *cb);

extern int jfp_utf8_to_euc(mblk_t *, mblk_t **, unsigned char *, int *);
extern int jfp_euc_to_utf8(mblk_t *, mblk_t **, unsigned char *, int *);
extern void jfptrimmsg(mblk_t **);
extern mblk_t *jfpallocb(void);

static struct module_info jconvrumiinfo = {
	132,
	"jconvru",
	0,
	512,
	128,
	64
};

static struct qinit jconvrurinit = {
	jconvrurput,
	NULL,
	jconvruopen,
	jconvruclose,
	NULL,
	&jconvrumiinfo,
	NULL
};

static struct module_info jconvrumoinfo = {
	132,
	"jconvru",
	0,
	512,
	300,	/* Why these values differs from that in jconvrumiinfo? */
	0	/* Why values differs from that in jconvrumiinfo? */
};

static struct qinit jconvruwinit = {
	jconvruwput,
	NULL,
	jconvruopen,
	jconvruclose,
	NULL,
	&jconvrumoinfo,
	NULL
};

struct streamtab jconvruinfo = {
	&jconvrurinit,
	&jconvruwinit,
	NULL,
	NULL
};

#define	RAW	1
#define	NOT_RAW	2
#define	NONE	3

typedef struct {
	int		do_conv;	/* indicate doing conversion or not */
	unsigned char	r_euc[EUC_LEN_MAX];	/* read side buffer */
	int		r_state;	/* read side state */
	unsigned char	w_utf[UTF8_LEN_MAX];	/* write side buffer */
	int		w_state;	/* wirte side state */
	unsigned char	ioc_utf[UTF8_LEN_MAX];	/* TIOCSTI buffer */
	int		ioc_state;	/* TIOCSTI state */
	int		intended_off;	/* User toggled-off intentionally? */
	int		notify;		/* let the lower know conv. toggling */
	struct termios	t_modes;	/* Store terminal parameters */
	uint		fake_ioctl;	/* Store "fake" M_IOCTL id */
} rutf8_state_t;

/*ARGSUSED*/
int
jconvruopen(queue_t *q, dev_t *dev, int oflag, int sflag, cred_t *cred_p)
{
	register rutf8_state_t *sp;
	mblk_t	*mcp,		/* M_CTL for auto-conversion */
		*meucp,		/* M_IOCTL for CSWIDTH */
		*me_data;	/* M_DATA for CSWIDTH */
	eucioc_t *euciocp;
	struct iocblk *iocp;
	mc_m_ctl_msg_t	*mctlp;

	if (sflag != MODOPEN) {
		return (OPENFAIL);
	}
	if (q->q_ptr != NULL) {
		return (0);	/* already attached */
	}
	qprocson(q);

	/* Memory allocation */
	sp = (rutf8_state_t *)
		kmem_zalloc(sizeof (rutf8_state_t), KM_NOSLEEP);
	if (sp == (rutf8_state_t *)NULL)
		return (ENOMEM);
	mcp = allocb(sizeof (mc_m_ctl_msg_t), BPRI_MED);
	if (mcp == (mblk_t *)NULL) {
		kmem_free((void *)sp, sizeof (rutf8_state_t));
		return (ENOMEM);
	}
	meucp = mkiocb(EUC_WSET);
	if (meucp == (mblk_t *)NULL) {
		freeb(mcp);
		kmem_free((void *)sp, sizeof (rutf8_state_t));
		return (ENOMEM);
	}
	me_data = allocb(sizeof (eucioc_t), BPRI_MED);
	if (me_data == (mblk_t *)NULL) {
		freeb(meucp);
		freeb(mcp);
		kmem_free((void *)sp, sizeof (rutf8_state_t));
		return (ENOMEM);
	}

	sp->do_conv = ON;	/* do code conversion */
	sp->r_state = 0;
	sp->ioc_state = 0;
	sp->w_state = 0;

	/*
	 * Formulate an M_CTL message to query if lower module is
	 * pushed.
	 */
	mcp->b_wptr = mcp->b_rptr + sizeof (mc_m_ctl_msg_t);
	mcp->b_datap->db_type = M_CTL;
	mctlp = (mc_m_ctl_msg_t *)mcp->b_rptr;
	mctlp->mc_m_ctl_cmd = MC_PEERQUERY;
	putnext(WR(q), mcp);

	/*
	 * Set CSWIDTH for PCK
	 */
	me_data->b_wptr = me_data->b_rptr + sizeof (eucioc_t);
	me_data->b_datap->db_type = M_DATA;
	euciocp = (eucioc_t *)me_data->b_rptr;
	euciocp->eucw[0] = (unsigned char)1;
	euciocp->eucw[1] = (unsigned char)2;
	euciocp->eucw[2] = (unsigned char)1;
	euciocp->eucw[3] = (unsigned char)2;
	euciocp->scrw[0] = (unsigned char)1;
	euciocp->scrw[1] = (unsigned char)2;
	euciocp->scrw[2] = (unsigned char)1;
	euciocp->scrw[3] = (unsigned char)2;
#ifdef DEBUG
	{
	int i = 0;
	cmn_err(CE_NOTE, "me_data is 0x%x", me_data);
	cmn_err(CE_NOTE, "me_data->b_rptr = 0x%x", me_data->b_rptr);
	cmn_err(CE_NOTE, "me_data->b_wptr = 0x%x", me_data->b_wptr);
	for (i = 0; i < 4; i++) {
		cmn_err(CE_NOTE, "eucw[%d] are %d", i, euciocp->eucw[i]);
		cmn_err(CE_NOTE, "scrw[%d] are %d", i, euciocp->scrw[i]);
		}
	}
#endif
	iocp = (struct iocblk *)meucp->b_rptr;
	sp->fake_ioctl = iocp->ioc_id;
	iocp->ioc_count = sizeof (eucioc_t);
	meucp->b_cont = me_data;
#ifdef DEBUG
	{
	int i = 0;
	cmn_err(CE_NOTE, "Sent meucp");
	cmn_err(CE_NOTE, "me_data is 0x%x", me_data);
	cmn_err(CE_NOTE, "me_data->b_rptr = 0x%x", me_data->b_rptr);
	cmn_err(CE_NOTE, "me_data->b_wptr = 0x%x", me_data->b_wptr);
	cmn_err(CE_NOTE, "meucp is 0x%x", meucp);
	cmn_err(CE_NOTE, "meucp->b_cont = 0x%x", meucp->b_cont);
	for (i = 0; i < 4; i++) {
		eucioc_t *euciocp =
			(eucioc_t *)((mblk_t *)meucp->b_cont)->b_rptr;
		cmn_err(CE_NOTE, "eucw[%d] are %d", i, euciocp->eucw[i]);
		cmn_err(CE_NOTE, "scrw[%d] are %d", i, euciocp->scrw[i]);
		}
	}
#endif
	putnext(WR(q), meucp);

	q->q_ptr = (void *)sp;
	WR(q)->q_ptr = (void *)sp;
	return (0);
}

/*ARGSUSED*/
static int
jconvruclose(queue_t *q, int flag, cred_t *cred_p)
{
	register rutf8_state_t *sp = (rutf8_state_t *)q->q_ptr;

	qprocsoff(q);
	kmem_free(sp, sizeof (rutf8_state_t));
	q->q_ptr = NULL;
	return (0);
}

/*
 * Write side code conversion.
 *	From UTF-8 to EUC.
 */
int
jconvruwput(queue_t *q, mblk_t *mp)
{
	rutf8_state_t	*sp = (rutf8_state_t *)q->q_ptr;
	mblk_t 		*nmp = NULL;
	int		ret_val;

	switch (mp->b_datap->db_type) {
	default:
		putnext(q, mp);
		return (0);
	case M_IOCTL:
		do_ioctl(q, mp);
		return (0);
	case M_DATA:
		if (mp->b_rptr == mp->b_wptr) {	/* May be EOF */
#ifdef DBG_W
			cmn_err(CE_NOTE,
				"rswput():	mp->b_rptr == mp->b_wptr. putnext()");
#endif
			putnext(q, mp);
			return (0);
		}
		break;
	}
	if (sp->do_conv == OFF) {
		putnext(q, mp);
		return (0);
	}
	if ((ret_val = jfp_utf8_to_euc(mp, &nmp, sp->w_utf, &(sp->w_state)))
		!= 0)
		return (ret_val);
	freemsg(mp);
	if (nmp)
		putnext(q, nmp);
	return (0);
}

/*
 * Read side code conversion.
 *	From EUC to UTF-8.
 */
static int
jconvrurput(queue_t *q, mblk_t *mp)
{
	rutf8_state_t *sp = (rutf8_state_t *)q->q_ptr;
	mblk_t 		*nmp = NULL;
	int		ret_val;

	switch (mp->b_datap->db_type) {
	default:
		putnext(q, mp);
		return (0);
	case M_IOCACK:
	{
		struct iocblk *iocp = (struct iocblk *)mp->b_rptr;
		if ((iocp->ioc_cmd == EUC_WSET) &&
			(iocp->ioc_id == sp->fake_ioctl)) {
#ifdef DBG_R
			cmn_err(CE_NOTE, "eucw properly set.");
#endif
			freemsg(mp);
		} else {
			putnext(q, mp);
		}
		return (0);
	}
	case M_IOCNAK:
	{
		struct iocblk *iocp = (struct iocblk *)mp->b_rptr;
		if ((iocp->ioc_cmd == EUC_WSET) &&
			(iocp->ioc_id == sp->fake_ioctl)) {
			cmn_err(CE_WARN, "PCK CSWIDTH setting failed.");
			freemsg(mp);
		} else {
			putnext(q, mp);
		}
		return (0);
	}
	case M_CTL:
	{
		mc_m_ctl_msg_t	*mctlp;

		mctlp = (mc_m_ctl_msg_t *)mp->b_rptr;
		if (mctlp->mc_m_ctl_cmd == MC_PEERQUERY) {
			sp->notify = ON;
			freemsg(mp);
			return (0);
		} else if (mctlp->mc_m_ctl_cmd == MC_DO_CONVERT) {
			freemsg(mp);
			if (sp->do_conv != ON) {
				cmn_err(CE_WARN,
				"jconvru: Confuse conversion status.\n");
				return (-1);
			} else {
				return (0);
			}
		} else if (mctlp->mc_m_ctl_cmd == MC_NO_CONVERT) {
			freemsg(mp);
			if (sp->do_conv != OFF) {
				cmn_err(CE_WARN,
				"jconvru: Confuse conversion status.\n");
				return (-1);
			} else {
				return (0);
			}
		}
		break;
	}
	case M_DATA:
		if (mp->b_rptr == mp->b_wptr) {	/* May be EOF */
#ifdef DBG_R
			cmn_err(CE_NOTE,
				"READ: mp->b_rptr == mp->b_wptr. putnext()");
#endif
			putnext(q, mp);
			return (0);
		}
		break;
	}
	if (sp->do_conv == OFF) {
		putnext(q, mp);
		return (0);
	}
	if ((ret_val = jfp_euc_to_utf8(mp, &nmp , sp->r_euc, &(sp->r_state)))
		!= 0)
		return (ret_val);
	freemsg(mp);
	if (nmp)
		putnext(q, nmp);
	return (0);
}

/*
 * XXX - Usually, modules should issue qreply()
 *	to notify M_IOCACK/M_IOCNAK. We'll use M_CTL
 *	message to inform to lower modules.
 */
static void
do_ioctl(queue_t *q, mblk_t *mp)
{
	register rutf8_state_t	*sp;
	register struct iocblk	*iocp;
	mblk_t			*mcp,
				*nmp = NULL;
	mblk_t			*bp, *nbp;	/* For TIOCSTI mblk */
	unsigned char		*cp;		/* For TIOCSTI mblk */
	mc_m_ctl_msg_t		*mctlp;
	int			ret_val;

	iocp = (struct iocblk *)mp->b_rptr;
	sp = (rutf8_state_t *)q->q_ptr;

	switch (iocp->ioc_cmd) {
	case TIOCSTI:
		ret_val = jfp_utf8_to_euc(mp->b_cont,
			&nmp, sp->ioc_utf, &(sp->ioc_state));
		if (ret_val != 0) {
			/* Memory problem. Just putnext() */
			if (nmp)
				freeb(nmp);
			putnext(q, mp);
		} else {
			freeb(mp->b_cont);
			if (nmp) {
				/*
				 * Because ptem processes only one byte
				 * per ioctl(), we have to split multibyte
				 * chars explicitly. Not sure we can re-use
				 * the same M_IOCTL message.
				 */
#ifdef DBG_W
				cmn_err(CE_NOTE, "ruwput(): TIOCSTI converted");
#endif
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
#ifdef DBG_W
				cmn_err(CE_NOTE, "rswput(): TIOCSTI split. Just ACK");
#endif
				/* Char split. Just send ACK */
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
		sp->intended_off = OFF;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		qreply(q, mp);
		if (sp->notify) {
			mcp = allocb(sizeof (mc_m_ctl_msg_t), BPRI_MED);
			if (mcp == (mblk_t *)NULL) {
				cmn_err(CE_NOTE, "jconvru: No blocks.");
				return;
			}
			mcp->b_datap->db_type = M_CTL;
			mcp->b_wptr = mcp->b_rptr + sizeof (mc_m_ctl_msg_t);
			mctlp = (mc_m_ctl_msg_t *)mcp->b_rptr;
			mctlp->mc_m_ctl_cmd = MC_DO_CONVERT;
			putnext(WR(q), mcp);
		}
		break;
	case EUC_OXLOFF:
		sp->do_conv = OFF;
		sp->intended_off = ON;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		qreply(q, mp);
		if (sp->notify) {
			mcp = allocb(sizeof (mc_m_ctl_msg_t), BPRI_MED);
			if (mcp == (mblk_t *)NULL) {
				cmn_err(CE_NOTE, "jconvru: No blocks.");
				return;
			}
			mcp->b_datap->db_type = M_CTL;
			mcp->b_wptr = mcp->b_rptr + sizeof (mc_m_ctl_msg_t);
			mctlp = (mc_m_ctl_msg_t *)mcp->b_rptr;
			mctlp->mc_m_ctl_cmd = MC_NO_CONVERT;
			putnext(WR(q), mcp);
		}
		break;
	case TCSETS:
	case TCSETSF:
	case TCSETSW:
		sp->t_modes.c_iflag =
			((struct termios *)mp->b_cont->b_rptr)->c_iflag;
		sp->t_modes.c_oflag =
			((struct termios *)mp->b_cont->b_rptr)->c_oflag;
		sp->t_modes.c_cflag =
			((struct termios *)mp->b_cont->b_rptr)->c_cflag;
		sp->t_modes.c_lflag =
			((struct termios *)mp->b_cont->b_rptr)->c_lflag;
		bcopy((char *)
			((struct termios *)mp->b_cont->b_rptr)->c_cc,
			(void*)&(sp->t_modes.c_cc),
			NCCS);
		if (sp->intended_off == OFF) {
			if (check_param(&(sp->t_modes)) == RAW) {
				sp->do_conv = OFF;
				if (sp->notify) {
					mcp = allocb(sizeof (mc_m_ctl_msg_t),
						BPRI_MED);
					if (mcp == (mblk_t *)NULL) {
						cmn_err(CE_NOTE,
							"jconvru: No blocks.");
						return;
					}
					mcp->b_datap->db_type = M_CTL;
					mcp->b_wptr = mcp->b_rptr +
						sizeof (mc_m_ctl_msg_t);
					mctlp = (mc_m_ctl_msg_t *)mcp->b_rptr;
					mctlp->mc_m_ctl_cmd =
						MC_NO_CONVERT;
					putnext(WR(q), mcp);
				}
			} else if (check_param(&(sp->t_modes)) == NOT_RAW) {
				sp->do_conv = ON;
				if (sp->notify) {
					mcp = allocb(sizeof (mc_m_ctl_msg_t),
						BPRI_MED);
					if (mcp == (mblk_t *)NULL) {
						cmn_err(CE_NOTE,
							"jconvru: No blocks.");
						return;
					}
					mcp->b_datap->db_type = M_CTL;
					mcp->b_wptr = mcp->b_rptr +
						sizeof (mc_m_ctl_msg_t);
					mctlp = (mc_m_ctl_msg_t *)mcp->b_rptr;
					mctlp->mc_m_ctl_cmd =
						MC_DO_CONVERT;
					putnext(WR(q), mcp);
				}
			}
		}
		putnext(q, mp);	/* Send TCSETS message */
		break;
	case TCSETA:
	case TCSETAF:
	case TCSETAW:
		sp->t_modes.c_iflag =
		(tcflag_t)((struct termio *)mp->b_cont->b_rptr)->c_iflag;
		sp->t_modes.c_oflag =
		(tcflag_t)((struct termio *)mp->b_cont->b_rptr)->c_oflag;
		sp->t_modes.c_cflag =
		(tcflag_t)((struct termio *)mp->b_cont->b_rptr)->c_cflag;
		sp->t_modes.c_lflag =
		(tcflag_t)((struct termio *)mp->b_cont->b_rptr)->c_lflag;
		bcopy((char *)((struct termio *)mp->b_cont->b_rptr)->c_cc,
		(void*)&(sp->t_modes.c_cc),
		NCC);
		if (sp->intended_off == OFF) {
			if (check_param(&(sp->t_modes)) == RAW) {
				sp->do_conv = OFF;
				if (sp->notify) {
					mcp = allocb(sizeof (mc_m_ctl_msg_t),
						BPRI_MED);
					if (mcp == (mblk_t *)NULL) {
						cmn_err(CE_NOTE,
							"jconvru: No blocks.");
						return;
					}
					mcp->b_datap->db_type = M_CTL;
					mcp->b_wptr = mcp->b_rptr +
						sizeof (mc_m_ctl_msg_t);
					mctlp = (mc_m_ctl_msg_t *)mcp->b_rptr;
					mctlp->mc_m_ctl_cmd =
						MC_NO_CONVERT;
					putnext(WR(q), mcp);
				}
			} else if (check_param(&(sp->t_modes)) == NOT_RAW) {
				sp->do_conv = ON;
				if (sp->notify) {
					mcp = allocb(sizeof (mc_m_ctl_msg_t),
						BPRI_MED);
					if (mcp == (mblk_t *)NULL) {
						cmn_err(CE_NOTE,
							"jconvru: No blocks.");
						return;
					}
					mcp->b_datap->db_type = M_CTL;
					mcp->b_wptr = mcp->b_rptr +
						sizeof (mc_m_ctl_msg_t);
					mctlp = (mc_m_ctl_msg_t *)mcp->b_rptr;
					mctlp->mc_m_ctl_cmd =
						MC_DO_CONVERT;
					putnext(WR(q), mcp);
				}
			}
		}
		putnext(q, mp);	/* Send TCSETA message */
		break;
	default:
		putnext(q, mp);
		break;
	}
}

#define	IFLAGS	(ISTRIP|INPCK|IGNCR|ICRNL|INLCR)
#define	OFLAGS	(OPOST)
#define	CFLAGS	(PARENB|CS8)
#define	LFLAGS	(ISIG|ICANON)

static int
check_param(struct termios *cb)
{
	if (((cb->c_iflag & IFLAGS) == 0) &&
		((cb->c_oflag & OFLAGS) == 0) &&
		((cb->c_cflag & CFLAGS) == CS8) &&
		((cb->c_lflag & LFLAGS) == 0) &&
		(cb->c_cc[VMIN] == 1) &&
		(cb->c_cc[VTIME] == 0)) {
			return (RAW);
	} else if (((cb->c_iflag & IFLAGS) != 0) ||
		((cb->c_oflag & OFLAGS) != 0) ||
		((cb->c_cflag & CFLAGS) != CS8) ||
		((cb->c_lflag & LFLAGS) != 0) ||
		(cb->c_cc[VMIN] != 1) ||
		(cb->c_cc[VTIME] != 0)) {
			return (NOT_RAW);
	}
	return (NONE);
}
