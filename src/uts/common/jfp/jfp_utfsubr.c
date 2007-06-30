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

#pragma ident  "@(#)jfp_utfsubr.c	1.2	04/09/03 SMI"

/*
 * Subroutines for UTF-8 handling.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/cmn_err.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>
#include <sys/euc.h>

#define	JFP_J2U_STREAMS
#include <jfp_jis_to_ucs2.h>
#define	JFP_U2E_STREAMS
#include <jfp_ucs2_to_euc16.h>

#include "jfp_ucsutf.h"
#include "jfp_euc16.h"

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

#define	UE_GETBYTE(c) \
	c = -1; \
	if (bp->b_rptr < bp->b_wptr) { \
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

#define	EU_GETBYTE(c) 	UE_GETBYTE(c)

static _jfp_euc16_t
jfp_utf8_to_euc16(const unsigned char *utf)
{
	_jfp_ucs2_t	ucs2;

#ifdef DBG_U2E_DAT
	cmn_err(CE_NOTE, "U2E:	utf[] = %02x,%02x,%02x,%02x,%02x,%02x",
		utf[0], utf[1], utf[2], utf[3], utf[4], utf[5], utf[6]);
#endif
	if (ISUTF8_1B(utf[0])) {
		ucs2 = utf[0] & 0x7f;
	} else if (ISUTF8_2B(utf[0])) {
		ucs2 = utf[0] & 0x1f;
		ucs2 <<= 6;
		ucs2 |= utf[1] & 0x3f;
	} else if (ISUTF8_3B(utf[0])) {
		ucs2 = utf[0] & 0x0f;
		ucs2 <<= 6;
		ucs2 |= utf[1] & 0x3f;
		ucs2 <<= 6;
		ucs2 |= utf[2] & 0x3f;
	} else { /* cannot convert to UCS2 */
		return (_EUC16_DEFAULTCHAR);
	}

#ifdef DBG_U2E_DAT
	cmn_err(CE_NOTE, "E2U:	utf -> ucs2 0x%04x", ucs2);
#endif
	return (_jfp_ucs2_to_euc16(ucs2));
}

static _jfp_ucs2_t
jfp_euc_to_ucs2(const unsigned char *euc)
{
	_jfp_ucs2_t		ucs2;

#ifdef	DBG_E2U_DAT
	cmn_err(CE_NOTE, "E2U:	euc[] = %02x,%02x,%02x",
		euc[0], euc[1], euc[2]);
#endif

	if (ISEUC1FIRST(euc[0])) {
		ucs2 = _jfp_tbl_jisx0208_to_ucs2[((euc[0] & 0x7f) - 0x21) * 94 +
						(euc[1] & 0x7f) - 0x21];
	} else if (ISEUC2FIRST(euc[0])) {
		ucs2 = _jfp_tbl_jisx0201kana_to_ucs2[euc[1] - 0xa1];
	} else /* if (ISEUC3FIRST(euc[0])) */ {
		ucs2 = _jfp_tbl_jisx0212_to_ucs2[((euc[1] & 0x7f) - 0x21) * 94 +
						(euc[2] & 0x7f) - 0x21];
	}

#ifdef	DBG_E2U_DAT
	cmn_err(CE_NOTE, "E2U:	-> \\u0x%04x", ucs2);
#endif

	return (ucs2);
}

int
jfp_utf8_to_euc(
	mblk_t	*mp,
	mblk_t	**nmp,
	unsigned char *utf,
	int	*state
)
{
	register mblk_t	*bp = NULL;
	int 		c = 0;
	int		ret_val	= 0;
	mblk_t 		*nbp = NULL;

	for (bp = mp; ; ) {
		UE_GETBYTE(c);
		if (c == -1)
			break;
		if (*state == 0) {
first:
			if (ISASCII(c)) {
				APPEND_CHAR((unsigned char)c);
				*state = 0;
			} else if (ISUTF8_MBFIRST(c)) {
				utf[(*state)++] = (unsigned char)c;
			} else { /* corrupted */
				APPEND_CHAR((unsigned char)c);
				*state = 0;
			}
		} else {
			if (ISUTF8_SUBSEQ(c)) {
				utf[(*state)++] = (unsigned char)c;
				if (UTF8_LEN(utf[0]) == *state) {
					_jfp_euc16_t 	e;
					e = jfp_utf8_to_euc16(utf);
#ifdef DBG_U2E_DAT
cmn_err(CE_NOTE, "U2E: euc16 0x%04x", e);
#endif
					if (ISEUC16_CS0(e)) {
						APPEND_CHAR(EUC16_CS0_GET1(e));
					} else if (ISEUC16_CS1(e)) {
						APPEND_CHAR(EUC16_CS1_GET1(e));
#ifdef DBG_U2E_DAT
cmn_err(CE_NOTE, "U2E: append 0x%02x", EUC16_CS1_GET1(e));
#endif
						APPEND_CHAR(EUC16_CS1_GET2(e));
#ifdef DBG_U2E_DAT
cmn_err(CE_NOTE, "U2E: append 0x%02x", EUC16_CS1_GET2(e));
#endif
					} else if (ISEUC16_CS2(e)) {
						APPEND_CHAR(SS2);
						APPEND_CHAR(EUC16_CS2_GET1(e));
					} else if (ISEUC16_CS3(e)) {
						APPEND_CHAR(SS3);
						APPEND_CHAR(EUC16_CS3_GET1(e));
						APPEND_CHAR(EUC16_CS3_GET2(e));
					} else {
						/* EMPTY */
					}
					*state = 0;
				}
			} else { /* corrupted */
				int	i;
				for (i = 0; i < *state - 1; i++) {
					APPEND_CHAR((int)utf[i]);
				}
				*state = 0;
				goto first;
			}
		}
	}

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
jfp_euc_to_utf8(
	mblk_t	*mp,
	mblk_t	**nmp,
	unsigned char *euc,
	int	*state
)
{
	register mblk_t	*bp = NULL;
	int 		c = 0;
	int		ret_val	= 0;
	mblk_t 		*nbp = NULL;

	for (bp = mp; ; ) {
		EU_GETBYTE(c);
		if (c == -1)
			break;
		if (*state == 0) {
first:
			if (ISASCII(c)) {
				APPEND_CHAR((unsigned char)c);
				*state = 0;
			} else if (ISEUC_MBFIRST(c)) {
				euc[(*state)++] = (unsigned char)c;
			} else { /* corrupted */
				APPEND_CHAR((unsigned char)c);
				*state = 0;
			}
		} else {
			if (ISEUC_SUBSEQ(c)) {
				euc[(*state)++] = (unsigned char)c;
				if (EUCLEN(euc[0]) == *state) {
					_jfp_ucs2_t	ucs2;
					ucs2 = jfp_euc_to_ucs2(euc);
					if (ISUCS2_1B(ucs2)) {
						APPEND_CHAR(UCS2_1B_GET1(ucs2));
					} else if (ISUCS2_2B(ucs2)) {
						APPEND_CHAR(UCS2_2B_GET1(ucs2));
						APPEND_CHAR(UCS2_2B_GET2(ucs2));
					} else if (ISUCS2_3B(ucs2)) {
						APPEND_CHAR(UCS2_3B_GET1(ucs2));
						APPEND_CHAR(UCS2_3B_GET2(ucs2));
						APPEND_CHAR(UCS2_3B_GET3(ucs2));
					} else {
						/* EMPTY */
					}
					*state = 0;
				}
			} else { /* corrupted */
				int	i;
				for (i = 0; i < *state - 1; i++) {
					APPEND_CHAR((int)euc[i]);
				}
				*state = 0;
				goto first;
			}
		}
	}

	if (*nmp == NULL)
		*nmp = nbp;
	else
		linkb(*nmp, nbp);

	return (0);
}
