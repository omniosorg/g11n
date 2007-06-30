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
 * Copyright (c) 1996,1997, by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma ident  "@(#)jfp_sjsubr.c 1.8     97/10/08 SMI"

/*
 * Subroutines for PCK(a.k.a., SJIS) handling.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <errno.h>
#include <sys/stream.h>
#include <sys/cmn_err.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>
#include "sjmap.h"

static unsigned short lookuptbl(unsigned short);

extern mblk_t	*jfpallocb(void);
extern int	jfpbappend(mblk_t **, int);

#define	APPEND_CHAR(c) \
	while ((ret_val = jfpbappend(&nbp, (c))) <= 0) { \
		if (ret_val == 0) { \
			if (*nmp == NULL) \
				*nmp = nbp; \
			else \
				linkb(*nmp, nbp); \
			nbp = NULL; \
			continue; \
		} else {	/* allocb() failed. */ \
			return (ENOMEM); \
		} \
	}

#define	WR_GETBYTE(c) \
	c = -1; \
	if (*ucp) { \
		c = *ucp; \
		*ucp = NULL; \
	} else if (bp->b_rptr < bp->b_wptr) { \
		c = *bp->b_rptr++; \
	} else { \
		while (bp->b_cont) { \
			bp = bp->b_cont; \
			if (bp->b_rptr < bp->b_wptr) { \
				c = *bp->b_rptr++; \
				break; \
			} \
		} \
	}

#define	RD_GETBYTE(c) \
	c = -1; \
	if ((*ucap)[0]) { \
		c = (*ucap)[0]; \
		(*ucap)[0] = NULL; \
	} else if ((*ucap)[1]) { \
		c = (*ucap)[1]; \
		(*ucap)[1] = NULL; \
	} else if (bp->b_rptr < bp->b_wptr) { \
		c = *bp->b_rptr++; \
	} else { \
		while (bp->b_cont) { \
			bp = bp->b_cont; \
			if (bp->b_rptr < bp->b_wptr) { \
				c = *bp->b_rptr++; \
				break; \
			} \
		} \
	}

#define	SS2	0x8e
#define	SS3	0x8f

/*
 * lookuptbl()
 * Return the index number if its index-ed number
 * is the same as dest value.
 */
static unsigned short
lookuptbl(unsigned short dest)
{
	unsigned short tmp;
	int i;
	int sz = (sizeof (sjtoibmext) / sizeof (sjtoibmext[0]));

	for (i = 0; i < sz; i++) {
		tmp = sjtoibmext[i];
		if (tmp == dest)
			return ((i + 0xfa40 + ((i / 0xc0) * 0x40)));
	}
	return (0xffff);
}

int
sj2euc(mblk_t *mp, mblk_t **nmp, unsigned char *ucp)
{
	register mblk_t *bp = NULL;
	int c = 0;
	int cc = 0;
	int ret_val = 0;
	mblk_t *nbp = NULL;

	for (bp = mp; ; ) {
		WR_GETBYTE(c);
		if (c == -1) {
			goto end_sj2euc;
		}
		if (ISASC((unsigned char)c)) {
#ifdef DBG_S2E_DAT
			cmn_err(CE_NOTE,
			"S2E:	ASCII. 0x%x", (unsigned char)c);
#endif
			APPEND_CHAR((unsigned char)c);
		} else if (ISSJKANA((unsigned char)c)) {
#ifdef DBG_S2E_DAT
			cmn_err(CE_NOTE,
			"S2E:	KANA. 0x%x 0x%x", SS2, (unsigned char)c);
#endif
			APPEND_CHAR(SS2);
			APPEND_CHAR((unsigned char)c);
		} else if (ISSJKANJI1((unsigned char)c)) {
			WR_GETBYTE(cc);
			if (cc == -1) {
				*ucp = (unsigned char)c;
#ifdef DBG_S2E_DAT
				cmn_err(CE_NOTE,
					"S2E:	CS1. 1st splitted. backup.");
#endif
				goto end_sj2euc;
			}
#ifdef DBG_S2E_DAT
			cmn_err(CE_NOTE,
				"S2E:	CS1. (c, cc)=(0x%x,0x%x)", c, cc);
#endif
			c = sjtojis1[(c - 0x80)];
			if (0x9f <= cc)
				c += 0x01;
			if (ISSJKANJI2(cc)) {
				APPEND_CHAR((unsigned char)(c | 0x80));
				APPEND_CHAR((unsigned char)
					(sjtojis2[cc] | 0x80));
			} else {
				/* No destination code. Put source code */
				APPEND_CHAR((unsigned char)c);
				APPEND_CHAR((unsigned char)cc);
			}
		} else if (ISSJSUPKANJI1((unsigned char)c)) {
			WR_GETBYTE(cc);
			if (cc == -1) {
				*ucp = (unsigned char)c;
#ifdef DBG_S2E_DAT
				cmn_err(CE_NOTE,
				"S2E:	CS3. 1st splitted. backup.");
#endif
				goto end_sj2euc;
			}
#ifdef DBG_S2E_DAT
			cmn_err(CE_NOTE,
				"S2E:	CS3. (c, cc)=(0x%x,0x%x)", c, cc);
#endif
			c = sjtojis1[(c - 0x80)];
			if (0x9f <= cc)
				c += 0x01;
			if (ISSJKANJI2(cc)) {
				APPEND_CHAR(SS3);
				APPEND_CHAR((unsigned char)(c | 0x80));
				APPEND_CHAR((unsigned char)
						(sjtojis2[cc] | 0x80));
			} else {
				/* No destination code. Put source code */
				APPEND_CHAR((unsigned char)c);
				APPEND_CHAR((unsigned char)cc);
			}
		} else if (ISSJIBM((unsigned char)c) || /* IBM */
			ISSJNECIBM((unsigned char)c)) { /* NEC/IBM */
			unsigned short dest;
			/*
			 * We need a special treatment for each codes.
			 * By adding some offset number for them, we
			 * can process them as the same way of that of
			 * extended IBM chars.
			 */
			WR_GETBYTE(cc);
			if (cc == -1) {
				*ucp = (unsigned char)c;
#ifdef DBG_S2E_DAT
				cmn_err(CE_NOTE,
				"S2E:	CS3. 1st splitted. backup.");
#endif
				goto end_sj2euc;
			}
			if (!ISSJKANJI2(cc)) {
				APPEND_CHAR((unsigned char)c);
				APPEND_CHAR((unsigned char)cc);
				continue;
			}
			dest = (c << 8) | cc;
			if ((0xed40 <= dest) && (dest <= 0xeffc)) {
#ifdef DBG_S2E_DAT
				cmn_err(CE_NOTE,
				"S2E:	0x%x. Remap for NEC/IBM", dest);
#endif
				REMAP_NEC(dest);
				if (dest < 0xfa40) {
					APPEND_CHAR((unsigned char)c);
					APPEND_CHAR((unsigned char)cc);
					continue;
				}
			}
			if ((dest == 0xfa54) || (dest == 0xfa5b)) {
#ifdef DBG_S2E_DAT
				cmn_err(CE_NOTE,
				"S2E:	0x%x. IBM chars in JIS208", dest);
#endif
				if (dest == 0xfa54) {
					APPEND_CHAR(0xa2);
					APPEND_CHAR(0xcc);
				} else {
					APPEND_CHAR(0xa2);
					APPEND_CHAR(0xe8);
				}
				continue;
			}
			dest = dest - 0xfa40 - (((dest>>8) - 0xfa) * 0x40);
			dest = sjtoibmext[dest];
#ifdef DBG_S2E_DAT
			cmn_err(CE_NOTE,
			"S2E:	0x%x%x. IBM chars in JIS212", SS3, dest);
#endif
			if (dest == 0xffff) {
				/* No destination code. Put source code */
				APPEND_CHAR(c);
				APPEND_CHAR(cc);
			} else {
				APPEND_CHAR(SS3);
				APPEND_CHAR((dest>>8) & 0xff);
				APPEND_CHAR(dest & 0xff);
			}
		} else if ((0xeb <= c) && (c <= 0xec)) {
			WR_GETBYTE(cc);
			if (cc == -1) {
				*ucp = (unsigned char)c;
#ifdef DBG_S2E_DAT
				cmn_err(CE_NOTE,
				"S2E:	UNDEFINED. 1st splitted. backup.");
#endif
				goto end_sj2euc;
			}
#ifdef DBG_S2E_DAT
			cmn_err(CE_NOTE, "S2E:	UNDEFINED. Put Source code.");
#endif
			APPEND_CHAR((unsigned char)c);
			APPEND_CHAR((unsigned char)cc);
		} else {
			/*
			 * Unknown character. Append anyway.
			 */
#ifdef DBG_S2E_DAT
			cmn_err(CE_NOTE, "S2E:	Unknown");
#endif
			APPEND_CHAR(c);
		}
	}
end_sj2euc:
	if (*nmp == NULL)
		*nmp = nbp;
	else
		linkb(*nmp, nbp);
#ifdef DBG_S2E_LINK
	{
	mblk_t *bpp;
	for (bpp = *nmp; bpp; bpp = bpp->b_cont)
		cmn_err(CE_NOTE,
		"S2E:	bpp=0x%x, bpp->b_rptr=0x%x, bpp->b_wptr=0x%x",
		bpp, bpp->b_rptr, bpp->b_wptr);
	}
#endif
	return (0);
}

int
euc2sj(mblk_t **mp, unsigned char (*ucap)[2])
{
	register mblk_t *bp,	    /* Points current msg block */
			*bpp,
			*nmp;	   /* Holds new msg block */
	unsigned char   *rdp,	   /* Next unread data */
			*wrp,	   /* Next unwritten data */
			*newwrp;	/* New unwritten boundary */
	register unsigned char c1, c2;

	for (bp = *mp; bp; ) {
#ifdef DBG_E2S_LINK
		cmn_err(CE_NOTE, "E2S: bp=0x%x", bp);
#endif
		rdp = bp->b_rptr;
		wrp = bp->b_wptr;
		newwrp = rdp;
		if ((*ucap)[0]) {
			/*
			 * Char is already chopped
			 */
			if (((*ucap)[0] == SS3) && !((*ucap)[1]) &&
				((bp->b_wptr - bp->b_rptr) <= 1)) {
				(*ucap)[1] = *bp->b_rptr++;
				continue;
			} else {
				if ((nmp = jfpallocb()) == (mblk_t *)NULL) {
					cmn_err(CE_WARN,
						"E2S: jfpallocb() failed.");
					return (ENOMEM);
				}
#ifdef DBG_E2S_LINK
				cmn_err(CE_NOTE,
				"E2S: New block is 0x%x", nmp);
#endif
				if ((*ucap)[0] == SS3) {
					if ((*ucap)[1]) {
#ifdef DBG_E2S_DATA
						cmn_err(CE_NOTE,
						"E2S: Data is 0x%x,0x%x,0x%x",
						(*ucap)[0],
						(*ucap)[1],
						*(bp->b_rptr));
#endif
						*(nmp->b_wptr)++ = (*ucap)[0];
						*(nmp->b_wptr)++ = (*ucap)[1];
						*(nmp->b_wptr)++ =
							*(bp->b_rptr)++;
					} else {
#ifdef DBG_E2S_DATA
						cmn_err(CE_NOTE,
						"E2S: Data is 0x%x,0x%x,0x%x",
						(*ucap)[0],
						*(bp->b_rptr),
						*((bp->b_rptr) + 1));
#endif
						*(nmp->b_wptr)++ = (*ucap)[0];
						*(nmp->b_wptr)++ =
							*(bp->b_rptr)++;
						*(nmp->b_wptr)++ =
							*(bp->b_rptr)++;
					}
				} else {
#ifdef DBG_E2S_DATA
					cmn_err(CE_NOTE,
					"E2S: Data is 0x%x,0x%x",
					(*ucap)[0], *(bp->b_rptr));
#endif
					*(nmp->b_wptr)++ = (*ucap)[0];
					*(nmp->b_wptr)++ = *(bp->b_rptr)++;
				}
				(*ucap)[0] = (*ucap)[1] = (unsigned char)NULL;
				if (bp == *mp) {
#ifdef DBG_E2S_LINK
					cmn_err(CE_NOTE,
					"E2S: Prepend New mblk 0x%x, to 0x%x",
					nmp, *mp);
#endif
					nmp->b_cont = *mp;
					nmp->b_next = (*mp)->b_next;
					nmp->b_prev = (*mp)->b_prev;
					*mp = bp = nmp;	/* Set next msg block */
#ifdef DBG_E2S_LINK
					cmn_err(CE_NOTE,
					"E2S: mblk to be processed next = 0x%x",
					bp);
#endif
					continue;
				}
				for (bpp = *mp; bpp; bpp = bpp->b_cont) {
					if (bpp->b_cont == bp) {
#ifdef DBG_E2S_LINK
						cmn_err(CE_NOTE,
						"E2S: Insert New mblk:");
						cmn_err(CE_NOTE,
						"0x%x, 0x%x & 0x%x",
						nmp, bpp, bp);
#endif
						nmp->b_cont = bp;
						bpp->b_cont = nmp;
						bp = nmp;
						break;
					}
				}
			}
#ifdef DBG_E2S_LINK
			cmn_err(CE_NOTE,
			"E2S: mblk to be processed next = 0x%x",
			bp);
#endif
			continue;
		} else {
			/*
			 * No char is splitted.
			 * Parse each mblk.
			 */
			while (rdp < wrp) {
				if (ISASC(*rdp)) {
#ifdef DBG_E2S_DATA
					cmn_err(CE_NOTE,
					"E2S: ASCII. 0x%x", *rdp);
#endif
					*newwrp++ = *rdp++;
					continue;
				} else if ((*rdp) == SS2) {
					if ((rdp + 1) == wrp) {
#ifdef DBG_E2S_DATA
						cmn_err(CE_NOTE,
						"E2S: KANA. 1st split. Backup.");
#endif
						(*ucap)[0] =
							(unsigned char)SS2;
						rdp++;
						continue;
					}
#ifdef DBG_E2S_DATA
					cmn_err(CE_NOTE,
					"E2S: KANA. 0x%x", *rdp);
#endif
					*newwrp++ = *(rdp + 1);
					rdp += 2;
					continue;
				} else if (ISCS1(*rdp)) {
					if ((rdp + 1) == wrp) {
#ifdef DBG_E2S_DATA
						cmn_err(CE_NOTE,
						"E2S: CS1. 1st split. Backup.");
#endif
						(*ucap)[0] =
							(unsigned char)*rdp;
						rdp++;
						continue;
					}
					if (!ISCS1(*(rdp + 1))) {
#ifdef DBG_E2S_DATA
						cmn_err(CE_NOTE,
						"E2S: CS1. Illegal. 0x%x, 0x%x",
						*rdp, *(rdp + 1));
#endif
						*newwrp++ = *rdp;
						*newwrp++ = *(rdp + 1);
						rdp += 2;
						continue;
					}
					c1 = (*rdp & 0x7f);
					c2 = (*(rdp + 1) & 0x7f);
					if ((c1 % 2) == 0)
						c2 += 0x80;
#ifdef DBG_E2S_DATA
					cmn_err(CE_NOTE,
					"E2S: CS1. Legal. 0x%x, 0x%x",
					jis208tosj1[c1],
					jistosj2[c2]);
#endif
					*newwrp++ = jis208tosj1[c1];
					*newwrp++ = jistosj2[c2];
					rdp += 2;
					continue;
				} else if ((*rdp) == SS3) {
					unsigned short dest;
					if ((rdp + 1) == wrp) {
#ifdef DBG_E2S_DATA
						cmn_err(CE_NOTE,
						"E2S: CS3. 1st split. Backup.");
#endif
						(*ucap)[0] =
							(unsigned char)SS3;
						rdp++;
						continue;
					}
					if ((rdp + 2) == wrp) {
#ifdef DBG_E2S_DATA
						cmn_err(CE_NOTE,
						"E2S: CS3. 2nd split. Backup.");
#endif
						(*ucap)[0] =
							(unsigned char)SS3;
						(*ucap)[1] =
							(unsigned char)*(rdp+1);
						rdp += 2;
						continue;
					}
					if (!ISCS3(*(rdp + 1)) ||
						!ISCS3(*(rdp + 2))) {
#ifdef DBG_E2S_DATA
						cmn_err(CE_NOTE,
						"E2S: CS3. Illegal. 0x%x,%x,%x",
						SS3, *(rdp + 1), *(rdp + 2));
#endif
						*newwrp++ = SS3;
						*newwrp++ = *(rdp + 1);
						*newwrp++ = *(rdp + 2);
						rdp += 3;
						continue;
					}
					if (*(rdp + 1) < 0xf5) { /* check IBM */
						dest = (*(rdp + 1) << 8) |
							*(rdp + 2);
						dest = lookuptbl(dest);
						if (dest == 0xffff) {
#ifdef DBG_E2S_DATA
							cmn_err(CE_NOTE,
							"E2S: CS3. Illegal IBM. 0x%x,%x",
							*(rdp + 1), *(rdp + 2));
#endif
							*newwrp++ = ((SJGETA >> 8) & 0xff);
							*newwrp++ = (SJGETA & 0xff);
							rdp += 3;
							continue;
						}
#ifdef DBG_E2S_DATA
						cmn_err(CE_NOTE,
						"E2S: CS3. Legal IBM. 0x%x,%x",
						*(rdp + 1), *(rdp + 2));
#endif
						*newwrp++ = ((dest >> 8) &
								0xff);
						*newwrp++ = (dest & 0xff);
						rdp += 3;
						continue;
					}
					c1 = (*(rdp + 1) & 0x7f);
					c2 = (*(rdp + 2) & 0x7f);
					if ((c1 % 2) == 0)
						c2 += 0x80;
#ifdef DBG_E2S_DATA
					cmn_err(CE_NOTE,
					"E2S: CS3. Legal. 0x%x,%x",
					jis212tosj1[c1],
					jistosj2[c2]);
#endif
					*newwrp++ = jis212tosj1[c1];
					*newwrp++ = jistosj2[c2];
					rdp += 3;
					continue;
				} else {
					/*
					 * Unknown. Anyway, put
					 */
#ifdef DBG_E2S_DATA
					cmn_err(CE_NOTE,
					"E2S: Unknown. 0x%x",
					*rdp);
#endif
					*newwrp++ = *rdp++;
					continue;
				}
			}
#ifdef DBG_E2S_DATA
			cmn_err(CE_NOTE,
			"E2S: New bp->b_wptr'd be 0x%x",
			newwrp);
#endif
			bp->b_wptr = newwrp;
#ifdef DBG_E2S_LINK
			cmn_err(CE_NOTE,
			"E2S: mblk to be processed next = 0x%x",
			bp->b_cont);
#endif
			bp = bp->b_cont;
		}
	}
#ifdef DBG_S2E_LINK
	{
	mblk_t *bpp;
	for (bpp = *mp; bpp; bpp = bpp->b_cont)
		cmn_err(CE_NOTE,
		"E2S: bpp=0x%x, bpp->b_rptr=0x%x, bpp->b_wptr=0x%x",
		bpp, bpp->b_rptr, bpp->b_wptr);
	}
#endif
	return (0);
}
