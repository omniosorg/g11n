/*
 * Copyright (c) 1996,1999 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma	ident	"@(#)u8sb.c	1.1 99/03/05 SMI"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <sys/errno.h>
#include <sys/termio.h>
#include <sys/conf.h>
#include <sys/modctl.h>
#include <sys/stropts.h>
#include <sys/ddi.h>
#include "common.h"


#if defined(ISO_8859_1)

#include "iso-8859-1_to_utf8.h"
#include "utf8_to_iso-8859-1.h"

#define MODNAME		"u8lat1"
#define MODID		122
#define MODCOMMENT	"UTF-8 <-> ISO 8859-1 code conversion"

#elif defined(ISO_8859_2)

#include "iso-8859-2_to_utf8.h"
#include "utf8_to_iso-8859-2.h"

#define MODNAME		"u8lat2"
#define MODID		123
#define MODCOMMENT	"UTF-8 <-> ISO 8859-2 code conversion"

#elif defined(KOI8_R)

#include "koi8-r_to_utf8.h"
#include "utf8_to_koi8-r.h"

#define MODNAME		"u8koi8"
#define MODID		124
#define MODCOMMENT	"UTF-8 <-> koi8-r code conversion"

#endif


extern struct streamtab u8sbinfo;
extern void k_bappend(mblk_t**, mblk_t**, unsigned char);


/*
 * Per queue instances are single-threaded since the q_ptr
 * field of queues need to be shared among threads.
 */
static struct fmodsw fsw = {
	MODNAME,
	&u8sbinfo,
	D_NEW | D_MP | D_MTPERMOD
};

/*
 * Module linkage information for the kernel.
 */

extern struct mod_ops mod_strmodops;

static struct modlstrmod modlstrmod = {
	&mod_strmodops,
	MODCOMMENT,
	&fsw
};

static struct modlinkage modlinkage = {
	MODREV_1, (void*)&modlstrmod, NULL
};


_init()
{
	return(mod_install(&modlinkage));
}


_fini()
{
	return(mod_remove(&modlinkage));
}


_info(struct modinfo* modinfop)
{
	return(mod_info(&modlinkage, modinfop));
}


static int u8sbopen(queue_t* q, dev_t dev, int oflag, int sflag, cred_t* cr);
static int u8sbclose(queue_t* q, int flag, cred_t* cr);
static int u8sbrput(queue_t* q, mblk_t* mp);
static int u8sbwput(queue_t* q, mblk_t* mp);


static struct module_info u8sbmiinfo = {
	MODID,
	MODNAME,
	0,
	INFPSZ,
	128,
	64
};

static struct qinit u8sbrinit = {
	u8sbrput,
	NULL,
	u8sbopen,
	u8sbclose,
	NULL,
	&u8sbmiinfo,
	NULL
};

static struct module_info u8sbmoinfo = {
	MODID,	
	MODNAME,
	0,
	INFPSZ,
	300,
	0
};

static struct qinit u8sbwinit = {
	u8sbwput,
	NULL,
	NULL,
	NULL,
	NULL,
	&u8sbmoinfo,
	NULL
};

struct streamtab u8sbinfo = {
	&u8sbrinit,
	&u8sbwinit,
	NULL,
	NULL
};


/*
 * Input/Output status
 */
#define FIRST	1	/* The first character (could be an ASCII char.) */
#define SECOND	2	/* The second character composed. */
#define THIRD	3	/* The third character composed. */
#define FOURTH	4	/* The fourth character composed. */
#define FIFTH	5	/* The fifth character composed. */
#define SIXTH	6	/* The sixth character composed. */


typedef struct {
	mblk_t*		k_savbp;	/* pointer to start of message block */

	int		w_state;	/* write side */
	unsigned char	w_compose[6];
} u8sb_state_t;


static int
u8sbopen(queue_t* q, dev_t dev, int flag, int sflag, cred_t* cr)
{
	register u8sb_state_t* k_state;
	register mblk_t* bp;

	if (q->q_ptr != NULL)
		return (0);	/* already attached */

	if ((bp = (mblk_t*)allocb(sizeof(u8sb_state_t), BPRI_MED)) == NULL) {
		printf("u8sbopen: can't allocate state structure\n");
		return (OPENFAIL);
	}
	bp->b_wptr += sizeof(u8sb_state_t);
	k_state = (u8sb_state_t*)bp->b_rptr;
	k_state->k_savbp = bp;

	k_state->w_state = FIRST;

	q->q_ptr = (caddr_t)k_state;
	WR(q)->q_ptr = (caddr_t)k_state;
	qprocson(q);
	return (0);
}


static int
u8sbclose(queue_t* q, int flag, cred_t* cr)
{
	register u8sb_state_t* k_state = (u8sb_state_t*)q->q_ptr;

	qprocsoff(q);
	freeb(k_state->k_savbp);
	q->q_ptr = NULL;
}


/*
 * Read side code conversion - from EUC to UTF-8
 */

static int
u8sbrput(queue_t* q, mblk_t* mp)
{
	register u8sb_state_t* k_state = (u8sb_state_t*)q->q_ptr;
	register mblk_t* bp;
	register unsigned char c;
	mblk_t* nbp;
	mblk_t* nmp;
	unsigned int u8;
	int i;
	int sz;
	unsigned int ret_val;

	if (mp->b_datap->db_type == M_FLUSH) {
		if (*mp->b_rptr & FLUSHR)
			flushq(RD(q), FLUSHDATA);
		putnext(q, mp);
		return;
	}

	if (mp->b_datap->db_type != M_DATA) {
		putnext(q, mp);
		return;
	}

	nbp = nmp = NULL;
	for (bp = mp; bp != NULL; bp = bp->b_cont) {
		while (bp->b_rptr < bp->b_wptr) {
			c = *bp->b_rptr;
			sz = sb_u8_tbl[c].size;
			u8 = sb_u8_tbl[c].u8;
			for (i = 1; i < sz; i++)
				k_bappend(&nmp, &nbp,
					(uchar_t)((u8 >> ((sz - i) * 8))&0xff));
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
}


/*
 * Write side code conversion - from UTF-8 to EUC
 */

static int
u8sbwput(queue_t* q, mblk_t* mp)
{
	register u8sb_state_t* k_state = (u8sb_state_t*)q->q_ptr;
	register mblk_t* bp;
	register unsigned char c;
	mblk_t* nbp;
	mblk_t* nmp;
	void k_next_state(u8sb_state_t*, mblk_t**, mblk_t**, unsigned char);

	if (mp->b_datap->db_type == M_FLUSH) {
		if (*mp->b_rptr & FLUSHW)
			flushq(WR(q), FLUSHDATA);
		putnext(q, mp);
		return;
	}

	if (mp->b_datap->db_type != M_DATA) {
		putnext(q, mp);
		return;
	}

	nbp = nmp = (mblk_t*)NULL;
	for (bp = mp; bp != (mblk_t*)NULL; bp = bp->b_cont) {
		while (bp->b_rptr < bp->b_wptr) {
			if ((c = *bp->b_rptr) & 0x80)
				k_next_state(k_state, &nbp, &nmp, c);
			else
				k_bappend(&nmp, &nbp, c);
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
}


/*
 * Following function should be enhanced and optimized.
 */

void
k_next_state(u8sb_state_t* k_state, mblk_t** nbp, mblk_t** nmp,
		unsigned char c)
{
	unsigned int wc;
	int ret_val = 0;
	unsigned char k_find_single_byte_char(unsigned int);

	if (k_state->w_state == FIRST) {
		k_state->w_compose[0] = c;
		k_state->w_state = SECOND;
	} else if (k_state->w_state == SECOND) {
		if (K_UTF_2_BYTE(k_state->w_compose[0])) {
			wc = k_state->w_compose[0];
			wc = (wc << 8) | (unsigned int)c;

			c = k_find_single_byte_char(wc);
			k_bappend(nmp, nbp, c);
			k_state->w_state = FIRST;
		} else {
			k_state->w_compose[1] = c;
			k_state->w_state = THIRD;
		}
	} else if (k_state->w_state == THIRD) {
		if (K_UTF_3_BYTE(k_state->w_compose[0])) {
			wc = k_state->w_compose[0];
			wc = (wc << 8) | (unsigned char)k_state->w_compose[1];
			wc = (wc << 8) | (unsigned char)c;

			c = k_find_single_byte_char(wc);
			k_bappend(nmp, nbp, c);
			k_state->w_state = FIRST;
		} else {
			k_state->w_compose[2] = c;
			k_state->w_state = FOURTH;
		}
	} else if (k_state->w_state == FOURTH) {
		k_state->w_compose[3] = c;

		if (K_UTF_4_BYTE(k_state->w_compose[0]))
			goto _non_identical;
		k_state->w_state = FIFTH;
	} else if (k_state->w_state == FIFTH) {
		k_state->w_compose[4] = c;

		if (K_UTF_5_BYTE(k_state->w_compose[0]))
			goto _non_identical;
		k_state->w_state = SIXTH;
	} else {
		k_state->w_compose[5] = c;
		goto _non_identical;
	}

	return;

_non_identical:
	k_bappend(nmp, nbp, NON_IDENTICAL);
	k_state->w_state = FIRST;

	return;
}


unsigned char
k_find_single_byte_char(unsigned int u8)
{
	int i, l, h;

	i = l = 0;
	h = (sizeof(u8_sb_tbl) / sizeof(to_sb_table_component_t)) - 1;
	while (l <= h) {
		i = (l + h) / 2;
		if (u8_sb_tbl[i].u8 == u8)
			break;
		else if (u8_sb_tbl[i].u8 < u8)
			l = i + 1;
		else
			h = i - 1;
	}

	return (u8_sb_tbl[i].u8 == u8) ? u8_sb_tbl[i].sb : NON_IDENTICAL;
}


/*
 * Append a character to a message block.
 *
 * If (*bpp) is null, it will allocate a new block.
 * When the message block is full link it to the message
 * 1 otherwise.
 */

#define MODBLKSZ	16		/* size of message blocks */

void
k_bappend(mblk_t** mpp, mblk_t** bpp, unsigned char ch)
{
	mblk_t	*bp;

	if (bp = *bpp) {
		if (bp->b_wptr >= bp->b_datap->db_lim) {
			if (*mpp == NULL)
				*mpp = *bpp;
			else
				linkb(*mpp, *bpp);
			*bpp = NULL;
			k_bappend(mpp, bpp, ch);
			return;
		}
	} else
		while ((*bpp = bp = (mblk_t*)allocb(MODBLKSZ, BPRI_MED)) ==
		    NULL)
			delay(10);
	*bp->b_wptr++ = (unsigned char)ch;
}
