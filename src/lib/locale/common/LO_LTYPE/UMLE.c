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
 * COPYRIGHT AND PERMISSION NOTICE
 *
 * Copyright (c) 1991-2005 Unicode, Inc. All rights reserved. Distributed
 *  under the Terms of Use in http://www.unicode.org/copyright.html.
 *
 * This file has been modified by Sun Microsystems, Inc.
 *
*/
/*
 * Copyright 1999-2002 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
#pragma ident "@(#)UMLE.c 1.142	04/09/02 SMI"


#include <stdio.h>
#include <wchar.h>
#include <sys/isa_defs.h>
#include <errno.h> 
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <langinfo.h>
#include <sys/layout.h>
#include "layout_int.h"
#include "biditbl.h"

/* Added by SACHIN for debugging */
#include <fcntl.h>
#include <unistd.h>

/*
 * MB_LEN_MAX is 5 right now. Until we have bigger MB_LEN_MAX, we use
 * following for UTF-8 locale.
 */
#define UTF_8_LEN_MAX           	6

#define UTF_8_ONE(u)            	((u) < 0x80)
#define UTF_8_MORE_THAN_ONE(u)  	((u) >= 0xc0)
#define UTF_8_TWO(u)            	(((u) & 0xe0) == 0xc0)
#define UTF_8_THREE(u)          	(((u) & 0xf0) == 0xe0)
#define UTF_8_FOUR(u)           	(((u) & 0xf8) == 0xf0)
#define UTF_8_FIVE(u)           	(((u) & 0xfc) == 0xf8)
#define UTF_8_SIX(u)            	(((u) & 0xfe) == 0xfc)

/*
 * Following "6" and "0x3f" came from 10xx xxxx bit representation of UTF-8
 * characters' byte.
 */
#define BIT_SHIFT               	6
#define BIT_MASK                	0x3f

#define UCHAR                   	unsigned char
#define	UMLE_CACHE_SIZE			1024
#define	NUMLEVELS			16

/* Common Non Defined Class */
#define _ND				0
/*
 * Thai character classes
 */
#define _NC				1
#define _UC				(1<<1)
#define _BC				(1<<2)
#define _SC				(1<<3)
#define _AV				(1<<4)
#define _BV				(1<<5)
#define _TN				(1<<6)
#define _AD				(1<<7)
#define _BD				(1<<8)
#define _AM				(1<<9)

#define NoTailCons			_NC
#define UpTailCons			_UC
#define BotTailCons			_BC
#define SpltTailCons		_SC
#define Cons				(NoTailCons|UpTailCons|BotTailCons|SpltTailCons)
#define AboveVowel			_AV
#define BelowVowel			_BV
#define Tone				_TN
#define AboveDiac			_AD
#define BelowDiac			_BD
#define SaraAm				_AM

/*
 * Arabic character classes
 */
#define	_4F				1
#define	_2F				(1<<1)
#define	_SD				(1<<2)
#define	_VW				(1<<3)
#define	_LM				(1<<4)
#define	_AL				(1<<5)

/*
 * Hebrew character classes
 */
#define _CNDA				1L
#define	_ALEF				(1L<<1)
#define _VAV				(1L<<2)
#define _YOD				(1L<<3)
#define	_LAM				(1L<<4)
#define	_SHIN_CN			(1L<<5)
#define	_DOUB_YOD			(1L<<6)
#define	_DAGESH				(1L<<7)
#define	_SHIN_DOT			(1L<<8)
#define	_SIN_DOT			(1L<<9)
#define	_HOLAM				(1L<<10)
#define	_HIRIQ				(1L<<11)
#define	_PATAH				(1L<<12)
#define	_POINTS				(1L<<13)
#define	_MARKS				(1L<<14)

/* Sub Classes */
#define	_BET				(1L<<15)
#define	_KAF				(1L<<16)
#define	_PE				(1L<<17)

#define	_RAFE				(1L<<18)

#define	_QAMATS				(1L<<19)

#define	_NarrowCons			(1L<<20)
#define	_WideCons			(1L<<21)

#define	_WC  _WideCons
#define	_NRC _NarrowCons
#define	_NS  (_DAGESH|_SHIN_DOT|_SIN_DOT|_HOLAM|_HIRIQ|_PATAH|_POINTS|_MARKS|_RAFE|_QAMATS)


/*
 * Devanagari character classes
 */
#define	_NP				1L
#define _UP				(1L<<1)
#define _IV				(1L<<2)
#define _CN				(1L<<3)
#define _CK				(1L<<4)
#define _RC				(1L<<5)
#define _NM				(1L<<6)
#define _IM				(1L<<7)
#define _HL				(1L<<8)
#define _NK				(1L<<9)
#define _VD				(1L<<10)
#define _HD				(1L<<11)
#define	_II_M				(1L<<12)
#define	_EY_M				(1L<<13)
#define	_AI_M				(1L<<14)
#define	_OW1_M				(1L<<15)
#define	_OW2_M				(1L<<16)
#define	_MS				(1L<<17)
#define _AYE_M				(1L<<18)
#define _EE_M				(1L<<19)
#define _AWE_M				(1L<<20)
#define _O_M				(1L<<21)
#define _RM				(_II_M|_EY_M|_AI_M|_OW1_M|_OW2_M|_AYE_M|_EE_M|_AWE_M|_O_M)

/*
 * Tamil character classes
 */

#define	_VM				1L		// Vowel Modifier 
#define _TMV			(1L<<1)		// Vowel 
#define _TMC			(1L<<2)		// Consonant
#define _TNM			(1L<<3)		// Normal Matra 
#define _TRM			(1L<<4)		// Repositioned matra
#define _SM				(1L<<5)		// Split matra  Tamil 
#define _PM				(1L<<6)		// Part of a matra
#define _TMH			(1L<<7)		// Halant 
#define _TMD			(1L<<8)		// Digits 


/*
 * Kannada character classes
 */

#define	_N_VM				1
#define _V					(1<<1)	/* Vowel */
#define _N_CONS				(1<<2)
#define _NUK_CONS			(1<<3)
#define _N_M				(1<<4)	/* Normal Matra */
#define _HAL				(1<<5)
#define _NUK				(1<<6)
#define _VED				(1<<7)
#define _DIG				(1<<8)

/*
 * Gujarati character classes
 */

#define	_GJ_N_VM			1
#define _GJ_SP_VM			(1<<1)
#define _GJ_V				(1<<2)	
#define _GJ_N_CONS			(1<<3)
#define _GJ_RA_CONS			(1<<4)
#define _GJ_N_M				(1<<5)
#define _GJ_R_M				(1<<6)
#define _GJ_I_M				(1<<7)
#define _GJ_II_M			(1<<8)
#define _GJ_HAL				(1<<9)
#define _GJ_NUK				(1<<10)
#define _GJ_VED				(1<<11)
#define _GJ_DIG				(1<<12)
#define _GJ_EY_M			(1L<<13)
#define _GJ_AI_M			(1L<<14)
#define _GJ_OW1_M			(1L<<15)
#define _GJ_OW2_M			(1L<<16)
#define _GJ_MS				(1L<<17)
#define _GJ_AYE_M			(1L<<18)
#define _GJ_AWE_M			(1L<<19)
#define _GJ_RM				(_GJ_II_M|_GJ_EY_M|_GJ_AI_M|_GJ_OW1_M|_GJ_OW2_M|_GJ_AYE_M|_GJ_AWE_M)

/*
 * Bengali character classes
 */

#define	_BN_N_VM	1
#define _BN_SP_VM	(1L<<1)
#define _BN_V		(1L<<2)		/* Vowel */
#define _BN_N_CONS	(1L<<3)
#define _BN_NUK_CONS	(1L<<4)
#define _BN_RA_CONS	(1L<<5)
#define _BN_N_M		(1L<<6)		/* Normal Matra */
#define _BN_I_M		(1L<<7)		/* Choti I Matra */
#define _BN_II_M	(1L<<8)		/*  II Matra */
#define _BN_HAL		(1L<<9)
#define _BN_NUK		(1L<<10)
#define _BN_MISC	(1L<<11)
#define _BN_DIG		(1L<<12)
#define _BN_EY_M	(1L<<13)
#define _BN_AI_M	(1L<<14)
#define _BN_OW1_M	(1L<<15)
#define _BN_OW2_M	(1L<<16)


/*
 * Telugu character classes
 */

#define	_TLVM					1			// Vowel modifiers
#define _TLV					(1<<1)		// Vowel
#define _TLC					(1<<2)		// Consonants
#define _TLM					(1<<3)		// Normal Matra 
#define _TLH					(1<<4)		// Halant
#define _TLD					(1<<5)		// Digits
#define _TLMISC					(1<<6)		// Misc

/*
 * Malayalam character classes
 */
/* C-DAC FILLING Here */

#define	_M_VM		1
#define	_M_V		(1<<1)
#define	_M_C		(1<<2)
#define	_M_M		(1<<3)
#define	_M_H		(1<<4)
#define	_M_DIG		(1<<5)
#define	_M_MISC		(1<<6)
#define _M_RM		(1<<7)
#define _M_SM		(1<<8)


/*
 * Punjabi character classes
 *
 * C-DAC FILLING Here 
 */

#define _PVM		1
#define _PV			(1<<1)
#define _PC			(1<<2)
#define _PNC		(1<<3)
#define	_PN			(1<<4)
#define _PNM		(1<<5)
#define _P_IM		(1<<6)
#define _PH			(1<<7)
#define _PDIG		(1<<8)
#define _PMISC		(1<<9)

/*
 * Thai character types
 */
#define __CTRL				0
#define __NON				1
#define __CONS				2
#define __LV				3
#define __FV1				4
#define __FV2				5
#define __FV3				6
#define __BV1				7
#define __BV2				8
#define __BD				9
#define __TONE				10
#define __AD1				11
#define __AD2				12
#define __AD3				13
#define __AV1				14
#define __AV2				15
#define __AV3				16

/* Common Non-defined type */
#define	__ND				0
/*
 * Arabic character types
 */
#define	__4F				1
#define	__2F				2
#define	__SD				3
#define	__VW				4
#define	__LM				5
#define	__AL				6

/*
 * Hebrew character types
 */
#define __SP				1
#define	__ALEF				2
#define	__LAM				3
#define __NS				4
#define __DA				5

/*
 * Devanagari character types
 */
#define __UP				1
#define	__NP                2
#define __IV				3
#define __CN				4
#define __CK				5
#define __RC				6
#define __NM				7
#define __RM				8
#define __IM				9
#define __HL				10
#define __NK				11
#define __VD				12
#define __HD				13

/*
 * Tamil Character types
 */


#define __VM				1	// Vowel Modifier
#define __TMV				2	// Vowel
#define __TMC				3	// Consonant
#define __TMM				4	// Normal Matra
#define __TMR				5	// Repositioned Matra
#define __TMS				6	// Split Matra
#define __TMH				7	// Halant
#define __TMD				8	// Digit
#define __TMP				9	// Part of Matra


 
 /*
 * Kannada character types
 */
#define __VM				1
#define __V					2
#define __C					3
#define __NC				4
#define __M					5
#define __H					6
#define __N					7
#define __KVD				8
#define __D					9

/*
 * Gujarati character types
 */
#define __GJ_VM				1
#define __GJ_V				2
#define __GJ_C				3
#define __GJ_RC				4
#define __GJ_M				5
#define __GJ_IM				6
#define __GJ_H				7
#define __GJ_N				8
#define __GJ_VD				9
#define __GJ_D				10

/*
 * Bengali character types
 */
#define __BN_VM		1
#define __BN_V		2
#define __BN_C		3
#define __BN_RC		4
#define __BN_M		5
#define __BN_IM		6
#define __BN_H		7
#define __BN_N		8
#define __BN_MSC	9
#define __BN_D		10

/*
 * Telugu character types
 */


#define __TLVM				1
#define __TLV				2
#define __TLC				3
#define __TLM				4
#define __TLH				5
#define __TLD				6
#define __TLMISC			7


/*
 * Malayalam character types
 */
/* C-DAC FILLING Here */

#define __MLVM		1
#define __MLV		2
#define __MLC		3
#define __MLM		4
#define __MLH		5
#define __MLD		6
#define __MLMIS		7


/*
 * Punjabi character types
 */
/* C-DAC FILLING Here */

#define __PVM		1
#define __PV		2
#define __PC		3
#define __PNC		4
#define __PN		5
#define __PM		6
#define __PIM		7
#define __PH		8
#define __PD		9
#define __PMIS		10


/*
 * Devanagari Glyphing Type State
 */
#define	St0		0
#define	St1		1
#define	St2		2
#define	St3		3
#define	St4		4
#define	St5		5
#define	St6		6
#define	St7		7
#define	St8		8
#define	St9		9
#define	St10	10
#define	St11	11
#define	St12	12
#define	St13	13
#define	St14	14
#define	St15	15
#define	St16	16
#define	St17	17
#define	St18	18
#define	St19	19
#define	St20	20

#define	MIN(a, b)			((a) <= (b) ? (a) : (b))
#define	MAX_CORE_CONS_BUF		8

#define	MAP_SIZE				243
#define	MAX_STATE				21
#define	MAX_DEVA_TYPE			14

#define MAP_TAMIL_SIZE			174
#define MAX_TAMIL_STATE			10		
#define MAX_TAMIL_TYPE			10		

#define	MAP_KANN_SIZE			415
#define	MAX_KANN_STATE			14
#define	MAX_KANN_TYPE			10

#define	MAP_GUJA_SIZE			201
#define	MAX_GUJA_STATE			21
#define	MAX_GUJA_TYPE			11

#define	MAP_BENG_SIZE			338
#define	MAX_BENG_STATE			21
#define	MAX_BENG_TYPE			11


#define	MAP_TLG_SIZE			700		// TELUGU
#define	MAX_TLG_STATE			13
#define	MAX_TLG_TYPE			8

#define	MAP_ML_SIZE			146 //166
#define	MAX_ML_STATE		10
#define	MAX_ML_TYPE			8
#define SINGLE_CONSONANT_SIGN 1
#define DOUBLE_CONSONANT_SIGN 2


#define	MAP_PN_SIZE			168
#define	MAX_PN_STATE		12
#define	MAX_PN_TYPE			11

#define UMLEmalloc(size)		malloc(((size) > 0 ? (size) : 1))
#define UMLErealloc(ptr, size)		realloc((ptr), ((size) > 0 ? (size) : 1))

#define	UMLE_INFO(obj)			((UMLEInfo)((obj)->private_data))

#define	CURINFO(obj)			(UMLE_INFO(obj)->CurInfo)
#define	LOGICALSTRMTYPE(obj)		(CURINFO(obj)->LogicalStrmType)
#define	NUM_UNIT(obj)			(CURINFO(obj)->num_unit)
#define	OUTSIZE(obj)			(CURINFO(obj)->outsize)
#define	INPMBCZ(obj)			(CURINFO(obj)->inpmb_cz)
#define	INPWCCZ(obj)			(CURINFO(obj)->inpwc_cz)
#define	PROPCZ(obj)			(CURINFO(obj)->prop_cz)
#define	I2OCZ(obj)			(CURINFO(obj)->i2o_cz)
#define	O2ICZ(obj)			(CURINFO(obj)->o2i_cz)
#define	OUTCZ(obj)			(CURINFO(obj)->out_cz)
#define	UNITCZ(obj)			(CURINFO(obj)->unit_cz)
#define	MAPPING_V2L(obj)		(CURINFO(obj)->v2l)
#define	MAPPING_V2L_CACHE(obj)		(CURINFO(obj)->v2l_cache)
#define MAPPING_V2L_AT(obj, i)		(CURINFO(obj)->v2l[(i)])
#define	MAPPING_L2V(obj)		(CURINFO(obj)->l2v)
#define	MAPPING_L2V_CACHE(obj)		(CURINFO(obj)->l2v_cache)
#define MAPPING_L2V_AT(obj, i)		(CURINFO(obj)->l2v[(i)])
#define	VISUAL_EMBEDDINGS(obj)		(CURINFO(obj)->visual_blocks)
#define	VISUAL_EMBEDDINGS_CACHE(obj)	(CURINFO(obj)->visual_blocks_cache)
#define	VISUAL_EMBEDDINGS_AT(obj, i)	((CURINFO(obj)->visual_blocks)[(i)])
#define	UNIT_TO_MB_IDX(obj)		(CURINFO(obj)->unit_to_mb_idx)
#define	UNIT_TO_MB_IDX_CACHE(obj)	(CURINFO(obj)->unit_to_mb_idx_cache)
#define	UNIT_TO_MB_IDX_AT(obj, i)	((CURINFO(obj)->unit_to_mb_idx)[(i)])
#define	CURSHPGRP_IDX(obj)		(CURINFO(obj)->CurShpGrpIdx)
#define	MBINPBUF(obj)			(CURINFO(obj)->mbInpBuf)
#define	MBINPBUF_AT(obj, i)		(CURINFO(obj)->mbInpBuf[(i)])
#define	MBINPBUF_CACHE(obj)		(CURINFO(obj)->mbInpBuf_cache)
#define	WCINPBUF(obj)			(CURINFO(obj)->wcInpBuf)
#define	WCINPBUF_AT(obj, i)		(CURINFO(obj)->wcInpBuf[(i)])
#define	WCINPBUF_CACHE(obj)		(CURINFO(obj)->wcInpBuf_cache)

#define	WCTLGINPBUF(obj)		(CURINFO(obj)->wcTlgInpBuf)
#define	WCTLGINPBUF_AT(obj, i)	(CURINFO(obj)->wcTlgInpBuf[(i)])
#define	WCTLGINPBUF_CACHE(obj)	(CURINFO(obj)->wcTlgInpBuf_cache)

#define	LOGICAL_EMBEDDINGS(obj)		(CURINFO(obj)->embeddings)
#define	LOGICAL_EMBEDDINGS_AT(obj, i)	(CURINFO(obj)->embeddings[(i)])
#define	LOGICAL_EMBEDDINGS_CACHE(obj)	(CURINFO(obj)->embeddings_cache)
#define	DIRS(obj)			(CURINFO(obj)->dirs)
#define	DIRS_AT(obj, i)			(CURINFO(obj)->dirs[(i)])
#define	DIRS_CACHE(obj)			(CURINFO(obj)->dirs_cache)
#define	IS_FILECODE(obj)		(LOGICALSTRMTYPE(obj) == FileCode)
#define	IS_PROCESSCODE(obj)		(LOGICALSTRMTYPE(obj) == ProcessCode)
#define	OUTBUF(obj)			(CURINFO(obj)->OutBuf)
#define	OUTBUF_AT(obj, i)		(CURINFO(obj)->OutBuf[(i)*SHPNUMBYTES(obj)])
#define	OUTBUF_CACHE(obj)		(CURINFO(obj)->OutBuf_cache)
#define	OUTTOINP(obj)			(CURINFO(obj)->OutToInp)
#define	OUTTOINP_AT(obj, i)		(CURINFO(obj)->OutToInp[(i)])
#define	OUTTOINP_CACHE(obj)		(CURINFO(obj)->OutToInp_cache)
#define	INPTOOUT(obj)			(CURINFO(obj)->InpToOut)
#define	INPTOOUT_AT(obj, i)		(CURINFO(obj)->InpToOut[(i)])
#define	INPTOOUT_CACHE(obj)		(CURINFO(obj)->InpToOut_cache)
#define	TMPINPTOOUT(obj)		(CURINFO(obj)->TmpInpToOut)
#define	TMPINPTOOUT_AT(obj, i)		(CURINFO(obj)->TmpInpToOut[(i)])
#define	TMPINPTOOUT_CACHE(obj)		(CURINFO(obj)->TmpInpToOut_cache)
#define	PROPERTY(obj)			(CURINFO(obj)->Property)
#define	PROPERTY_AT(obj, i)		(CURINFO(obj)->Property[(i)])
#define	PROPERTY_CACHE(obj)		(CURINFO(obj)->Property_cache)
#define	IS_REVERSE_AT(obj, i)		((VISUAL_EMBEDDINGS_AT((obj), i) & 0x01) != 0)

#define	III(obj)			(CURINFO(obj)->i)
#define	JJJ(obj)			(CURINFO(obj)->j)
#define	WIDTH(obj)			(CURINFO(obj)->width)
#define	O_IDX(obj)			(CURINFO(obj)->o_idx)
#define	TMP_O_IDX(obj)			(CURINFO(obj)->tmp_o_idx)
#define	FIRST_O_IDX(obj)		(CURINFO(obj)->first_o_idx)
#define	NEXT_CLUSTER(obj)		(CURINFO(obj)->n_c)
#define	ARABIC_SHAPING_STATE(obj)	(CURINFO(obj)->arabic_state)
#define	ArabicCurrentClusterType(obj)	(CURINFO(obj)->arabic_current_cluster_type)
#define	HebrewCurrentClusterType(obj)	(CURINFO(obj)->hebrew_current_cluster_type)

#define	NumDevaCoreCons(obj)		(CURINFO(obj)->num_deva_cons)
#define	DevaJJJ(obj)			(CURINFO(obj)->dev_j)
#define	DevaClusterTypeState(obj)	(CURINFO(obj)->deva_cluster_type_state)

#define	NumTamilCoreCons(obj)		(CURINFO(obj)->num_tamil_cons)
#define	TamilJJJ(obj)			(CURINFO(obj)->tamil_j)
#define	TamilClusterTypeState(obj)	(CURINFO(obj)->tamil_cluster_type_state)

#define	NumKannCoreCons(obj)		(CURINFO(obj)->num_kann_cons)
#define	KannJJJ(obj)			(CURINFO(obj)->kan_j)
#define	KannClusterTypeState(obj)	(CURINFO(obj)->kann_cluster_type_state)

#define	NumGujaCoreCons(obj)		(CURINFO(obj)->num_guja_cons)
#define	GujaJJJ(obj)			(CURINFO(obj)->guj_j)
#define	GujaClusterTypeState(obj)	(CURINFO(obj)->guja_cluster_type_state)

#define	NumBengCoreCons(obj)		(CURINFO(obj)->num_beng_cons)
#define	BengJJJ(obj)			(CURINFO(obj)->bng_j)
#define	BengClusterTypeState(obj)	(CURINFO(obj)->beng_cluster_type_state)

#define	NumTlgCoreCons(obj)		(CURINFO(obj)->num_tlg_cons)
#define	TlgJJJ(obj)			(CURINFO(obj)->tlg_j)
#define	TlgClusterTypeState(obj)	(CURINFO(obj)->tlg_cluster_type_state)
// Below macros are only for Telugu
#define TlgHalantCount(obj)			(CURINFO(obj)->tlg_halant_count)
#define TlgVattuCount(obj)			(CURINFO(obj)->tlg_vattu_count)
#define TlgVattuPrevious(obj)		(CURINFO(obj)->tlg_vattu_previous)
#define TlgVattuIndex(obj)			(CURINFO(obj)->tlg_vattu_index)
#define TlgVattuList(obj,i)			(CURINFO(obj)->tlg_vattu_list[(i)])


#define	NumMlCoreCons(obj)		(CURINFO(obj)->num_ml_cons)
#define	MlJJJ(obj)				(CURINFO(obj)->ml_j)
#define	MlClusterTypeState(obj)	(CURINFO(obj)->ml_cluster_type_state)

#define	NumPnCoreCons(obj)		(CURINFO(obj)->num_pn_cons)
#define	PnJJJ(obj)				(CURINFO(obj)->pn_j)
#define	PnClusterTypeState(obj)	(CURINFO(obj)->pn_cluster_type_state)
// Below macros are only for Punjabi
#define PnHalantCount(obj)			(CURINFO(obj)->pn_halant_count)

#define	NChrsInCluster(obj)		(CURINFO(obj)->n_chrs_in_cluster)
#define	CURR_WC_CLUSTER			(WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))))
#define	END_HDRT(obj)			(CURINFO(obj)->end_hdrt)

#define	SHPCHRSETGRP(obj)		((UMLE_INFO(obj))->ShpChrsetGrp[CURSHPGRP_IDX(obj)])
#define	TRANSFORM_FROM(obj)		((SHPCHRSETGRP(obj)).TransformFROM)
#define	SHPNUMBYTES(obj)		((SHPCHRSETGRP(obj)).ShpNumBytes)
#define	MBINPUNITLEN_F(obj)		((TRANSFORM_FROM(obj)).MbUnitLen)
#define	MBTOWC(obj)			((TRANSFORM_FROM(obj)).MbToWcUnit)
#define MAXBYTES(obj)			((TRANSFORM_FROM(obj)).maxbytes)
#define	MAPPS(obj, submap)		((TRANSFORM_FROM(obj)).maps[(submap)])
#define	TABLE_MAP(obj)			((TRANSFORM_FROM(obj)).TableMap)
#define	MAX_NUM_ENTRIES(obj)		(TABLE_MAP(obj).NumRanges)
#define	START(obj, idx)			(TABLE_MAP(obj).UCS4Range[(idx)].start)
#define	END(obj, idx)			(TABLE_MAP(obj).UCS4Range[(idx)].end)
#define	FUNC(obj, idx)			(TABLE_MAP(obj).UCS4Range[(idx)].func)
/*
#define FUNC_POINTER(obj, wc)		(MAPPS(obj, (wc)/256).is_leaf ? (MAPPS(obj, (wc)/256).d.func) :\
						(MAPPS(obj, (wc)/256).d.funcs[(wc)%256]) )
#define ACTIVATE_FUNC(obj, wc)		(MAPPS(obj, (wc)/256).is_leaf ? (MAPPS(obj, (wc)/256).d.func)(obj) :\
						(MAPPS(obj, (wc)/256).d.funcs[(wc)%256])(obj) )
*/
#define	FUNC_POINTER(obj, wc)		((void(*)())GetFuncPointer((obj), (wc)))
#define	ACTIVATE_FUNC(obj, wc)		((FUNC_POINTER((obj), (wc)))(obj))

#define	FuncUCS4(obj)			((TRANSFORM_FROM(obj)).func_ucs4)
#define	ACTIVATE_F_UCS4(obj)		(((TRANSFORM_FROM(obj)).func_ucs4)(obj))

#define	THAI_CHRTYPE_TBL(obj)		((TRANSFORM_FROM(obj)).ThaiChrTypeTbl)
#define	HEBREW_CHRTYPE_TBL(obj)		((TRANSFORM_FROM(obj)).HebrewChrTypeTbl)
#define	ARABIC_CHRTYPE_TBL(obj)		((TRANSFORM_FROM(obj)).ArabicChrTypeTbl)
#define	DEVA_CHRTYPE_TBL(obj)		((TRANSFORM_FROM(obj)).DevaChrTypeTbl)
#define	TAMIL_CHRTYPE_TBL(obj)		((TRANSFORM_FROM(obj)).TamilChrTypeTbl)
#define	KANN_CHRTYPE_TBL(obj)		((TRANSFORM_FROM(obj)).KannChrTypeTbl)
#define	GUJA_CHRTYPE_TBL(obj)		((TRANSFORM_FROM(obj)).GujaChrTypeTbl)
#define	BENG_CHRTYPE_TBL(obj)		((TRANSFORM_FROM(obj)).BengChrTypeTbl)
#define	TLG_CHRTYPE_TBL(obj)		((TRANSFORM_FROM(obj)).TlgChrTypeTbl)
#define	ML_CHRTYPE_TBL(obj)			((TRANSFORM_FROM(obj)).MlChrTypeTbl)
#define	PN_CHRTYPE_TBL(obj)			((TRANSFORM_FROM(obj)).PnChrTypeTbl)

#define	THAI_CHRCLS_TBL(obj)		((TRANSFORM_FROM(obj)).ThaiChrClsTbl)
#define	HEBREW_CHRCLS_TBL(obj)		((TRANSFORM_FROM(obj)).HebrewChrClsTbl)
#define	ARABIC_CHRCLS_TBL(obj)		((TRANSFORM_FROM(obj)).ArabicChrClsTbl)
#define	DEVA_CHRCLS_TBL(obj)		((TRANSFORM_FROM(obj)).DevaChrClsTbl)
#define	TAMIL_CHRCLS_TBL(obj)		((TRANSFORM_FROM(obj)).TamilChrClsTbl)
#define	KANN_CHRCLS_TBL(obj)		((TRANSFORM_FROM(obj)).KannChrClsTbl)
#define	GUJA_CHRCLS_TBL(obj)		((TRANSFORM_FROM(obj)).GujaChrClsTbl)
#define	BENG_CHRCLS_TBL(obj)		((TRANSFORM_FROM(obj)).BengChrClsTbl)
#define	TLG_CHRCLS_TBL(obj)			((TRANSFORM_FROM(obj)).TlgChrClsTbl)
#define	ML_CHRCLS_TBL(obj)			((TRANSFORM_FROM(obj)).MlChrClsTbl)
#define	PN_CHRCLS_TBL(obj)			((TRANSFORM_FROM(obj)).PnChrClsTbl)

/*
#define	THAI_COMPOSE_TBL(obj)		((TRANSFORM_FROM(obj)).ThaiComposeTbl)
#define	HEBREW_COMPOSE_TBL(obj)		((TRANSFORM_FROM(obj)).HebrewComposeTbl)
#define	ARABIC_COMPOSE_TBL(obj)		((TRANSFORM_FROM(obj)).ArabicComposeTbl)
#define DEVA_COMPOSE_TBL(obj)		((TRANSFORM_FROM(obj)).DevaComposeTbl)
*/
#define	THAI_COMPOSE_TBL(obj)		(ThaiComposeTbl)
#define	HEBREW_COMPOSE_TBL(obj)		(HebrewComposeTbl)
#define	ARABIC_COMPOSE_TBL(obj)		(ArabicComposeTbl)
#define	DEVA_COMPOSE_TBL(obj)		(DevaComposeTbl)
#define	TAMIL_COMPOSE_TBL(obj)		(TamilComposeTbl)
#define	KANN_COMPOSE_TBL(obj)		(KannComposeTbl)
#define	GUJA_COMPOSE_TBL(obj)		(GujaComposeTbl)
#define	BENG_COMPOSE_TBL(obj)		(BengComposeTbl)
#define	TLG_COMPOSE_TBL(obj)		(TlgComposeTbl)
#define	TLG_VATTU_COMPOSE_TBL(obj)	(TlgVattuComposeTbl)
#define	ML_COMPOSE_TBL(obj)		    (MlComposeTbl)
#define	PN_COMPOSE_TBL(obj)		    (PnComposeTbl)





#define	THAI_GLYPH_TBL(obj)			((TRANSFORM_FROM(obj)).ThaiGlyphTbl)
#define	HEBREW_GLYPH_TBL(obj)		((TRANSFORM_FROM(obj)).HebrewGlyphTbl)
#define	ARABIC_GLYPH_TBL(obj)		((TRANSFORM_FROM(obj)).ArabicGlyphTbl)
/*
#define	DEVA_GLYPH_TBL(obj)		((TRANSFORM_FROM(obj)).DevaGlyphTbl)
*/
#define	DEVA_GLYPH_TBL(obj)		(DevaGlyphTbl)
#define	TAMIL_GLYPH_TBL(obj)	(TamilGlyphTbl)
#define	KANN_GLYPH_TBL(obj)		(KannGlyphTbl)
#define	GUJA_GLYPH_TBL(obj)		(GujaGlyphTbl)
#define	BENG_GLYPH_TBL(obj)		(BengGlyphTbl)
#define	TLG_GLYPH_TBL(obj)		(TlgGlyphTbl)
#define	ML_GLYPH_TBL(obj)		(MlGlyphTbl)
#define	PN_GLYPH_TBL(obj)		(PnGlyphTbl)

/* Only for Telugu */
#define V_BOT    1
#define V_SIDE   2

#define TLG_VATTU_TYPE(obj)		(TlgVattuTypeTbl)



#define	IsBiDiType(obj, wc, mask)	(((unsigned)(wc) > 0xffff) ? (ON) : (bidi_class_masks[bidi_class_idx[0][(wc)]] & (mask)))

#define ucs2tis(wc)			(unsigned int)((unsigned int)(wc) - 0x0E00 + 0x20)
#define ucs2iso8859_8(wc)		(unsigned int)((unsigned int)(wc) - 0x0590 + 0x10)
#define ucs2iso8859_6(wc)		(unsigned int)((unsigned int)(wc) - 0x0600)
#define	ucs2alif(wc)			(unsigned int)((unsigned int)(wc) - 0x0622)
#define ucs2HebrewCons(wc)		(unsigned int)((unsigned int)(wc) - 0x05D0)
#define ucs2HebrewHIRIQ(wc)		(unsigned int)((unsigned int)(wc) - 0x05B4)
#define	ucs2HebrewCantPunct(wc)		(unsigned int)((unsigned int)(wc) - 0x0590)
#define ucs2index(wc)			(unsigned int)((unsigned int)(wc) - 0x0900)
#define ucs2Tam_index(wc)		(unsigned int)((unsigned int)(wc) - 0x0B80)
#define ucs2kann_index(wc)		(unsigned int)((unsigned int)(wc) - 0x0C80)
#define ucs2guja_index(wc)		(unsigned int)((unsigned int)(wc) - 0x0A80)
#define ucs2beng_index(wc)		(unsigned int)((unsigned int)(wc) - 0x0980)
#define ucs2tlg_index(wc)		(unsigned int)((unsigned int)(wc) - 0x0C00)
#define ucs2ml_index(wc)		(unsigned int)((unsigned int)(wc) - 0x0D00)
#define ucs2pn_index(wc)		(unsigned int)((unsigned int)(wc) - 0x0A00)
#define ucs2hebrew_8(wc)		(unsigned int)((unsigned int)(wc) - 0x0590 + 0xa0)

#define	IsThaiCharCls(obj, wc, mask)	(THAI_CHRCLS_TBL(obj)[(ucs2tis((wc)))] & (mask))
#define	IsHebrewCharCls(obj, wc, mask)	(HEBREW_CHRCLS_TBL(obj)[(ucs2iso8859_8((wc)))] & (mask))
#define	IsArabicCharCls(obj, wc, mask)	(ARABIC_CHRCLS_TBL(obj)[(ucs2iso8859_6((wc)))] & (mask))
#define IsDevaCharCls(obj, wc, mask)	(DEVA_CHRCLS_TBL(obj)[(ucs2index((wc)))] & (mask))
#define IsTamilCharCls(obj, wc, mask)	(TAMIL_CHRCLS_TBL(obj)[(ucs2Tam_index((wc)))] & (mask))
#define IsKannCharCls(obj, wc, mask)	(KANN_CHRCLS_TBL(obj)[(ucs2kann_index((wc)))] & (mask))
#define IsGujaCharCls(obj, wc, mask)	(GUJA_CHRCLS_TBL(obj)[(ucs2guja_index((wc)))] & (mask))
#define IsBengCharCls(obj, wc, mask)	(BENG_CHRCLS_TBL(obj)[(ucs2beng_index((wc)))] & (mask))
#define IsTlgCharCls(obj, wc, mask)		(TLG_CHRCLS_TBL(obj)[(ucs2tlg_index((wc)))] & (mask))
#define IsMlCharCls(obj, wc, mask)		(ML_CHRCLS_TBL(obj)[(ucs2ml_index((wc)))] & (mask))
#define IsPnCharCls(obj, wc, mask)		(PN_CHRCLS_TBL(obj)[(ucs2pn_index((wc)))] & (mask))

#define	IsThaiComposible(obj, wc1, wc2)	   (THAI_COMPOSE_TBL(obj)[THAI_CHRTYPE_TBL(obj)[ucs2tis((wc1))]]\
								 [THAI_CHRTYPE_TBL(obj)[ucs2tis((wc2))]])
#define	IsHebrewComposible(obj, wc1, wc2)  (HEBREW_COMPOSE_TBL(obj)[HEBREW_CHRTYPE_TBL(obj)[ucs2iso8859_8((wc1))]]\
								   [HEBREW_CHRTYPE_TBL(obj)[ucs2iso8859_8((wc2))]])
#define	IsArabicComposible(obj, wc1, wc2)  (ARABIC_COMPOSE_TBL(obj)[ARABIC_CHRTYPE_TBL(obj)[ucs2iso8859_6((wc1))]]\
								   [ARABIC_CHRTYPE_TBL(obj)[ucs2iso8859_6((wc2))]])
#define IsDevaComposible(obj,wc1,wc2)	(DEVA_COMPOSE_TBL(obj)[DEVA_CHRTYPE_TBL(obj)[ucs2index((wc1))]]\
                                                              [DEVA_CHRTYPE_TBL(obj)[ucs2index((wc2))]])


#define IsTamilComposible(obj,wc1,wc2)	(TAMIL_COMPOSE_TBL(obj)[TAMIL_CHRTYPE_TBL(obj)[ucs2Tam_index((wc1))]]\
                                                               [TAMIL_CHRTYPE_TBL(obj)[ucs2Tam_index((wc2))]])

#define IsKannComposible(obj,wc1,wc2)	(KANN_COMPOSE_TBL(obj)[KANN_CHRTYPE_TBL(obj)[ucs2kann_index((wc1))]]\
                                                              [KANN_CHRTYPE_TBL(obj)[ucs2kann_index((wc2))]])

#define IsGujaComposible(obj,wc1,wc2)	(GUJA_COMPOSE_TBL(obj)[GUJA_CHRTYPE_TBL(obj)[ucs2guja_index((wc1))]]\
                                                              [GUJA_CHRTYPE_TBL(obj)[ucs2guja_index((wc2))]])

#define IsBengComposible(obj,wc1,wc2)	(BENG_COMPOSE_TBL(obj)[BENG_CHRTYPE_TBL(obj)[ucs2beng_index((wc1))]]\
                                                              [BENG_CHRTYPE_TBL(obj)[ucs2beng_index((wc2))]])

#define IsTlgComposible(obj,wc1,wc2)	(TLG_COMPOSE_TBL(obj)[TLG_CHRTYPE_TBL(obj)[ucs2tlg_index((wc1))]]\
                                                             [TLG_CHRTYPE_TBL(obj)[ucs2tlg_index((wc2))]])

#define IsMlComposible(obj,wc1,wc2)	(ML_COMPOSE_TBL(obj)[ML_CHRTYPE_TBL(obj)[ucs2ml_index((wc1))]]\
                                                        [ML_CHRTYPE_TBL(obj)[ucs2ml_index((wc2))]])

#define IsPnComposible(obj,wc1,wc2)	(PN_COMPOSE_TBL(obj)[PN_CHRTYPE_TBL(obj)[ucs2pn_index((wc1))]]\
                                                        [PN_CHRTYPE_TBL(obj)[ucs2pn_index((wc2))]])



#define IsVattuComposible(obj,wc1,wc2) (TLG_VATTU_COMPOSE_TBL(obj)[wc1][wc2])

#define	GetDeva_nMaxFocBlockTbl(obj, wc)	(Deva_nMaxFocBlockTbl[ucs2index((wc))])
#define	GetTamil_nMaxFocBlockTbl(obj, wc)	(Tamil_nMaxFocBlockTbl[ucs2Tam_index((wc))])
#define	GetKann_nMaxFocBlockTbl(obj, wc)	(Kann_nMaxFocBlockTbl[ucs2kann_index((wc))])
#define	GetGuja_nMaxFocBlockTbl(obj, wc)	(Guja_nMaxFocBlockTbl[ucs2guja_index((wc))])
#define	GetBeng_nMaxFocBlockTbl(obj, wc)	(Beng_nMaxFocBlockTbl[ucs2beng_index((wc))])
#define	GetTlg_nMaxFocBlockTbl(obj, wc)		(Tlg_nMaxFocBlockTbl[ucs2tlg_index((wc))])
#define	GetMl_nMaxFocBlockTbl(obj, wc)		(Ml_nMaxFocBlockTbl[ucs2ml_index((wc))])
#define	GetPn_nMaxFocBlockTbl(obj, wc)		(Pn_nMaxFocBlockTbl[ucs2pn_index((wc))])

#define	IsDevaKernSpaceGlyphs(obj, glyph)	((glyph >= 0xE700F830 && glyph <= 0xE700F83E) ? TRUE : FALSE)
/* NOT REQUIRED FOR TAMIL  */
#define	IsGujaKernSpaceGlyphs(obj, glyph)	((glyph >= 0xE700F559 && glyph <= 0xE700F569) ? TRUE : FALSE)
#define	IsBengKernSpaceGlyphs(obj, glyph)	((glyph >= 0xE700F46E && glyph <= 0xE700F478) ? TRUE : FALSE)

#define	IsTlgKernSpaceGlyphs(obj, glyph) ( (  glyph == 0xE700F59C || glyph == 0xE700F5A8 || \
					  glyph == 0xE700F5A9 || glyph == 0xE700F5AA || \
					  glyph == 0xE700F5B2 || glyph == 0xE700F5BC || \
					  glyph == 0xE700F5CC || glyph == 0xE700F5DB || \
					  glyph == 0xE700F5E2 || glyph == 0xE700F5E6 || \
					  glyph == 0xE700F600 || glyph == 0xE700F602  \
					)? TRUE:FALSE )
 
/* NOT REQUIRED FOR MALAYALAM  */

#define	IsDevaMatraAtStemGlyphs(obj, glyph)	(((glyph >= 0xF811 && glyph <= 0xF813) || \
						  (glyph >= 0xF81E && glyph <= 0xF82F) || \
						  (glyph >= 0x0962 && glyph <= 0x0963) || \
						  (glyph >= 0x0941 && glyph <= 0x0948) || \
						  (glyph == 0x093C) || \
						  (glyph == 0x094D)) ? TRUE : FALSE)

/* NOT REQUIRED FOR TAMIL  */

#define	IsGujaMatraAtStemGlyphs(obj, glyph)	(((glyph <= 0xF53E && glyph >= 0xF53C) || \
						  (glyph <= 0xF558  && glyph >= 0xF549) || \
						  (glyph == 0x0ACD || glyph == 0x0ABC) || \
						  (glyph == 0x0AC8 || glyph == 0x0AB7) || \
						  (glyph <= 0x0AC5 && glyph >= 0x0AC1)) ? TRUE : FALSE)

#define	IsBengMatraAtStemGlyphs(obj, glyph)	(((glyph <= 0x09C4 && glyph >= 0x09C1) || \
						  (glyph == 0x09CD || glyph == 0x09BC) || \
						  (glyph == 0xF36D)) ? TRUE : FALSE)


#define IsTlgMatraAtStemGlyph(obj, glyph )  ( ( glyph == 0xE700F5AD  || glyph == 0xE700F5AF || \
												glyph == 0xE700F5DE  || glyph == 0xE700F5E3 || \
												glyph == 0xE700F5E7  || glyph == 0xE700F5F3 || \
												glyph == 0xE700F5F8  || glyph == 0xE700F5FD || \
												glyph == 0xE700F5F4  || glyph == 0xE700F601 || \
												glyph == 0xE700F5CD  || glyph == 0xE700F5CE || \
												( ( glyph >= 0xE700F5B3) && ( glyph <= 0xE700fF5B5) )|| \
												( ( glyph >= 0xE700F5B8) && ( glyph <= 0xE700F5B9) )|| \
												( ( glyph >= 0xE700F5BF) && ( glyph <= 0xE700F5C8) )|| \
												( ( glyph >= 0xE700F5D3) && ( glyph <= 0xE700F5D5) )|| \
												( ( glyph >= 0xE7000C3E) && ( glyph <= 0xE7000C4C) )  \
												) ? TRUE:FALSE )

/* NOT REQUIRED FOR MALAYALAM  */
/* NOT REQUIRED FOR PUNJABI */


// Additional macros for Telugu 
#define IsTlgBottomVattu(obj,UniChar )  ( (  ( UniChar >= 0xF604)  && ( UniChar <= 0xF606) || \
									   ( UniChar >= 0xF609)  && ( UniChar <= 0xF60E) || \
									   ( UniChar >= 0xF610)  && ( UniChar <= 0xF613) || \
									   ( UniChar == 0xF61B)  ||  ( UniChar == 0xF61C)||\
									   ( UniChar == 0xF5A2)  ||( UniChar == 0xF5A4) \
									)? TRUE:FALSE)


#define IsTlgSideVattu(obj, chISFOC )  ( ( (chISFOC == 0xF603) || (chISFOC == 0xF607) || \
								    (chISFOC >= 0xF614) && (chISFOC <= 0xF61A) || \
									(chISFOC == 0xF5A0) || (chISFOC == 0xF5A1) ||\
									(chISFOC == 0xF5A3) || (chISFOC == 0xF61D)\
									)?TRUE:FALSE )


#define IsOutputTlgSideVattu(obj, chISFOC )  ( ( (chISFOC == 0xE700F603) || (chISFOC == 0xE700F607) || \
								    (chISFOC >= 0xE700F614) && (chISFOC <= 0xE700F61A) || \
									(chISFOC == 0xE700F5A0) || (chISFOC == 0xE700F5A1) ||\
									(chISFOC == 0xE700F5A3) || (chISFOC == 0xE700F61D)\
									)?TRUE:FALSE )


// Additional macros for Malayalam

#define IsMlConsComposibile(obj,wc1,wc2) ( ( ( (wc1 == 0x0D15) && (wc2 == 0x0D15) ) || ( (wc1 == 0x0D15) && (wc2 == 0x0D33) ) ||\
											 ( (wc1 == 0x0D15) && (wc2 == 0x0D37) ) || ( (wc1 == 0x0D15) && (wc2 == 0x0D19) ) ||\
										     ( (wc1 == 0x0D15) && (wc2 == 0x0D1F) ) || ( (wc1 == 0x0D15) && (wc2 == 0x0D24) ) ||\
										     ( (wc1 == 0x0D15) && (wc2 == 0x0D2F) ) || ( (wc1 == 0x0D15) && (wc2 == 0x0D31) ) ||\
										     ( (wc1 == 0x0D15) && (wc2 == 0x0D35) ) ||\
										     ( (wc1 == 0x0D17) && (wc2 == 0x0D17) ) || ( (wc1 == 0x0D17) && (wc2 == 0x0D33) ) ||\
										     ( (wc1 == 0x0D17) && (wc2 == 0x0D2E) ) || ( (wc1 == 0x0D17) && (wc2 == 0x0D28) ) ||\
										     ( (wc1 == 0x0D19) && (wc2 == 0x0D19) ) || ( (wc1 == 0x0D1A) && (wc2 == 0x0D1A) ) ||\
										     ( (wc1 == 0x0D1A) && (wc2 == 0x0D1B) ) || ( (wc1 == 0x0D1C) && (wc2 == 0x0D1C) ) ||\
										     ( (wc1 == 0x0D1C) && (wc2 == 0x0D1E) ) || ( (wc1 == 0x0D1E) && (wc2 == 0x0D1A) ) ||\
										     ( (wc1 == 0x0D1E) && (wc2 == 0x0D1E) ) || ( (wc1 == 0x0D1F) && (wc2 == 0x0D1F) ) ||\
										     ( (wc1 == 0x0D21) && (wc2 == 0x0D21) ) || ( (wc1 == 0x0D23) && (wc2 == 0x0D1F) ) ||\
										     ( (wc1 == 0x0D23) && (wc2 == 0x0D23) ) || ( (wc1 == 0x0D23) && (wc2 == 0x0D21) ) ||\
										     ( (wc1 == 0x0D24) && (wc2 == 0x0D24) ) || ( (wc1 == 0x0D24) && (wc2 == 0x0D25) ) ||\
										     ( (wc1 == 0x0D24) && (wc2 == 0x0D38) ) || ( (wc1 == 0x0D24) && (wc2 == 0x0D2D) ) ||\
										     ( (wc1 == 0x0D24) && (wc2 == 0x0D2E) ) || ( (wc1 == 0x0D26) && (wc2 == 0x0D26) ) ||\
										     ( (wc1 == 0x0D26) && (wc2 == 0x0D27) ) || ( (wc1 == 0x0D28) && (wc2 == 0x0D24) ) ||\
										     ( (wc1 == 0x0D28) && (wc2 == 0x0D26) ) || ( (wc1 == 0x0D28) && (wc2 == 0x0D28) ) ||\
											 ( (wc1 == 0x0D28) && (wc2 == 0x0D2E) ) || ( (wc1 == 0x0D28) && (wc2 == 0x0D2A) ) ||\
										     ( (wc1 == 0x0D28) && (wc2 == 0x0D27) ) || ( (wc1 == 0x0D28) && (wc2 == 0x0D25) ) ||\
											 ( (wc1 == 0x0D2A) && (wc2 == 0x0D2A) ) || ( (wc1 == 0x0D2A) && (wc2 == 0x0D33) ) ||\
											 ( (wc1 == 0x0D2C) && (wc2 == 0x0D2C) ) || ( (wc1 == 0x0D2C) && (wc2 == 0x0D33) ) ||\
											 ( (wc1 == 0x0D2C) && (wc2 == 0x0D27) ) || ( (wc1 == 0x0D2C) && (wc2 == 0x0D26) ) ||\
										     ( (wc1 == 0x0D2E) && (wc2 == 0x0D2E) ) || ( (wc1 == 0x0D2E) && (wc2 == 0x0D33) ) ||\
											 ( (wc1 == 0x0D2F) && (wc2 == 0x0D2F) ) || ( (wc1 == 0x0D30) && (wc2 == 0x0D30) ) ||\
										     ( (wc1 == 0x0D32) && (wc2 == 0x0D33) ) || ( (wc1 == 0x0D33) && (wc2 == 0x0D33) ) ||\
											 ( (wc1 == 0x0D35) && (wc2 == 0x0D35) ) || ( (wc1 == 0x0D36) && (wc2 == 0x0D33) ) ||\
										     ( (wc1 == 0x0D36) && (wc2 == 0x0D35) ) || ( (wc1 == 0x0D36) && (wc2 == 0x0D1A) ) ||\
										     ( (wc1 == 0x0D37) && (wc2 == 0x0D1F) ) || ( (wc1 == 0x0D38) && (wc2 == 0x0D33) ) ||\
											 ( (wc1 == 0x0D38) && (wc2 == 0x0D38) ) || ( (wc1 == 0x0D38) && (wc2 == 0x0D30) ) ||\
											 ( (wc1 == 0x0D38) && (wc2 == 0x0D2E) ) || ( (wc1 == 0x0D38) && (wc2 == 0x0D25) ) ||\
											 ( (wc1 == 0x0D39) && (wc2 == 0x0D33) ) || ( (wc1 == 0x0D39) && (wc2 == 0x0D2E) ) ||\
											 ( (wc1 == 0x0D39) && (wc2 == 0x0D28) ) ) ? TRUE:FALSE )
										 

#define IsValidPnVowel(obj, chISFOC )		( ( (chISFOC > 0xE7000A04) && (chISFOC < 0xE7000A0B) || \
						    (chISFOC > 0xE7000A0E) && (chISFOC < 0xE7000A11) || \
					  	    (chISFOC > 0xE7000A12) && (chISFOC < 0xE7000A15) \
		 				  )?TRUE:FALSE )

										 
#define IsValidPnCons(obj, chISFOC )		( ( (chISFOC > 0xE7000A14) && (chISFOC < 0xE7000A29) || \
				   		    (chISFOC > 0xE7000A29) && (chISFOC < 0xE7000A31) || \
						    (chISFOC > 0xE7000A31) && (chISFOC < 0xE7000A34) || \
						    (chISFOC > 0xE7000A34) && (chISFOC < 0xE7000A37) || \
												(chISFOC > 0xE7000A37) && (chISFOC < 0xE7000A3A) || \
												(chISFOC > 0xE7000A58) && (chISFOC < 0xE7000A5D) || \
												(chISFOC == 0xE7000A5E) )?TRUE:FALSE )


#define IsValidPnMatra(obj, chISFOC )		( ( (chISFOC >  0xE7000A3D) && (chISFOC <  0xE7000A43) || \
												(chISFOC >  0xE7000A46) && (chISFOC <  0xE7000A49) || \
												(chISFOC >  0xE7000A4A) && (chISFOC <  0xE7000A4D) || \
												(chISFOC >= 0xE700F327) && (chISFOC <= 0xE700F328) || \
												(chISFOC >= 0xE700F32E) && (chISFOC <= 0xE700F32F) || \
												(chISFOC >= 0xE700F331) && (chISFOC <= 0xE700F332) || \
												(chISFOC >= 0xE700F334) && (chISFOC <= 0xE700F335) || \
												(chISFOC == 0xE700F33B) )?TRUE:FALSE )


#define IsValidPnVowelMod(obj, chISFOC )		( ( (chISFOC >  0xE7000A02) && (chISFOC <  0xE7000A70) || \
													(chISFOC >= 0xE700F32B) && (chISFOC <= 0xE700F32C) || \
													(chISFOC == 0xE700F326) )?TRUE:FALSE )


#define	UpdateDevaClusterTypeState(obj, CurSte, wc) \
		DevaStateTable[(CurSte)][DEVA_CHRTYPE_TBL(obj)[ucs2index((wc))]]

#define	UpdateTamilClusterTypeState(obj, CurSte, wc) \
		TamilStateTable[(CurSte)][TAMIL_CHRTYPE_TBL(obj)[ucs2Tam_index((wc))]]

#define	UpdateKannClusterTypeState(obj, CurSte, wc) \
		KannStateTable[(CurSte)][KANN_CHRTYPE_TBL(obj)[ucs2kann_index((wc))]]

#define	UpdateGujaClusterTypeState(obj, CurSte, wc) \
		GujaStateTable[(CurSte)][GUJA_CHRTYPE_TBL(obj)[ucs2guja_index((wc))]]

#define	UpdateBengClusterTypeState(obj, CurSte, wc) \
		BengStateTable[(CurSte)][BENG_CHRTYPE_TBL(obj)[ucs2beng_index((wc))]]

#define	UpdateTlgClusterTypeState(obj, CurSte, wc) \
		TlgStateTable[(CurSte)][TLG_CHRTYPE_TBL(obj)[ucs2tlg_index((wc))]]

#define	UpdateMlClusterTypeState(obj, CurSte, wc) \
		MlStateTable[(CurSte)][ML_CHRTYPE_TBL(obj)[ucs2ml_index((wc))]]

#define	UpdatePnClusterTypeState(obj, CurSte, wc) \
		PnStateTable[(CurSte)][PN_CHRTYPE_TBL(obj)[ucs2pn_index((wc))]]



#define	OUTPUT_OUT2INP_CACHE_CHECK	if ((O_IDX(obj) + 1)*SHPNUMBYTES(obj) > OUTCZ(obj))\
					     OutputCacheRealloc(obj);

#define	MAX_SHPDCHRSETS			1

#define	MAX_NUM_RANGES			115	

#define CacheFree(pointer, stack_cache_array) \
    if ((pointer) != ((char*)(stack_cache_array))) UMLEFree(pointer);

/*
 * Following vector shows remaining bytes in a UTF-8 character.
 * Index will be the first byte of the character.
 */
static const char remaining_bytes_tbl[0x100] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   
    /* BE  BF  C0  C1  C2  C3  C4  C5  C6  C7   */
    0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
   
    /* C8  C9  CA  CB  CC  CD  CE  CF  D0  D1   */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
   
    /* D2  D3  D4  D5  D6  D7  D8  D9  DA  DB   */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
   
    /* DC  DD  DE  DF  E0  E1  E2  E3  E4  E5   */
    1,  1,  1,  1,  2,  2,  2,  2,  2,  2,
   
    /* E6  E7  E8  E9  EA  EB  EC  ED  EE  EF   */
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
   
    /* F0  F1  F2  F3  F4  F5  F6  F7  F8  F9   */
    3,  3,  3,  3,  3,  3,  3,  3,  4,  4,

    /* FA  FB  FC  FD  FE  FF   */
    4,  4,  5,  5,  0,  0,
};

/*
 * Following is a vector of bit-masks to get used bits in the first byte of
 * a UTF-8 character.  Index is remaining bytes at above of the character.
 */
static const char masks_tbl[6] = { 0x00, 0x1f, 0x0f, 0x07, 0x03, 0x01 };

typedef	unsigned long		bidi_type_t;

typedef struct	_RangeRec {
    u_int		start;
    u_int		end;
    void		(*func)(LayoutObj);
} RangeRec;

typedef struct	_MappingTable {
    int			NumRanges;
    RangeRec		UCS4Range[MAX_NUM_RANGES];
} MappingTable;

typedef	int			thai_cls_t;
typedef	int			arabic_cls_t;
typedef	long		hebrew_cls_t;
typedef	long		deva_cls_t;
typedef	long		tamil_cls_t;
typedef	long		kann_cls_t;
typedef	long		guja_cls_t;
typedef	long		beng_cls_t;
typedef	long		tlg_cls_t;
typedef	long		ml_cls_t;
typedef	long		pn_cls_t;

typedef struct {
  const int ShiftDown_TONE_AD[8];
  const int ShiftDownLeft_TONE_AD[8];
  const int ShiftLeft_TONE_AD[8];
  const int ShiftLeft_AV[7];
  const int ShiftDown_BV_BD[3];
  const int TailCutCons[4];
} ThaiGlyphTable;

typedef	struct {
  const int ConsDAGESH[32];
  const int ALEF_Vowel[5];
  const int ConsRAFE[32];
  const int PunctForWideCons[64];
  const int PunctForNarrowCons[64];
} HebrewGlyphTable;

typedef	struct {
  const int IsolatedForm[128];
  const int InitialForm[128];
  const int MiddleForm[128];
  const int FinalForm[128];
  const int ComBaseLineForm[128];
  const int ComNoBaseLineForm[128];
  const int IsolatedLamAlifForm[6];
  const int FinalLamAlifForm[6];
} ArabicGlyphTable;

typedef	int	StateTypeID;

typedef struct {
  const wchar_t strISCII[MAX_CORE_CONS_BUF];
  const unsigned short strISFOC[MAX_CORE_CONS_BUF];
} DevaGlyphEntry, TamilGlyphEntry, KannGlyphEntry, GujaGlyphEntry, BengGlyphEntry, TlgGlyphEntry,MlGlyphEntry, PnGlyphEntry;

/*
typedef struct {
  const GlyphEntry[MAP_SIZE];
} DevaGlyphTable;
*/

/*
 * Thai character class table
 */
static const thai_cls_t ThaiChrClsTbl[128] = {
  /*	   0,	1,   2,	  3,   4,   5,	 6,   7,
	   8,	9,   A,	  B,   C,   D,	 E,   F	 */

  /*00*/ _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
	 _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
  /*10*/ _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
	 _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
  /*20*/ _ND, _NC, _NC, _NC, _NC, _NC, _NC, _NC,
	 _NC, _NC, _NC, _NC, _NC, _SC, _BC, _BC,
  /*30*/ _SC, _NC, _NC, _NC, _NC, _NC, _NC, _NC,
	 _NC, _NC, _NC, _UC, _NC, _UC, _NC, _UC,
  /*40*/ _NC, _NC, _NC, _NC, _ND, _NC, _ND, _NC,
	 _NC, _NC, _NC, _NC, _UC, _NC, _NC, _ND,
  /*50*/ _ND, _AV, _ND, _AM, _AV, _AV, _AV, _AV,
	 _BV, _BV, _BD, _ND, _ND, _ND, _ND, _ND,
  /*60*/ _ND, _ND, _ND, _ND, _ND, _ND, _ND, _AD,
	 _TN, _TN, _TN, _TN, _AD, _AD, _AD, _ND,
  /*70*/ _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
	 _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
};

/*
 * Hebrew character class table
 */
static const hebrew_cls_t HebrewChrClsTbl[128] = {
  /*	   0,	1,   2,	  3,   4,   5,	 6,   7
	   8,	9,   A,	  B,   C,   D,	 E,   F	 */

  /*0 */ _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
	 _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,

  /* Cantillation */
  /*1 */ _ND, _NS, _NS, _NS, _NS, _NS, _NS, _NS,
	 _NS, _NS, _NS, _NS, _NS, _NS, _NS, _NS,
  /*2 */ _NS, _NS, _ND, _NS, _NS, _NS, _NS, _NS,
	 _NS, _NS, _NS, _NS, _NS, _NS, _NS, _NS,

  /* Punctuation */
  /*3 */ _POINTS,    _POINTS,   _POINTS,    _POINTS,
          _HIRIQ,    _POINTS,   _POINTS,     _PATAH,
	 _QAMATS,     _HOLAM,       _ND,    _POINTS,
         _DAGESH,     _MARKS,       _ND,      _RAFE,
  /*4 */     _ND,  _SHIN_DOT,  _SIN_DOT,        _ND,
         _POINTS,        _ND,       _ND,        _ND,
	     _ND,        _ND,       _ND,        _ND,
             _ND,        _ND,       _ND,        _ND,

  /* Consonances */
  /*5 */      _ALEF|_WC,  _CNDA|_BET|_WC,  _CNDA|_WC,      _CNDA|_WC,
              _CNDA|_WC, _CNDA|_VAV|_NRC, _CNDA|_NRC,        _ND|_WC,
	      _CNDA|_WC, _CNDA|_YOD|_NRC,  _CNDA|_WC, _CNDA|_KAF|_WC,
         _CNDA|_LAM|_WC,         _ND|_WC,  _CNDA|_WC,       _ND|_NRC,

  /*6 */     _CNDA|_NRC,          _CNDA|_WC,           _WC, _CNDA|_WC,
          _CNDA|_PE|_WC,                _WC,     _CNDA|_WC, _CNDA|_WC,
	      _CNDA|_WC, _CNDA|_SHIN_CN|_WC,     _CNDA|_WC,       _ND,
                    _ND,                _ND,           _ND,       _ND,

  /* Yiddish */
  /*7 */            _ND,                _ND, _DOUB_YOD|_WC,       _ND,
                    _ND,                _ND,           _ND,       _ND,
	            _ND,                _ND,           _ND,       _ND,
                    _ND,                _ND,           _ND,       _ND,
};

/*
 * Arabic character class table
 */
static const arabic_cls_t ArabicChrClsTbl[128] = {
  /*	   0,	1,   2,	  3,   4,   5,	 6,   7 
	   8,	9,   A,	  B,   C,   D,	 E,   F	 */

  /*00*/ _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
	 _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
  /*10*/ _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
	 _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
  /*20*/ _ND, _ND, _AL, _AL, _ND, _AL, _ND, _AL,
	 _4F, _2F, _4F, _4F, _4F, _4F, _4F, _2F,
  /*30*/ _2F, _2F, _2F, _4F, _4F, _4F, _4F, _ND,
	 _ND, _4F, _4F, _ND, _ND, _ND, _ND, _ND,
  /*40*/ _ND, _4F, _4F, _4F, _LM, _4F, _4F, _4F,
	 _2F, _2F, _4F, _VW, _VW, _VW, _VW, _VW,
  /*50*/ _VW, _SD, _VW, _ND, _ND, _ND, _ND, _ND,
	 _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
  /*60*/ _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
	 _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
  /*70*/ _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
	 _ND, _ND, _ND, _ND, _ND, _ND, _ND, _ND,
};

/*
 * Devagari character class table
 */
static const deva_cls_t DevaChrClsTbl[128] = {
/*              0,       1,       2,           3,          4,       5,    6,   7,
                8,       9,       A,           B,          C,       D,    E,   F, */
/* 0 */       _ND,     _UP,      _UP,        _NP,        _ND,     _IV,  _IV, _IV,
              _IV,     _IV,      _IV,        _IV,        _IV,     _IV,  _IV, _IV,
/* 1 */       _IV,     _IV,      _IV,        _IV,        _IV, _CK|_MS,  _CK, _CK,
              _CN,     _CN,      _CN,        _CN,        _CK,     _CN,  _CN, _CN|_MS,
/* 2 */   _CN|_MS, _CK|_MS,  _CK|_MS,        _CN,        _CN,     _CN,  _CN, _CN,
              _CN,     _CN,      _CN,    _CK|_MS,        _CN,     _CN,  _CN, _CN,
/* 3 */       _RC,     _CN,      _CN,        _CN,        _CN,     _CN,  _CN, _CN,
              _CN, _CN|_MS,      _ND,        _ND,        _NK,     _VD,  _NM, _IM,
/* 4 */     _II_M,     _NM,      _NM,        _NM,        _NM,  _AYE_M,_EE_M, _EY_M,
            _AI_M,  _AWE_M,     _O_M,     _OW1_M,     _OW2_M,     _HL,  _ND, _ND,
/* 5 */       _ND,     _VD,      _VD,        _VD,        _VD,     _ND,  _ND, _ND,
              _CN,     _CN,      _CN,        _CN,        _CN,     _CN,  _CN, _CN,
/* 6 */       _IV,     _IV,      _NM,        _NM,        _ND,     _ND,  _HD, _HD,
              _HD,     _HD,      _HD,        _HD,        _HD,     _HD,  _HD, _HD,
/* 7 */       _ND,     _ND,      _ND,        _ND,        _ND,     _ND,  _ND, _ND,
              _ND,     _ND,      _ND,        _ND,        _ND,     _ND,  _ND, _ND,
};



/*
 * Tamil character class table
 */
static const tamil_cls_t TamilChrClsTbl[128] = 
{

/*			0		1		2		3		4		5		6		7		*
			8		9		A		B		C		D		E		F		*/

/*0B8*/	   _ND,    _ND,    _VM,    _VM,    _ND,    _TMV,   _TMV,   _TMV,	
		   _TMV,   _TMV,   _TMV,   _ND,    _ND,    _ND,    _TMV,   _TMV,
/*0B9*/	   _TMV,   _ND,    _TMV,   _TMV,   _TMV,   _TMC,   _ND,    _ND,
		   _ND,	   _TMC,   _TMC,   _ND,    _TMC,   _ND,    _TMC,   _TMC,
/*0BA*/	   _ND,	   _ND,    _ND,    _TMC,   _TMC,   _ND,    _ND,    _ND,	
		   _TMC,   _TMC,   _TMC,   _ND,    _ND,    _ND,    _TMC,   _TMC,
/*0BB*/	   _TMC,   _TMC,   _TMC,   _TMC,   _TMC,   _TMC,   _ND,    _TMC,
		   _TMC,   _TMC,   _ND,    _ND,    _ND,    _ND,    _TNM,    _TNM,
/*0BC*/	   _TNM,    _TNM,  _TNM,   _ND,    _ND,    _ND,    _TRM,    _TRM,		
		   _TRM,    _ND,   _SM,    _SM,    _SM,    _TMH,   _ND,    _ND,
/*0BD*/	   _ND,    _ND,	   _ND,    _ND,    _ND,    _ND,    _ND,    _PM,
		   _ND,    _ND,    _ND,    _ND,    _ND,    _ND,    _ND,    _ND,
/*0BE*/	   _ND,    _ND,    _ND,    _ND,    _ND,    _ND,    _ND,    _TMD,
		   _TMD,   _TMD,   _TMD,   _TMD,   _TMD,   _TMD,   _TMD,   _TMD,
/*0BF*/	   _TMD,   _TMD,   _TMD,   _ND,    _ND,    _ND,    _ND,    _ND,	
		   _ND,	   _ND,    _ND,    _ND,    _ND,    _ND,    _ND,    _ND

};




/*
 * Kannada character class table
 */

static const kann_cls_t KannChrClsTbl[128] = {
/*     0,*      *   1   * *   2  *      *   3  *      *   4  *  *   5  *	     *   6  *    *  7
       8 *      *   9   * *   A  *      *   B  *      *   C  *  *   D  *	     *   E  *    *  F */

/*0*/ _ND,	_ND,	   _N_VM,	  _N_VM,	_ND,      _V,     	_V,	   _V,
      _V,       _V,	   _V,		  _V,		_N_M,	  _ND,		_V,	   _V,

/*1*/ _V,	_ND,	   _V,		  _V,		_V,	  _N_CONS,	_N_CONS,   _N_CONS,
      _N_CONS,	_N_CONS,   _N_CONS,	  _N_CONS,	_NUK_CONS,_N_CONS,	_N_CONS,   _N_CONS,

/*2*/ _N_CONS,	_N_CONS,   _N_CONS,	  _N_CONS,	_N_CONS,  _N_CONS,	_N_CONS,   _N_CONS,
      _N_CONS,	_ND,	   _N_CONS,	  _NUK_CONS,	_N_CONS,  _N_CONS,	_N_CONS,   _N_CONS,

/*3*/ _N_CONS,	_N_CONS,   _N_CONS,	  _N_CONS,	_ND,  _N_CONS,	_N_CONS,   _N_CONS,
      _N_CONS,	_N_CONS,   _ND,		  _ND,		_ND,	  _ND,		_N_M,	   _N_M,

/*4*/ _N_M,	_N_M,	   _N_M,	  _N_M,		_N_M,	  _ND,		_N_M,      _N_M,
      _N_M,	_ND,	   _N_M,	  _N_M,		_N_M,	  _HAL,		_ND,	   _ND,

/*5*/ _ND,	_ND,	   _ND,		  _ND,		_ND,	  __M,		__M,	   _ND,
      _ND,	_ND,	   _ND,		  _ND,		_ND,	  _ND,		__M,	   _ND,

/*6*/ _V,	_N_M,	   _ND,		  _ND,		_ND,	  _ND,		_DIG,	   _DIG,
      _DIG,	_DIG,	   _DIG,	  _DIG,		_DIG,	  _DIG,		_DIG,	   _DIG,

/*7*/ _ND,	_ND,	   _ND,		  _ND,		_ND,	   _ND,		_ND,	   _ND,
      _ND,	_ND,	   _ND,		  _ND,		_ND,	   _ND,		_ND,	   _ND, 
};


/*
 * Gujarati character class table
 */

static const guja_cls_t GujaChrClsTbl[128] = {

/*		0			1			2			3			4			5			6			7	
 		8			9			A			B			C			D			E			F */

/*0*/   _ND,		_GJ_SP_VM,	_GJ_SP_VM,	_GJ_N_VM,	_ND,		_GJ_V,		_GJ_V,		_GJ_V,
		_GJ_V,		_GJ_V,		_GJ_V,		_GJ_V,		_ND,		_GJ_V,		_ND,		_GJ_V,

/*1*/   _GJ_V,		_GJ_V,		_ND,		_GJ_V,		_GJ_V,		_GJ_N_CONS|_GJ_MS,	_GJ_N_CONS,	_GJ_N_CONS,
		_GJ_N_CONS,	_GJ_N_CONS,	_GJ_N_CONS, 	_GJ_N_CONS,	_GJ_N_CONS,	_GJ_N_CONS, 	_GJ_N_CONS,	_GJ_N_CONS|_GJ_MS,
			  
/*2*/	_GJ_N_CONS|_GJ_MS,	_GJ_N_CONS|_GJ_MS,	_GJ_N_CONS|_GJ_MS,	_GJ_N_CONS,	_GJ_N_CONS,	_GJ_N_CONS,	_GJ_N_CONS|_GJ_MS,	_GJ_N_CONS,
		_GJ_N_CONS,	_ND,		_GJ_N_CONS,	_GJ_N_CONS|_GJ_MS,	_GJ_N_CONS,	_GJ_N_CONS,	_GJ_N_CONS,	_GJ_N_CONS,
			  
/*3*/	_GJ_RA_CONS,	_ND,		_GJ_N_CONS,	_GJ_N_CONS,	_ND,		_GJ_N_CONS,	_GJ_N_CONS,	_GJ_N_CONS,
		_GJ_N_CONS,	_GJ_N_CONS|_GJ_MS,	_ND,		_ND,		_GJ_NUK,	_GJ_VED,	_GJ_N_M,	_GJ_I_M,

/*4*/	_GJ_II_M,		_GJ_N_M,	_GJ_N_M,	_GJ_N_M,	_GJ_N_M,	_GJ_AYE_M,	_ND,		_GJ_EY_M,
		_GJ_AI_M,	_GJ_AWE_M,	_ND,		_GJ_OW1_M,	_GJ_OW2_M,	_GJ_HAL,	_ND,		_ND,
		  
/*5*/	_GJ_VED,	_ND,		_ND,		_ND,		_ND,		_ND,		_ND,		_ND,
		_ND,		_ND,		_ND,		_ND,		_ND,		_ND,		_ND,		_ND,
			 
/*6*/	_GJ_V,		_ND,		_ND,		_ND,		_ND,		_ND,		_GJ_DIG,	_GJ_DIG,
		_GJ_DIG,	_GJ_DIG,	_GJ_DIG,	_GJ_DIG,	_GJ_DIG,	_GJ_DIG,	_GJ_DIG,	_GJ_DIG,

/*7*/	_ND,		_ND,		_ND,		_ND,		_ND,		_ND,		_ND,		_ND,
		_ND,		_ND,		_ND,		_ND,		_ND,		_ND,		_ND,		_ND, 

};


/*
 * Bengali character class table
 */

static const beng_cls_t BengChrClsTbl[128] = {

/*	0		1		2		3		4		5		6		7	*/

/* 	8		9		A		B		C		D		E		F	*/

/*0*/   _ND,		_BN_SP_VM,	_BN_N_VM,	_BN_N_VM,	_ND,		_BN_V,		_BN_V,		_BN_V,
	_BN_V,		_BN_V,		_BN_V,		_BN_V,		_BN_V,		_ND,		_ND,		_BN_V,

/*1*/   _BN_V,		_ND,		_ND,		_BN_V,		_BN_V,		_BN_N_CONS,	_BN_N_CONS	,	_BN_N_CONS,
	_BN_N_CONS,	_BN_N_CONS,	_BN_N_CONS, 	_BN_N_CONS,	_BN_N_CONS,	_BN_N_CONS, 	_BN_N_CONS,	_BN_N_CONS,
			  
/*2*/	_BN_N_CONS,	_BN_NUK_CONS,	_BN_NUK_CONS,	_BN_N_CONS,	_BN_N_CONS,	_BN_N_CONS,	_BN_N_CONS,	_BN_N_CONS,
	_BN_N_CONS,	_ND,		_BN_N_CONS,	_BN_N_CONS,	_BN_N_CONS,	_BN_N_CONS,	_BN_N_CONS,	_BN_NUK_CONS,
			  
/*3*/	_BN_RA_CONS,	_ND,		_BN_N_CONS,	_ND,	_ND,		_ND,	_BN_N_CONS,	_BN_N_CONS,
	_BN_N_CONS,	_BN_N_CONS,	_ND,		_ND,		_BN_NUK,	_ND,	_BN_N_M,	_BN_I_M,

/*4*/	_BN_II_M,		_BN_N_M,	_BN_N_M,	_BN_N_M,	_BN_N_M,	_ND,	_ND,		_BN_EY_M,
	_BN_AI_M,	_ND,	_ND,		_BN_OW1_M,	_BN_OW2_M,	_BN_HAL,	_ND,		_ND,
		  
/*5*/	_ND,	_ND,		_ND,		_ND,		_ND,		_ND,		_ND,		_BN_N_M|_BN_OW2_M,
	_ND,		_ND,		_ND,		_ND,		_BN_N_CONS,		_BN_N_CONS,		_ND,		_BN_N_CONS,
			 
/*6*/	_BN_MISC,		_BN_MISC,		_BN_MISC,		_BN_MISC,		_ND,		_ND,		_BN_DIG,	_BN_DIG,
	_BN_DIG,	_BN_DIG,	_BN_DIG,	_BN_DIG,	_BN_DIG,	_BN_DIG,	_BN_DIG,	_BN_DIG,

/*7*/	_BN_MISC,		_BN_MISC,		_BN_MISC,		_BN_MISC,		_BN_MISC,		_BN_MISC,		_BN_MISC,		_BN_MISC,
	_BN_MISC,		_BN_MISC,		_BN_MISC,		_ND,		_ND,		_ND,		_ND,		_ND, 
} ;


/*
 * Telugu character class table
 */

static const tlg_cls_t TlgChrClsTbl[128] = 
{

/*************************************************************************
		0		*1		*2		*3		*4		*5		*6		*7		*
**************************************************************************
			8		*9		*A		*B		*C		*D		*E		*F
***************************************************************************/
/*0C0*/_ND,	   _TLVM,  _TLVM,  _TLVM,   _ND,    _TLV,	_TLV,	_TLV,
		   _TLV,	_TLV,	_TLV,	_TLV,	_TLV,	_ND,	_TLV,	_TLV,

/*0C1*/_TLV,	_ND,	_TLV,	_TLV,	_TLV,	_TLC,	_TLC,	_TLC,
			_TLC,	_TLC,	_TLC,	_TLC,	_TLC,	_TLC,	_TLC,	_TLC,		

/*0C2*/_TLC,	_TLC,	_TLC,	_TLC,	_TLC,	_TLC,	_TLC,	_TLC,
 		   _TLC,	_ND,	_TLC,	_TLC,	_TLC,	_TLC,	_TLC,	_TLC,

/*0C3*/_TLC,	_TLC,	_TLC,	_TLC,	_ND,	_TLC,	_TLC,	_TLC,

			_TLC,	_TLC,	_ND,	_ND,	_ND,	_ND,	_TLM,	_TLM,
/*0C4*/_TLM,	_TLM,	_TLM,	_TLM,	_TLM,	_ND,	_TLM,	_TLM,
			_TLM,	_ND,	_TLM,	_TLM,	_TLM,	_TLH,	_ND,	_ND,

/*0C5*/_ND,		_ND,	_ND,	_ND,	_ND,  _TLMISC, _TLMISC, _ND,
			_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,

/*0C6*/_TLV,	_TLV,	_ND,	_ND,	_ND,	_ND,	_TLD,	_TLD,
			_TLD,	_TLD,	_TLD,	_TLD,	_TLD,	_TLD,	_TLD,	_TLD,

/*0C7*/_ND,		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,
			_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND
};



static const ml_cls_t MlChrClsTbl[128] = 
{

/************************************************************************
		0	*		1	*		2	*		3	*			4	*		5	*	6	*	7	*
*************************************************************************
		8	*		9	*		A	*		B	*			C	*		D	*	E	*	F
*************************************************************************/
//0D0
		_ND,		_ND,	_M_VM,			_M_VM,			_ND,	_M_V,	_M_V,		_M_V,
		_M_V,		_M_V,	_M_V,			_M_V,			_M_V,	_ND,	_M_V,		_M_V,
//0D1																					
		_M_V,		_ND,	_M_V,			_M_V,			_M_V,	_M_C,	_M_C,		_M_C,
		_M_C,		_M_C,	_M_C,			_M_C,			_M_C,	_M_C,	_M_C,		_M_C,
//0D2																					
		_M_C,		_M_C,	_M_C,			_M_C,			_M_C,	_M_C,	_M_C,		_M_C,	
		_M_C,		_ND,	_M_C,			_M_C,			_M_C,	_M_C,	_M_C,		_M_C,
//0D3																					
		_M_C,		_M_C,	_M_C,			_M_C,			_M_C,	_M_C,	_M_C,		_M_C,		
		_M_C,		_M_C,	_ND,			_ND,			_ND,	_ND,	_M_M,		_M_M,	
//0D4																					
		_M_M,		_M_M,	_M_M,			_M_M,			_ND,	_ND,	_M_M|_M_RM,	_M_M|_M_RM,
		_M_M|_M_RM,	_ND,	_M_M|_M_SM,		_M_M|_M_SM,	_M_M|_M_SM,	_M_H,	_ND,		_ND,
//0D5																					
		_ND,		_ND,	_ND,			_ND,			_ND,	_ND,	_ND,		_M_M,
		_ND,		_ND,	_ND,			_ND,			_ND,	_ND,	_ND,		_ND,	
//0D6																					
		_M_V,		_M_V,	_ND,			_ND,			_ND,	_ND,	_M_DIG,		_M_DIG,
		_M_DIG,		_M_DIG,	_M_DIG,			_M_DIG,			_M_DIG,	_M_DIG,	_M_DIG,		_M_DIG,
//0D7																					
		_ND,		_ND,	_ND,			_ND,			_ND,	_ND,	_ND,		_ND,
		_ND,		_ND,	_ND,			_ND,			_ND,	_ND,	_ND,		_ND
 };															




/*
 * Punjabi  character class table
 */

static const pn_cls_t PnChrClsTbl[128] = 
{
 
/************************************************************************
		0	*	1	*	2	*	3	*	4	*	5	*	6	*	7	*
*************************************************************************
		8	*    9	*	A	*	B	*	C	*	D	*	E	*	F
*************************************************************************/
//0A0
		_ND,	_ND,	_PVM,	_ND,	_ND,	_PV,	_PV,	_PV,	
		_PV,	_PV,	_PV,	_ND,	_ND,	_ND,	_ND,	_PV,
//0A1
		_PV,	_ND,	_ND,	_PV,	_PV,	_PNC,	_PNC,	_PNC,	
		_PC,	_PC,	_PC,	_PC,	_PNC,	_PC,	_PC,	_PC,
//0A2
		_PC,	_PNC,	_PNC,	_PC,	_PC,	_PC,	_PC,	_PC,
		_PC,	_ND,	_PC,	_PNC,	_PC,	_PC,	_PC,	_PC,
//0A3
		_PC,	_ND,	_PC,	_PC,	_ND,	_PC,	_PC,	_ND,	
		_PC,	_PC,	_ND,	_ND,	_PN,	_ND,	_PNM,	_P_IM,
//0A4
		_PNM,	_PNM,	_PNM,	_ND,	_ND,	_ND,	_ND,	_PNM,
		_PNM,	_ND,	_ND,	_PNM,	_PNM,	_PH,	_ND,	_ND,
//0A5
		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,					
		_ND,	_PC,	_PC,	_PC,	_PC,	_ND,	_PC,	_ND,
//0A6
		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_PDIG,	_PDIG,
		_PDIG,	_PDIG,	_PDIG,	_PDIG,	_PDIG,	_PDIG,	_PDIG,	_PDIG,
//0A7
		_PMISC,	_PMISC,	_PMISC,	_PMISC,	_PMISC,	_ND,	_ND,	_ND,																						
		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,
 };															



/*
 * Thai character type table
 */
static const int ThaiChrTypeTbl[128] = {
  /*	   0,	1,   2,	  3,   4,   5,	 6,   7,
	   8,	9,   A,	  B,   C,   D,	 E,   F	 */

  /*00*/ __CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,
	 __CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,
  /*10*/ __CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,
	 __CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,__CTRL,
  /*20*/  __NON,__CONS,__CONS,__CONS,__CONS,__CONS,__CONS,__CONS,
	 __CONS,__CONS,__CONS,__CONS,__CONS,__CONS,__CONS,__CONS,
  /*30*/ __CONS,__CONS,__CONS,__CONS,__CONS,__CONS,__CONS,__CONS,
	 __CONS,__CONS,__CONS,__CONS,__CONS,__CONS,__CONS,__CONS,
  /*40*/ __CONS,__CONS,__CONS,__CONS, __FV3,__CONS, __FV3,__CONS,
	 __CONS,__CONS,__CONS,__CONS,__CONS,__CONS,__CONS, __NON,
  /*50*/  __FV1, __AV2, __FV1, __FV1, __AV1, __AV3, __AV2, __AV3,
	  __BV1, __BV2,  __BD, __NON, __NON, __NON, __NON, __NON,
  /*60*/   __LV,  __LV,  __LV,  __LV,  __LV, __FV2, __NON, __AD2,
	 __TONE,__TONE,__TONE,__TONE, __AD1, __AD1, __AD3, __NON,
  /*70*/  __NON, __NON, __NON, __NON, __NON, __NON, __NON, __NON,
	  __NON, __NON, __NON, __NON, __NON, __NON, __NON,__CTRL,
};

/*
 * Hebrew character type table
 */
static const int HebrewChrTypeTbl[128] = {
  /*	    0,	    1,    2,    3,    4,     5,    6,    7,
            8,      9,    A,    B,    C,     D,    E,    F, */

  /*00*/ __ND,   __ND, __ND, __ND, __ND,  __ND, __ND, __ND,
	 __ND,   __ND, __ND, __ND, __ND,  __ND, __ND, __ND,

  /* Cantillation */
  /*10*/ __ND,   __NS, __NS, __NS, __NS,  __NS, __NS, __NS,
  	 __NS,   __NS, __NS, __NS, __NS,  __NS, __NS, __NS,
  /*20*/ __NS,   __NS, __ND, __NS, __NS,  __NS, __NS, __NS,
  	 __NS,   __NS, __NS, __NS, __NS,  __NS, __NS, __NS,

  /* Punctuation */
  /*30*/ __NS,   __NS, __NS, __NS, __NS,  __NS, __NS, __NS,
  	 __NS,   __NS, __ND, __NS, __DA,  __NS, __SP, __NS,
  /*40*/ __SP,   __NS, __NS, __SP, __NS,  __ND, __ND, __ND,
  	 __ND,   __ND, __ND, __ND, __ND,  __ND, __ND, __ND,

  /* Consonance */
  /*50*/ __ALEF, __SP, __SP, __SP, __SP,  __SP, __SP, __SP,
  	 __SP,   __SP, __SP, __SP,__LAM,  __SP, __SP, __SP,
  /*60*/ __SP,   __SP, __SP, __SP, __SP,  __SP, __SP, __SP,
  	 __SP,   __SP, __SP, __ND, __ND,  __ND, __ND, __ND,

  /* Yiddish */
  /*70*/ __SP,   __SP, __SP, __SP, __SP,  __ND, __ND, __ND,
	 __ND,   __ND, __ND, __ND, __ND,  __ND, __ND, __ND,
};

/*
 * Arabic character type table
 */
static const int ArabicChrTypeTbl[128] = {
  /*	   0,	1,   2,	  3,   4,   5,	 6,   7 
	   8,	9,   A,	  B,   C,   D,	 E,   F	 */

  /*00*/ __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
	 __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
  /*10*/ __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
	 __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
  /*20*/ __ND, __ND, __AL, __AL, __ND, __AL, __ND, __AL,
	 __4F, __2F, __4F, __4F, __4F, __4F, __4F, __2F,
  /*30*/ __2F, __2F, __2F, __4F, __4F, __4F, __4F, __ND,
	 __ND, __4F, __4F, __ND, __ND, __ND, __ND, __ND,
  /*40*/ __ND, __4F, __4F, __4F, __LM, __4F, __4F, __4F,
	 __2F, __2F, __4F, __VW, __VW, __VW, __VW, __VW,
  /*50*/ __VW, __SD, __VW, __ND, __ND, __ND, __ND, __ND,
	 __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
  /*60*/ __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
	 __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
  /*70*/ __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
	 __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
};

/*
 * Devanagari character type table
 */
static const int DevaChrTypeTbl[128] =
{
/*         0,    1,    2,    3,    4,    5,    6,    7,
           8,    9,    A,    B,    C,    D,    E,    F, */
/* 0 */ __ND, __UP, __UP, __NP, __ND, __IV, __IV, __IV,
        __IV, __IV, __IV, __IV, __IV, __IV, __IV, __IV,
/* 1 */ __IV, __IV, __IV, __IV, __IV, __CK, __CK, __CK,
        __CN, __CN, __CN, __CN, __CK, __CN, __CN, __CN,
/* 2 */ __CN, __CK, __CK, __CN, __CN, __CN, __CN, __CN,
        __CN, __CN, __CN, __CK, __CN, __CN, __CN, __CN,
/* 3 */ __RC, __CN, __CN, __CN, __CN, __CN, __CN, __CN,
        __CN, __CN, __ND, __ND, __NK, __VD, __NM, __IM,
/* 4 */ __RM, __NM, __NM, __NM, __NM, __RM, __RM, __RM,
        __RM, __RM, __RM, __RM, __RM, __HL, __ND, __ND,
/* 5 */ __ND, __VD, __VD, __VD, __VD, __ND, __ND, __ND,
        __CN, __CN, __CN, __CN, __CN, __CN, __CN, __CN,
/* 6 */ __IV, __IV, __NM, __NM, __ND, __ND, __HD, __HD,
        __HD, __HD, __HD, __HD, __HD, __HD, __HD, __HD,
/* 7 */ __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
        __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
};


/*
 * Tamil Character type table
 */
static const int TamilChrTypeTbl[128] = 
{
/*		  0		  1		  2		  3		  4		  5		  6		  7		
		  8		  9		  A		  B		  C		  D		  E		  F */
/*0B8 */ _ND,	 _ND,	__VM,	__VM,	 _ND,	__TMV,	__TMV,	__TMV,
		__TMV,	__TMV,	__TMV,	 _ND,	 _ND,	 _ND,	__TMV,	__TMV,
/*0B9 */__TMV,	 _ND,	__TMV,	__TMV,	__TMV,	__TMC,	 _ND,	 _ND,	
		 _ND,	__TMC,	__TMC,	 _ND,	__TMC,	 _ND,	__TMC,	__TMC,
/*0BA */ _ND,	 _ND,	 _ND,	__TMC,	__TMC,	 _ND,	 _ND,	 _ND,
		__TMC,	__TMC,	__TMC,	 _ND,	 _ND,	 _ND,	__TMC,	__TMC,
/*0BB */__TMC,	__TMC,	__TMC,	__TMC,	__TMC,	__TMC,	 _ND,	__TMC,
		__TMC,	__TMC,	 _ND,  	 _ND,    _ND,	 _ND,	__TMM,	__TMM,
/*0BC */__TMM,	__TMM,	__TMM,	 _ND,	 _ND,	 _ND,	__TMR,	__TMR,
		__TMR,	 _ND,	__TMS,	__TMS,	__TMS,	__TMH,	 _ND,	 _ND,
/*0BD */ _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 __TMP,	
		 _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND,		
/*0BE */ _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND,	__TMD,
		__TMD,	__TMD,	__TMD,	__TMD,	__TMD,	__TMD,	__TMD,	__TMD,
/*0BF */__TMD,	__TMD,	__TMD,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND,
		 _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND,	 _ND
};



/*
 * Kannada Character type table
 */
static const int KannChrTypeTbl[128] =
{

/* *  0 *	*   1   * *   2  * *  3  *  *   4 * *   5  *     *   6  * *  7  *     
   *  8 *	*   9   * *   A  * *  B  *  *   C * *   D  *     *   E  * *  F  **/

/*0*/  _ND,	 _ND,	   __VM,	__VM,	   _ND,	   	__V,	    __V,	__V,
       __V,	 __V,	   __V,		__V,	   __M,	   	_ND,	    __V,	__V,

/*1*/  __V,	 _ND,	   __V,		__V,	   __V,	   	__C,	    __C,	__C,
       __C,	 __C,	   __C,		__C,	   __NC,   	__C,	    __C,	__C,

/*2*/  __C,	 __C,	   __C,		__C,	   __C,	  	__C,	    __C,	__C,
       __C,	 _ND,	   __C,		__NC,	   __C,	   	__C,	    __C,	__C,

/*3*/  __C,	 __C,	   __C,		__C,	   _ND,	   	__C,	    __C,	__C,
       __C,	 __C,	   _ND,		_ND,	   _ND,	   	_ND,	    __M,	__M,

/*4*/  __M,	 __M,	   __M,		__M,	   __M,	   	_ND,	    __M,	__M,
       __M,	 _ND,	   __M,		__M,	   __M,	   	__H,	    _ND,	_ND,

/*5*/  _ND,	 _ND,	   _ND,		_ND,	   _ND,   	__M,	    __M,	_ND,
       _ND,	 _ND,	   _ND,		_ND,	   _ND,	   	_ND,	    __M,	_ND,

/*6*/  __V,	 __M,	   _ND,		_ND,	   _ND,    	_ND,	    __D,	__D,
       __D,	 __D,	   __D,		__D,	   __D,	    __D,	    __D,	__D,

/*7*/  _ND,	 _ND,	   _ND,		_ND,	   _ND,	   	_ND,	    _ND,	_ND,
       _ND,	 _ND,	   _ND,		_ND,	   _ND,	   	_ND,	    _ND,	_ND,
};


/*
 * Gujarati Character type table
 */
static const int GujaChrTypeTbl[128] =
{

/*		0		1			2			3			4		5		6		7	
 		8		9			A			B			C		D		E		F */
/*0*/   _ND,	__GJ_VM,	__GJ_VM,	__GJ_VM,	_ND,	__GJ_V,	__GJ_V,	__GJ_V,
		__GJ_V,	__GJ_V,		__GJ_V,		__GJ_V,		_ND,	__GJ_V,	_ND,	__GJ_V,

/*1*/   __GJ_V,	__GJ_V,		_ND,		__GJ_V,		__GJ_V,	__GJ_C,	__GJ_C,	__GJ_C,
		__GJ_C,	__GJ_C,		__GJ_C,		__GJ_C,		__GJ_C,	__GJ_C,	__GJ_C,	__GJ_C,

/*2*/   __GJ_C,	__GJ_C,		__GJ_C,		__GJ_C,		__GJ_C,	__GJ_C,	__GJ_C,	__GJ_C,
		__GJ_C,	_ND,		__GJ_C,		__GJ_C,		__GJ_C,	__GJ_C,	__GJ_C,	__GJ_C,

/*3*/   __GJ_RC,	_ND,		__GJ_C,		__GJ_C,		_ND,	__GJ_C,	__GJ_C,	__GJ_C,
		__GJ_C,	__GJ_C,		_ND,		_ND,		__GJ_N,	__GJ_VD,__GJ_M,	__GJ_IM,

/*4*/   __GJ_M,	__GJ_M,		__GJ_M,		__GJ_M,		__GJ_M,	__GJ_M,	_ND,	__GJ_M,
		__GJ_M,	__GJ_M,		_ND,		__GJ_M,		__GJ_M,	__GJ_H,	_ND,	_ND,

/*5*/   __GJ_VD,_ND,		_ND,		_ND,		_ND,	_ND,	_ND,	_ND,
		_ND,	_ND,		_ND,		_ND,		_ND,	_ND,	_ND,	_ND,

/*6*/   __GJ_V,	_ND,		_ND,		_ND,		_ND,	_ND,	__GJ_D,	__GJ_D,
		__GJ_D,	__GJ_D,		__GJ_D,		__GJ_D,		__GJ_D,	__GJ_D,	__GJ_D,	__GJ_D,

/*7*/   _ND,	_ND,		_ND,		_ND,		_ND,	_ND,	_ND,	_ND,
		_ND,	_ND,		_ND,		_ND,		_ND,	_ND,	_ND,	_ND,
};

/*
 * Bengali Character type table
 */
static const int BengChrTypeTbl[128] =
{
//	0			1			2			3			4		5		6		7	
// 	8			9			A			B			C		D		E		F

/*0*/   _ND,		__BN_VM,	__BN_VM,	__BN_VM,	_ND,	__BN_V,	__BN_V,	__BN_V,
		__BN_V,		__BN_V,		__BN_V,		__BN_V,		__BN_V,	_ND,	_ND,	__BN_V,

/*1*/   __BN_V,		_ND,		_ND,		__BN_V,		__BN_V,	__BN_C,	__BN_C,	__BN_C,
		__BN_C,		__BN_C,		__BN_C,		__BN_C,		__BN_C,	__BN_C,	__BN_C,	__BN_C,
	
/*2*/   __BN_C,		__BN_C,		__BN_C,	__BN_C,		__BN_C,	__BN_C,	__BN_C,	__BN_C,
		__BN_C,		_ND,		__BN_C,		__BN_C,		__BN_C,	__BN_C,	__BN_C,	__BN_C,
	
/*3*/   __BN_RC,	_ND,		__BN_C,		_ND,		_ND,	_ND,	__BN_C,	__BN_C,
		__BN_C,		__BN_C,		_ND,		_ND,		__BN_N,	_ND,	__BN_M,__BN_IM,
	
/*4*/   __BN_M,		__BN_M,		__BN_M,		__BN_M,		__BN_M,	_ND,	_ND,	__BN_IM,
		__BN_IM,	_ND,		_ND,		__BN_IM,	__BN_IM,__BN_H,	_ND,	_ND,

/*5*/   _ND,		_ND,		_ND,		_ND,		_ND,	_ND,	_ND,	__BN_MSC,
		_ND,		_ND,		_ND,		_ND,		__BN_C,	__BN_C,	_ND,	__BN_C,

/*6*/   __BN_MSC,	__BN_MSC,	__BN_MSC,	__BN_MSC,	_ND,	_ND,	__BN_D,	__BN_D,
		__BN_D,		__BN_D,		__BN_D,		__BN_D,		__BN_D,	__BN_D,	__BN_D,	__BN_D,
	
/*7*/   __BN_MSC,	__BN_MSC,	__BN_MSC,	__BN_MSC,	__BN_MSC,	__BN_MSC,	__BN_MSC,	__BN_MSC,
		__BN_MSC,	__BN_MSC,	__BN_MSC,	_ND,		_ND,	_ND,	_ND,	_ND,
};

/*
 * Telugu Character type table
 */

static const int TlgChrTypeTbl[128] = 
{

/* ########################################################################
	    0	#	1	#	2	#	3	#	4	#	5	#	6	#	7		
###########################################################################
			8	#	9	#	A	#	B	#	C	#	D	#	E	#	F
############################################################################ */

/*0C0*/	_ND,  __TLVM, __TLVM, __TLVM,   _ND,   __TLV,  __TLV,  __TLV,		
		  __TLV,  __TLV,  __TLV,  __TLV,  __TLV,   _ND,	   __TLV,   __TLV,
	
/*0C1*/__TLV,  _ND,	   __TLV, __TLV,  __TLV,   __TLC,  __TLC,   __TLC,
		  __TLC,   __TLC,  __TLC,  __TLC,  __TLC,  __TLC,  __TLC,  __TLC,
		
/*0C2*/__TLC,  __TLC,  __TLC,  __TLC,  __TLC,   __TLC,  __TLC,  __TLC,
		  __TLC,   _ND,	   __TLC,  __TLC,  __TLC,  __TLC,  __TLC,  __TLC,
		
/*0C3*/__TLC,  __TLC,  __TLC,  __TLC,	_ND,	__TLC,	__TLC,	__TLC,		
		  __TLC,  __TLC,	_ND,	_ND,	_ND,	_ND,   __TLM,   __TLM,
		
/*0C4*/__TLM,  __TLM,  __TLM,  __TLM,  __TLM,	 _ND,	__TLM,	__TLM,
		  __TLM,	_ND,   __TLM,  __TLM,	__TLM,  __TLH,	_ND,	_ND,
		
/*0C5*/_ND,	   _ND,		 _ND,	 _ND,  _ND, __TLMISC, __TLMISC,  _ND,
			_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,
		
/*0C6*/__TLV, __TLV,	_ND,	_ND,	_ND,	_ND,	__TLD,	 __TLD,
			__TLD,	__TLD,	__TLD,	__TLD,	__TLD,	__TLD,	__TLD,	__TLD,

/*0C7*/_ND,		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,						
			_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,
		
 };

/*
 * Malayalam Character type table
 */

static const int MlChrTypeTbl[128] = 
{

/* #######################################################################
		0	#	1	#	2	#	3	#	4	#	5	#	6	#	7		
##########################################################################
		8	#   9	#	A	#	B	#	C	#	D	#	E	#	F
########################################################################## */

//0D0	
		_ND,	_ND,	__MLVM,	__MLVM,	_ND,	__MLV,	__MLV,	__MLV,	
		__MLV,	__MLV,	__MLV,	__MLV,	__MLV,	_ND,	__MLV,	__MLV,
//0D1
		__MLV,	_ND,	__MLV,	__MLV,	__MLV,	__MLC,	__MLC,	__MLC,
		__MLC,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,
//0D2
		__MLC,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,
		__MLC,	_ND,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,
//0D3
		__MLC,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,	__MLC,
		__MLC,	__MLC,	_ND,	_ND,	_ND,	_ND,	__MLM,	__MLM,	
//0D4
		__MLM,	__MLM,	__MLM,	__MLM,	_ND,	_ND,	__MLM,	__MLM,
		__MLM,	_ND,	__MLM,	__MLM,	__MLM,	__MLH,	_ND,	_ND,
//0D5
		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	__MLM,
		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	
//0D6
		__MLV,	__MLV,	_ND,	_ND,	_ND,	_ND,	__MLD,	__MLD,
		__MLD,	__MLD,	__MLD,	__MLD,	__MLD,	__MLD,	__MLD,	__MLD,	
//0D7
		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,
		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,

 };


/*
 * Punjabi Character type table
 */

static const int PnChrTypeTbl[128] = 
{

/* #######################################################################
		0	#	1	#	2	#	3	#	4	#	5	#	6	#	7		
##########################################################################
		8	#   9	#	A	#	B	#	C	#	D	#	E	#	F
########################################################################## */

//0A0	
		_ND,	_ND,	__PVM,	_ND,	_ND,	__PV,	__PV,	__PV,
		__PV,	__PV,	__PV,	_ND,	_ND,	_ND,	_ND,	__PV,
//0A1
		__PV,	_ND,	_ND,	__PV,	__PV,	__PNC,	__PNC,	__PNC,
		__PC,	__PC,	__PC,	__PC,	__PNC,	__PC,	__PC,	__PC,	
//0A2
		__PC,	__PNC,	__PNC,	__PC,	__PC,	__PC,	__PC,	__PC,
		__PC,	_ND,	__PC,	__PNC,	__PC,	__PC,	__PC,	__PC,	
//0A3
		__PC,	_ND,	__PC,	__PC,	_ND,	__PC,	__PC,	_ND,	
		__PC,	__PC,	_ND,	_ND,	__PN,	_ND,	__PM,	__PIM,
//0A4
		__PM,	__PM,	__PM,	_ND,	_ND,	_ND,	_ND,	__PM,
		__PM,	_ND,	_ND,	__PM,	__PM,	__PH,	_ND,	_ND,
//0A5
		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	
		_ND,	__PC,	__PC,	__PC,	__PC,	_ND,	__PC,	_ND,
//0A6
		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	__PD,	__PD,
		__PD,	__PD,	__PD,	__PD,	__PD,	__PD,	__PD,	__PD,
//0A7
		__PMIS,	__PMIS,	__PMIS,	__PMIS,	__PMIS,	_ND,	_ND,	_ND,
		_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,	_ND,
};

/*
 * Thai character composible table
 */
static const int ThaiComposeTbl[17][17] = {
      /* Cn */ /* 0, 1, 2, 3, 4, 5, 6, 7,   8, 9, A, B, C, D, E, F  */
  /* Cn-1 00 */ { 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* 10 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* 20 */	{ 0, 0, 0, 0, 0, 0, 0, 1,   1, 1, 1, 1, 1, 1, 1, 1, 1 },
  /* 30 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* 40 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* 50 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* 60 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* 70 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 1, 1, 0, 0, 0, 0, 0 },
  /* 80 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 1, 0, 0, 0, 0, 0, 0 },
  /* 90 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* A0 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* B0 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* C0 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* D0 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* E0 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 1, 1, 0, 0, 0, 0, 0 },
  /* F0 */	{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 1, 0, 0, 0, 0, 0, 0 },
  		{ 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 1, 0, 1, 0, 0, 0, 0 },
};

/*
 * Hebrew character composible table
 */
static const int HebrewComposeTbl[6][6] = {
            /*   ND,SP,AL,LM,NS,DA  */
      /* Cn */ /* 0, 1, 2, 3, 4, 5, */
/* Cn-1 0 */	{ 0, 0, 0, 0, 0, 0 }, /* ND */
  /* 1 */	{ 0, 0,	0, 0, 1, 1 }, /* SP */
#ifdef	HEBREW_PRESENT_FORM
  /* 2 */	{ 0, 0, 0, 1, 1, 1 }, /* AL */
#endif
  /* 2 */	{ 0, 0, 0, 0, 1, 1 }, /* AL */
  /* 3 */       { 0, 0, 0, 0, 1, 1 }, /* LM */
  /* 4 */	{ 0, 0, 0, 0, 0, 0 }, /* NS */
  /* 5 */	{ 0, 0,	0, 0, 1, 0 }, /* DA */
};

/*
 * Arabic character composible table
 */
static const int ArabicComposeTbl[7][7] = {
      /* Cn */ /* 0, 1, 2, 3, 4, 5, 6 */
/* Cn-1 00 */	{ 0, 0, 0, 0, 0, 0, 0 },
  /* 10 */	{ 0, 0,	1, 1, 1, 0, 0 },
  /* 20 */	{ 0, 0, 0, 1, 1, 0, 0 },
  /* 30 */	{ 0, 0,	0, 0, 1, 0, 0 },
  /* 40 */	{ 0, 0, 0, 0, 0, 0, 0 },
  /* 50 */	{ 0, 0, 0, 1, 1, 0, 1 },
  /* 60 */	{ 0, 0, 0, 1, 1, 0, 0 },
};

/*
 * Devanagari character composible table
 */
static const int DevaComposeTbl[14][14] = {
  /*      ND, UP, NP, IV, CN, CK, RC, NM, RM, IM, HL, NK, VD, HD, */
  /* 0  */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* ND */
  /* 1  */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* UP */
  /* 2  */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* NP */
  /* 3  */ 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* IV */
  /* 4  */ 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0, /* CN */
  /* 5  */ 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  1,  0,  0, /* CK */
  /* 6  */ 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0, /* RC */
  /* 7  */ 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* NM */
  /* 8  */ 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* RM */
  /* 9  */ 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* IM */
  /* 10 */ 0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0, /* HL */
  /* 11 */ 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0, /* NK */
  /* 12 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* VD */
  /* 13 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* HD */
};


/*
 * Tamil character composible table
 */
static const int TamilComposeTbl[10][10] = {

/*			ND		VM		TMV		TMC		TMM		TMR		TMS		TMH		TMD		TMP */
/*# ND */   0,		0,		0,	 	0,		0,		0,		0,		0,		0,		0,		
/*# VM */	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*# TMV*/	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*# TMC*/	0,		0,		0,		0,		1,		1,		1,		1,		0,		0,
/*# TMM*/	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*# TMR*/	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*# TMS*/	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*# TMH*/	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*# TMD*/	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*# TMP*/	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,

};



/*
 * Kannada character composible table
 */
static const int KannComposeTbl[10][10] = {
/*     *  	0	1	2	3	4	5	6	7	8	*	
/*        	VM	V	C	NC	M	H	N	VD	D	*/
/* 0 */0, 	0,	0,	0,	0,	0,	0,	0,	0,	0,/**/
/* 1 */0, 	0,	0,	0,	0,	0,	0,	0,	0,	0,/*VM*/
/* 2 */0, 	1,	0,	0,	0,	0,	0,	0,	0,	0,/*V */
/* 3 */0, 	1,	0,	0,	0,	1,	1,	0,	0,	0,/*C */
/* 4 */0, 	1,	0,	0,	0,	1,	1,	1,	0,	0,/*NC*/
/* 5 */0, 	1,	0,	0,	0,	0,	0,	0,	0,	0,/*M */
/* 6 */0, 	0,	0,	1,	1,	0,	0,	0,	0,	0,/*H */
/* 7 */0, 	1,	0,	0,	0,	1,	1,	0,	0,	0,/*N */
/* 8 */0, 	0,	0,	0,	0,	0,	0,	0,	0,	0,/*VD*/
/* 9 */0, 	0,	0,	0,	0,	0,	0,	0,	0,	0,/*D*/
};


/*
 * Gujarati character composible table
 */
static const int GujaComposeTbl[11][11] = {

/*     0	1	2	3	4	5	6	7	8	9	10	*/
/*     		VM	V	C	RC	M	IM	H	N	VD	D	*/

/* 0 */0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/****/
/* 1 */0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*VM*/
/* 2 */0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*V */
/* 3 */0,	1,	0,	0,	0,	1,	1,	1,	0,	0,	0,	/*C */
/* 4 */0,	1,	0,	0,	0,	1,	1,	1,	0,	0,	0,	/*RC */
/* 5 */0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*M */
/* 6 */0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*IM */
/* 7 */0,	0,	0,	1,	1,	0,	0,	0,	0,	0,	0,	/*H */
/* 8 */0,	1,	0,	0,	0,	1,	1,	1,	0,	0,	0,	/*N */
/* 9 */0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*VD*/
/* 10*/0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*D */

};


/*
 * Bengali character composible table
 */
static const int BengComposeTbl[11][11] = {
//     0	1	2	3	4	5	6	7	8	9	10
/*     ND	VM	V	C	RC	M	IM	H	N	VD	D
/* 0 */0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/****/
/* 1 */0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*VM*/
/* 2 */0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*V */
/* 3 */0,	1,	0,	0,	0,	1,	1,	1,	0,	0,	0,	/*C */
/* 4 */0,	1,	0,	0,	0,	1,	1,	1,	1,	0,	0,	/*RC*/
/* 5 */0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*M */
/* 6 */0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*IM */
/* 7 */0,	0,	0,	1,	1,	0,	0,	0,	0,	0,	0,	/*H */
/* 8 */0,	1,	0,	0,	0,	1,	1,	1,	0,	0,	0,	/*N */
/* 9 */0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*VD*/
/* 10*/0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	/*D */
};


/*
 * Telugu character composible table
 */
static const int TlgComposeTbl[MAX_TLG_TYPE][MAX_TLG_TYPE] = 
{
/*			ND		TLVM	TLV		TLC		TLM		TLH		TLD		TLMISC */
/*######### 0  ##### 1 ##### 2  #### 3 ##### 4 #### 5  #### 6  ##### 7  ######## */
/*ND*/      0,		0,		0,		0,		0,		0,		0,		0,

/*TLVM*/	0,		0,		0,		0,		0,		0,		0,		0,

/*TLV*/		0,		1,		0,		0,		0,		0,		0,		0,

/*TLC*/		0,		1,		0,		0,		1,		1,		0,		0,

/*TLM*/		0,		1,		0,		0,		0,		0,		0,		0,

/*TLH*/		0,		0,		0,		1,		0,		0,		0,		0,

/*TLD*/		0,		0,		0,		0,		0,		0,		0,		0,

/*TLMaISC*/	0,		0,		0,		0,		0,		0,		0,		0,

};

// Only for Telugu
static const int TlgVattuComposeTbl[3][3] =
{
/*			0	BOT		SIDE */
/* 0*/		0,	1,		1,	
/*BOT*/		0,	0,		1,	
/*SIDE*/	0,	0,		1,
};



static const int MlComposeTbl[MAX_ML_TYPE][MAX_ML_TYPE] = 
{
/*############################################################################
			_ND		__MLVM	__MLV	__MLC	__MLM	__MLH	__MLD	__MLMIS
############################################################################## */
//_ND
			0,		0,		0,		0,		0,		0,		0,		0,
//__MLVM
			0,		0,		0,		0,		0,		0,		0,		0,	
//__MLV
			0,		1,		0,		0,		0,		0,		0,		0,
//__MLC
			0,		1,		0,		0,		1,		1,		0,		0,
//__MLM
			0,		1,		0,		0,		0,		0,		0,		0,	
//__MLH
			0,		0,		0,		1,		0,		0,		0,		0,
//__MLD
			0,		0,		0,		0,		0,		0,		0,		0,
//__MLMIS
			0,		0,		0,		0,		0,		0,		0,		0,
};


/*
 * Punjabi character composible table
 */
static const int PnComposeTbl[MAX_PN_TYPE][MAX_PN_TYPE] = 
{
			/* C-DAC FILLING Here */
/*##############################################################################################
			_ND	__PVM	__PV	__PC	__PNC	__PN	__PNM	__PIM	__PH	__PD	__PMIS
################################################################################################ */
/* _ND  */	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*__PVM	*/	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		
/*__PV  */	0,		1,		0,		0,		0,		0,		0,		0,		0,		0,		0,	
/*__PC	*/	0,		1,		0,		0,		0,		0,		1,		1,		1,		0,		0,
/*__PNC */	0,		1,		0,		0,		0,		1,		1,		1,		1,		0,		0,
/*__PN  */ 	0,		1,		0,		0,		0,		0,		1,		1,		1,		0,		0,
/*__PNM  */	0,		1,		0,		0,		0,		0,		0,		0,		0,		0,		0,		
/*__PIM */	0,		1,		0,		0,		0,		0,		0,		0,		0,		0,		0,		
/*__PH	*/	0,		0,		0,		1,		1,		0,		0,		0,		0,		0,		0,
/*__PD  */	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*__PMIS*/	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,

/*############################################################################################*/

};



/*
 * Thai Glyph Tables
 */
static const ThaiGlyphTable ThaiGlyphTbl = {
    { 0xE7, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0xED, 0xEE },
    { 0x9A, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x99, 0xEE },
    { 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0x99, 0xEE },
    { 0x98, 0x00, 0x00, 0x81, 0x82, 0x83, 0x84 },
    { 0xFC, 0xFD, 0xFE },
    { 0x90, 0x00, 0x00, 0x80 },
};

/*
 * Hebrew Glyph Tables
 */
static const HebrewGlyphTable HebrewGlyphTbl = {
 /* Consonance with Dagesh */
 {
  /* 00 */    0x0000, 0xFB31, 0xFB32, 0xFB33, 0xFB34, 0xFB35, 0xFB36, 0x0000,
              0xFB38, 0xFB39, 0xFB3A, 0xFB3B, 0xFB3C, 0x0000, 0xFB3E, 0x0000,
  /* 10 */    0xFB40, 0xFB41, 0x0000, 0xFB43, 0xFB44, 0x0000, 0xFB46, 0xFB47,
              0xFB48, 0xFB49, 0xFB4A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 },

 /* Hebrew-Alef + [QAMATS/HIRIQ/PATAH] Dot */
 { 0xFB30, 0x0000, 0x0000, 0xFB2E, 0xFB2F },

 /* Consonance with RAFE */
 {
  /* 00 */    0x0000, 0xFB4C, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0xFB4D, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 10 */    0x0000, 0x0000, 0x0000, 0x0000, 0xFB4E, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 },

 /* Cant/Point/Punctuation for WIDE consonances */
 {
  /* 00 */    0x0000, 0x0591, 0x0592, 0x0593, 0x0594, 0x0595, 0x0596, 0x0597,
              0x0598, 0x0599, 0x059A, 0x059B, 0x059C, 0x059D, 0x059E, 0x059F,
  /* 10 */    0x05A0, 0x05A1, 0x0000, 0x05A3, 0x05A4, 0x05A5, 0x05A6, 0x05A7,
              0x05A8, 0x05A9, 0x05AA, 0x05AB, 0x05AC, 0x05AD, 0x05AE, 0x05AF,
  /* 20 */    0x05B0, 0x05B1, 0x05B2, 0x05B3, 0x05B4, 0x05B5, 0x05B6, 0x05B7,
              0x05B8, 0x05B9, 0x0000, 0x05BB, 0x05BC, 0x05BD, 0x05BE, 0x05BF,
  /* 30 */    0x05C0, 0x05C1, 0x05C2, 0x05C3, 0x05C4, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 },

 /* Cant/Point/Punctuation for NARROW consonances */
 {
  /* 00 */    0x0000, 0x0591, 0x0592, 0x0593, 0x0594, 0x0595, 0x0596, 0x0597,
              0x0598, 0x0599, 0x059A, 0x059B, 0x059C, 0x059D, 0x059E, 0x059F,
  /* 10 */    0x05A0, 0x05A1, 0x0000, 0x05A3, 0x05A4, 0x05A5, 0x05A6, 0x05A7,
              0x05A8, 0x05A9, 0x05AA, 0x05AB, 0x05AC, 0x05AD, 0x05AE, 0x05AF,
  /* 20 */    0xF8E0, 0xF8E1, 0xF8E2, 0xF8E3, 0xF8E4, 0xF8E5, 0xF8E6, 0xF8E7,
              0xF8E8, 0xF8E9, 0x0000, 0xF8EB, 0xF8EC, 0xF8ED, 0x05BE, 0xF8EF,
  /* 30 */    0x05C0, 0xF8F1, 0xF8F2, 0x05C3, 0xF8F4, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 },
 
};

/*
 * Arabic Glyph Tables
 */
static const ArabicGlyphTable ArabicGlyphTbl = {
  /* Isolated Form */
  { 
  /* 00 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x060C, 0x0000, 0x0000, 0x0000,
  /* 10 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x061B, 0x0000, 0x0000, 0x0000, 0x061F,
  /* 20 */    0x0000, 0xFE80, 0xFE81, 0xFE83, 0xFE85, 0xFE87, 0xFE89, 0xFE8D,
              0xFE8F, 0xFE93, 0xFE95, 0xFE99, 0xFE9D, 0xFEA1, 0xFEA5, 0xFEA9,
  /* 30 */    0xFEAB, 0xFEAD, 0xFEAF, 0xFEB1, 0xFEB5, 0xFEB9, 0xFEBD, 0xFEC1,
              0xFEC5, 0xFEC9, 0xFECD, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 40 */    0x0640, 0xFED1, 0xFED5, 0xFED9, 0xFEDD, 0xFEE1, 0xFEE5, 0xFEE9,
              0xFEED, 0xFEEF, 0xFEF1, 0xFE70, 0xFE72, 0xFE74, 0xFE76, 0xFE78,
  /* 50 */    0xFE7A, 0xFE7C, 0xFE7E, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 60 */    0x0660, 0x0661, 0x0662, 0x0663, 0x0664, 0x0665, 0x0666, 0x0667,
	      0x0668, 0x0669, 0x066A, 0x066B, 0x066C, 0x066D, 0x0000, 0x0000,
  /* 70 */    0x0670, 0x0671, 0x0672, 0x0673, 0x0674, 0x0675, 0x0676, 0x0677,
	      0x0678, 0x0679, 0x067A, 0x067B, 0x067C, 0x067D, 0x067E, 0x067F,
  },

  /* Initial Form */
  {
  /* 00 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 10 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 20 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFE8B, 0x0000,
              0xFE91, 0x0000, 0xFE97, 0xFE9B, 0xFE9F, 0xFEA3, 0xFEA7, 0x0000,
  /* 30 */    0x0000, 0x0000, 0x0000, 0xFEB3, 0xFEB7, 0xFEBB, 0xFEBF, 0xFEC3,
              0xFEC7, 0xFECB, 0xFECF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 40 */    0x0000, 0xFED3, 0xFED7, 0xFEDB, 0xFEDF, 0xFEE3, 0xFEE7, 0xFEEB,
              0x0000, 0x0000, 0xFEF3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 50 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 60 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 70 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  },

  /* Middle Form */
  {
  /* 00 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 10 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 20 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFE8C, 0x0000,
              0xFE92, 0x0000, 0xFE98, 0xFE9C, 0xFEA0, 0xFEA4, 0xFEA8, 0x0000,
  /* 30 */    0x0000, 0x0000, 0x0000, 0xFEB4, 0xFEB8, 0xFEBC, 0xFEC0, 0xFEC4,
              0xFEC8, 0xFECC, 0xFED0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 40 */    0x0000, 0xFED4, 0xFED8, 0xFEDC, 0xFEE0, 0xFEE4, 0xFEE8, 0xFEEC,
              0x0000, 0x0000, 0xFEF4, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 50 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 60 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 70 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  },

  /* Final Form */
  {
  /* 00 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 10 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 20 */    0x0000, 0x0000, 0xFE82, 0xFE84, 0xFE86, 0xFE88, 0xFE8A, 0xFE8E,
              0xFE90, 0xFE94, 0xFE96, 0xFE9A, 0xFE9E, 0xFEA2, 0xFEA6, 0xFEAA,
  /* 30 */    0xFEAC, 0xFEAE, 0xFEB0, 0xFEB2, 0xFEB6, 0xFEBA, 0xFEBE, 0xFEC2,
              0xFEC6, 0xFECA, 0xFECE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 40 */    0x0000, 0xFED2, 0xFED6, 0xFEDA, 0xFEDE, 0xFEE2, 0xFEE6, 0xFEEA,
              0xFEEE, 0xFEF0, 0xFEF2, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 50 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 60 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 70 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  },

  /* Combine with baseline Form */
  {
  /* 00 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 10 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 20 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 30 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 40 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0xFE71, 0x0000, 0x0000, 0xFE77, 0xFE79,
  /* 50 */    0xFE7B, 0xFE7D, 0xFE7F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 60 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 70 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  },

  /* Combine with non-baseline Form */
  {
  /* 00 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 10 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 20 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 30 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 40 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
              0x0000, 0x0000, 0x0000, 0x064B, 0x064C, 0x064D, 0x064E, 0x064F,
  /* 50 */    0x0650, 0x0651, 0x0652, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 60 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  /* 70 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  },

  /* Isolated Lam-Alif ligature Form */
  { 0xFEF5, 0xFEF7, 0x0000, 0xFEF9, 0x0000, 0xFEFB },

  /* Final Lam-Alif ligature Form */
  { 0xFEF6, 0xFEF8, 0x0000, 0xFEFA, 0x0000, 0xFEFC },
};


StateTypeID	DevaStateTable[MAX_STATE][MAX_DEVA_TYPE] = {

/*   ND,   UP,   NP,   IV,   CN,   CK,   RC,   NM,   RM,
     IM,   HL,   NK,   VD,   HD */

  /* State 0 */
   St11,   St1, St1,  St2,  St4,  St4, St12,  St1,  St1,
    St1,   St1, St1,  St1, St11,
  /* State 1 */
    St1,  St1,  St1,  St1,  St1,  St1,  St1,  St1,  St1,
    St1,  St1,  St1,  St1,  St1,
  /* State 2 */
    St2,  St3,  St3,  St2,  St2,  St2,  St2,  St2,  St2,
    St2,  St2,  St2,  St2,  St2,
  /* State 3 */
    St3,  St3,  St3,  St3,  St3,  St3,  St3,  St3,  St3,
    St3,  St3,  St3,  St3,  St3,
  /* State 4 */
    St4,  St8,  St8,  St4,  St4,  St4,  St4,  St6,  St6,
    St9,  St5,  St4,  St4,  St4,
  /* State 5 */
    St5,  St5,  St5,  St5,  St4,  St4,  St4,  St5,  St5,
    St5,  St5,  St5,  St5,  St5,
  /* State 6 */
    St6,  St7,  St7,  St6,  St6,  St6,  St6,  St6,  St6,
    St6,  St6,  St6,  St6,  St6,
  /* State 7 */
    St7,  St7,  St7,  St7,  St7,  St7,  St7,  St7,  St7,
    St7,  St7,  St7,  St7,  St7,

/*   ND,   UP,   NP,   IV,   CN,   CK,   RC,   NM,   RM,
     IM,   HL,   NK,   VD,   HD */

  /* State 8 */
    St8,  St8,  St8,  St8,  St8,  St8,  St8,  St8,  St8,
    St8,  St8,  St8,  St8,  St8,
  /* State 9 */
    St9, St10, St10,  St9,  St9,  St9,  St9,  St9,  St9,
    St9,  St9,  St9,  St9,  St9,
  /* State 10 */
   St10, St10, St10, St10, St10, St10, St10, St10, St10,
   St10, St10, St10, St10, St10,
  /* State 11 */
   St11, St11, St11, St11, St11, St11, St11, St11, St11,
   St11, St11, St11, St11, St11,
  /* State 12 */
   St12,  St8,  St8, St12, St12, St12, St12,  St6,  St6,
    St9, St13, St12, St12, St12,
  /* State 13 */
   St13, St13, St13, St13, St14, St14, St14, St13, St13,
   St13, St13, St13, St13, St13,
  /* State 14 */
   St14, St18, St18, St14, St14, St14, St14, St16, St16,
   St19, St15, St14, St14, St14,
  /* State 15 */
   St15, St15, St15, St15, St14, St14, St14, St15, St15,
   St15, St15, St15, St15, St15,

/*   ND,   UP,   NP,   IV,   CN,   CK,   RC,   NM,   RM,
     IM,   HL,   NK,   VD,   HD */

  /* State 16 */
   St16, St17, St17, St16, St16, St16, St16, St16, St16,
   St16, St16, St16, St16, St16,
  /* State 17 */
   St17, St17, St17, St17, St17, St17, St17, St17, St17,
   St17, St17, St17, St17, St17,
  /* State 18 */
   St18, St18, St18, St18, St18, St18, St18, St18, St18,
   St18, St18, St18, St18, St18,
  /* State 19 */
   St19, St20, St20, St19, St19, St19, St19, St19, St19,
   St19, St19, St19, St19, St19,
  /* State 20 */
   St20, St20, St20, St20, St20, St20, St20, St20, St20,
   St20, St20, St20, St20, St20,
};


/* Tamil State Table */

StateTypeID	TamilStateTable[MAX_TAMIL_STATE][MAX_TAMIL_TYPE] = 
{

	  	       /*ND _VM _TMV _TMC _TNM _TMR _TMS _PM  _TMH	_TMD */
	
	/* State 0 */	St8, St7, St1, St2, St8, St8, St8, St8, St8, St8,

	/* State 1 */	St1, St1, St1, St1, St1, St1, St1, St1, St1, St1,

	/* State 2 */	St2, St2, St2, St2, St4, St5, St6, St2, St3, St2,

	/* State 3 */	St3, St3, St3, St2, St3, St3, St3, St3, St3, St3,

	/* State 4 */	St4, St4, St4, St4, St4, St4, St4, St4, St4, St4,

	/* State 5 */	St5, St5, St5, St5, St5, St5, St5, St5, St5, St5,

	/* State 6 */	St6, St6, St6, St6, St6, St6, St6, St6, St6, St6,

	/* State 7 */	St7, St7, St7, St7, St7, St7, St7, St7, St7, St7,

	/* State 8 */	St8, St8, St8, St8, St8, St8, St8, St8, St8, St8,

	/* State 9 */	St9, St9, St9, St9, St9, St9, St9, St9, St9, St9,
};



StateTypeID	KannStateTable[MAX_KANN_STATE][MAX_KANN_TYPE] = {

/*  ND		VM		V		C		NC		M		H		N		VD		D	*/

	/* STATE 0 */
	St13,	St1,	St2,	St4,	St4,	St1,	St1,	St1,	St13,	St13,
	/* STATE 1 */
	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,
	/* STATE 2 */
	St2,	St3,	St2,	St2,	St2,	St1,	St2,	St2,	St2,	St2,
	/* STATE 3 */
	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,
	/* STATE 4 */
	St4,	St8,	St4,	St4,	St4,	St6,	St5,	St4,	St4,	St4,
	/* STATE 5 */
	St5,	St5,	St5,	St9,	St9,	St5,	St5,	St5,	St5,	St5,
	/* STATE 6 */
	St6,	St7,	St6,	St6,	St6,	St6,	St6,	St6,	St6,	St6,
	/* STATE 7 */
	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,
	/* STATE 8 */
	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,
	/* STATE 9 */
	St9,	St12,	St9,	St9,	St9,	St10,	St5,	St9,	St9,	St9,
	/* STATE 10 */
	St10,	St11,	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,
	/* STATE 11 */
	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,
	/* STATE 12 */
	St12,	St12,	St12,	St12,	St12,	St12,	St12,	St12,	St12,	St12,
	/* STATE 13 */
	St13,	St13,	St13,	St13,	St13,	St13,	St13,	St13,	St13,	St13,
};


StateTypeID	GujaStateTable[MAX_GUJA_STATE][MAX_GUJA_TYPE] = {
/*	ND		VM		V		C		RC		M		IM		H		N		VD		D	*/
	/* State 0 */
	St11,	St1,	St2,	St4,	St12,	St1,	St1,	St1,	St1,	St11,	St11,
	/*State 1*/
	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,
	/* State 2 */
	St2,	St3,	St2,	St2,	St2,	St2,	St2,	St2,	St2,	St2,	St2,
	/* State 3 */
	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,
	/* State 4 */
	St4,	St8,	St4,	St4,	St4,	St6,	St9,	St5,	St4,	St4,	St4,
	/* State 5 */
	St5,	St5,	St5,	St4,	St4,	St5,	St5,	St5,	St5,	St5,	St5,
	/* State 6 */
	St6,	St7,	St6,	St6,	St6,	St6,	St6,	St6,	St6,	St6,	St6,
	/*State 7*/
	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,
	/*State 8*/
	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,
	/*State 9*/
	St9,	St10,	St9,	St9,	St9,	St9,	St9,	St9,	St9,	St9,	St9,
	/*State 10*/
	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,
	/*State 11*/
	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,
	/*State 12*/
	St12,	St12,	St12,	St12,	St12,	St12,	St12,	St13,	St12,	St12,	St12,
	/*State 13*/
	St13,	St13,	St13,	St14,	St14,	St13,	St13,	St13,	St13,	St13,	St13,
	/*State 14*/
	St14,	St18,	St14,	St14,	St14,	St16,	St19,	St15,	St14,	St14,	St14,
	/*State 15*/
	St15,	St15,	St15,	St14,	St14,	St15,	St15,	St15,	St15,	St15,	St15,
	/*State 16*/
	St16,	St17,	St16,	St16,	St16,	St16,	St16,	St16,	St16,	St16,	St16,
	/*State 17*/
	St17,	St17,	St17,	St17,	St17,	St17,	St17,	St17,	St17,	St17,	St17,
	/*State 18*/
	St18,	St18,	St18,	St18,	St18,	St18,	St18,	St18,	St18,	St18,	St18,
	/*State 19*/
	St19,	St20,	St19,	St19,	St19,	St19,	St19,	St19,	St19,	St19,	St19,
	/*State 20*/
	St20,	St20,	St20,	St20,	St20,	St20,	St20,	St20,	St20,	St20,	St20,
};

StateTypeID	BengStateTable[MAX_BENG_STATE][MAX_BENG_TYPE] = {
/*	ND	VM	V	C	RC	M	IM	H	N	VD	D	*/
	/* State 0 */
	St11,	St1,	St2,	St4,	St12,	St1,	St1,	St1,	St1,	St11,	St11,
	/* State 1 */
	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,
	/* State 2 */
	St2,	St3,	St2,	St2,	St2,	St2,	St2,	St2,	St2,	St2,	St2,
	/* State 3 */
	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,	St3,
	/* State 4 */
	St4,	St8,	St4,	St4,	St4,	St6,	St9,	St5,	St4,	St4,	St4,
	/* State 5 */
	St5,	St5,	St5,	St4,	St4,	St5,	St5,	St5,	St5,	St5,	St5,
	/* State 6 */
	St6,	St7,	St6,	St6,	St6,	St6,	St6,	St6,	St6,	St6,	St6,
	/* State 7 */
	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,
	/* State 8 */
	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,
	/* State 9 */
	St9,	St10,	St9,	St9,	St9,	St9,	St9,	St9,	St9,	St9,	St9,
	/* State 10 */
	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,
	/* State 11 */
	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,
	/* State 12 */
	St12,	St12,	St12,	St12,	St12,	St12,	St12,	St13,	St12,	St12,	St12,
	/* State 13 */
	St13,	St13,	St13,	St14,	St14,	St13,	St13,	St13,	St13,	St13,	St13,
	/* State 14 */
	St14,	St18,	St14,	St14,	St14,	St16,	St19,	St15,	St14,	St14,	St14,
	/* State 15 */
	St15,	St15,	St15,	St14,	St14,	St15,	St15,	St15,	St15,	St15,	St15,
	/* State 16 */
	St16,	St17,	St16,	St16,	St16,	St16,	St16,	St16,	St16,	St16,	St16,
	/* State 17 */
	St17,	St17,	St17,	St17,	St17,	St17,	St17,	St17,	St17,	St17,	St17,
	/* State 18 */
	St18,	St18,	St18,	St18,	St18,	St18,	St18,	St18,	St18,	St18,	St18,
	/* State 19 */
	St19,	St20,	St19,	St19,	St19,	St19,	St19,	St19,	St19,	St19,	St19,
	/* State 20 */
	St20,	St20,	St20,	St20,	St20,	St20,	St20,	St20,	St20,	St20,	St20,
};



StateTypeID	TlgStateTable[MAX_TLG_STATE][MAX_TLG_TYPE] = 
{

/*##################################################################
		 ND	   TLVM	   TLV	   TLC	   TLM	   TLH	   TLD	  TLMISC	
#################################################################### */
/* 0 */	 St0,		St12,		St1,		St3,		St12,		St12,		St12,		St12,									
										
/* 1 */	 St1,		St2,		St1,		St1,		St1,		St1,		St1,		St1,	
										
/* 2 */	 St2,		St2,		St2,		St2,		St2,		St2,		St2,		St2,	
										
/* 3 */	 St3, 	St11,		St3,		St3,		St9,		St4,		St3,		St3,	
										
/* 4 */	 St4,		St4,		St4,		St5,		St4,		St4,		St4,		St4,	
										
/* 5 */	 St5,		St8,		St5,		St5,		St6,		St4,		St5,		St5,	
										
/* 6 */	 St6,		St7,		St6,		St6,		St6,		St6,		St6,		St6,	
										
/* 7 */	 St7,		St7,		St7,		St7,		St7,		St7,		St7,		St7,	
										
/* 8 */	 St8,		St8,		St8,		St8,		St8,		St8,		St8,		St8,	
										
/* 9 */  St9,	    St10,		St9,		St9,		St9,		St9,		St9,		St9,	
										
/* 10*/  St10,	 St10,	St10,		St10,		St10,		St10,		St10,		St10,	
										
/* 11*/  St11,	 St11,	St11,		St11,		St11,		St11,		St11,		St11,	

/* 12*/  St12,	 St12,	St12,		St12,		St12,		St12,		St12,		St12,	

/*################################################################# */

};




StateTypeID	MlStateTable[MAX_ML_STATE][MAX_ML_TYPE] = 
{

/*#######################################################################
		 ND			VD	#		V	#		C		#	M		#	H		#	D		#	MISC	
######################################################################### */
/* 0 */	 St0,		St8,		St1,		St3,		St8,		St8,		St9,		St8,
													
/* 1 */	 St1,		St2,		St1,		St1,		St1,		St1,		St1,		St1,
													
/* 2 */	 St2,		St2,		St2,		St2,		St2,		St2,		St2,		St2,
													
/* 3 */	 St3,		St7,		St3,		St3,		St5,		St4,		St3,		St3,
													
/* 4 */  St4,		St4,		St4,		St3,		St4,		St4,		St4,		St4,
													
/* 5 */  St5,		St6,		St5,		St5,		St5,		St5,		St5,		St5,
													
/* 6 */  St6,		St6,		St6,		St6,		St6,		St6,		St6,		St6,
													
/* 7 */  St7,		St7,		St7,		St7,		St7,		St7,		St7,		St7,
													
/* 8 */  St8,		St8,		St8,		St8,		St8,		St8,		St8,		St8,
													
/* 9 */  St9,		St9,		St9,		St9,		St9,		St9,		St9,		St9,
													
/*######################################################################## */

};


StateTypeID PnStateTable [MAX_PN_STATE][MAX_PN_TYPE] =
{
/*     ND		VD	#	V	#	C	#	NC	#	N	#	M	#	IM	#	H	#	D	#	MISC	*/
/* 0 */	St0,	St10,	St1,	St3,	St3,	St11,	St10,	St10,	St10,	St11,	St10,	

/* 1 */	St1,	St2,	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,	St1,

/* 2 */	St2,	St2,	St2,	St2,	St2,	St2,	St2,	St2,	St2,	St2,	St2,

/* 3 */	St3,	St9,	St3,	St3,	St3,	St3,	St5,	St7,	St4,	St3,	St3,

/* 4 */	St4,	St4,	St4,	St3,	St3,	St4,	St4,	St4,	St4,	St4,	St4,

/* 5 */	St5,	St6,	St5,	St5,	St5,	St5,	St5,	St5,	St5,	St5,	St5,

/* 6 */	St6,	St6,	St6,	St6,	St6,	St6,	St6,	St6,	St6,	St6,	St6,

/* 7 */	St7,	St8,	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,	St7,

/* 8 */	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,	St8,

/* 9 */	St9,	St9,	St9,	St9,	St9,	St9,	St9,	St9,	St9,	St9,	St9,

/* 10*/ St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,	St10,

/* 11*/	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11,	St11

};


int	Deva_nMaxFocBlockTbl[128] = {
/*       0,  1,  2,  3,  4,  5,  6,  7,
         8,  9,  A,  B,  C,  D,  E,  F, */
/* 0 */  0,  1,  1,  1,  0,  1,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
/* 1 */  1,  1,  1,  1,  1,  4,  4,  4,
		 4,  2,  4,  1,  4,  4,  3,  3,
/* 2 */  3,  3,  3,  2,  4,  4,  4,  4,
         4,  1,  4,  4,  4,  4,  4,  4,
/* 3 */  2,  2,  2,  2,  1,  4,  4,  3,
         5,  4,  0,  0,  2,  1,  1,  1,
/* 4 */  1,  1,  1,  1,  1,  1,  1,  1,
         1,  1,  1,  1,  1,  3,  0,  0,
/* 5 */  1,  1,  1,  1,  1,  0,  0,  0,
         1,  1,  1,  1,  1,  1,  1,  1,
/* 6 */  1,  1,  1,  1,  1,  1,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
/* 7 */  1,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
};


int	Tamil_nMaxFocBlockTbl[128] = 
{

/*		 0,  1,  2,  3,  4,  5,  6,  7,
         8,  9,  A,  B,  C,  D,  E,  F, */
/* 8 */  0,  0,  1,  1,  0,  1,  1,  1,
         1,  1,  1,  0,  0,  0,  1,  1,
/* 9 */  1,  1,  1,  1,  1,  3,  0,  0,
		 0,  2,  2,  0,  2,  0,  2,  2,
/* A */  0,  0,  0,  2,  2,  0,  0,  0,
         2,  2,  2,  0,  0,  0,  2,  2,
/* B */  2,  2,  2,  2,  2,  2,  0,  4,
         2,  2,  0,  0,  0,  0,  1,  1,
/* C */  1,  1,  1,  0,  0,  0,  1,  1,
         1,  0,  1,  1,  1,  1,  0,  0,
/* D */  0,  0,  0,  0,  0,  0,  0,  1,
         0,  0,  0,  0,  0,  0,  0,  0,
/* E */  0,  0,  0,  0,  0,  0,  0,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
/* F */  1,  1,  1,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
};


int	Kann_nMaxFocBlockTbl[128] = {
/*       0,	1,	2,	3,	4,	5,	6,	7,
         8,	9,	A,	B,	C,	D,	E,	F, */
/* 0 */  0,	0,	2,	2,	0,	2,	2,	2,
	 2,	2,	2,	2,	2,	0,	2,	2,
/* 1 */	 2,	0,	2,	2,	2,	5,	5,	5,
	 5,	3,	5,	5,	5,	6,	5,	5,
/* 2 */  5,	5,	6,	5,	5,	5,	5,	6,
	 5,	0,	5,	6,	5,	6,	5,	7,
/* 3 */  5,	1,	5,	5,	0,	5,	5,	5,
	 5,	5,	0,	0,	0,	0,	1,	1,
/* 4 */	 1,	1,	1,	1,	1,	0,	1,	1,
	 1,	0,	1,	1,	1,	2,	0,	0,
/* 5 */	 0,	0,	0,	0,	0,	1,	1,	0,
	 0,	0,	0,	0,	0,	0,	1,	0,
/* 6 */	 1,	1,	0,	0,	0,	0,	1,	1,
	 1,	1,	1,	1,	1,	1,	1,	1,
/* 7 */	 0,	0,	0,	0,	0,	0,	0,	0,
	 0,	0,	0,	0,	0,	0,	0,	0,
};


int	Guja_nMaxFocBlockTbl[128] = {
/*       0,	1,	2,	3,	4,	5,	6,	7,
         8,	9,	A,	B,	C,	D,	E,	F, */
/* 0 */  0,	1,	1,	1,	0,	1,	1,	1,
		 2,	2,	2,	2,	0,	1,	0,	1,
/* 1 */  1,	1,	0,	1,	1,	4,	4,	4,
		 4,	2,	4,	2,	4,	3,	2,	3,
/* 2 */  3,	3,	3,	2,	4,	4,	4,	4,
		 4,	0,	4,	4,	4,	4,	4,	2,
/* 3 */  3,	0,	2,	2,	0,	4,	4,	2,
		 4,	3,	0,	0,	1,	1,	1,	1,
/* 4 */  1,	1,	1,	1,	1,	1,	0,	1,
		 1,	1,	0,	1,	1,	3,	0,	0,
/* 5 */  1,	0,	0,	0,	0,	0,	0,	0,
		 0,	0,	0,	0,	0,	0,	0,	0,
/* 6 */  1,	0,	0,	0,	0,	0,	1,	1,
		 1,	1,	1,	1,	1,	1,	1,	1,
/* 7 */  0,	0,	0,	0,	0,	0,	0,	0,
		 0,	0,	0,	0,	0,	0,	0,	0,
};



int	Beng_nMaxFocBlockTbl[128] = {
/*       0,	1,	2,	3,	4,	5,	6,	7,
         8,	9,	A,	B,	C,	D,	E,	F, */
/* 0 */  0,	1,	1,	1,	0,	1,	1,	1,
		 1,	1,	1,	1,	1,	0,	0,	1,
/* 1 */  1,	0,	0,	1,	1,	4,	3,	3,
		 3,	5,	5,	3,	5,	3,	3,	3,
/* 2 */  3,	3,	3,	5,	5,	3,	5,	4,
		 5,	0,	4,	3,	5,	4,	5,	1,
/* 3 */  2,	0,	3,	0,	0,	0,	4,	5,
		 5,	3,	0,	0,	1,	0,	1,	1,
/* 4 */  1,	1,	1,	1,	1,	0,	0,	1,
		 1,	0,	0,	1,	1,	2,	0,	0,
/* 5 */  0,	0,	0,	0,	0,	0,	0,	1,
		 0,	0,	0,	0,	1,	1,	0,	1,
/* 6 */  1,	1,	1,	1,	0,	0,	1,	1,
		 1,	1,	1,	1,	1,	1,	1,	1,
/* 7 */  1,	1,	1,	1,	1,	1,	1,	1,
		 1,	1,	1,	0,	0,	0,	0,	0,
};


int	Tlg_nMaxFocBlockTbl[128] = {
/*       0,  1,  2,  3,  4,  5,  6,  7,
         8,  9,  A,  B,  C,  D,  E,  F, */
/* 0 */  0,  1,  1,  1,  0,  1,  1,  1,
         1,  1,  1,  1,  1,  0,  1,  1,
/* 1 */  1,  0,  1,  1,  1,  3,  2,  2,
		 2,  2,  2,  2,  2,  2,  2,  2,
/* 2 */  2,  2,  2,  2,  2,  2,  2,  2,
         2,  0,  2,  2,  2,  2,  2,  2,
/* 3 */  2,  2,  2,  2,  0,  2,  2,  2,
         2,  2,  0,  0,  0,  0,  1,  1,
/* 4 */  1,  1,  1,  1,  1,  0,  1,  1,
         1,  0,  1,  1,  1,  2,  0,  0,
/* 5 */  0,  0,  0,  0,  0,  1,  1,  0,
         0,  0,  0,  0,  0,  0,  0,  0, 
/* 6 */  1,  1,  0,  0,  0,  0,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
/* 7 */  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
};



int	Ml_nMaxFocBlockTbl[128] = {
/*       0,  1,  2,  3,  4,  5,  6,  7,
         8,  9,  A,  B,  C,  D,  E,  F, */
/* 0 */  0,  1,  1,  1,  0,  1,  1,  1,
         1,  1,  1,  1,  1,  0,  1,  1,
/* 1 */  1,  0,  1,  1,  1,  3,  1,  3,
		 1,  3,  3,  1,  3,  1,  3,  3,
/* 2 */  1,  3,  1,  3,  3,  1,  3,  1,
         3,  0,  3,  1,  3,  1,  3,  4,
/* 3 */  5,  3,  3,  3,  1,  4,  3,  3,
         5,  3,  0,  0,  0,  0,  1,  1,
/* 4 */  1,  1,  1,  1,  0,  0,  1,  1,
         1,  0,  1,  1,  1,  2,  0,  0,
/* 5 */  0,  0,  0,  0,  0,  0,  0,  1,
         0,  0,  0,  0,  0,  0,  0,  0, 
/* 6 */  1,  1,  0,  0,  0,  0,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
/* 7 */  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
};

int	Pn_nMaxFocBlockTbl[128] = {
/*       0,  1,  2,  3,  4,  5,  6,  7,
         8,  9,  A,  B,  C,  D,  E,  F, */
/* 0 */  0,  0,  1,  0,  0,  2,  2,  2,
         2,  1,  2,  0,  0,  0,  0,  1,
/* 1 */  2,  0,  0,  1,  2,  5,  5,  5,
		 3,  3,  3,  3,  5,  3,  3,  3,
/* 2 */  3,  5,  5,  3,  3,  3,  3,  3,
         3,  0,  3,  5,  3,  3,  3,  3,
/* 3 */  3,  0,  3,  3,  0,  3,  3,  0,
         3,  3,  0,  0,  1,  0,  2,  1,
/* 4 */  2,  2,  2,  0,  0,  0,  0,  2,
         2,  0,  0,  2,  2,  3,  0,  0,
/* 5 */  0,  0,  0,  0,  0,  0,  0,  1,
         0,  1,  1,  1,  1,  0,  1,  0, 
/* 6 */  1,  1,  0,  0,  0,  0,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
/* 7 */  1,  1,  1,  1,  1,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
};


/*
 * Devanagari Glyph Tables
 */
static const DevaGlyphEntry DevaGlyphTbl[MAP_SIZE] = {
  /* Vowel Modifiers */
  { {0x0901,0x0},                       {0x0901,0x0} },
  { {0x0902,0x0},                       {0x0902,0x0} },
  { {0x0903,0x0},                       {0x0903,0x0} },

  /* vowels 13 */
  { {0x0905,0x0},                       {0x0905,0x0} },
  { {0x0906,0x0},                       {0x0906,0x0} },
  { {0x0907,0x0},                       {0x0907,0x0} },
  { {0x0908,0x0},                       {0x0908,0x0} },
  { {0x0909,0x0},                       {0x0909,0x0} },
  { {0x090a,0x0},                       {0x090a,0xF830,0x0} },
  { {0x090b,0x0},                       {0x090b,0xF831,0x0} },
  { {0x0960,0x0},                       {0x0960,0xF831,0x0} },					
  { {0x090c,0x0},                       {0x090c,0xF83D,0x0} },
  { {0x0961,0x0},                       {0x0961,0xF83D,0x0} },
  { {0x090d,0x0},                       {0x090d,0x0} },
  { {0x090e,0x0},                       {0x090e,0x0} },
  { {0x090f,0x0},                       {0x090f,0x0} },
  { {0x0910,0x0},                       {0x0910,0x0} },
  { {0x0911,0x0},                       {0x0911,0x0} },
  { {0x0912,0x0},                       {0x0912,0x0} },
  { {0x0913,0x0},                       {0x0913,0x0} },
  { {0x0914,0x0},                       {0x0914,0x0} },
											
  /* Vowel signs */
  { {0x093e,0x0},                       {0x093e,0x0} },
  { {0x093f,0x0},                       {0x093f,0x0} },
  { {0x0940,0x0},                       {0x0940,0x0} },
  { {0x0941,0x0},                       {0x0941,0x0} },
  { {0x0942,0x0},                       {0x0942,0x0} },
  { {0x0943,0x0},                       {0x0943,0x0} },
  { {0x0944,0x0},                       {0x0944,0x0} },
  { {0x0945,0x0},                       {0x0945,0x0} },
  { {0x0946,0x0},                       {0x0946,0x0} },
  { {0x0947,0x0},                       {0x0947,0x0} },
  { {0x0948,0x0},                       {0x0948,0x0} },
  { {0x0949,0x0},                       {0x0949,0x0} },
  { {0x094a,0x0},                       {0x094a,0x0} },
  { {0x094b,0x0},                       {0x094b,0x0} },
  { {0x094c,0x0},                       {0x094c,0x0} },
  { {0x0962,0x0},                       {0x0962,0x0} },
  { {0x0963,0x0},                       {0x0963,0x0} },

  /* Consonants */
  /* ka -> ka + kern space */
  { {0x0915,0x0},                       {0x0915,0xF832,0x0} },
  /* ka nukta  -> ka nukta + kern space */
  { {0x0915,0x093c,0x0},                {0x0958,0xF832,0x0} },
  /* ka + halant  -> half ka */
  { {0x0915,0x094d,0x0},                {0xF7C1,0x0} },
  /* ka nukta + halant-> half ka nukta */
  { {0x0915,0x093c,0x094d,0x0},         {0xF7C2,0x0} },

  { {0x0915,0x094d,0x0915,0x0},         {0xF845,0xF832,0x0} },

  /* ka + halant + ta -> kta + kern space */
  { {0x0915,0x094d,0x0924,0x0},         {0xF7C4,0xF832,0x0} },
  /* ka + halant + ra -> kra + kern space */
  { {0x0915,0x094d,0x0930,0x0},         {0xF7C3,0xF832,0x0} },
  { {0x0915,0x094d,0x0930,0x094d,0x0},  {0xF7C3,0x094d,0xF832,0x0} },
  /* ka + halant + SHa -> kSHa */
  { {0x0915,0x094d,0x0937,0x0},         {0xF7C5,0x093E,0x0} }, 	
  /* ka + halant + SHa + halant -> half kSHa */
  { {0x0915,0x094d,0x0937,0x094d,0x0},  {0xF7C5,0x0} },

  /* kha 6 */
  { {0x0916,0x0},                       {0x0916,0x0} },
  { {0x0916,0x094d,0x0},                {0xF7C6,0x0} },
  { {0x0916,0x093c,0x0},                {0x0959,0x0} },
  { {0x0916,0x093c,0x094d,0x0},         {0xF7C7,0x0} },
  { {0x0916,0x094d,0x0930,0x0},         {0xF7C8,0x093E,0x0} },
  { {0x0916,0x094d,0x0930,0x094d,0x0},  {0xF7C8,0x0} },

  /* ga 6 */
  { {0x0917,0x0},                       {0x0917,0x0} },
  { {0x0917,0x094d,0x0},                {0xF7C9,0x0} },
  { {0x0917,0x093c,0x0},                {0x095a,0x0} },
  { {0x0917,0x093c,0x094d,0x0},         {0xF7CA,0x0} },
  { {0x0917,0x094d,0x0930,0x0},         {0xF7CB,0x093E,0x0} },
  { {0x0917,0x094d,0x0930,0x094d,0x0},  {0xF7CB,0x0} },

  /* gha 4 */
  { {0x0918,0x0},                       {0x0918,0x0} },
  { {0x0918,0x094d,0x0},                {0xF7CC,0x0} },
  { {0x0918,0x094d,0x0930,0x0},         {0xF7CD,0x093E,0x0} },
  { {0x0918,0x094d,0x0930,0x094d,0x0},  {0xF7CD,0x0} },

  /* nga 1 */
  { {0x0919,0x0},                       {0x0919,0xF833,0x0} },

  /* added by ram */
  /* { {0x0919,0x094d,0x0},                {0x0919,0x094d,0xF833,0x0} }, */

  /* cha 4 */
  { {0x091a,0x0},                       {0x091a,0x0} },
  /* cha  + halant -> half cha   */
  { {0x091a,0x094d,0x0},                {0xF7CE,0x0} },
  /* cha + halant ra -> chra  */
  { {0x091a,0x094d,0x0930,0x0},         {0xF7CF,0x093E,0x0} },
  /* cha  + halant ra + halant  */
  { {0x091a,0x094d,0x0930,0x094d,0x0},  {0xF7CF,0x0} },

  /* chha 1 */
  { {0x091b,0x0},                       {0x091b,0xF834,0x0} },

  /* added by ram ...chha + halant -> chha+ halant */
  /*  { {0x091b,0x094d,0x0 },                {0x091b,0x094d,0xF834,0x0} }, */

  /* ja 6 */
  /* ja  */
  { {0x091c,0x0},                       {0x091c,0x0} },
  /* ja + halant -> half ja  */
  { {0x091c,0x094d,0x0},                {0xF7D0,0x0} },
  /* ja + nukta -> za */
  { {0x091c,0x093c,0x0},                {0xF7D1,0x093E,0x0} },
  /* ja + nukta + halant -> half za */
  { {0x091c,0x093c,0x094d,0x0 },        {0xF7D1,0x0} },
  /* ja + halant + ra -> jra  */
  { {0x091c,0x094d,0x0930,0x0},         {0xF7D2,0x093E,0x0} },
  /* ja + halant + ra + halant -> */
  { {0x091c,0x094d,0x0930,0x094d,0x0},  {0xF7D2,0x0} },

  /* dna 2 */
  /* ja + halant + jna -> dna  */
  { {0x091c,0x094d,0x091e,0x0},         {0xF7D3,0x093E,0x0} },
  /* ja + halant + jna -> half dna  */
  { {0x091c,0x094d,0x091e,0x094d,0x0},  {0xF7D3,0x0} },

  /* jha 4 */
  /* jha  */
  { {0x091d,0x0},                       {0x091d,0x0} },
  /* jha + halant -> half jha  */
  { {0x091d,0x094d,0x0},                {0xF7D4,0x0} },
  /* jha + halant -> jhra */
  { {0x091d,0x094d,0x0930,0x0},         {0xF7D5,0x093E,0x0} },
  /*  jha + halant -> half jhra */
  { {0x091d,0x094d,0x0930,0x094d,0x0},  {0xF7D5,0x0} },

  /* nya 2 */
  /* nya */
  { {0x091e,0x0},                       {0x091e,0x0} },
  /* nya + halant -> half nya */
  { {0x091e,0x094d,0x0},                {0xF7D6,0x0} },

  { {0x091e,0x094d,0x091c,0x0},         {0xF846,0x0} },

  /* Ta 3 */
  /* Ta -> Ta + kern space */
  { {0x091f,0x0},                       {0x091F,0xF835,0x0} },

  /* added by ram */
  { {0x091f,0x094d,0x0},                {0x091f,0x094d,0xF835,0x0} },

  /* Ta + halant + Ta -> TaTa + kern space */
  { {0x091f,0x094d,0x091f,0x0},         {0xF7D7,0xF835,0x0} },
  /* Ta + halant + Tha -> TaTha + kern space */
  { {0x091f,0x094d,0x0920,0x0},         {0xF7D8,0xF835,0x0} },

  /* Tha 2 */
  /* Tha -> Tha + kern space */
  { {0x0920,0x0},                       {0x0920,0xF836,0x0} },

  /*added by ram */
  { {0x0920,0x094d,0x0},                {0x0920,0x094d,0xF836,0x0} },

  /* Tha + halant + Tha -> + kern space */
  { {0x0920,0x094d,0x0920,0x0},         {0xF7D9,0xF836,0x0} },

  /* Da 1 */
  /* Da  -> Da  + kern space */
  { {0x0921,0x0},                       {0x0921,0xF837,0x0} },

  /* added by ram */
  { {0x0921,0x094d,0x0},                {0x0921,0x094d,0xF837,0x0} },
	 
  /* Da nukta 1 */
  /* Da + nukta -> + kern space */
  { {0x0921,0x093c,0x0},                {0x095c,0xF837,0x0} },

  /* Da + nukta+ halant */
  { {0x0921,0x093c,0x094d,0x0},         {0x095c,0x094d,0xF837,0x0} },

  { {0x0921,0x094d,0x0917,0x0},         {0xF847,0xF837,0x0} },

  /* Da halant Da 1 */
  /* Da + halant + Da - > + kern space */
  { {0x0921,0x094d,0x0921,0x0},         {0xF7DA,0xF837,0x0} },


  /* Da halant Dha 1 */
  /* Da + halant + Dha -> + kern space */
  { {0x0921,0x094d,0x0922,0x0},         {0xF7DB,0xF837,0x0} },


  /* Dha 1 */
  /* Dha  + kern space */
  { {0x0922,0x0},                       {0x0922,0xF838,0x0} },

  /* Dha nukta 1 */
  /* Dha + nukta -> + kern space */
  { {0x0922,0x093c,0x0},                {0x095d,0xF838,0x0} },
  { {0x0922,0x093c,0x094d,0x0},         {0x095d,0x094d,0xF838,0x0} },	

  /* added by Ram */
  { {0x0922,0x094d,0x0},                {0x0922,0x094d,0xF838,0x0} },


  /* Nna 2 */
  /* Nna */
  { {0x0923,0x0},                       {0x0923,0x0} },
  /* Nna + halant -> half Nna */
  { {0x0923,0x094d,0x0},                {0xF7DC,0x0} },

  /* ta 6 */
  /* ta */
  { {0x0924,0x0},                       {0x0924,0x0} },
  /* ta + halant -> half ta */
  { {0x0924,0x094d,0x0},                {0xF7DD,0x0} },
  /* ta + halant + ra -> half tra */
  { {0x0924,0x094d,0x0930,0x0},         {0xF7DE,0x093E,0x0} },
  /* ta + halant + ra + halant -> half tra  */
  { {0x0924,0x094d,0x0930,0x094d,0x0},  {0xF7DE,0x0} },
  /* ta + halant + ta -> */
  { {0x0924,0x094d,0x0924,0x0},         {0xF7DF,0x093E,0x0} },
  /* ta + halant + ta + halant -> */
  { {0x0924,0x094d,0x0924,0x094d,0x0},  {0xF7DF,0x0} },

  /* tha 4 */
  /* tha */
  { {0x0925,0x0},                       {0x0925,0x0} },
  /* tha + halant -> half tha */
  { {0x0925,0x094d,0x0},                {0xF7E0,0x0} },
  /* tha + halant + ra ->  */
  { {0x0925,0x094d,0x0930,0x0},         {0xF7E1,0x093E,0x0} },
  /* tha + halant + ra + halant -> */
  { {0x0925,0x094d,0x0930,0x094d,0x0},  {0xF7E1,0x0} },

  /* da 1 */
  /* da -> da + kern space */
  { {0x0926,0x0},                       {0x0926,0xF839,0x0} },
  /* da + halant -> half da + kern space */
  { {0x0926,0x094d,0x0},                {0x0926,0x094d,0xF839,0x0} },


  /* da ri 1
  /* da + ru -> da + ru + kern space */
  { {0x0926,0x0943,0x0},                {0xF7E2,0xF839,0x0} },

  /* da halant ra 1 */
  /* da + halant + ra -> dra + kern space  */
  { {0x0926,0x094d,0x0930,0x0},         {0xF7E3,0xF839,0x0} },
  { {0x0926,0x094d,0x0930,0x094d,0x0},  {0xF7E3,0x094d,0xF839,0x0} },

  { {0x0926,0x094d,0x0918,0x0},         {0xF848,0xF839,0x0} },

  /* da halant da 1 */
  /* da + halant + da  -> + kern space */
  { {0x0926,0x094d,0x0926,0x0},         {0xF7E4,0xF839,0x0} },

  /* da halant dha 1 */
  /* da + halant + dha -> + kern space  */
  { {0x0926,0x094d,0x0927,0x0},         {0xF7E5,0xF839,0x0} },

  { {0x0926,0x094d,0x092c,0x0},         {0xF849,0xF839,0x0} },

  { {0x0926,0x094d,0x092d,0x0},		{0xF844,0xF839,0x0} },

  /* da halant ma 1 */
  /* da + halant + ma -> + kern space */
  { {0x0926,0x094d,0x092e,0x0},         {0xF7E6,0xF839,0x0} },

  /* da halant ya 1 */
  /* da + halant + ya -> + kern space */
  { {0x0926,0x094d,0x092f,0x0},         {0xF7E7,0xF839,0x0} },

  /* da halant va 1 */
  /* da + halant + va -> + kern space  */
  { {0x0926,0x094d,0x0935,0x0},         {0xF7E8,0xF839,0x0} },

  /* dha 4 */
  /* Dha  */
  { {0x0927,0x0},                       {0x0927,0x0} },
  /* Dha + halant - > half Dha */
  { {0x0927,0x094d,0x0},                {0xF7E9,0x0} },
  /* Dha + halant + ra -> half Dhra */
  { {0x0927,0x094d,0x0930,0x0},         {0xF7EA,0x093E,0x0} },
  /* Dha + halant + ra + halant ->  */
  { {0x0927,0x094d,0x0930,0x094d,0x0},  {0xF7EA,0x0} },

  /* na 6 */
  /* na */
  { {0x0928,0x0},                       {0x0928,0x0} },
  /* na + halant  ->  half na */
  { {0x0928,0x094d,0x0},                {0xF7EB,0x0} },
  /* na + halant + ra  ->  	 */
  { {0x0928,0x094d,0x0930,0x0},         {0xF7EC,0x093E,0x0} },
  /* na + halant + ra + halant -> */
  { {0x0928,0x094d,0x0930,0x094d,0x0},  {0xF7EC,0x0} },
  /* na + halant + na -> */
  { {0x0928,0x094d,0x0928,0x0},         {0xF7ED,0x093E,0x0} },
  /* na + halant + na + halant -> */
  { {0x0928,0x094d,0x0928,0x094d,0x0},  {0xF7ED,0x0} },

  { {0x0929,0x0},                       {0x0929,0x0} },

  /* pa 4 */
  /* pa */
  { {0x092a,0x0},                       {0x092a,0x0} },
  /* pa + halant -> half pa */
  { {0x092a,0x094d,0x0},                {0xF7EE,0x0} },
  /* pa + halant +ra -> pra */
  { {0x092a,0x094d,0x0930,0x0},         {0xF7EF,0x093E,0x0} },
  /* pa + halant + ra + halant -> half pra */
  { {0x092a,0x094d,0x0930,0x094d,0x0},  {0xF7EF,0x0} },


  /* pha 5 */
  /* pha -> pha + kern space */
  { {0x092b,0x0 },                      {0x092b,0xF832,0x0} },
  /* pha + halant -> half pha */
  { {0x092b,0x094d,0x0},                {0xF7F0,0x0} },
  /* pha nukta	-> pha nukta + kern space */
  { {0x092b,0x093c,0x0},                {0x095e,0xF832,0x0} },
  /* pha nukta + halant-> half pha nukta */
  { {0x092b,0x093c,0x094d,0x0},         {0xF7F1,0x0} },
  /* pha + halant + ra -> fra + kern space */
  { {0x092b,0x094d,0x0930,0x0},         {0xF7F5,0xF832,0x0} },
  /* pha + halant + ra -> fra + kern space */
  { {0x092b,0x094d,0x0930,0x094d,0x0},  {0xF7F5,0xF832,0x094d,0x0} },

  /* ba 4 */
  /* ba */
  { {0x092c,0x0},                       {0x092c,0x0} },
  /* ba + halant -> half ba */
  { {0x092c,0x094d,0x0},                {0xF7F6,0x0} },
  /* ba + halant + ra -> */
  { {0x092c,0x094d,0x0930,0x0},         {0xF7F7,0x093E,0x0} },
  /* ba + halant ra + halant -> */
  { {0x092c,0x094d,0x0930,0x094d,0x0},  {0xF7F7,0x0} },

  /* bha 4 */
  /* bha  */
  { {0x092d,0x0},                       {0x092d,0x0} },
  /* bha + halant -> half halant  */
  { {0x092d,0x094d,0x0},                {0xF7F8,0x0} },
  /* bha + halant + ra ->  */
  { {0x092d,0x094d,0x0930,0x0},         {0xF7F9,0x093E,0x0} },
  /* bha + halant + ra + halant ->  */
  { {0x092d,0x094d,0x0930,0x094d,0x0},  {0xF7F9,0x0} },
  /* ma 4 */
  /* ma  */
  { {0x092e,0x0},                       {0x092e,0x0} },
  /* ma + halant -> half ma */
  { {0x092e,0x094d,0x0},                {0xF7FA,0x0} },
  /* ma + halant + ra -> */
  { {0x092e,0x094d,0x0930,0x0},         {0xF7FB,0x093E,0x0} },
  /* ma + halant + ra + halant ->  */
  { {0x092e,0x094d,0x0930,0x094d,0x0},  {0xF7FB,0x0} },
																
  /* ya 4 */
  /* ya */
  { {0x092f,0x0},                       {0x092f,0x0} },
  /* ya + halant -> half ya */
  { {0x092f,0x094d,0x0},                {0xF7FC,0x0} },
  /* ya + halant + ra -> */
  { {0x092f,0x094d,0x0930,0x0},         {0xF7FD,0x093E,0x0} },
  /* ya + halant + ra + halant -> */
  { {0x092f,0x094d,0x0930,0x094d,0x0},  {0xF7FD,0x0} },

  /* ra 3 */
  /* ra  */
  { {0x0930,0x0 },                      {0x0930,0xF83A,0x0} },
  /*  { {0x0930,0x094d,0x0},                 {0xF812,0x0} }, */
  /* ra + u -> Ru + kern space */
  { {0x0930,0x0941,0x0},                {0xF800,0xF83B,0x0} },
  /* ra + U -> RU + kern space */
  { {0x0930,0x0942,0x0},                {0xF801,0xF83C,0x0} },

  { {0x0931,0x0},                       {0x0931,0x0} },
  { {0x0931,0x094d,0x0},                {0xF7FF,0x0} },

  /* la 2 */
  /* la  */
  { {0x0932,0x0},                       {0x0932,0x0} },
  /* la + halant -> */
  { {0x0932,0x094d,0x0},                {0xF802,0x0} },
  /* La 2 */
  /* La */
  { {0x0933,0x0},                       {0x0933,0x0} },
  /* La + halant -> half La */
  { {0x0933,0x094d,0x0},                {0xF803,0x0} },

  { {0x0934,0x0} ,                      {0x0934,0x0} },
  /* va 4 */
  /* va */
  { {0x0935,0x0},                       {0x0935,0x0} },
  /* va + halant -> half va */
  { {0x0935,0x094d,0x0},                {0xF804,0x0} },
  /* va + halant + ra ->  */
  { {0x0935,0x094d,0x0930,0x0},         {0xF805,0x093E,0x0} },
  /* va + halant + ra + halant ->   */
  { {0x0935,0x094d,0x0930,0x094d,0x0},  {0xF805,0x0} },

  /* sha 6 */
  /* sha */
  { {0x0936,0x0},                       {0x0936,0x0} },
  /* sha + halant -> half sha */
  { {0x0936,0x094d,0x0},                {0xF806,0x0} },

  { {0x0936,0x094d,0x091a,0x0},		{0xF83F,0x0} },
  { {0x0936,0x094d,0x0928,0x0},		{0xF840,0x0} },

  /* sha + halant + va -> shwa */
  { {0x0936,0x094d,0x0935,0x0},         {0xF807,0x093E,0x0} },
  /* sha + halant + va + halant -> half shwa */
  { {0x0936,0x094d,0x0935,0x094d,0x0},  {0xF807,0x0} },
  /* sha + halant + ra -> shra */
  { {0x0936,0x094d,0x0930,0x0},         {0xF808,0x093E,0x0} },
  /* sha + halant + ra + halant -> half shra */
  { {0x0936,0x094d,0x0930,0x094d,0x0},  {0xF808,0x0} },
  /* SHa 2 */
  /* SHa */
  { {0x0937,0x0},                       {0x0937,0x0} },
  /* SHa + halant -> half SHa  */
  { {0x0937,0x094d,0x0},                {0xF809,0x0} },

  { {0x0937,0x094d,0x091f,0x0},		{0xF841,0xF835,0x0} },
  { {0x0937,0x094d,0x0920,0x0},		{0xF842,0xF836,0x0} },

  /* sa 4 */
  /* sa */
  { {0x0938,0x0},                       {0x0938,0x0} },
  /* sa + halant -> half sa */
  { {0x0938,0x094d,0x0},                {0xF80A,0x0} },
  /* sa + halant + ra ->  */
  { {0x0938,0x094d,0x0930,0x0},         {0xF80B,0x093E,0x0} },
  /* sa + halan + ra + halant -> */
  { {0x0938,0x094d,0x0930,0x094d,0x0},  {0xF80B,0x0} },

  { {0x0938,0x094d,0x0924,0x094d,0x0930,0x0}, {0xF843,0x0} },

  /* ha 2 */
  /* ha + kern space  */
  { {0x0939,0x0},                       {0x0939,0xF83E,0x0} },
  /* ha + halant  -> half ha */
  { {0x0939,0x094d,0x0},                {0xF80C,0xF83E,0x0} },
  /* ha + Rii + matra -> */
  { {0x0939,0x0943,0x0},                {0xF80D,0xF83E,0x0} },
  /* ha + halant + ra -> */
  { {0x0939,0x094d,0x0930,0x0},         {0xF80E,0xF83E,0x0} },
  /* ha + halant + ra + halant -> */
  { {0x0939,0x094d,0x0930,0x094d,0x0},  {0xF80E,0x094d,0xF83E,0x0} },

  { {0x0939,0x094d,0x0923,0x0},         {0xF84D,0xF83E,0x0} },
  { {0x0939,0x094d,0x0928,0x0},         {0xF84C,0xF83E,0x0} },

  /* ha + halant + ma -> */
  { {0x0939,0x094d,0x092e,0x0},         {0xF80F,0xF83E,0x0} },
  /* ha + halant + ya -> */
  { {0x0939,0x094d,0x092f,0x0},         {0xF810,0xF83E,0x0} },

  { {0x0939,0x094d,0x0932,0x0},         {0xF84A,0xF83E,0x0} },
  { {0x0939,0x094d,0x0935,0x0},         {0xF84B,0xF83E,0x0} },

  { {0x0958,0x0},                       {0x0958,0xF832,0x0} },
  { {0x0959,0x0},                       {0x0959,0x0} },
  { {0x095a,0x0},                       {0x095a,0x0} },
  { {0x095b,0x0},                       {0x095b,0x0} },
  { {0x095c,0x0},                       {0x095c,0xF837,0x0} },
  { {0x095d,0x0},                       {0x095d,0xF838,0x0} },
  { {0x095e,0x0},                       {0x095e,0xF832,0x0} },
  { {0x095f,0x0},                       {0x095f,0x0} },

  /*"\xd8\x", "", */

  /* misc 5 */
	
  /* nukta */
  { {0x093c,0x0},                       {0x093c,0x0} },
  /* nukta + u matra */
  { {0x093c,0x0941,0x0},                {0xF81E,0x0} },
  /* nukta + Uu matra -> */
  { {0x093c,0x0942,0x0},                {0xF821,0x0} },
	
  /* halant */
  { {0x094d,0x0},                       {0x094d,0x0} },
  /* halant + ya -> */
  { {0x094d,0x092f,0x0},                {0xF7FE,0x0} },
  /* halant + ra  */
  { {0x094d,0x0930,0x0},                {0xF811,0x0} },
  /* halant + ra + halant */
  { {0x094d,0x0930,0x094d,0x0},         {0x094d,0x0930,0x0930,0x0} },
  /* halant + ra + u matra -> */
  { {0x094d,0x0930,0x0941,0x0},         {0xF81F,0x0} },
  /* halant + ra + Uu matra ->  */
  { {0x094d,0x0930,0x0942,0x0},         {0xF822,0x0} },
		
  /* Vedic Characters */
  { {0x093d,0x0},                       {0x093d,0x0} },
  { {0x0950,0x0},                       {0x0950,0x0} },
  { {0x0951,0x0},                       {0x0951,0x0} },
  { {0x0952,0x0},                       {0x0952,0x0} },
  { {0x0953,0x0},                       {0x0953,0x0} },
  { {0x0954,0x0},                       {0x0954,0x0} },
  { {0x0964,0x0},                       {0x0964,0x0} },
  { {0x0965,0x0},                       {0x0965,0x0} },
  { {0x0970,0x0},                       {0x0970,0x0} },
											
  /* Dig09its */
  { {0x0966,0x0},                       {0x0966,0x0} },
  { {0x0967,0x0},                       {0x0967,0x0} },
  { {0x0968,0x0},                       {0x0968,0x0} },
  { {0x0969,0x0},                       {0x0969,0x0} },
  { {0x096a,0x0},                       {0x096a,0x0} },
  { {0x096b,0x0},                       {0x096b,0x0} },
  { {0x096c,0x0},                       {0x096c,0x0} },
  { {0x096d,0x0},                       {0x096d,0x0} },
  { {0x096e,0x0},                       {0x096e,0x0} },
  { {0x096f,0x0},                       {0x096f,0x0} },
};

static const TamilGlyphEntry TamilGlyphTbl[MAP_TAMIL_SIZE] = 
{

	// Vowel Modifiers
	{ {0x0B82,0x0},				{0x0B82,0x0} },
	{ {0x0B83,0x0},				{0x0B83,0x0} },
	
	// Vowel
	{ {0x0B85,0x0},				{0x0B85,0x0} },
	{ {0x0B86,0x0},				{0x0B86,0x0} },
	{ {0x0B87,0x0},				{0x0B87,0x0} },
	{ {0x0B88,0x0},				{0x0B88,0x0} },
	{ {0x0B89,0x0},				{0x0B89,0x0} },
	{ {0x0B8A,0x0},				{0x0B8A,0x0} },		
	
	
	{ {0x0B8E,0x0},				{0x0B8E,0x0} },
	{ {0x0B8F,0x0},				{0x0B8F,0x0} },

	{ {0x0B90,0x0},				{0x0B90,0x0} },
	{ {0x0B92,0x0},				{0x0B92,0x0} },
	{ {0x0B93,0x0},				{0x0B93,0x0} },
	{ {0x0B94,0x0},				{0x0B94,0x0} },			

	// Consonants
	// Ka
	{ {0x0B95,0x0},				{0x0B95,0x0} },		
	{ {0x0B95,0x0BCD,0x0},		{0xF716,0x0 } },
	{ {0x0B95,0x0BBF,0x0},		{0xF6E2,0x0 } },
	{ {0x0B95,0x0BC0,0x0},		{0xF6F8,0x0 } },
	{ {0x0B95,0x0BC1,0x0},		{0xF737,0x0 } },
	{ {0x0B95,0x0BC2,0x0},		{0xF74A,0x0 } },
	{ {0x0B95,0x0BCD,0x0BB7,0x0}, {0xF762,0x0} },
	
	//Nga
	{ {0x0B99,0x0},				{0x0B99,0x0} },		
	{ {0x0B99,0x0BCD,0x0},			{0xF717,0x0} },
	{ {0x0B99,0x0BBF,0x0},			{0xF6E3,0x0} },
	{ {0x0B99,0x0BC0,0x0},			{0xF6F9,0x0} },
	{ {0x0B99,0x0BC1,0x0},			{0xF738,0x0} },
	{ {0x0B99,0x0BC2,0x0},			{0xF74B,0x0} },
	
	// Cha
	{ {0x0B9A,0x0},				{0x0B9A,0x0} },		
	{ {0x0B9A,0x0BCD,0x0},			{0xF718,0x0} },
	{ {0x0B9A,0x0BBF,0x0},			{0xF6E4,0x0} },
	{ {0x0B9A,0x0BC0,0x0},			{0xF6FA,0x0} },
	{ {0x0B9A,0x0BC1,0x0},			{0xF739,0x0} },
	{ {0x0B9A,0x0BC2,0x0},			{0xF74C,0x0} },
	
	//Jha
	{ {0x0B9C,0x0},				{0x0B9C,0x0} },		
	{ {0x0B9C,0x0BCD,0x0},			{0xF713,0x0 } },
	{ {0x0B9C,0x0BBF,0x0},			{0xF6F5,0x0 } },
	{ {0x0B9C,0x0BC0,0x0},			{0xF70B,0x0 } },
	{ {0x0B9C,0x0BC1,0x0},			{0x0B9C,0xF764,0x0 } },	
	{ {0x0B9C,0x0BC2,0x0},			{0x0B9C,0xF765,0x0 } }, 
	
	// Jna
	{ {0x0B9E,0x0},				{0x0B9E,0x0} },		
	{ {0x0B9E,0x0BCD,0x0},		{0xF719,0x0} },
	{ {0x0B9E,0x0BBF,0x0},		{0xF6E5,0x0} },
	{ {0x0B9E,0x0BC0,0x0},		{0xF6FB,0x0} },
	{ {0x0B9E,0x0BC1,0x0},		{0xF73A,0x0} },
	{ {0x0B9E,0x0BC2,0x0},		{0xF74D,0x0} },
	
	//Ta
	{ {0x0B9F,0x0},				{0x0B9F,0x0} },			
	{ {0x0B9F,0x0BCD,0x0},		{0xF71A,0x0 } },
	{ {0x0B9F,0x0BBF,0x0},		{0xF735,0x0 } },
	{ {0x0B9F,0x0BC0,0x0},		{0xF736,0x0 } },
	{ {0x0B9F,0x0BC1,0x0},		{0xF73B,0x0 } },
	{ {0x0B9F,0x0BC2,0x0},		{0xF74E,0x0 } },
	
	// Na
	{ {0x0BA3,0x0},				{0x0BA3,0x0} },	
	{ {0x0BA3,0x0BCD,0x0},		{0xF71B,0x0} },
	{ {0x0BA3,0x0BBF,0x0},		{0xF6E6,0x0} },
	{ {0x0BA3,0x0BC0,0x0},		{0xF6FC,0x0} },
	{ {0x0BA3,0x0BC1,0x0},		{0xF73C,0x0} },
	{ {0x0BA3,0x0BC2,0x0},		{0xF74F,0x0} },
	
	//ta
	{ {0x0BA4,0x0},				{0x0BA4,0x0} },		
	{ {0x0BA4,0x0BCD,0x0},		{0xF71C,0x0} },
	{ {0x0BA4,0x0BBF,0x0},		{0xF6E7,0x0} },
	{ {0x0BA4,0x0BC0,0x0},		{0xF6FD,0x0} },
	{ {0x0BA4,0x0BC1,0x0},		{0xF73D,0x0} },
	{ {0x0BA4,0x0BC2,0x0},		{0xF750,0x0} },
		
	// Na
	{ {0x0BA8,0x0},				{0x0BA8,0x0} },			
	{ {0x0BA8,0x0BCD,0x0},		{0xF71D,0x0} },
	{ {0x0BA8,0x0BBF,0x0},		{0xF6E8,0x0} },
	{ {0x0BA8,0x0BC0,0x0},		{0xF6FE,0x0} },
	{ {0x0BA8,0x0BC1,0x0},		{0xF73F,0x0} },
	{ {0x0BA8,0x0BC2,0x0},		{0xF751,0x0} },
	
	//Na+nukta
	{ {0x0BA9,0x0},				{0x0BA9,0x0} },			
	{ {0x0BA9,0x0BCD,0x0},		{0xF731,0x0} },
	{ {0x0BA9,0x0BBF,0x0},		{0xF6F2,0x0} },
	{ {0x0BA9,0x0BC0,0x0},		{0xF708,0x0} },
	{ {0x0BA9,0x0BC1,0x0},		{0xF749 ,0x0} },
	{ {0x0BA9,0x0BC2,0x0},		{0xF761 ,0x0} },
	
	// Pa
	{ {0x0BAA,0x0},				{0x0BAA,0x0} },			 
	{ {0x0BAA,0x0BCD,0x0},		{0xF71E,0x0} },
	{ {0x0BAA,0x0BBF,0x0},		{0xF6E9,0x0} },
	{ {0x0BAA,0x0BC0,0x0},		{0xF6FF,0x0} },
	{ {0x0BAA,0x0BC1,0x0},		{0xF740,0x0} },
	{ {0x0BAA,0x0BC2,0x0},		{0xF752,0x0} },
	
		//Ma
	{ {0x0BAE,0x0},				{0x0BAE,0x0} },				
	{ {0x0BAE,0x0BCD,0x0},		{0xF71F,0x0} },
	{ {0x0BAE,0x0BBF,0x0},		{0xF6EA,0x0} },
	{ {0x0BAE,0x0BC0,0x0},		{0xF700,0x0} },
	{ {0x0BAE,0x0BC1,0x0},		{0xF741,0x0} },
	{ {0x0BAE,0x0BC2,0x0},		{0xF753,0x0} },
	
	// Ya
	{ {0x0BAF,0x0},				{0x0BAF,0x0} },				
	{ {0x0BAF,0x0BCD,0x0},		{0xF720,0x0 } },
	{ {0x0BAF,0x0BBF,0x0},		{0xF6EB,0x0 } },
	{ {0x0BAF,0x0BC0,0x0},		{0xF701,0x0 } },
	{ {0x0BAF,0x0BC1,0x0},		{0xF742,0x0 } },
	{ {0x0BAF,0x0BC2,0x0},		{0xF754,0x0 } },
	
	//Ra
	{ {0x0BB0,0x0},				{0x0BB0,0x0} },				
	{ {0x0BB0,0x0BCD,0x0},		{0xF729,0x0} },
	{ {0x0BB0,0x0BBF,0x0},		{0xF6EC,0x0} },
	{ {0x0BB0,0x0BC0,0x0},		{0xF702,0x0} },
	{ {0x0BB0,0x0BC1,0x0},		{0xF743,0x0} },
	{ {0x0BB0,0x0BC2,0x0},		{0xF755,0x0} },
	
	// Ra + nukta
	{ {0x0BB1,0x0},				{0x0BB1,0x0} },				
	{ {0x0BB1,0x0BCD,0x0},		{0xF730,0x0 } },
	{ {0x0BB1,0x0BBF,0x0},		{0xF6F1,0x0 } },
	{ {0x0BB1,0x0BC0,0x0},		{0xF707,0x0 } },
	{ {0x0BB1,0x0BC1,0x0},		{0xF748,0x0 } },
	{ {0x0BB1,0x0BC2,0x0},		{0xF760,0x0 } },
	
	//la
	{ {0x0BB2,0x0},				{0x0BB2,0x0} },				
	{ {0x0BB2,0x0BCD,0x0},		{0xF72A,0x0} },
	{ {0x0BB2,0x0BBF,0x0},		{0xF6ED,0x0} },
	{ {0x0BB2,0x0BC0,0x0},		{0xF703,0x0} },
	{ {0x0BB2,0x0BC1,0x0},		{0xF744,0x0} },
	{ {0x0BB2,0x0BC2,0x0},		{0xF75C,0x0} },
	
	// La
	{ {0x0BB3,0x0},				{0x0BB3,0x0} },				
	{ {0x0BB3,0x0BCD,0x0},		{0xF72D,0x0} },
	{ {0x0BB3,0x0BBF,0x0},		{0xF6F0,0x0} },
	{ {0x0BB3,0x0BC0,0x0},		{0xF706,0x0} },
	{ {0x0BB3,0x0BC1,0x0},		{0xF747,0x0} },
	{ {0x0BB3,0x0BC2,0x0},		{0xF75F,0x0} },
	
	//La+nukta
	{ {0x0BB4,0x0},				{0x0BB4,0x0} },				
	{ {0x0BB4,0x0BCD,0x0},		{0xF72C,0x0} },
	{ {0x0BB4,0x0BBF,0x0},		{0xF6EF,0x0 } },
	{ {0x0BB4,0x0BC0,0x0},		{0xF705,0x0 } },
	{ {0x0BB4,0x0BC1,0x0},		{0xF746,0x0 } },
	{ {0x0BB4,0x0BC2,0x0},		{0xF75E,0x0 } },
	
	// Va
	{ {0x0BB5,0x0},				{0x0BB5,0x0} },				
	{ {0x0BB5,0x0BCD,0x0},		{0xF72B,0x0} },
	{ {0x0BB5,0x0BBF,0x0},		{0xF6EE,0x0} },
	{ {0x0BB5,0x0BC0,0x0},		{0xF704,0x0} },
	{ {0x0BB5,0x0BC1,0x0},		{0xF745,0x0} },
	{ {0x0BB5,0x0BC2,0x0},		{0xF75D,0x0} },
	
	//Sha
	{ {0x0BB7,0x0},				{0x0BB7,0x0} },				
	{ {0x0BB7,0x0BCD,0x0},		{0xF712,0x0} },
	{ {0x0BB7,0x0BCD,0x0BB0,0x0},   {0xF712,0x0BB0,0x0} },
	{ {0x0BB7,0x0BBF,0x0},		{0xF6F4,0x0} },
	{ {0x0BB7,0x0BC0,0x0},		{0xF70A,0x0} },
	{ {0x0BB7,0x0BC1,0x0},		{0x0BB7,0xF764,0x0} },
	{ {0x0BB7,0x0BC2,0x0},		{0x0BB7,0xF765,0x0} },
	{ {0x0BB7,0x0BCD,0x0BB0,0x0BC0,0x0}, {0xF763,0x0} },
	
	// Sa 
	{ {0x0BB8,0x0},				{0x0BB8,0x0} },				
	{ {0x0BB8,0x0BCD,0x0},		{0xF711 ,0x0} },
	{ {0x0BB8,0x0BBF,0x0},		{0xF6F3,0x0} },
	{ {0x0BB8,0x0BC0,0x0},		{0xF709,0x0} },
	{ {0x0BB8,0x0BC1,0x0},		{0x0BB8,0xF764,0x0} },
	{ {0x0BB8,0x0BC2,0x0},		{0x0BB8,0xF765,0x0} },
	
	//Ha
	{ {0x0BB9,0x0},				{0x0BB9,0x0} },	  			
	{ {0x0BB9,0x0BCD,0x0},		{0xF714,0x0} },
	{ {0x0BB9,0x0BBF,0x0},		{0xF6F6,0x0} },
	{ {0x0BB9,0x0BC0,0x0},		{0xF70C,0x0} },
	{ {0x0BB9,0x0BC1,0x0},		{0x0BB9,0xF764,0x0} },
	{ {0x0BB9,0x0BC2,0x0},		{0x0BB9,0xF765,0x0} },
	
	  // Matra

	{ {0x0BBE,0x0},			{0x0BBE,0x0} },		
	{ {0x0BBF,0x0},			{0x0BBF,0x0} },					
	{ {0x0BC0,0x0},			{0x0BC0,0x0} },					
	{ {0x0BC1,0x0},			{0x0BC1,0x0} },					
	{ {0x0BC2,0x0},			{0x0BC2,0x0} },
	{ {0x0BC6,0x0},			{0x0BC6,0x0} },
	{ {0x0BC7,0x0},			{0x0BC7,0x0} },
	{ {0x0BC8,0x0},			{0x0BC8,0x0} },
	{ {0x0BCA,0x0},			{0x0BCA,0x0} },
	{ {0x0BCB,0x0},			{0x0BCB,0x0} },
	{ {0x0BCC,0x0},			{0x0BCC,0x0} },

	//Halant
	{ {0x0BCD,0x0},			{0x0BCD,0x0 } },		

	{ {0x0BD7,0x0},			{0x0BD7,0x0 } },
	
	//	Digits
	{ {0x0BE7,0x0},			{0x0BE7,0x0 } },
	{ {0x0BE8,0x0},			{0x0BE8,0x0 } },
	{ {0x0BE9,0x0},			{0x0BE9,0x0 } },
	{ {0x0BEA,0x0},			{0x0BEA,0x0 } },
	{ {0x0BEB,0x0},			{0x0BEB,0x0 } },
	{ {0x0BEC,0x0},			{0x0BEC,0x0 } },
	{ {0x0BED,0x0},			{0x0BED,0x0 } },
	{ {0x0BEE,0x0},			{0x0BEE,0x0 } },
	{ {0x0BEF,0x0},			{0x0BEF,0x0 } },

	{ {0x0BF0,0x0},			{0x0BF0,0x0 } },
	{ {0x0BF1,0x0},			{0x0BF1,0x0 } },
	{ {0x0BF2,0x0},			{0x0BF2,0x0 } },

};


static const KannGlyphEntry KannGlyphTbl[MAP_KANN_SIZE] = {
	//Vowel Modifiers
	
	{ {0x0C82,0x0},				{0x0C82,0x0} },
	{ {0x0c83,0x0},				{0x0C83,0x0} },
	
	// vowels 
	
	{ {0x0C85,0x0},				{0x0C85,0x0} },
	{ {0x0C86,0x0},				{0x0C86,0x0} },
	{ {0x0C87,0x0},				{0x0C87,0x0} },
	{ {0x0C88,0x0},				{0x0C88,0x0} },
	{ {0x0C89,0x0},				{0x0C89,0x0} },
	{ {0x0C8A,0x0},				{0x0C8A,0x0} },	
	{ {0x0C8B,0x0},				{0x0C8B,0x0} },	
	{ {0x0C8C,0x0},				{0x0C8C,0x0} },					
	{ {0x0C8E,0x0},				{0x0C8E,0x0} } ,
	{ {0x0C8F,0x0},				{0x0C8F,0x0} },
	{ {0x0C90,0x0},				{0x0C90,0x0} },
	{ {0x0C91,0x0},				{0x0C91,0x0} },
	{ {0x0C92,0x0},				{0x0C92,0x0} },
	{ {0x0C93,0x0},				{0x0C93,0x0} },
	{ {0x0C94,0x0},				{0x0C94,0x0} },

	// Vowel signs 
	
	{ {0x0CBE,0x0},				{0x0CBE,0x0} },
	{ {0x0CBF,0x0},				{0x0CBF,0x0} },
	{ {0x0CC0,0x0},				{0x0CC0,0x0} },
	{ {0x0CC1,0x0},				{0x0CC1,0x0} },
	{ {0x0CC2,0x0},				{0x0CC2,0x0} },
	{ {0x0CC3,0x0},				{0x0CC3,0x0} },
	{ {0x0CC4,0x0},				{0x0CC4,0x0} },
	{ {0x0CC6,0x0},				{0x0CC6,0x0} },
	{ {0x0CC7,0x0},				{0x0CC7,0x0} },
	{ {0x0CC8,0x0},				{0x0CC8,0x0} },
	{ {0x0CCA,0x0},				{0x0CCA,0x0} },
	{ {0x0CCB,0x0},				{0x0CCB,0x0} },
	{ {0x0CCC,0x0},				{0x0CCC,0x0} } ,
	{ {0x0CCd,0x0},				{0x0CCd,0x0} } ,
	{ {0x0Cd5,0x0},				{0x0Cd5,0x0} } ,
	{ {0x0Cd6,0x0},				{0x0Cd6,0x0} } ,
	{ {0x0Cde,0x0},				{0x0Cde,0x0} } ,
	{ {0x0Ce0,0x0},				{0x0Ce0,0x0} } ,
	{ {0x0Ce1,0x0},				{0x0Ce1,0x0} } ,

	// Consonants
	
	//Ka
	
	{ {0x0C95,0x0},		{0x0C95,0x0} }, 	// ka 
	{ {0X0C95,0x0CBE,0x0},	{0xf64d,0x0cbe,0x0} }, //ka + 
	{ {0x0C95,0x0CBF,0x0},	{0xf64e,0x0} }, 	//ka +  
	{ {0x0c95,0x0cc0,0x0},  {0xf64e,0x0cd5, 0x0} },	//ka +  
	{ {0x0c95,0x0cc6,0x0},	{0xf64d,0xf686,0x0} }, //ka + 
	{ {0x0c95,0x0cc7,0x0},	{0xf64d,0xf686,0x0cd5,0x0} }, //ka + 
	{ {0x0c95,0x0cc8,0x0},	{0xf64d,0xf686,0xf687,0x0} }, //ka + 
	{ {0x0c95,0x0cca,0x0},	{0xf64d,0xf686,0x0cc2,0x0} }, //ka + 
	{ {0x0c95,0x0ccb,0x0},	{0xf64d,0xf686,0x0cc2,0x0cd5,0x0} }, //ka + 
	{ {0x0c95,0x0ccc,0x0},	{0xf64d,0xf688,0x0} }, //ka +

	//Kha

	{ {0x0C96,0x0},		{0x0C96,0x0} }, 	// kha 
	{ {0X0C96,0x0CBE,0x0},	{0xf64f,0x0cbe,0x0} }, // kha + 
	{ {0x0C96,0x0CBF,0x0},	{0xf650,0x0} }, 	//kha + 
	{ {0x0c96,0x0cc0,0x0},  {0xf650,0x0cd5, 0x0} },	//kha + 
	{ {0x0c96,0x0cc6,0x0},	{0xf64f,0xf686,0x0} }, //kha + 
	{ {0x0c96,0x0cc7,0x0},	{0xf64f,0xf686,0x0cd5,0x0} }, //kha + 
	{ {0x0c96,0x0cc8,0x0},	{0xf64f,0xf686,0xf687,0x0} }, //kha + 
	{ {0x0c96,0x0cca,0x0},	{0xf64f,0xf686,0x0cc2,0x0} }, //kha + 
	{ {0x0c96,0x0ccb,0x0},	{0xf64f,0xf686,0x0cc2,0x0cd5,0x0} }, //kha + 
	{ {0x0c96,0x0ccc,0x0},	{0xf64f,0xf688,0x0} }, //kha + 

	//ga
	
	{ {0x0C97,0x0},		{0x0C97,0x0} }, 	// ga  
	{ {0X0C97,0x0cbe,0x0},	{0xf651,0x0cbe,0x0} }, // ga + 
	{ {0x0C97,0x0cbf,0x0},	{0xf652,0x0} }, 	//ga + 
	{ {0x0c97,0x0cc0,0x0},  {0xf652,0x0cd5, 0x0} },	//ga + 
	{ {0x0c97,0x0cc6,0x0},	{0xf651,0xf686,0x0} }, //ga + 
	{ {0x0c97,0x0cc7,0x0},	{0xf651,0xf686,0x0cd5,0x0} }, //ga + 
	{ {0x0c97,0x0cc8,0x0},	{0xf651,0xf686,0xf687,0x0} }, //ga + 
	{ {0x0c97,0x0cca,0x0},	{0xf651,0xf686,0x0cc2,0x0} }, //ga + 
	{ {0x0c97,0x0ccb,0x0},	{0xf651,0xf686,0x0cc2,0x0cd5,0x0} }, //ga + 
	{ {0x0c97,0x0ccc,0x0},	{0xf651,0xf688,0x0} }, //ga + 

	//gha

	{ {0x0C98,0x0},		{0x0C98,0x0} }, 	// gha  
	{ {0X0C98,0x0cbe,0x0},	{0xf653,0x0cbe,0x0} }, // gha + 
	{ {0x0C98,0x0cbf,0x0},	{0xf654,0x0} }, 	//gha + 
	{ {0x0c98,0x0cc0,0x0},  {0xf654,0x0cd5, 0x0} },	//gha + 
	{ {0x0c98,0x0cc6,0x0},	{0xf653,0xf685,0x0} }, //gha + 
	{ {0x0c98,0x0cc7,0x0},	{0xf653,0xf686,0x0cd5,0x0} }, //gha + 
	{ {0x0c98,0x0cc8,0x0},	{0xf653,0xf686,0xf687,0x0} }, //gha + 
	{ {0x0c98,0x0cca,0x0},	{0xf653,0xf686,0x0cc2,0x0} }, //gha + 
	{ {0x0c98,0x0ccb,0x0},	{0xf653,0xf686,0x0cc2,0x0cd5,0x0} }, //gha + 
	{ {0x0c98,0x0ccc,0x0},	{0xf653,0xf688,0x0} }, //gha + 
	
	// dna nukta
	
	{ {0x0c99,0x0},		{0x0c99,0x0} },
	
	///cha

	{ {0x0C9a,0x0},		{0x0C9a,0x0} }, 	//  cha
	{ {0X0C9a,0x0cbe,0x0},	{0xf655,0x0cbe,0x0} }, // cha + 
	{ {0x0C9a,0x0cbf,0x0},	{0xf656,0x0} }, 	//cha + 
	{ {0x0c9a,0x0cc0,0x0},  {0xf656,0x0cd5, 0x0} },	//cha + 
	{ {0x0c9a,0x0cc6,0x0},	{0xf655,0xf686,0x0} }, //cha + 
	{ {0x0c9a,0x0cc7,0x0},	{0xf655,0xf686,0x0cd5,0x0} }, //cha + 
	{ {0x0c9a,0x0cc8,0x0},	{0xf655,0xf686,0xf687,0x0} }, //cha + 
	{ {0x0c9a,0x0cca,0x0},	{0xf655,0xf686,0x0cc2,0x0} }, //cha + 
	{ {0x0c9a,0x0ccb,0x0},	{0xf655,0xf686,0x0cc2,0x0cd5,0x0} }, //cha + 
	{ {0x0c9a,0x0ccc,0x0},	{0xf655,0xf688,0x0} }, //cha + 
	
	//Chha

	{ {0x0C9b,0x0},		{0x0C9b,0x0} }, 	// chha
	{ {0X0C9b,0x0cbe,0x0},	{0xf657,0x0cbe,0x0} }, // chha + 
	{ {0x0c9b,0x0cc0,0x0},  {0xf658,0x0cd5,0x0} }, //chha + 
	{ {0x0c9b,0x0cc6,0x0},	{0xf657,0xf686,0x0} }, //chha + 
	{ {0x0c9b,0x0cc7,0x0},	{0xf657,0xf686,0x0cd5,0x0} }, //chha + 
	{ {0x0c9b,0x0cc8,0x0},	{0xf657,0xf686,0xf687,0x0} }, //chha + 
	{ {0x0c9b,0x0cca,0x0},	{0xf657,0xf686,0x0cc2,0x0} }, //chha + 
	{ {0x0c9b,0x0ccb,0x0},	{0xf657,0xf686,0x0cc2,0x0cd5,0x0} }, //chha + 
	{ {0x0c9b,0x0ccc,0x0},	{0xf657,0xf688,0x0} }, //chha + 
	
	//ja

	{ {0x0C9c,0x0},		{0x0C9c,0x0} }, 	// ja
	{ {0X0C9c,0x0cbe,0x0},	{0xf659,0x0cbe,0x0} }, // ja + 
	{ {0x0C9c,0x0cbf,0x0},	{0xf65a,0x0} }, 	//ja + 
	{ {0x0c9c,0x0cc0,0x0},  {0xf65a,0x0cd5, 0x0} },	//ja + 
	{ {0x0c9c,0x0cc6,0x0},	{0xf659,0xf686,0x0} }, //ja + 
	{ {0x0c9c,0x0cc7,0x0},	{0xf659,0xf686,0x0cd5,0x0} }, //ja + 
	{ {0x0c9c,0x0cc8,0x0},	{0xf659,0xf686,0xf687,0x0} }, //ja + 
	{ {0x0c9c,0x0cca,0x0},	{0xf659,0xf686,0x0cc2,0x0} }, //ja + 
	{ {0x0c9c,0x0ccb,0x0},	{0xf659,0xf686,0x0cc2,0x0cd5,0x0} }, //ja + 
	{ {0x0c9c,0x0ccc,0x0},	{0xf659,0xf688,0x0} }, //ja + 
	
		
	// zha

	{ {0x0c9d,0x0}, 	{0x0c9d,0x0} },	//zha  
	{ {0x0c9d,0x0cbe,0x0},	{0xf65b,0xf682,0xf65c,0xf65d,0x0cbe,0x0} }, // zha +
	{ {0x0c9d,0x0cbf,0x0},	{0xf65e,0xf65c,0xf6ad,0x0} }, // zha +
	{ {0x0c9d,0x0cc0,0x0},	{0xf65e,0xf65c,0xf6ad,0x0cd5,0x0} }, // zha +
	{ {0x0c9d,0x0cc6,0x0},	{0xf65b,0xf686,0xf65c,0xf6ad,0x0} }, // zha +
	{ {0x0c9d,0x0cc7,0x0},	{0xf65b,0xf686,0xf65c,0xf6ad,0x0cd5,0x0} },//zha +
	{ {0x0c9d,0x0cc8,0x0},	{0xf65b,0xf686,0xf65c,0xf6ad,0xf687,0x0} },//zha +
	{ {0x0c9d,0x0cca,0x0},	{0xf65b,0xf686,0xf65c,0x0cc2,0x0} },//zha +
	{ {0x0c9d,0x0ccb,0x0},	{0xf65b,0xf686,0xf65c,0x0cc2,0x0cd5,0x0} },//zha +
	{ {0x0c9d,0x0ccc,0x0},	{0xf65b,0xf682,0xf65c,0xf65d,0xf688,0x0} },//zha +
	

	// gna
	
	{ {0x0c9e,0x0},		{0x0c9e,0x0} },	//gna  
	{ {0x0c9e,0x0cc6,0x0},	{0x0c9e,0xf686,0x0} },
	{ {0x0c9e,0x0cc7,0x0},	{0x0c9e,0xf686,0x0cd5,0x0} },
	{ {0x0c9e,0x0cc8,0x0},	{0x0c9e,0xf686,0xf687,0x0} },
	{ {0x0c9e,0x0cca,0x0},	{0x0c9e,0xf686,0x0cc2,0x0} },
	{ {0x0c9e,0x0ccb,0x0},	{0x0c9e,0xf686,0x0cc2,0x0cd5,0x0} },
	{ {0x0c9e,0x0ccc,0x0},	{0x0c9e,0xf688,0x0} },
	
	//Ta

	{ {0x0C9f,0x0},		{0x0C9f,0x0} }, 	// Ta -> Ta
	{ {0X0C9f,0x0cbe,0x0},	{0xf65f,0x0cbe,0x0} }, // Ta + 
	{ {0x0C9f,0x0cbf,0x0},	{0xf660,0x0} }, 	//Ta + 
	{ {0x0c9f,0x0cc0,0x0},  {0xf660,0x0cd5, 0x0} },	//Ta + 
	{ {0x0c9f,0x0cc6,0x0},	{0xf65f,0xf686,0x0} }, //Ta + 
	{ {0x0c9f,0x0cc7,0x0},	{0xf65f,0xf686,0x0cd5,0x0} }, //Ta + 
	{ {0x0c9f,0x0cc8,0x0},	{0xf65f,0xf686,0xf687,0x0} }, //Ta + 
	{ {0x0c9f,0x0cca,0x0},	{0xf65f,0xf686,0x0cc2,0x0} }, //Ta + 
	{ {0x0c9f,0x0ccb,0x0},	{0xf65f,0xf686,0x0cc2,0x0cd5,0x0} }, //Ta + 
	{ {0x0c9f,0x0ccc,0x0},	{0xf65f,0xf688,0x0} }, //Ta + 
	
	//Tha

	{ {0x0ca0,0x0},		{0x0ca0,0x0} }, 	// Tha -> Tha
	{ {0X0ca0,0x0cbe,0x0},	{0xf661,0x0cbe,0x0} }, // Tha + 
	{ {0x0ca0,0x0cbf,0x0},	{0xf662,0x0} }, 	//Tha + 
	{ {0x0ca0,0x0cc0,0x0},  {0xf662,0x0cd5, 0x0} },	//Tha + 
	{ {0x0ca0,0x0cc6,0x0},	{0xf661,0xf686,0x0} }, //Tha + 
	{ {0x0ca0,0x0cc7,0x0},	{0xf661,0xf686,0x0cd5,0x0} }, //Tha + 
	{ {0x0ca0,0x0cc8,0x0},	{0xf661,0xf686,0xf687,0x0} }, //Tha + 
	{ {0x0ca0,0x0cca,0x0},	{0xf661,0xf686,0x0cc2,0x0} }, //Tha + 
	{ {0x0ca0,0x0ccb,0x0},	{0xf661,0xf686,0x0cc2,0x0cd5,0x0} }, //Tha + 
	{ {0x0ca0,0x0ccc,0x0},	{0xf661,0xf688,0x0} }, //Tha + 
	
	//dda

	{ {0x0ca1,0x0},		{0x0ca1,0x0} }, 	// dda -> dda
	{ {0X0ca1,0x0cbe,0x0},	{0xf663,0x0cbe,0x0} }, // dda + 
	{ {0x0ca1,0x0cbf,0x0},	{0xf664,0x0} }, 	//dda + 
	{ {0x0ca1,0x0cc0,0x0},  {0xf664,0x0cd5, 0x0} },	//dda + 
	{ {0x0ca1,0x0cc6,0x0},	{0xf663,0xf686,0x0} }, //dda + 
	{ {0x0ca1,0x0cc7,0x0},	{0xf663,0xf686,0x0cd5,0x0} }, //dda + 
	{ {0x0ca1,0x0cc8,0x0},	{0xf663,0xf686,0xf687,0x0} }, //dda + 
	{ {0x0ca1,0x0cca,0x0},	{0xf663,0xf686,0x0cc2,0x0} }, //dda + 
	{ {0x0ca1,0x0ccb,0x0},	{0xf663,0xf686,0x0cc2,0x0cd5,0x0} }, //dda + 
	{ {0x0ca1,0x0ccc,0x0},	{0xf663,0xf688,0x0} }, //dda + 
	
	// ddha mostly -> dda + 0xf665

	{ {0x0ca2,0x0},		{0x0ca2,0x0} }, 	// ddha -> ddha
	{ {0X0ca2,0x0cbe,0x0},	{0xf663,0xf665,0x0cbe,0x0} }, // ddha + 
	{ {0x0ca2,0x0cbf,0x0},	{0xf664,0xf6c0,0x0} }, 	//ddha + 
	{ {0x0ca2,0x0cc0,0x0},  {0xf664,0xf6c0,0x0cd5, 0x0} },	//ddha + 
	{ {0x0ca2,0x0cc6,0x0},	{0xf663,0xf665,0xf686,0x0} }, //ddha + 
	{ {0x0ca2,0x0cc7,0x0},	{0xf663,0xf665,0xf686,0x0cd5,0x0} }, //ddha + 
	{ {0x0ca2,0x0cc8,0x0},	{0xf663,0xf665,0xf686,0xf687,0x0} }, //ddha + 
	{ {0x0ca2,0x0cca,0x0},	{0xf663,0xf665,0xf686,0x0cc2,0x0} }, //ddha + 
	{ {0x0ca2,0x0ccb,0x0},	{0xf663,0xf665,0xf686,0x0cc2,0x0cd5,0x0} }, //ddha + 
	{ {0x0ca2,0x0ccc,0x0},	{0xf663,0xf665,0xf688,0x0} }, //ddha + 
	
	// na
	
	{ {0x0ca3,0x0},		{0x0ca3,0x0} },
	{ {0x0ca3,0x0cbe,0x0},	{0xf666,0x0cbe,0x0} },
	{ {0x0ca3,0x0cc6,0x0},	{0xf666,0xf686,0x0} },
	{ {0x0ca3,0x0cc7,0x0},	{0xf666,0xf686,0x0cd5,0x0} },
	{ {0x0ca3,0x0cc8,0x0},	{0xf666,0xf686,0xf687,0x0} },
	{ {0x0ca3,0x0cca,0x0},	{0xf666,0xf686,0x0cc2,0x0} },
	{ {0x0ca3,0x0ccb,0x0},	{0xf666,0xf686,0x0cc2,0x0cd5,0x0} },
	{ {0x0ca3,0x0ccc,0x0},	{0xf666,0xf688,0x0} },

		
	//ta

	{ {0x0ca4,0x0},		{0x0ca4,0x0} }, 	// ta -> ta
	{ {0X0ca4,0x0cbe,0x0},	{0xf667,0x0cbe,0x0} }, // ta + 
	{ {0x0ca4,0x0cbf,0x0},	{0xf668,0x0} }, 	//ta + 
	{ {0x0ca4,0x0cc0,0x0},  {0xf668,0x0cd5, 0x0} },	//ta + 
	{ {0x0ca4,0x0cc6,0x0},	{0xf667,0xf686,0x0} }, //ta + 
	{ {0x0ca4,0x0cc7,0x0},	{0xf667,0xf686,0x0cd5,0x0} }, //ta + 
	{ {0x0ca4,0x0cc8,0x0},	{0xf667,0xf686,0xf687,0x0} }, //ta + 
	{ {0x0ca4,0x0cca,0x0},	{0xf667,0xf686,0x0cc2,0x0} }, //ta + 
	{ {0x0ca4,0x0ccb,0x0},	{0xf667,0xf686,0x0cc2,0x0cd5,0x0} }, //ta + 
	{ {0x0ca4,0x0ccc,0x0},	{0xf667,0xf688,0x0} }, //ta + 
	
	//tha

	{ {0x0ca5,0x0},		{0x0ca5,0x0} }, 	// tha -> tha
	{ {0X0ca5,0x0cbe,0x0},	{0xf669,0x0cbe,0x0} }, // tha + 
	{ {0x0ca5,0x0cbf,0x0},	{0xf66a,0x0} }, 	//tha + 
	{ {0x0ca5,0x0cc0,0x0},  {0xf66a,0x0cd5, 0x0} },	//tha + 
	{ {0x0ca5,0x0cc6,0x0},	{0xf669,0xf686,0x0} }, //tha + 
	{ {0x0ca5,0x0cc7,0x0},	{0xf669,0xf686,0x0cd5,0x0} }, //tha + 
	{ {0x0ca5,0x0cc8,0x0},	{0xf669,0xf686,0xf687,0x0} }, //tha + 
	{ {0x0ca5,0x0cca,0x0},	{0xf669,0xf686,0x0cc2,0x0} }, //tha + 
	{ {0x0ca5,0x0ccb,0x0},	{0xf669,0xf686,0x0cc2,0x0cd5,0x0} }, //tha + 
	{ {0x0ca5,0x0ccc,0x0},	{0xf669,0xf688,0x0} }, //tha + 
	
	
	//da

	{ {0x0ca6,0x0},		{0x0ca6,0x0} }, 	// da -> da
	{ {0X0ca6,0x0cbe,0x0},	{0xf66b,0x0cbe,0x0} }, // da + 
	{ {0x0ca6,0x0cbf,0x0},	{0xf66c,0x0} }, 	//da + 
	{ {0x0ca6,0x0cc0,0x0},  {0xf66c,0x0cd5, 0x0} },	//da + 
	{ {0x0ca6,0x0cc6,0x0},	{0xf66b,0xf686,0x0} }, //da + 
	{ {0x0ca6,0x0cc7,0x0},	{0xf66b,0xf686,0x0cd5,0x0} }, //da + 
	{ {0x0ca6,0x0cc8,0x0},	{0xf66b,0xf686,0xf687,0x0} }, //da + 
	{ {0x0ca6,0x0cca,0x0},	{0xf66b,0xf686,0x0cc2,0x0} }, //da + 
	{ {0x0ca6,0x0ccb,0x0},	{0xf66b,0xf686,0x0cc2,0x0cd5,0x0} }, //da + 
	{ {0x0ca6,0x0ccc,0x0},	{0xf66b,0xf688,0x0} }, //da + 

	// dha  mostly - > da + 0xf665

	{ {0x0ca7,0x0},		{0x0ca7,0x0} }, 	// dha -> dha
	{ {0X0ca7,0x0cbe,0x0},	{0xf66b,0xf665,0x0cbe,0x0} }, // dha + 
	{ {0x0ca7,0x0cbf,0x0},	{0xf66c,0xf6c0,0x0} }, 	//dha + 
	{ {0x0ca7,0x0cc0,0x0},  {0xf66c,0xf6c0,0x0cd5, 0x0} },	//dha +
	{ {0x0ca7,0x0cc6,0x0},	{0xf66b,0xf665,0xf686,0x0} }, //dha + 
	{ {0x0ca7,0x0cc7,0x0},	{0xf66b,0xf665,0xf686,0x0cd5,0x0} }, //dha + 
	{ {0x0ca7,0x0cc8,0x0},	{0xf66b,0xf665,0xf686,0xf687,0x0} }, //dha + 
	{ {0x0ca7,0x0cca,0x0},	{0xf66b,0xf665,0xf686,0x0cc2,0x0} }, //dha + 
	{ {0x0ca7,0x0ccb,0x0},	{0xf66b,0xf665,0xf686,0x0cc2,0x0cd5,0x0} }, //dha + 
	{ {0x0ca7,0x0ccc,0x0},	{0xf66b,0xf665,0xf688,0x0} }, //dha + 

	
	// Na

	{ {0x0ca8,0x0},		{0x0ca8,0x0} }, 	// Na -> Na
	{ {0X0ca8,0x0cbe,0x0},	{0xf66d,0x0cbe,0x0} }, // Na + 
	{ {0x0ca8,0x0cbf,0x0},	{0xf66e,0x0} }, 	//Na + 
	{ {0x0ca8,0x0cc0,0x0},  {0xf66e,0x0cd5, 0x0} },	//Na + 
	{ {0x0ca8,0x0cc6,0x0},	{0xf66d,0xf686,0x0} }, //Na + 
	{ {0x0ca8,0x0cc7,0x0},	{0xf66d,0xf686,0x0cd5,0x0} }, //Na + 
	{ {0x0ca8,0x0cc8,0x0},	{0xf66d,0xf686,0xf687,0x0} }, //Na + 
	{ {0x0ca8,0x0cca,0x0},	{0xf66d,0xf686,0x0cc2,0x0} }, //Na + 
	{ {0x0ca8,0x0ccb,0x0},	{0xf66d,0xf686,0x0cc2,0x0cd5,0x0} }, //Na + 
	{ {0x0ca8,0x0ccc,0x0},	{0xf66d,0xf688,0x0} }, //Na + 
	
	//pa
	
	{ {0x0caa,0x0},		{0x0caa,0x0} }, 	// pa -> pa
	{ {0X0caa,0x0cbe,0x0},	{0xf66f,0x0cbe,0x0} }, // pa + 
	{ {0x0caa,0x0cbf,0x0},	{0xf670,0x0} }, 	//pa + 
	{ {0x0caa,0x0cc0,0x0},  {0xf670,0x0cd5,0x0} },	//pa + 
	{ {0x0caa,0x0cc1,0x0},	{0x0caa,0xf683,0xf6ae,0x0} },	//pa +
	{ {0x0caa,0x0cc2,0x0},	{0x0caa,0xf684,0x0} },	//pa +	
	{ {0x0caa,0x0cc6,0x0},	{0xf66f,0xf686,0x0} }, //pa + 
	{ {0x0caa,0x0cc7,0x0},	{0xf66f,0xf686,0x0cd5,0x0} }, //pa + 
	{ {0x0caa,0x0cc8,0x0},	{0xf66f,0xf686,0xf687,0x0} }, //pa + 
	{ {0x0caa,0x0cca,0x0},	{0xf66f,0xf686,0xf684,0x0} }, //pa + 
	{ {0x0caa,0x0ccb,0x0},	{0xf66f,0xf686,0xf684,0x0cd5,0x0} }, //pa + 
	{ {0x0caa,0x0ccc,0x0},	{0xf66f,0xf688,0x0} }, //pa + 
	
	
	// fha
		
	{ {0x0cab,0x0},		{0x0cab,0x0} }, 	// fha -> fha
	{ {0X0cab,0x0cbe,0x0},	{0xf66f,0xf665,0x0cbe,0x0} }, // fha + 
	{ {0x0cab,0x0cbf,0x0},	{0xf670,0xf6c0,0x0} }, 	//fha + 
	{ {0x0cab,0x0cc0,0x0},  {0xf670,0xf6c0,0x0cd5,0x0} },	//fha + 
	{ {0x0cab,0x0cc1,0x0},	{0x0cab,0xf683,0xf6ae,0x0} },	//fha +	
	{ {0x0cab,0x0cc2,0x0},	{0x0cab,0xf684,0x0} },	//fha +		
	{ {0x0cab,0x0cc6,0x0},	{0xf66f,0xf665,0xf686,0x0} }, //fha + 
	{ {0x0cab,0x0cc7,0x0},	{0xf66f,0xf665,0xf686,0x0cd5,0x0} }, //fha + 
	{ {0x0cab,0x0cc8,0x0},	{0xf66f,0xf665,0xf686,0xf687,0x0} }, //fha + 
	{ {0x0cab,0x0cca,0x0},	{0xf66f,0xf665,0xf686,0xf684,0x0} }, //fha + 
	{ {0x0cab,0x0ccb,0x0},	{0xf66f,0xf665,0xf686,0xf684,0x0cd5,0x0} }, //fha + 
	{ {0x0cab,0x0ccc,0x0},	{0xf66f,0xf665,0xf688,0x0} }, //fha + 
		
	//ba
	
	{ {0x0cac,0x0},		{0x0cac,0x0} }, 	// ba -> ba
	{ {0X0cac,0x0cbe,0x0},	{0xf671,0x0cbe,0x0} }, // ba + 
	{ {0x0cac,0x0cbf,0x0},	{0xf672,0x0} }, 	//ba + 
	{ {0x0cac,0x0cc0,0x0},  {0xf672,0x0cd5, 0x0} },	//ba + 
	{ {0x0cac,0x0cc6,0x0},	{0xf671,0xf686,0x0} }, //ba + 
	{ {0x0cac,0x0cc7,0x0},	{0xf671,0xf686,0x0cd5,0x0} }, //ba + 
	{ {0x0cac,0x0cc8,0x0},	{0xf671,0xf686,0xf687,0x0} }, //ba + 
	{ {0x0cac,0x0cca,0x0},	{0xf671,0xf686,0x0cc2,0x0} }, //ba + 
	{ {0x0cac,0x0ccb,0x0},	{0xf671,0xf686,0x0cc2,0x0cd5,0x0} }, //ba + 
	{ {0x0cac,0x0ccc,0x0},	{0xf671,0xf688,0x0} }, //ba + 
	
	// bha
		
	{ {0x0cad,0x0},		{0x0cad,0x0} }, 	// bha -> bha
	{ {0X0cad,0x0cbe,0x0},	{0xf671,0xf665,0x0cbe,0x0} }, // bha + 
	{ {0x0cad,0x0cbf,0x0},	{0xf672,0xf6c0,0x0} }, 	//bha + 
	{ {0x0cad,0x0cc0,0x0},  {0xf672,0xf6c0,0x0cd5, 0x0} },	//bha + 
	{ {0x0cad,0x0cc6,0x0},	{0xf671,0xf665,0xf686,0x0} }, //bha + 
	{ {0x0cad,0x0cc7,0x0},	{0xf671,0xf665,0xf686,0x0cd5,0x0} }, //bha + 
	{ {0x0cad,0x0cc8,0x0},	{0xf671,0xf665,0xf686,0xf687,0x0} }, //bha + 
	{ {0x0cad,0x0cca,0x0},	{0xf671,0xf665,0xf686,0x0cc2,0x0} }, //bha + 
	{ {0x0cad,0x0ccb,0x0},	{0xf671,0xf665,0xf686,0x0cc2,0x0cd5,0x0} }, //bha + 
	{ {0x0cad,0x0ccc,0x0},	{0xf671,0xf665,0xf688,0x0} }, //bha + 
	
	//ma
	
	{ {0x0cae,0x0},		{0x0cae,0x0} },		//ma -> ma 
	{ {0x0cae,0x0cbe,0x0},	{0xf673,0xf65d,0x0cbe,0x0} },
	{ {0x0cae,0x0cbf,0x0},	{0xf674,0xf6ad,0x0} },
	{ {0x0cae,0x0cc0,0x0},	{0xf674,0xf6ad,0x0cd5,0x0} },
	{ {0x0cae,0x0cc6,0x0},	{0xf673,0xf686,0xf6ad,0x0} },
	{ {0x0cae,0x0cc7,0x0},	{0xf673,0xf686,0xf6ad,0x0cd5,0x0} },
	{ {0x0cae,0x0cc8,0x0},	{0xf673,0xf686,0xf6ad,0x0f687,0x0} },
	{ {0x0cae,0x0cca,0x0},	{0xf673,0xf686,0x0cc2,0x0} },
	{ {0x0cae,0x0ccb,0x0},	{0xf673,0xf686,0x0cc2,0x0cd5,0x0} },
	{ {0x0cae,0x0ccc,0x0},	{0x0cb5,0xf65d,0xf688,0x0} },
	
	// ya
	
	{ {0x0caf,0x0},		{0x0caf,0x0} },
	{ {0x0caf,0x0cbe,0x0},	{0xf6ac,0xf6ad,0xf682,0xf65d,0xf6bb,0x0cbe,0x0} },
	{ {0x0caf,0x0cbf,0x0},	{0xf675,0xf6ad,0x0} },
	{ {0x0caf,0x0cc0,0x0},	{0xf675,0xf6ad,0x0} },
	{ {0x0caf,0x0cc6,0x0},	{0xf6ac,0xf65d,0xf686,0xf6ad,0x0} },
	{ {0x0caf,0x0cc7,0x0},	{0xf6ac,0xf65d,0xf686,0xf6ad,0x0cd5,0x0} },
	{ {0x0caf,0x0cc8,0x0},	{0xf6ac,0xf65d,0xf686,0xf6ad,0xf687,0x0} },
	{ {0x0caf,0x0cca,0x0},	{0xf6ac,0xf65d,0xf686,0x0cc2,0x0} },
	{ {0x0caf,0x0ccb,0x0},	{0xf6ac,0xf65d,0xf686,0x0cc2,0x0cd5,0x0} },
	{ {0x0caf,0x0ccc,0x0},	{0xf6ac,0xf65d,0xf682,0xf65d,0xf688,0x0} },


	//ra
	
	{ {0x0cb0,0x0},		{0x0cb0,0x0} }, 	// ra -> ra
	{ {0X0cb0,0x0cbe,0x0},	{0xf65b,0x0cbe,0x0} }, // ra + 
	{ {0x0cb0,0x0cbf,0x0},	{0xf65e,0x0} }, 	//ra + 
	{ {0x0cb0,0x0cc0,0x0},  {0xf65e,0x0cd5, 0x0} },	//ra + 
	{ {0x0cb0,0x0cc6,0x0},	{0xf65b,0xf686,0x0} }, //ra + 
	{ {0x0cb0,0x0cc7,0x0},	{0xf65b,0xf686,0x0cd5,0x0} }, //ra + 
	{ {0x0cb0,0x0cc8,0x0},	{0xf65b,0xf686,0xf687,0x0} }, //ra + 
	{ {0x0cb0,0x0cca,0x0},	{0xf65b,0xf686,0x0cc2,0x0} }, //ra + 
	{ {0x0cb0,0x0ccb,0x0},	{0xf65b,0xf686,0x0cc2,0x0cd5,0x0} }, //ra + 
	{ {0x0cb0,0x0ccc,0x0},	{0xf65b,0xf688,0x0} }, //ra + 
	
	//raa
	
	{ {0x0cb1,0x0},		{0x0cb1,0x0} }, 	// ra -> ra

	//la

	{ {0x0cb2,0x0},		{0x0cb2,0x0} }, 	// la -> la
	{ {0X0cb2,0x0cbe,0x0},	{0xf676,0x0cbe,0x0} }, // la + 
	{ {0x0cb2,0x0cbf,0x0},	{0xf677,0x0} }, 	//la + 
	{ {0x0cb2,0x0cc0,0x0},  {0xf677,0x0cd5, 0x0} },	//la + 
	{ {0x0cb2,0x0cc6,0x0},	{0xf676,0xf686,0x0} }, //la + 
	{ {0x0cb2,0x0cc7,0x0},	{0xf676,0xf686,0x0cd5,0x0} }, //la + 
	{ {0x0cb2,0x0cc8,0x0},	{0xf676,0xf686,0xf687,0x0} }, //la + 
	{ {0x0cb2,0x0cca,0x0},	{0xf676,0xf686,0x0cc2,0x0} }, //la + 
	{ {0x0cb2,0x0ccb,0x0},	{0xf676,0xf686,0x0cc2,0x0cd5,0x0} }, //la + 
	{ {0x0cb2,0x0ccc,0x0},	{0xf676,0xf688,0x0} }, //la + 

	//adda
	
	{ {0x0cb3,0x0},		{0x0cb3,0x0} }, 	// adda -> adda
	{ {0X0cb3,0x0cbe,0x0},	{0xf680,0x0cbe,0x0} }, // adda + 
	{ {0x0cb3,0x0cbf,0x0},	{0xf681,0x0} }, 	//adda + 
	{ {0x0cb3,0x0cc0,0x0},  {0xf681,0x0cd5, 0x0} },	//adda + 
	{ {0x0cb3,0x0cc6,0x0},	{0xf680,0xf686,0x0} }, //adda + 
	{ {0x0cb3,0x0cc7,0x0},	{0xf680,0xf686,0x0cd5,0x0} }, //adda + 
	{ {0x0cb3,0x0cc8,0x0},	{0xf680,0xf686,0xf687,0x0} }, //adda + 
	{ {0x0cb3,0x0cca,0x0},	{0xf680,0xf686,0x0cc2,0x0} }, //adda + 
	{ {0x0cb3,0x0ccb,0x0},	{0xf680,0xf686,0x0cc2,0x0cd5,0x0} }, //adda + 
	{ {0x0cb3,0x0ccc,0x0},	{0xf680,0xf688,0x0} }, //adda + 
	
	// va
	
	{ {0x0cb5,0x0},		{0x0cb5,0x0} }, 	// va -> va
	{ {0X0cb5,0x0cbe,0x0},	{0xf673,0x0cbe,0x0} }, // va + 
	{ {0x0cb5,0x0cbf,0x0},	{0xf674,0x0} }, 	//va + 
	{ {0x0cb5,0x0cc1,0x0},	{0x0cb5,0xf683,0xf6ae,0x0} },	//va +
	{ {0x0cb5,0x0cc2,0x0},	{0x0cb5,0xf684,0x0} }, // va +
	{ {0x0cb5,0x0cc0,0x0},  {0xf674,0x0cd5, 0x0} },	//va + 
	{ {0x0cb5,0x0cc6,0x0},	{0xf673,0xf686,0x0} }, //va + 
	{ {0x0cb5,0x0cc7,0x0},	{0xf673,0xf686,0x0cd5,0x0} }, //va + 
	{ {0x0cb5,0x0cc8,0x0},	{0xf673,0xf686,0xf687,0x0} }, //va + 
	{ {0x0cb5,0x0cca,0x0},	{0xf673,0xf686,0xf684,0x0} }, //va + 
	{ {0x0cb5,0x0ccb,0x0},	{0xf673,0xf686,0xf684,0x0cd5,0x0} }, //va + 
	{ {0x0cb5,0x0ccc,0x0},	{0xf673,0xf688,0x0} }, //va + 

	//sha
	
	{ {0x0cb6,0x0},		{0x0cb6,0x0} }, 	// sha -> sha
	{ {0X0cb6,0x0cbe,0x0},	{0xf678,0x0cbe,0x0} }, // sha + 
	{ {0x0cb6,0x0cbf,0x0},	{0xf679,0x0} }, 	//sha + 
	{ {0x0cb6,0x0cc0,0x0},  {0xf679,0x0cd5, 0x0} },	//sha + 
	{ {0x0cb6,0x0cc6,0x0},	{0xf678,0xf686,0x0} }, //sha + 
	{ {0x0cb6,0x0cc7,0x0},	{0xf678,0xf686,0x0cd5,0x0} }, //sha + 
	{ {0x0cb6,0x0cc8,0x0},	{0xf678,0xf686,0xf687,0x0} }, //sha + 
	{ {0x0cb6,0x0cca,0x0},	{0xf678,0xf686,0x0cc2,0x0} }, //sha + 
	{ {0x0cb6,0x0ccb,0x0},	{0xf678,0xf686,0x0cc2,0x0cd5,0x0} }, //sha + 
	{ {0x0cb6,0x0ccc,0x0},	{0xf678,0xf688,0x0} }, //sha + 
	
	//Sha

	{ {0x0cb7,0x0},		{0x0cb7,0x0} }, 	// Sha
	{ {0X0cb7,0x0cbe,0x0},	{0xf67a,0x0cbe,0x0} }, // Sha + 
	{ {0x0cb7,0x0cbf,0x0},	{0xf67b,0x0} }, 	//Sha + 
	{ {0x0cb7,0x0cc0,0x0},  {0xf67b,0x0cd5, 0x0} },	//Sha + 
	{ {0x0cb7,0x0cc6,0x0},	{0xf67a,0xf686,0x0} }, //Sha + 
	{ {0x0cb7,0x0cc7,0x0},	{0xf67a,0xf686,0x0cd5,0x0} }, //Sha + 
	{ {0x0cb7,0x0cc8,0x0},	{0xf67a,0xf686,0xf687,0x0} }, //Sha + 
	{ {0x0cb7,0x0cca,0x0},	{0xf67a,0xf686,0x0cc2,0x0} }, //Sha + 
	{ {0x0cb7,0x0ccb,0x0},	{0xf67a,0xf686,0x0cc2,0x0cd5,0x0} }, //Sha + 
	{ {0x0cb7,0x0ccc,0x0},	{0xf67a,0xf688,0x0} }, //Sha + 
	
	//sa
	
	{ {0x0cb8,0x0},		{0x0cb8,0x0} }, 	// sa
	{ {0X0cb8,0x0cbe,0x0},	{0xf67c,0x0cbe,0x0} }, // sa + 
	{ {0x0cb8,0x0cbf,0x0},	{0xf67d,0x0} }, 	//sa + 
	{ {0x0cb8,0x0cc0,0x0},  {0xf67d,0x0cd5, 0x0} },	//sa + 
	{ {0x0cb8,0x0cc6,0x0},	{0xf67c,0xf686,0x0} }, //sa + 
	{ {0x0cb8,0x0cc7,0x0},	{0xf67c,0xf686,0x0cd5,0x0} }, //sa + 
	{ {0x0cb8,0x0cc8,0x0},	{0xf67c,0xf686,0xf687,0x0} }, //sa + 
	{ {0x0cb8,0x0cca,0x0},	{0xf67c,0xf686,0x0cc2,0x0} }, //sa + 
	{ {0x0cb8,0x0ccb,0x0},	{0xf67c,0xf686,0x0cc2,0x0cd5,0x0} }, //sa + 
	{ {0x0cb8,0x0ccc,0x0},	{0xf67c,0xf688,0x0} }, //sa + 

	//ha
	
	{ {0x0cb9,0x0},		{0x0cb9,0x0} }, 	// ha 
	{ {0X0cb9,0x0cbe,0x0},	{0xf67e,0x0cbe,0x0} }, // ha + 
	{ {0x0cb9,0x0cbf,0x0},	{0xf67f,0x0} }, 	//ha + 
	{ {0x0cb9,0x0cc0,0x0},  {0xf67f,0x0cd5, 0x0} },	//ha + 
	{ {0x0cb9,0x0cc6,0x0},	{0xf67e,0xf686,0x0} }, //ha + 
	{ {0x0cb9,0x0cc7,0x0},	{0xf67e,0xf686,0x0cd5,0x0} }, //ha + 
	{ {0x0cb9,0x0cc8,0x0},	{0xf67e,0xf686,0xf687,0x0} }, //ha + 
	{ {0x0cb9,0x0cca,0x0},	{0xf67e,0xf686,0x0cc2,0x0} }, //ha + 
	{ {0x0cb9,0x0ccb,0x0},	{0xf67e,0xf686,0x0cc2,0x0cd5,0x0} }, //ha + 
	{ {0x0cb9,0x0ccc,0x0},	{0xf67e,0xf688,0x0} }, //ha + 

	
	// Halant 
	
	{ {0x0ccd,0x0c95,0x0},	{0xf689,0xf6b0,0x0} },			// halant + ka
	{ {0x0ccd,0x0c95,0x0cc3,0x0},	{0xf689,0xf6ae,0x0cc3,0x0} },			// halant + ka
	{ {0x0ccd,0x0c96,0x0},	{0xf68a,0xf6b7,0x0} },			// halant + kha
	{ {0x0ccd,0x0c97,0x0},	{0xf68b,0xf6b4,0x0} },			// halant + ga
	{ {0x0ccd,0x0c98,0x0},	{0xf68c,0xf6b7,0x0} },			// halant + gha
	{ {0x0ccd,0x0c99,0x0},	{0xf68d,0xf6b8,0x0} },			// halant + dda
	{ {0x0ccd,0x0c9a,0x0},	{0xf68e,0xf6b8,0x0} },			// halant + cha
	{ {0x0ccd,0x0c9b,0x0},	{0xf68f,0xf6b7,0x0} },			// halant + Chaa
	{ {0x0ccd,0x0c9c,0x0},	{0xf690,0xf6b8,0x0} },			// halant + ja
	{ {0x0ccd,0x0c9d,0x0},	{0xf691,0xf6b1,0x0} },			// halant + jha
	{ {0x0ccd,0x0c9e,0x0},	{0xf692,0xf6bc,0x0} },			// halant + dhna
	{ {0x0ccd,0x0c9f,0x0},	{0xf693,0xf6b4,0x0} },			// halant + Ta
	{ {0x0ccd,0x0ca0,0x0},	{0xf694,0xf6b4,0x0} },			// halant + Tha
	{ {0x0ccd,0x0ca1,0x0},	{0xf695,0xf6b8,0x0} },			// halant + dha
	{ {0x0ccd,0x0ca2,0x0},	{0xf696,0xf6b8,0x0} },			// halant + Dha
	{ {0x0ccd,0x0ca3,0x0},	{0xf697,0xf6b8,0x0} },			// halant + na
	{ {0x0ccd,0x0ca4,0x0},	{0xf698,0xf6bc,0x0} },			// halant + ta
	{ {0x0ccd,0x0ca4,0x0cc3,0x0},	{0xf698,0xf6ac,0x0cc3,0x0} },			// halant + ta
	{ {0x0ccd,0x0ca5,0x0},	{0xf699,0xf6b8,0x0} },			// halant + tha
	{ {0x0ccd,0x0ca6,0x0},	{0xf69a,0xf6b8,0x0} },			// halant + da
	{ {0x0ccd,0x0ca7,0x0},	{0xf69b,0xf6b8,0x0} },			// halant + dha
	{ {0x0ccd,0x0ca8,0x0},	{0xf69c,0xf6b4,0x0} },			// halant + Na
	{ {0x0ccd,0x0caa,0x0},	{0xf69d,0xf6b4,0x0} },			// halant + pa
	{ {0x0ccd,0x0cab,0x0},	{0xf69e,0xf6b4,0x0} },			// halant + pha
	{ {0x0ccd,0x0cac,0x0},	{0xf69f,0xf6b4,0x0} },			// halant + ba
	{ {0x0ccd,0x0cad,0x0},	{0xf6a0,0xf6b4,0x0} },			// halant + bha
	{ {0x0ccd,0x0cae,0x0},	{0xf6a1,0xf6b7,0x0} },			// halant + ma
	{ {0x0ccd,0x0caf,0x0},	{0xf6a2,0xf6b7,0x0} },			// halant + ya
	{ {0x0ccd,0x0cb0,0x0},	{0xf6a3,0xf6b7,0x0} },			// halant + ra
	{ {0x0ccd,0x0cb1,0x0},	{0xf6a4,0xf6b7,0x0} },			// halant + raa
	{ {0x0ccd,0x0cb2,0x0},	{0xf6a5,0xf6b8,0x0} },			// halant + la
	{ {0x0ccd,0x0cb5,0x0},	{0xf6a6,0xf6b4,0x0} },			// halant + va
	{ {0x0ccd,0x0cb6,0x0},	{0xf6a7,0xf6b7,0x0} },			// halant + sha
	{ {0x0ccd,0x0cb7,0x0},	{0xf6a8,0xf6b8,0x0} },			// halant + Sha
	{ {0x0ccd,0x0cb8,0x0},	{0xf6a9,0xf6b4,0x0} },			// halant + sa
	{ {0x0ccd,0x0cb9,0x0},	{0xf6aa,0xf6b4,0x0} },			// halant + ha
	{ {0x0ccd,0x0cb3,0x0},	{0xf6ab,0xf6b1,0x0} },			// halant + ddaa
	
	// Digits
	
	{ {0x0ce6,0x0},		{0x0ce6, 0x0} },		//zero
	{ {0x0ce7,0x0},		{0x0ce7, 0x0} },		//one
	{ {0x0ce8,0x0},		{0x0ce8, 0x0} },		//two
	{ {0x0ce9,0x0},		{0x0ce9, 0x0} },		//three
	{ {0x0cea,0x0},		{0x0cea, 0x0} },		//four
	{ {0x0ceb,0x0},		{0x0ceb, 0x0} },		//five
	{ {0x0cec,0x0},		{0x0cec, 0x0} },		//six
	{ {0x0ced,0x0},		{0x0ced, 0x0} },		//seven
	{ {0x0cee,0x0},		{0x0cee, 0x0} },		//eight
	{ {0x0cef,0x0},		{0x0cef, 0x0} },		//nine

};


static const GujaGlyphEntry GujaGlyphTbl[MAP_GUJA_SIZE] = {

	//Vowel Modifiers 3

	{ {0x0A81,0x0},		{0x0A81,0x0} },
	{ {0x0A82,0x0},		{0x0A82,0x0} },
	{ {0x0A83,0x0},		{0x0A83,0x0} },

	// vowels 19

	{ {0x0A85,0x0},		{0x0A85,0x0} },
	{ {0x0A86,0x0},		{0x0A86,0x0} },
	{ {0x0A87,0x0},		{0x0A87,0x0} },
	{ {0x0A87,0x0A82,0x0},	{0xF4EC,0x0} },
	{ {0x0A88,0x0},		{0x0A88,0x0} },
	{ {0x0A88,0x0A82,0x0},	{0xF4ED,0x0} },
	{ {0x0A89,0x0},		{0x0A89,0x0} },
	{ {0x0A89,0x0A82,0x0},	{0xF4EE,0x0} },
	{ {0x0A8a,0x0},		{0x0A8a,0x0} },	 	
	{ {0x0A8a,0x0A82,0x0},	{0xF4EF,0x0} },	 	
	{ {0x0A8b,0x0},		{0x0A8b,0xF559,0x0} },	
	{ {0x0AE0,0x0},		{0x0AE0,0xF559,0X0} },	
	{ {0x0A8d,0x0},		{0x0A8d,0x0} },
	{ {0x0A8f,0x0},		{0x0A8f,0x0} },
	{ {0x0A90,0x0},		{0x0A90,0x0} },
	{ {0x0A91,0x0},		{0x0A91,0x0} },
	{ {0x0A93,0x0},		{0x0A93,0x0} },
	{ {0x0A94,0x0},		{0x0A94,0x0} },
										
	// Vowel signs 13							

	{ {0x0ABe,0x0},		{0x0ABe,0x0} },
	{ {0x0ABf,0x0},		{0x0ABf,0x0} },
	{ {0x0AC0,0x0},		{0x0AC0,0x0} },
	{ {0x0AC1,0x0},		{0x0AC1,0x0} },
	{ {0x0AC2,0x0},		{0x0AC2,0x0} },
	{ {0x0AC3,0x0},		{0x0AC3,0x0} },
	{ {0x0AC4,0x0},		{0x0AC4,0x0} },
	{ {0x0AC5,0x0},		{0x0AC5,0x0} },
	{ {0x0AC7,0x0},		{0x0AC7,0x0} },
	{ {0x0AC8,0x0},		{0x0AC8,0x0} },
	{ {0x0AC9,0x0},		{0x0AC9,0x0} },
	{ {0x0ACb,0x0},		{0x0ACb,0x0} },
	{ {0x0ACc,0x0},		{0x0ACc,0x0} } ,

	// Consonants
	// ka 6
	{ {0x0A95,0x0},		{0x0A95,0xF55A,0x0} },// ka -> ka + kern space
	{ {0x0A95,0x0ACd,0x0},	{0xF4F0,0xF55A,0x0} },// ka + halant -> half ka
	{ {0x0A95,0x0ACd,0x0AB0,0x0},{0xF4F2,0xF55A,0x0} },// ka + halant + ra -> kra + kern space
	{ {0x0A95,0x0ACd,0x0AB7,0x0},	{0xF4F3,0x0ABE,0x0} },// ka + halant + SHa -> kSHa 	
	{ {0x0A95,0x0ACd,0x0AB7,0x0ACd,0x0},	{0xF4F3,0x0} },// ka + halant + SHa + halant -> half kSHa
	{ {0x0A95,0x0ACd,0x0A95,0x0} ,	{0xF4F1,0xF55A,0x0} },// ka + halant + ka ->  	

	// kha 4

	{ {0x0A96,0x0},			{0x0A96,0x0} },
	{ {0x0A96,0x0ACd,0x0},		{0xF4F4,0x0} },
	{ {0x0A96,0x0ACd,0x0AB0,0x0},	{0xF4F5,0x0ABE ,0x0 } } ,
	{ {0x0A96,0x0ACd,0x0AB0,0x0ACd,0x0},{0xF4F5,0x0} } ,

	// ga 4
	{ {0x0A97,0x0},			{0x0A97,0x0} },
	{ {0x0A97,0x0ACd,0x0 },		{0xF4F6,0x0} },
	{ {0x0A97,0x0ACd,0x0AB0,0x0},	{0xF4F7,0x0ABE,0x0} },
	{ {0x0A97,0x0ACd,0x0AB0,0x0ACd,0x0},	{0xF4F7,0x0} },

	// gha 4
	{ {0x0A98,0x0},			{0x0A98,0x0} },
	{ {0x0A98,0x0ACd,0x0},		{0xF4F8,0x0} },
	{ {0x0A98,0x0ACd,0x0AB0,0x0},	{0xF4F9,0x0ABE,0x0 } },
	{ {0x0A98,0x0ACd,0x0AB0,0x0ACd,0x0},	{0xF4F9,0x0} },

	// nga 1
	{ {0x0A99,0x0} ,		{0x0A99,0xF55B,0x0} },

	// cha 4
	{ {0x0A9a,0x0},			{0x0A9a,0x0 } },
	{ {0x0A9a,0x0ACd,0x0},		{0xF4FA,0x0 } },// cha  + halant -> half cha  
	{ {0x0A9a,0x0ACd,0x0AB0,0x0},	{0xF4FA,0x0ABE,0x0} },// cha + halant ra-> chra 
	{ {0x0A9a,0x0ACd,0x0AB0,0x0ACd,0x0},	{0xF4FA,0x0 } },// cha  + halant ra + halant 

	// chha 1
	{ {0x0A9b,0x0} ,		{0x0A9b,0xF55C,0x0} },

	// ja 5
	{ {0x0A9c,0x0},			{0x0A9c,0xF55D,0x0} },// ja 
	{ {0x0A9c,0x0ACd,0x0},		{0xF4FC,0x0} },//ja + halant -> half ja 
	{ {0x0A9c,0x0ABE,0x0},		{0xF4FD,0x0} },// ja + Aa matra
	{ {0x0A9c,0x0AC0,0x0},		{0xF4FE,0xF55D,0x0} },// ja + Ee matra
	{ {0x0A9c,0x0ACd,0x0AB0,0x0},	{0xF4FF,0xF55D,0x0} },// ja + halant + ra 

	// dna 2
	{ {0x0A9c,0x0ACd,0x0A9e,0x0},	{0xF500,0x0ABE,0x0} },// ja + halant + jna ->  dna 
	{ {0x0A9c,0x0ACd,0x0A9e,0x0ACd,0x0},	{0xF500,0x0} },// ja + halant + jna ->  half dna 

	// jha 3
	{ {0x0A9d,0x0},		{0x0A9d,0xF55E,0x0} },// jha 
	{ {0x0A9d,0x0ACd,0x0},	{0xF501,0x0} },// jha + halant -> half jha 
	{ {0x0A9d,0x0ACd,0x0AB0,0x0},	{0xF502,0xF55E,0x0} },// jha + halant + ra -> jhra

	// nya 2
	{ {0x0A9e,0x0},			{0x0A9e,0x0} },// nya
	{ {0x0A9e,0x0ACd,0x0},		{0xF503,0x0} },// nya + halant -> half nya

	// Ta 3
	{ {0x0A9f,0x0},		{0x0A9F,0xF55F,0x0} },// Ta -> Ta + kern space
	{ {0x0A9f,0x0ACd,0x0A9f,0x0},	{0xF504,0xF55F,0x0} },// Ta+halant+Ta -> TaTa+kern space
	{ {0x0A9f,0x0ACd,0x0AA0,0x0},	{0xF505,0xF55F,0x0} },// Ta+halant+Tha -> TaTha+kern space

	// Tha 2
	{ {0x0AA0,0x0},			{0x0AA0,0xF560,0x0} },// Tha -> Tha + kern space
	{ {0x0AA0,0x0ACd,0x0AA0,0x0},	{0xF506,0xF560,0x0} },//Tha+halant+Tha ->   + kern space

	// Da 3
	{ {0x0AA1,0x0},			{0x0AA1,0xF561,0x0} },// Da  -> Da  + kern space
	{ {0x0AA1,0x0ACd,0x0AA1,0x0},	{0xF507,0xF561,0x0} },// Da+halant+Da ->  + kern space
	{ {0x0AA1,0x0ACd,0x0AA2,0x0},  {0xF508,0xF561,0x0} },// Da+halant+Dha ->  + kern space

	// Dha 2
	{ {0x0AA2,0x0},			{0x0AA2,0xF562,0x0} },// Dha+kern space 
	{ {0x0AA2,0x0ACd,0x0AA2,0x0} ,	{0xF509,0xF562,0x0} },// Dha+halant+Dha->
	// Nna 4
	{ {0x0AA3,0x0},			{0x0AA3,0x0} },// Nna
	{ {0x0AA3,0x0ACd,0x0 },		{0xF50A,0x0 } },// Nna + halant -> half Nna
	{ {0x0AA3,0x0AC1,0x0 },		{0xF50B,0x0 } },// Nna + UU matra -> 
	{ {0x0AA3,0x0AC2,0x0 },		{0xF50C,0x0 } },// Nna + uu matra -> 

	// ta 6
	{ {0x0AA4,0x0},			{0x0AA4,0x0} },// ta
	{ {0x0AA4,0x0ACd,0x0},		{0xF50D,0x0} },//ta+halant -> half ta
	{ {0x0AA4,0x0ACd,0x0AB0,0x0},	{0xF50E,0x0ABE,0x0} },// ta + halant + ra -> tra
	{ {0x0AA4,0x0ACd,0x0AB0,0x0ACd,0x0},{0xF50E,0x0} },// ta + halant + ra + halant ->  half tra 
	{ {0x0AA4,0x0ACd,0x0AA4,0x0},	{0xF50F,0x0ABE,0x0} },// ta + halant + ta -> 
	{ {0x0AA4,0x0ACd,0x0AA4,0x0ACd,0x0},{0xF50F,0x0} },// ta + halant + ta + halant -> 

	// tha 4
	{ {0x0AA5,0x0},			{0x0AA5,0x0 } },// tha
	{ {0x0AA5,0x0ACd,0x0},		{0xF510,0x0 } },// tha + halant -> half tha
	{ {0x0AA5,0x0ACd,0x0AB0,0x0},	{0xF511,0x0ABE,0x0} },// tha + halant + ra -> 
	{ {0x0AA5,0x0ACd,0x0AB0,0x0ACd,0x0},{0xF511,0x0 } },// tha + halant + ra + halant ->
	// da 2
	{ {0x0AA6,0x0 },		{0x0AA6,0xF563,0x0} },// da -> da + kern space
	{ {0x0AA6,0x0ACd,0x0 },		{0x0AA6,0x0ACd,0xF563,0x0} },// da + halant -> half da + kern space
	// da halant ra 1
	{ {0x0AA6,0x0ACd,0x0AB0,0x0} ,	{0xF512,0xF563,0x0} },// da + halant + ra -> dra + kern space 
	{ {0x0AA6,0x0ACd,0x0AB0,0x0ACd,0x0} ,	{0xF512,0x0ACd,0xF563,0x0} },

	// da halant ma 1
	{ {0x0AA6,0x0ACd,0x0AAe,0x0},	{0xF513,0x0} },// da + halant + ma ->

	// da halant da 1
	{ {0x0AA6,0x0ACd,0x0AA6,0x0 },	{0xF514,0xF563,0x0 } },// da + halant + da  - >  + kern space 

	// da halant dha 1
	{ {0x0AA6,0x0ACd,0x0AA7,0x0},	{0xF515,0xF563,0x0 } },// da + halant + dha ->   + kern space 

	// da halant ya 1
	{ {0x0AA6,0x0ACd,0x0AAf,0x0 },	{0xF516,0x0} },//da+halant+ya-> 

	// da halant va 1
	{ {0x0AA6,0x0ACd,0x0AB5,0x0},	{0xF517,0xF563,0x0 } },// da + halant + va ->   + kern space 
	// da ri 1
	{ {0x0AA6,0x0AC3,0x0},		{0xF518,0xF563,0x0 } },// da + ri matra

	// dha 4
	{ {0x0AA7,0x0},			{0x0AA7,0x0} },// Dha 
	{ {0x0AA7,0x0ACd,0x0},		{0xF519,0x0} },// Dha + halant - > half Dha
	{ {0x0AA7,0x0ACd,0x0AB0,0x0 },	{0xF51A,0x0ABE,0x0} },//Dha+halant+ra->
	{ {0x0AA7,0x0ACd,0x0AB0,0x0ACd,0x0},{0xF51A,0x0} },// Dha + halant + ra + halant -> 

	// na 6
	{ {0x0AA8,0x0 },		{0x0AA8,0x0} },// na
	{ {0x0AA8,0x0ACd,0x0 },		{0xF51B,0x0} },// na + halant  ->  half na
	{ {0x0AA8,0x0ACd,0x0AB0,0x0 },	{0xF51C,0x0ABE,0x0} },// na + halant + ra  ->  	
	{ {0x0AA8,0x0ACd,0x0AB0,0x0ACd,0x0 },	{0xF51C,0x0} },// na + halant + ra + halant ->
	{ {0x0AA8,0x0ACd,0x0AA8,0x0 },	{0xF51D,0x0ABE,0x0} },// na + halant + na ->
	{ {0x0AA8,0x0ACd,0x0AA8,0x0ACd,0x0 },	{0xF51D,0x0} },// na + halant + na + halant ->

	// pa 4
	{ {0x0AAa,0x0 },		{0x0AAa,0x0} },// pa
	{ {0x0AAa,0x0ACd,0x0 },		{0xF51E,0x0} },// pa + halant -> half pa 
	{ {0x0AAa,0x0ACd,0x0AB0,0x0 },	{0xF51F,0x0ABE,0x0} },// pa + halant +ra -> pra
	{ {0x0AAa,0x0ACd,0x0AB0,0x0ACd,0x0 },	{0xF51F,0x0} },// pa + halant + ra + halant -> half pra

	// pha 3
	{ {0x0AAb,0x0 },		{0x0AAb,0xF55A,0x0} },// pha -> pha + kern space
	{ {0x0AAb,0x0ACd,0x0 },		{0xF520,0xF55A} },// pha + halant	-> half pha
	{ {0x0AAb,0x0ACd,0x0AB0,0x0 },	{0xF521,0xF55A,0x0} },// pha + halant + ra -> fra + kern space

	// ba 4
	{ {0x0AAc,0x0 },		{0x0AAc,0x0} },// ba
	{ {0x0AAc,0x0ACd,0x0 },		{0xF522,0x0} },// ba + halant -> half ba
	{ {0x0AAc,0x0ACd,0x0AB0,0x0 },	{0xF523,0x0ABE,0x0} },// ba + halant + ra -> 
	{ {0x0AAc,0x0ACd,0x0AB0,0x0ACd,0x0 },	{0xF523,0x0} },// ba + halant ra + halant ->

	// bha 4
	{ {0x0AAd,0x0 },		{0x0AAd,0x0} },// bha 
	{ {0x0AAd,0x0ACd,0x0 },		{0xF524,0x0} },// bha + halant -> half halant 
	{ {0x0AAd,0x0ACd,0x0AB0,0x0 },	{0xF525,0x0ABE,0x0} },// bha + halant + ra -> 
	{ {0x0AAd,0x0ACd,0x0AB0,0x0ACd,0x0 },	{0xF525,0x0} },// bha + halant + ra + halant -> 

	// ma 4	
	{ {0x0AAe,0x0 },		{0x0AAe,0x0} },	// ma 
	{ {0x0AAe,0x0ACd,0x0 },		{0xF526,0x0} },// ma + halant -> half ma
	{ {0x0AAe,0x0ACd,0x0AB0,0x0 },	{0xF527,0x0ABE,0x0} },// ma + halant + ra ->
	{ {0x0AAe,0x0ACd,0x0AB0,0x0ACd,0x0 },	{0xF527,0x0} },// ma + halant + ra + halant -> 
	// ya 2	
	{ {0x0AAf,0x0 },		{0x0AAf,0x0} },// ya
	{ {0x0AAf,0x0ACd,0x0 },		{0xF528,0x0} },// ya + halant -> half ya

	// ra 5 
	{ {0x0AB0,0x0 },		{0x0AB0,0xF564,0x0} },// ra 
	{ {0x0AB0,0x0AC1,0x0 },		{0xF529,0xF565,0x0} },// ra + u -> Ru + kern space
	{ {0x0AB0,0x0AC2,0x0 },		{0xF52B,0xF564,0x0} },// ra + U -> RU + kern space
	{ {0x0AB0,0x0AC1,0x0AC1,0x0 },	{0x0AB0,0x0AC1,0xF565,0x0} },// ra + u + u -> Ru + kern space
	{ {0x0AB0,0x0AC2,0x0AC1,0x0 },	{0xF52A,0xF566,0x0} },// ra + U + U-> RU + kern space

	// la 2
	{ {0x0AB2,0x0 },		{0x0AB2,0x0} },// la 
	{ {0x0AB2,0x0ACd,0x0 },		{0xF52C,0x0} },// la + halant ->

	// La 2
	{ {0x0AB3,0x0 },		{0x0AB3,0x0} },// La
	{ {0x0AB3,0x0ACd,0x0 },		{0xF52D,0x0} },// La + halant -> half La

	// va 4
	{ {0x0AB5,0x0 },		{0x0AB5,0x0} },// va
	{ {0x0AB5,0x0ACd,0x0 },		{0xF52E,0x0} },// va + halant -> half va
	{ {0x0AB5,0x0ACd,0x0AB0,0x0 },	{0xF52F,0x0ABE,0x0} },// va + halant + ra -> 
	{ {0x0AB5,0x0ACd,0x0AB0,0x0ACd,0x0 },	{0xF52F,0x0} },// va + halant + ra + halant ->  

	// sha 8
	{ {0x0AB6,0x0 },		{0x0AB6,0x0} },// sha
	{ {0x0AB6,0x0ACd,0x0 },		{0xF530,0x0} },// sha + halant -> half sha
	{ {0x0AB6,0x0ACd,0x0AB0,0x0 },	{0xF532,0x0ABE,0x0} },// sha + halant + ra -> shra
	{ {0x0AB6,0x0ACd,0x0AB0,0x0ACd,0x0 },	{0xF532,0x0} },// sha + halant + ra + halant -> half shra
	{ {0x0AB6,0x0ACd,0x0AB5,0x0 },	{0xF533,0x0ABE,0x0} },// sha + halant + va -> shwa
	{ {0x0AB6,0x0ACd,0x0AB5,0x0ACd,0x0 },	{0xF533,0x0} },// sha + halant + va + halant -> half shwa
	{ {0x0AB6,0x0ACd,0x0A9A,0x0},	{0xF56A,0x0} },
	{ {0x0AB6,0x0ACd,0x0AA8,0x0},	{0xF56B,0x0} },

	// SHa 2
	{ {0x0AB7,0x0 },		{0x0AB7,0x0} },// SHa
	{ {0x0AB7,0x0ACd,0x0 },		{0xF534,0x0} },// SHa + halant -> half SHa 

	// sa 4
	{ {0x0AB8,0x0 },		{0x0AB8,0x0} },// sa
	{ {0x0AB8,0x0ACd,0x0 },		{0xF535,0x0} },// sa + halant -> half sa
	{ {0x0AB8,0x0ACd,0x0AB0,0x0 },	{0xF536,0x0ABE,0x0} },// sa + halant + ra -> 
	{ {0x0AB8,0x0ACd,0x0AB0,0x0ACd,0x0 },	{0xF536,0x0} },// sa + halan + ra + halant ->

	// ha 6
	{ {0x0AB9,0x0},			{0x0AB9,0xF567,0x0} },//ha + kern space 
	{ {0x0AB9,0x0ACd,0x0},		{0xF537,0xF567,0x0} },// ha + halant  -> half ha
	{ {0x0AB9,0x0ACd,0x0AB0,0x0},	{0xF538,0xF567,0x0} },// ha + halant + ra ->
	{ {0x0AB9,0x0AC3,0x0},		{0xF539,0xF567,0x0} },//ha + ri matra ->
	{ {0x0AB9,0x0ACd,0x0AAe,0x0},	{0xF53A,0x0} },// ha + halant + ma ->
	{ {0x0AB9,0x0ACd,0x0AAf,0x0},	{0xF53B,0x0} },// ha + halant + ya ->

	// misc 5
	
	{ {0x0ABc,0x0},			{0x0ABc,0x0} } ,
	
	{ {0x0ACd,0x0},			{0x0ACd,0x0} } ,
	{ {0x0ACd,0x0AB0,0x0 },		{0xF53C,0x0 } } ,// halant + ra 
	{ {0x0ACd,0x0AB0,0x0AC1,0x0 },	{0xF549,0x0} },// halant + ra + u matra ->
	{ {0x0ACd,0x0AB0,0x0AC2,0x0 },	{0xF54C,0x0} } ,// halant + ra + Uu matra -> 
	
	// Some Conjucts

	{ {0x0AB7,0x0ACd,0x0A9F,0x0},   {0xf56c,0xf55f,0x0} },//sha+halant+Ta
	{ {0x0AB7,0x0ACd,0x0AAA,0x0},   {0xf56c,0xf560,0x0} },//sha+halant+Tha
	{ {0x0AB8,0x0ACd,0x0AA4,0x0ACd,0x0AB0,0x0 },    {0xF56E,0x0} },// sa + halant + ta + halant + ra ->
	{ {0x0A9e,0x0ACd,0x0A9C,0x0},   {0xF570,0x0} },// nya + halant + ja
	{ {0x0AA1,0x0ACd,0x0A97,0x0},  {0xF571,0xF561,0x0} },// Da+halant+ga -> + kern space
	{ {0x0AA6,0x0ACd,0x0AAd,0x0},   {0xF56F,0xF563,0x0 } },// da + halant + bha ->   + kern space
	{ {0x0AA6,0x0ACd,0x0A98,0x0},   {0xF572,0xF563,0x0 } },// da + halant + gha ->   + kern space
	{ {0x0AA6,0x0ACd,0x0AAC,0x0},   {0xF573,0xF563,0x0 } },// da + halant + ba ->   + kern space
	{ {0x0AB9,0x0ACd,0x0AB2,0x0},   {0xF574,0xF567,0x0 } },// ha + halant + la ->   + kern space
	{ {0x0AB9,0x0ACd,0x0AB5,0x0},   {0xF575,0xF567,0x0 } },// ha + halant + va ->   + kern space
	{ {0x0AB9,0x0ACd,0x0AA8,0x0},   {0xF576,0xF567,0x0 } },// ha + halant + na ->   + kern space
	{ {0x0AB9,0x0ACd,0x0AA3,0x0},   {0xF577,0xF567,0x0 } },// ha + halant + Na ->   + kern space
	
	//Vedic Characters 2

	{ {0x0ABd,0x0},			{0x0ABd,0x0} },
	{ {0x0AD0,0x0},			{0x0AD0,0x0} },
									
	//Dig09its 10								
	{ {0x0AE6,0x0},			{0x0AE6,0x0} } ,
	{ {0x0AE7,0x0},			{0x0AE7,0x0} } ,
	{ {0x0AE8,0x0},			{0x0AE8,0x0} } ,
	{ {0x0AE9,0x0},			{0x0AE9,0x0} } ,
	{ {0x0AEa,0x0},			{0x0AEa,0x0} } ,
	{ {0x0AEb,0x0},			{0x0AEb,0x0} } ,
	{ {0x0AEc,0x0},			{0x0AEc,0x0} } ,
	{ {0x0AEd,0x0},			{0x0AEd,0x0} } ,
	{ {0x0AEe,0x0},			{0x0AEe,0x0} } ,
	{ {0x0AEf,0x0},			{0x0AEf,0x0} } 

};



static const BengGlyphEntry BengGlyphTbl[MAP_BENG_SIZE] = {

	//Vowel Modifiers 3

	{ {0x0981,0x0},		{0x0981,0x0} },
	{ {0x0982,0x0},		{0x0982,0x0} },
	{ {0x0983,0x0},		{0x0983,0x0} },

	// vowels 12

	{ {0x0985,0x0},		{0x0985,0x0} },
	{ {0x0986,0x0},		{0x0986,0x0} },
	{ {0x0987,0x0},		{0x0987,0x0} },
	{ {0x0988,0x0},		{0x0988,0x0} },
	{ {0x0989,0x0},		{0x0989,0x0} },
	{ {0x098a,0x0},		{0x098a,0x0} },	 	
	{ {0x098b,0x0},		{0x098b,0x0} },	
	{ {0x098c,0x0},		{0x098c,0x0} },
	{ {0x098f,0x0},		{0x098f,0x0} },
	{ {0x0990,0x0},		{0x0990,0x0} },
	{ {0x0993,0x0},		{0x0993,0x0} },
	{ {0x0994,0x0},		{0x0994,0x0} },
										
	// Vowel signs 12							

	{ {0x09Be,0x0},		{0x09Be,0x0} },
	{ {0x09Bf,0x0},		{0x09Bf,0x0} },
	{ {0x09C0,0x0},		{0x09C0,0x0} },
	{ {0x09C1,0x0},		{0x09C1,0x0} },
	{ {0x09C2,0x0},		{0x09C2,0x0} },
	{ {0x09C3,0x0},		{0x09C3,0x0} },
	{ {0x09C4,0x0},		{0x09C4,0x0} },
	{ {0x09C7,0x0},		{0x09C7,0x0} },
	{ {0x09C8,0x0},		{0x09C8,0x0} },
	{ {0x09Cb,0x0},		{0x09Cb,0x0} },
	{ {0x09Cc,0x0},		{0x09Cc,0x0} },
	{ {0x09D7,0x0},		{0x09D7,0x0} },

	// Consonants
	// ka 15

	{ {0x0995,0x0},		{0x0995,0xF46F,0x0} },// ka -> ka + kern space
	{ {0x0995,0x09cd,0x0},	{0xf371,0xf46f,0x0} },  
	{ {0x0995,0x09cd,0x0995,0x0},	{0xf37e,0xf46f,0x0} },  
	{ {0x0995,0x09cd,0x09A0,0x0},	{0xf37f,0xf46f,0x0} },  
	{ {0x0995,0x09cd,0x099f,0x0},	{0xf380,0xf46f,0x0} },  
	{ {0x0995,0x09cd,0x09b0,0x0},	{0xf381,0xf46f,0x0} },  
	{ {0x0995,0x09cd,0x09a4,0x0},	{0xf382,0xf478,0x0} },  
	{ {0x0995,0x09cd,0x09b2,0x0},	{0xf383,0xf46f,0x0} },  
	{ {0x0995,0x09cd,0x09b8,0x0},	{0xf384,0x0} },  
	{ {0x0995,0x09cd,0x09ae,0x0},	{0xf385,0x0} },  
	{ {0x0995,0x09cd,0x09b7,0x0},	{0xf451,0xf473,0x0} },  
	{ {0x0995,0x09cd,0x09b7,0x09cd,0x0},	{0xf372,0xf473,0x0} },  
	{ {0x0995,0x09cd,0x09b7,0x09c2,0x0},	{0xf448,0xf473,0x0} },  
	{ {0x0995,0x09cd,0x09b7,0x09a8,0x0},	{0xf552,0xf473,0x0} },  
	{ {0x0995,0x09cd,0x09b7,0x09ae,0x0},	{0xf553,0xf473,0x0} },  

	// kha 4

	{ {0x0996,0x0},			{0x0996,0x0} },
	{ {0x0996,0x09Cd,0x09A8,0x0},{0xF386,0x0} } ,
	{ {0x0996,0x09Cd,0x09B0,0x0},	{0xF387,0x0 } } ,
	{ {0x0996,0x09Cd,0x09B2,0x0},	{0xF388,0x0 } } ,
	
	// ga 8
	{ {0x0997,0x0},			{0x0997,0x0} },
	{ {0x0997,0x09Cd,0x0 },		{0xF373,0x0} },
	{ {0x0997,0x09Cd,0x09a7,0x0},	{0xF389,0x0} },
	{ {0x0997,0x09Cd,0x09B0,0x0},	{0xF38A,0x0} },
	{ {0x0997,0x09Cd,0x09ae,0x0},	{0xF38b,0x0} },
	{ {0x0997,0x09Cd,0x09a8,0x0},	{0xF38c,0x0} },
	{ {0x0997,0x09Cd,0x09B2,0x0},	{0xF38d,0x0} },
	{ {0x0997,0x09C1,0x0 },		{0xF464,0x0} }, // ga + uu matra

	// gha 4
	{ {0x0998,0x0},			{0x0998,0x0} },
	{ {0x0998,0x09Cd,0x09a8,0x0},	{0xF38e,0x0} },
	{ {0x0998,0x09Cd,0x09B0,0x0},	{0xF38f,0x0 } },
	{ {0x0998,0x09Cd,0x09B2,0x0},	{0xF390,0x0 } },

	// nga 9
	{ {0x0999,0x0} ,		{0x0999,0xF472,0x0} },
	{ {0x0999,0x09cd,0x0995,0x0},	{0xf391,0xf46f,0x0} },
	{ {0x0999,0x09cd,0x0995,0x09cd,0x09b7,0x0},	{0xf396,0xf473,0x0} },
	{ {0x0999,0x09cd,0x0997,0x0},	{0xf392,0x0} },
	{ {0x0999,0x09cd,0x0996,0x0},	{0xf393,0x0} },
	{ {0x0999,0x09cd,0x0998,0x0},	{0xf394,0x0} },
	{ {0x0999,0x09cd,0x09ae,0x0},	{0xf395,0x0} },
	{ {0x0999,0x09cd,0x09b0,0x0},	{0xf465,0xF472,0x0} },
	{ {0x0999,0x09cd,0x09ac,0x0},	{0xf466,0xF472,0x0} },

	// cha 11
	{ {0x099a,0x0},			{0x099a,0xf46e,0x0 } },
	{ {0x099a,0x09Cd,0x0},		{0xF374,0xf46e,0x0 } },  
	{ {0x099a,0x09Cd,0x099a,0x0},	{0xF397,0xf46e,0x0 } }, 
	{ {0x099a,0x09Cd,0x099e,0x0},	{0xF398,0x0} },/////////////////////// kern space add
	{ {0x099a,0x09Cd,0x09a8,0x0},	{0xF399,0x0} },
	{ {0x099a,0x09Cd,0x09ac,0x0},	{0xF39a,0x0} },
	{ {0x099a,0x09Cd,0x09b0,0x0},	{0xF39b,0x0} },
	{ {0x099a,0x09Cd,0x09b2,0x0},	{0xF39c,0x0} },
	{ {0x099a,0x09Cd,0x099b,0x0},	{0xF39d,0xf471,0x0} },
	{ {0x099a,0x09Cd,0x099b,0x09cd,0x09b0,0x0},	{0xF39e,0xf471,0x0} },
	{ {0x099a,0x09Cd,0x09ae,0x0},	{0xF457,0x0} },
	
	// chha 5
	{ {0x099b,0x0} ,		{0x099b,0xF471,0x0} },
	{ {0x099b,0x09Cd,0x09a8,0x0},	{0xf39f,0xf471,0x0} },
	{ {0x099b,0x09Cd,0x09b0,0x0},	{0xf3a0,0xf471,0x0} },
	{ {0x099b,0x09Cd,0x09b2,0x0},	{0xf3a1,0xf471,0x0} },
	{ {0x099b,0x09Cd,0x09ac,0x0},	{0xf3a2,0xf471,0x0} },

	// ja 8
	{ {0x099c,0x0},			{0x099c,0x0} },
	{ {0x099c,0x09Cd,0x099c,0x0},	{0xf3a4,0x0} },
	{ {0x099c,0x09Cd,0x099c,0x09Cd,0x09b0,0x0},	{0xf3a5,0x0} },
	{ {0x099c,0x09Cd,0x09b0,0x0},	{0xf3a3,0x0} },
	{ {0x099c,0x09Cd,0x09ac,0x0},	{0xf3a6,0x0} },
	{ {0x099c,0x09Cd,0x09ac,0x09Cd,0x09b0,0x0},	{0xf3a7,0x0} },
	{ {0x099c,0x09Cd,0x099e,0x0},	{0xf3a8,0x0} },/////////////////////// kern space add
	{ {0x099c,0x09Cd,0x099d,0x0},	{0xf3a9,0xf46f,0x0} },
	
	// jha 4
	{ {0x099d,0x0},		{0x099d,0xF46f,0x0} },
	{ {0x099d,0x09Cd,0x09a8,0x0},	{0xF3aa,0xF46f,0x0} },
	{ {0x099d,0x09Cd,0x09b0,0x0},	{0xF3ab,0xF46f,0x0} },
	{ {0x099d,0x09Cd,0x09b2,0x0},	{0xF3ac,0xF46f,0x0} },

	// nya 5      ///////////////////////////////// kern space add
	{ {0x099e,0x0},			{0x099e,0x0} },
	{ {0x099e,0x09Cd,0x099a,0x0},	{0xF3ad,0x0} },////////////////////// kern space add
	{ {0x099e,0x09Cd,0x099b,0x0},	{0xF3ae,0xf471,0x0} },
	{ {0x099e,0x09Cd,0x099c,0x0},	{0xF3af,0x0} },
	{ {0x099e,0x09Cd,0x099d,0x0},	{0xF3b0,0xf46f,0x0} },

	// Ta  8
	{ {0x099f,0x0},		{0x099F,0x0} },
	{ {0x099f,0x09cd,0x0},			{0xf375,0x0} },
	{ {0x099f,0x09Cd,0x099f,0x0},	{0xf3b1,0x0} },
	{ {0x099f,0x09Cd,0x09b0,0x0},	{0xF3b2,0x0} },
	{ {0x099f,0x09Cd,0x09b2,0x0},	{0xF3b3,0x0} },
	{ {0x099f,0x09Cd,0x09a8,0x0},	{0xF3b4,0x0} },
	{ {0x099f,0x09Cd,0x09ac,0x0},	{0xF3b5,0x0} },
	{ {0x099f,0x09Cd,0x09ae,0x0},	{0xF458,0x0} },

	// Tha 5
	{ {0x09A0,0x0},		{0x09A0,0xF471,0x0} },
	{ {0x09A0,0x09Cd,0x09B0,0x0},	{0xF46b,0xF471,0x0} },
	{ {0x09A0,0x09Cd,0x09B2,0x0},	{0xF46c,0xF471,0x0} },
	{ {0x09A0,0x09Cd,0x09a8,0x0},	{0xF46d,0xF471,0x0} },
	{ {0x09A0,0x09Cd,0x09ae,0x0},	{0xF459,0x0} },

	// Da  9
	{ {0x09A1,0x0},		{0x09A1,0xF470,0x0} },
	{ {0x09A1,0x09cd,0x0},		{0xf376,0xF470,0x0} },
	{ {0x09A1,0x09Cd,0x09Ac,0x0},	{0xf3b6,0xf470,0x0} },
	{ {0x09A1,0x09Cd,0x09A8,0x0},	{0xf3b7,0x0} },
	{ {0x09A1,0x09Cd,0x09b2,0x0},	{0xf3b8,0xf470,0x0} },
	{ {0x09A1,0x09Cd,0x09b0,0x0},	{0xf3b9,0xf470,0x0} },
	{ {0x09A1,0x09Cd,0x09A1,0x0},	{0xf3ba,0xf470,0x0} },
	{ {0x09A1,0x09Cd,0x0997,0x0},	{0xf3bb,0x0} },
	{ {0x09A1,0x09Cd,0x09ae,0x0},	{0xF45a,0x0} },

	// Dha 6
	{ {0x09A2,0x0},			{0x09A2,0xF46e,0x0} },
	{ {0x09A2,0x09Cd,0x09b0,0x0} ,	{0xf467,0xf46e,0x0} },
	{ {0x09A2,0x09Cd,0x09ae,0x0},	{0xF45b,0x0} },
	{ {0x09A2,0x09Cd,0x09b2,0x0} ,	{0xf468,0xf46e,0x0} },
	{ {0x09A2,0x09Cd,0x09a8,0x0} ,	{0xf469,0xf46e,0x0} },
	{ {0x09A2,0x09Cd,0x09ac,0x0} ,	{0xf46a,0xf46e,0x0} },
	
	// Nna 8
	{ {0x09A3,0x0},			{0x09A3,0x0} },
	{ {0x09A3,0x09Cd,0x09A3,0x0 },	{0xf3bc,0x0 } },
	{ {0x09A3,0x09Cd,0x099A,0x0 },	{0xf3bd,0xf46e,0x0 } },
	{ {0x09A3,0x09Cd,0x09A0,0x0 },	{0xf3be,0xf471,0x0 } },
	{ {0x09A3,0x09Cd,0x09A1,0x0 },	{0xf3bf,0xf470,0x0 } },
	{ {0x09A3,0x09Cd,0x09A1,0x09cd,0x09b0,0x0 },	{0xf3c0,0xf470,0x0 } },
	{ {0x09A3,0x09Cd,0x09Ac,0x0 },	{0xf3c1,0x0 } },
	{ {0x09A3,0x09Cd,0x09Ae,0x0 },	{0xf3c2,0x0 } },
	
	// ta 10
	{ {0x09A4,0x0},			{0x09A4,0xf470,0x0} },
	{ {0x09A4,0x09Cd,0x0},			{0xF365,0x0} },
	{ {0x09A4,0x09Cd,0x09ac,0x0},	{0xF3c3,0xf470,0x0} },
	{ {0x09A4,0x09Cd,0x09a8,0x0},	{0xF3c4,0xf470,0x0} },
	{ {0x09A4,0x09Cd,0x09b2,0x0},	{0xF3c5,0xf470,0x0} },
	{ {0x09A4,0x09Cd,0x09a4,0x0},	{0xF3c6,0xf470,0x0} },
	{ {0x09A4,0x09Cd,0x09a4,0x09cd,0x09ac,0x0},	{0xF3c7,0xf470,0x0} },
	{ {0x09A4,0x09Cd,0x09b0,0x0},	{0xF3c9,0x0} },
	{ {0x09A4,0x09Cd,0x09b0,0x09c1,0x0},	{0xF3ca,0xf470,0x0} },
	{ {0x09A4,0x09Cd,0x09b0,0x09c2,0x0},	{0xF3cb,0xf470,0x0} },
	
	// tha 6
	{ {0x09A5,0x0},			{0x09A5,0x0 } },
	{ {0x09A5,0x09Cd,0x09a8,0x0},	{0xf3cc,0x0} },
	{ {0x09A5,0x09Cd,0x09ac,0x0},	{0xf3cd,0x0} },
	{ {0x09A5,0x09Cd,0x09ae,0x0},	{0xf3ce,0x0} },
	{ {0x09A5,0x09Cd,0x09B0,0x0},	{0xf3cf,0x0} },
	{ {0x09A5,0x09Cd,0x09B2,0x0},	{0xf3d0,0x0} },
	
	// da 15
	{ {0x09A6,0x0 },		{0x09A6,0x0} },
	{ {0x09A6,0x09Cd,0x09B0,0x0} ,	{0xf3d5,0x0} },
	{ {0x09A6,0x09Cd,0x09B0,0x09c1,0x0} ,	{0xf3d1,0x0} },
	{ {0x09A6,0x09Cd,0x09B0,0x09c2,0x0} ,	{0xf3d2,0x0} },
	{ {0x09A6,0x09Cd,0x09B0,0x09cd,0x09ac,0x0} ,	{0xf3d7,0x0} },
	{ {0x09A6,0x09Cd,0x09a8,0x0} ,	{0xf3d3,0x0} },
	{ {0x09A6,0x09Cd,0x09ac,0x0} ,	{0xf3d4,0x0} },
	{ {0x09A6,0x09Cd,0x09A6,0x0} ,	{0xf3d6,0x0} },
	{ {0x09A6,0x09Cd,0x09a7,0x0} ,	{0xf3d8,0x0} },
	{ {0x09A6,0x09Cd,0x09a7,0x09cd,0x09ac,0x0} ,	{0xf3d9,0x0} },
	{ {0x09A6,0x09Cd,0x09ad,0x0} ,	{0xf3da,0x0} },
	{ {0x09A6,0x09Cd,0x09ad,0x09cd,0x09b0,0x0} ,	{0xf3db,0x0} },
	{ {0x09A6,0x09Cd,0x09af,0x0} ,	{0xf3dc,0x0} },
	{ {0x09A6,0x09Cd,0x0997,0x0} ,	{0xf3dd,0x0} },
	{ {0x09A6,0x09Cd,0x09ae,0x0} ,	{0xf3de,0x0} },

		
	// dha 8
	{ {0x09A7,0x0},			{0x09A7,0x0} },
	{ {0x09A7,0x09Cd,0x09B0,0x0 },	{0xf3df,0x0} },
	{ {0x09A7,0x09Cd,0x09b0,0x09c1,0x0 },	{0xf3e3,0x0} },
	{ {0x09A7,0x09Cd,0x09a8,0x0 },	{0xf3e0,0x0} },
	{ {0x09A7,0x09Cd,0x09a7,0x0 },	{0xf3e1,0x0} },
	{ {0x09A7,0x09Cd,0x09a7,0x09c2,0x0 },	{0xf3e4,0x0} },
	{ {0x09A7,0x09Cd,0x09ac,0x0 },	{0xf3e2,0x0} },
	{ {0x09A7,0x09Cd,0x09ae,0x0},	{0xF45c,0x0} },
	
	// na 20
	{ {0x09A8,0x0 },		{0x09A8,0x0} },
	{ {0x09A8,0x09cd,0x0 },			{0xf378,0x0} },
	{ {0x09A8,0x09Cd,0x09a8,0x0 },	{0xf3e5,0x0} },
	{ {0x09A8,0x09Cd,0x09ac,0x0 },	{0xf3e6,0x0} },
	{ {0x09A8,0x09Cd,0x09B0,0x0 },	{0xf3e7,0x0} },
	{ {0x09A8,0x09Cd,0x09B2,0x0 },	{0xf3e8,0x0} },
	{ {0x09A8,0x09Cd,0x09A0,0x0 },	{0xf3e9,0xf471,0x0} },
	{ {0x09A8,0x09Cd,0x09a1,0x0 },	{0xf3ea,0xf470,0x0} },
	{ {0x09A8,0x09Cd,0x09a1,0x09cd,0x09b0,0x0 },	{0xf3eb,0xf470,0x0} },
	{ {0x09A8,0x09Cd,0x09a6,0x0 },	{0xf3ec,0x0} },
	{ {0x09A8,0x09Cd,0x09a6,0x09cd,0x09ac,0x0 },	{0xf3ed,0x0} },
	{ {0x09A8,0x09Cd,0x09a6,0x09cd,0x09b0,0x0 },	{0xf3ee,0x0} },
	{ {0x09A8,0x09Cd,0x09a7,0x0 },	{0xf3ef,0x0} },
	{ {0x09A8,0x09Cd,0x09a7,0x09cd,0x09b0,0x0 },	{0xf3f0,0x0} },
	{ {0x09A8,0x09Cd,0x09a4,0x09cd,0x09b0,0x0 },	{0xf3f1,0x0} },
	{ {0x09A8,0x09Cd,0x09ae,0x0 },	{0xf3f2,0x0} },
	{ {0x09A8,0x09Cd,0x09af,0x0 },	{0xf3f3,0x0} },
	{ {0x09A8,0x09Cd,0x09a5,0x0 },	{0xf3f4,0x0} },
	{ {0x09A8,0x09Cd,0x09ac,0x0 },	{0xf3f5,0x0} },
	{ {0x09A8,0x09Cd,0x09B8,0x0 },	{0xf3f6,0x0} },	
	
	// pa 10
	{ {0x09Aa,0x0 },		{0x09Aa,0x0} },
	{ {0x09Aa,0x09Cd,0x0 },	{0xf379,0x0} },
	{ {0x09Aa,0x09Cd,0x09b0,0x0 },	{0xf3fb,0x0} },
	{ {0x09Aa,0x09Cd,0x09B0,0x09C1,0x0 },	{0xf3f7,0x0} },
	{ {0x09Aa,0x09Cd,0x09B0,0x09C2,0x0 },	{0xf3f8,0x0} },
	{ {0x09Aa,0x09Cd,0x09aa,0x0 },	{0xf3f9,0x0} },
	{ {0x09Aa,0x09Cd,0x09a8,0x0 },	{0xf3fa,0x0} },
	{ {0x09Aa,0x09Cd,0x09b8,0x0 },	{0xf3fc,0x0} },
	{ {0x09Aa,0x09Cd,0x09a4,0x0 },	{0xf3fd,0x0} },
	{ {0x09Aa,0x09Cd,0x09ae,0x0},	{0xF45e,0x0} },

	// pha 4
	{ {0x09Ab,0x0 },		{0x09Ab,0xf46f,0x0} },
	{ {0x09Ab,0x09Cd,0x09a8,0x0 },	{0xf3fe,0xf46f,0x0} },
	{ {0x09Ab,0x09Cd,0x09b0,0x0 },	{0xf3ff,0xf46f,0x0} },
	{ {0x09Ab,0x09Cd,0x09b2,0x0 },	{0xf400,0xf46f,0x0} },

	// ba 12
	{ {0x09Ac,0x0 },		{0x09Ac,0x0} },
	{ {0x09Ac,0x09c3,0x0 },		{0xf406,0x0} },
	{ {0x09Ac,0x09Cd,0x09ac,0x0 },		{0xf401,0x0} },
	{ {0x09Ac,0x09Cd,0x09ad,0x0 },		{0xf402,0x0} },
	{ {0x09Ac,0x09Cd,0x09a6,0x0 },		{0xf403,0x0} },
	{ {0x09Ac,0x09Cd,0x09a7,0x0 },		{0xf404,0x0} },
	{ {0x09Ac,0x09Cd,0x099c,0x0 },		{0xf405,0x0} },
	{ {0x09Ac,0x09Cd,0x09b0,0x0 },		{0xf407,0x0} },
	{ {0x09Ac,0x09Cd,0x09a8,0x0 },		{0xf408,0x0} },
	{ {0x09Ac,0x09Cd,0x09b0,0x09cd,0x09c1,0x0 },		{0xf409,0x0} },
	{ {0x09Ac,0x09Cd,0x09b0,0x09cd,0x09c2,0x0 },		{0xf40a,0x0} },
	{ {0x09Ac,0x09Cd,0x09ae,0x0},	{0xF45f,0x0} },

	// bha 7
	{ {0x09Ad,0x0 },		{0x09Ad,0xf470,0x0} },
	{ {0x09Ad,0x09Cd,0x09b0,0x0 },		{0xf40b,0x0} },
	{ {0x09Ad,0x09Cd,0x09b0,0x09c1,0x0 },		{0xf40e,0x0} },
	{ {0x09Ad,0x09Cd,0x09b0,0x09c2,0x0 },		{0xf40f,0x0} },
	{ {0x09Ad,0x09Cd,0x09b2,0x0 },		{0xf40c,0x0} },
	{ {0x09Ad,0x09Cd,0x09a8,0x0 },		{0xf40d,0x0} },
	{ {0x09Ad,0x09Cd,0x09ae,0x0},	{0xF460,0x0} },
	
	// ma  10
	{ {0x09Ae,0x0 },		{0x09Ae,0x0} },
	{ {0x09Ae,0x09Cd,0x09a8,0x0 },	{0xf410,0x0} },
	{ {0x09Ae,0x09Cd,0x09ac,0x0 },	{0xf411,0x0} },
	{ {0x09Ae,0x09Cd,0x09ad,0x0 },	{0xf412,0x0} },
	{ {0x09Ae,0x09Cd,0x09ad,0x09Cd,0x09B0,0x0 },	{0xf415,0x0} },
	{ {0x09Ae,0x09Cd,0x09B0,0x0 },	{0xf413,0x0} },
	{ {0x09Ae,0x09Cd,0x09B2,0x0 },	{0xf414,0x0} },
	{ {0x09Ae,0x09Cd,0x09aa,0x0 },	{0xf416,0x0} },
	{ {0x09Ae,0x09Cd,0x09aa,0x09Cd,0x09B0,0x0 },	{0xf417,0x0} },
	{ {0x09Ae,0x09Cd,0x09ab,0x0 },	{0xf418,0xf46f,0x0} },
	
	// ya 	1
	{ {0x09af,0x0 },		{0x09af,0x0} },
	
	// ra   4 
	{ {0x09B0,0x0 },		{0x09B0,0x0} },
	{ {0x09B0,0x09Cd,0x0 },	{0xf36d,0x0} },
	{ {0x09B0,0x09C1,0x0 },	{0xf419,0x0} },
	{ {0x09B0,0x09C2,0x0 },	{0xf41a,0x0} },

	// la   8
	{ {0x09B2,0x0 },		{0x09B2,0x0} }, 
	{ {0x09B2,0x09Cd,0x0 },	{0xf37a,0x0} },
	{ {0x09B2,0x09Cd,0x0995,0x0 },	{0xf41b,0xf46f,0x0} },
	{ {0x09B2,0x09Cd,0x09a1,0x0 },	{0xf41c,0xf470,0x0} },
	{ {0x09B2,0x09Cd,0x09b2,0x0 },	{0xf41d,0x0} },
	{ {0x09B2,0x09Cd,0x091e,0x0 },	{0xf41e,0x0} },
	{ {0x09B2,0x09Cd,0x09ac,0x0 },	{0xf41f,0x0} },
	{ {0x09B2,0x09Cd,0x09ae,0x0 },	{0xf420,0x0} },
	
	
	// sha  13
	{ {0x09B6,0x0 },		{0x09B6,0x0} },
	{ {0x09B6,0x09c1,0x0 },	{0xf423,0x0} },
	{ {0x09B6,0x09cd,0x0 },	{0xf37b,0x0} },
	{ {0x09B6,0x09Cd,0x099a,0x0 },		{0xf421,0xf46e,0x0} },
	{ {0x09B6,0x09Cd,0x099B,0x0 },		{0xf422,0xf471,0x0} },
	{ {0x09B6,0x09Cd,0x09af,0x0 },	{0xf424,0x0} },
	{ {0x09B6,0x09Cd,0x09b0,0x0},	{0xf425,0x0} },
	{ {0x09B6,0x09Cd,0x09b0,0x09c0,0x0},	{0xf426,0x0} },
	{ {0x09B6,0x09Cd,0x09a8,0x0},	{0xf427,0x0} },
	{ {0x09B6,0x09Cd,0x09ac,0x0},	{0xf428,0x0} },
	{ {0x09B6,0x09Cd,0x09ae,0x0},	{0xf429,0x0} },
	{ {0x09B6,0x09Cd,0x09b0,0x09c1,0x0},	{0xf42a,0x0} },
	{ {0x09B6,0x09Cd,0x09b0,0x09c2,0x0},	{0xf42b,0x0} },
	
	// SHa  15
	{ {0x09B7,0x0 },		{0x09B7,0x0} },
	{ {0x09B7,0x09Cd,0x0 },	{0xf37c,0x0} },
	{ {0x09B7,0x09Cd,0x09ae,0x0 },	{0xf42c,0x0} },
	{ {0x09B7,0x09Cd,0x0995,0x0 },	{0xf42d,0xf46f,0x0} },
	{ {0x09b7,0x09Cd,0x099f,0x0 },	{0xf42e,0x0} },
	{ {0x09b7,0x09Cd,0x099f,0x09cd,0x09b0,0x0 },	{0xf42f,0x0} },
	{ {0x09b7,0x09Cd,0x09a0,0x0 },	{0xf430,0xf471,0x0} },
	{ {0x09B7,0x09Cd,0x09ab,0x0 },	{0xf431,0xf46f,0x0} },
	{ {0x09B7,0x09Cd,0x09a3,0x0 },	{0xf432,0x0} },
	{ {0x09B7,0x09Cd,0x09aa,0x0 },	{0xf433,0x0} },
	{ {0x09B7,0x09Cd,0x09aa,0x09cd,0x09b0,0x0 },	{0xf434,0x0} },
	{ {0x09B7,0x09Cd,0x09af,0x0 },	{0xf435,0x0} },
	{ {0x09B7,0x09Cd,0x09a8,0x0 },	{0xf436,0x0} },
	{ {0x09B7,0x09Cd,0x09b0,0x0 },	{0xf437,0x0} },
	{ {0x09B7,0x09Cd,0x09b2,0x0 },	{0xf438,0x0} },
	
	// sa  17
	{ {0x09B8,0x0 },		{0x09B8,0x0} },
	{ {0x09B8,0x09cd,0x0 },				{0xf463,0x0} },
	{ {0x09B8,0x09cd,0x0995,0x0 },		{0xf439,0x0} },
	{ {0x09B8,0x09cd,0x0996,0x0 },		{0xf43a,0x0} },
	{ {0x09B8,0x09cd,0x09aa,0x0 },		{0xf43b,0x0} },
	{ {0x09B8,0x09cd,0x09aa,0x09cd,0x09b0,0x0 },		{0xf43c,0x0} },
	{ {0x09B8,0x09cd,0x09ae,0x0 },		{0xf43d,0x0} },
	{ {0x09B8,0x09cd,0x09a8,0x0 },		{0xf43e,0x0} },
	{ {0x09B8,0x09cd,0x09ac,0x0 },		{0xf43f,0x0} },
	{ {0x09B8,0x09cd,0x09a8,0x09cd,0x09b0,0x0 },		{0xf440,0x0} },
	{ {0x09B8,0x09cd,0x09b0,0x0 },		{0xf441,0x0} },
	{ {0x09B8,0x09cd,0x09b0,0x09c1,0x0 },		{0xf446,0x0} },
	{ {0x09B8,0x09cd,0x09b0,0x09c2,0x0 },		{0xf447,0x0} },
	{ {0x09B8,0x09cd,0x09b2,0x0 },		{0xf442,0x0} },
	{ {0x09B8,0x09cd,0x09af,0x0 },		{0xf443,0x0} },
	{ {0x09B8,0x09cd,0x09ab,0x0 },		{0xf444,0xf46f,0x0} },
	{ {0x09B8,0x09cd,0x09df,0x0 },		{0xf445,0x0} },
		

	// ha   8
	{ {0x09B9,0x0},			{0x09B9,0xf471,0x0} },
	{ {0x09B9,0x09C1,0x0},		{0xf449,0xf471,0x0} },
	{ {0x09B9,0x09C3,0x0},	{0xf44b,0xf471,0x0} },
	{ {0x09B9,0x09Cd,0x09A8,0x0},	{0xf44a,0xf471,0x0} },
	{ {0x09B9,0x09Cd,0x09Ac,0x0},	{0xf44c,0xf471,0x0} },
	{ {0x09B9,0x09Cd,0x09b0,0x0},	{0xf44d,0xf471,0x0} },
	{ {0x09B9,0x09Cd,0x09b2,0x0},	{0xf44e,0xf471,0x0} },
	{ {0x09B9,0x09Cd,0x09A3,0x0},	{0xf44f,0xf471,0x0} },

	// nukta 1
	
	{ {0x09Bc,0x0},			{0x09Bc,0x0} } ,
	
	// halant  5
	{ {0x09Cd,0x0},			{0x09Cd,0x0} },
	{ {0x09Cd,0x09af,0x0 },		{0xf36c,0x0 } },
	{ {0x09Cd,0x09B2,0x0 },		{0xf36e,0x0 } },
	{ {0x09Cd,0x09ac,0x0 },		{0xf36f,0x0 } },
	{ {0x09Cd,0x09B0,0x0 },		{0xf370,0x0 } },

	// nukta chars  3
	{ {0x09DC,0x0},			{0x09DC,0x0} },
	{ {0x09DD,0x0},			{0x09DD,0x0} },
	{ {0x09DF,0x0},			{0x09DF,0x0} },

	// misc     15

	{ {0x09E0,0x0},			{0x09E0,0x0} },
	{ {0x09E1,0x0},			{0x09E1,0x0} },
	{ {0x09E2,0x0},			{0x09E2,0x0} },
	{ {0x09E3,0x0},			{0x09E3,0x0} },
	{ {0x09F0,0x0},			{0x09F0,0x0} },
	{ {0x09F1,0x0},			{0x09F1,0x0} },
	{ {0x09F2,0x0},			{0x09F2,0x0} },
	{ {0x09F3,0x0},			{0x09F3,0x0} },
	{ {0x09F4,0x0},			{0x09F4,0x0} },
	{ {0x09F5,0x0},			{0x09F5,0x0} },
	{ {0x09F6,0x0},			{0x09F6,0x0} },
	{ {0x09F7,0x0},			{0x09F7,0x0} },
	{ {0x09F8,0x0},			{0x09F8,0x0} },
	{ {0x09F9,0x0},			{0x09F9,0x0} },
	{ {0x09Fa,0x0},			{0x09Fa,0x0} },
	
	//Digits 10								
	{ {0x09E6,0x0},			{0x09E6,0x0} },
	{ {0x09E7,0x0},			{0x09E7,0x0} },
	{ {0x09E8,0x0},			{0x09E8,0x0} },
	{ {0x09E9,0x0},			{0x09E9,0x0} },
	{ {0x09Ea,0x0},			{0x09Ea,0x0} },
	{ {0x09Eb,0x0},			{0x09Eb,0x0} },
	{ {0x09Ec,0x0},			{0x09Ec,0x0} },
	{ {0x09Ed,0x0},			{0x09Ed,0x0} },
	{ {0x09Ee,0x0},			{0x09Ee,0x0} },
	{ {0x09Ef,0x0},			{0x09Ef,0x0} } 

};

static const TlgGlyphEntry TlgGlyphTbl[MAP_TLG_SIZE] = 
{
	//Vowel Modifiers
	{ {0x0C01,0x0},					{0x0C01,0x0} },
	{ {0x0C02,0x0},					{0x0C02,0x0} },
	{ {0x0C03,0x0},					{0x0C03,0x0} },

	// Vowels
	{ {0x0C05,0x0},							{0x0C05,0x0} },
	{ {0x0C06,0x0},							{0x0C06,0x0} },
	{ {0x0C07,0x0},							{0x0C07,0x0} },
	{ {0x0C08,0x0},							{0x0C08,0x0} },
	{ {0x0C09,0x0},							{0x0C09,0x0} },
	{ {0x0C0a,0x0},							{0x0C0a,0x0} },					// kern space required 	
	{ {0x0C0b,0x0},							{0x0C0b,0x0} },					// kern space required
	{ {0x0C60,0x0},							{0x0C60,0x0} },					
	{ {0x0C0c,0x0},							{0x0C0c,0x0} },
	{ {0x0C61,0x0},							{0x0C61,0x0} } ,
	{ {0x0C0e,0x0},							{0x0C0e,0xF5E6,0x0} },
	{ {0x0C0f,0x0},							{0x0C0f,0x0} },
	{ {0x0C10,0x0},							{0x0C10,0x0} },
	{ {0x0C12,0x0},							{0x0C12,0x0} },
	{ {0x0C13,0x0},							{0x0C13,0x0} },
	{ {0x0C14,0x0},							{0x0C14,0x0} },

	// Consonants
	// Ka 
	{ {0x0C15,0x0},							{0x0C15,0xF5A8,0x0} },
	{ {0x0C15,0x0C3E,0x0}, 					{0xF5A6,0x0C3E,0x0} },
	{ {0x0C15,0x0C3F,0x0},					{0xF5A6,0x0C3F,0xF5A8,0x0} },
	{ {0x0C15,0x0C40,0x0},					{0xF5A6,0x0C40,0xF5A8,0x0} },
	{ {0x0C15,0x0C41,0x0},					{0x0C15,0xF5A8,0x0C41,0x0} },
	{ {0x0C15,0x0C42,0x0},					{0x0C15,0xF5A8,0x0C42,0x0} },
	{ {0x0C15,0x0C43,0x0},					{0x0C15,0xF5A8,0x0C43,0x0} },
	{ {0x0C15,0x0C44,0x0},					{0x0C15,0xF5A8,0x0C44,0x0} },

	{ {0x0C15,0x0C46,0x0},					{0x0C46,0xF5A6,0xF5A8,0x0} },
	{ {0x0C15,0x0C47,0x0},					{0x0C47,0xF5A6,0xF5A8,0x0} },
	{ {0x0C15,0x0C48,0x0},					{0x0C48,0xF5A6,0xF5A8,0x0} },

	{ {0x0C15,0x0C4A,0x0},					{0xF5A6,0x0C4A,0x0} },
	{ {0x0C15,0x0C4B,0x0},					{0xF5A6,0x0C4B,0x0} },
	{ {0x0C15,0x0C4C,0x0},					{0xF5A6,0x0C4C,0x0} },
	{ {0x0C15,0x0C4D},						{0xF5A6,0x0C4D,0xF5A8,0x0} },
	{ {0x0C15,0x0C4D,0x0C37,0x0},			{0xF5A7,0xF5B6,0xF5A7,0x0} },// incomplete need to add a tala kattu

	// Kha
	{ {0x0C16,0x0},							{0x0C16,0xF5E6,0x0} },
	{ {0x0C16,0x0C3E,0x0}, 					{0xF5EA,0xF5F8,0x0} },
	{ {0x0C16,0x0C3F,0x0},					{0xF5F0,0xF5E6,0x0} },
	{ {0x0C16,0x0C40,0x0},					{0xF5F0,0x0F5AD,0xF5E6,0x0} },
	{ {0x0C16,0x0C41,0x0},					{0x0C16,0xF5E6,0x0C41,0x0} },
	{ {0x0C16,0x0C42,0x0},					{0x0C16,0xF5E6,0x0C42,0x0} },
	{ {0x0C16,0x0C43,0x0},					{0x0C16,0xF5E6,0x0C43,0x0} },
	{ {0x0C16,0x0C44,0x0},					{0x0C16,0xF5E6,0x0C44,0x0} },

	{ {0x0C16,0x0C46,0x0},					{0xF5FB,0xF5EA,0xF5E6,0x0} },
	{ {0x0C16,0x0C47,0x0},					{0xF5FB,0xF5EA,0xF5E6,0x0C55,0x0} },	
	{ {0x0C16,0x0C48,0x0},					{0x0C56,0xF5FB,0xF5EA,0xF5E6,0x0} },

	{ {0x0C16,0x0C4A,0x0},					{0xF5EA,0xF5FC,0x0} },
	{ {0x0C16,0x0C4B,0x0},					{0xF5EA,0xF5FC,0x0C55,0x0} },
	{ {0x0C16,0x0C4C,0x0},					{0xF5EA,0xF5FD,0x0} },
	{ {0x0C16,0x0C4D,0x0},					{0xF5EA,0xF5FE,0x0} },


	// Ja
	{ {0x0C17,0x0},							{0x0C17,0xF5B2,0x0} },
	{ {0x0C17,0x0C3E,0x0}, 					{0xF5B0,0x0C3E,0x0} },
	{ {0x0C17,0x0C3F,0x0},					{0xF5B0,0xF5B3,0xF5B2} },
	{ {0x0C17,0x0C40,0x0},					{0xF5B0,0xF5B4,0xF5B2} },
	{ {0x0C17,0x0C41,0x0},					{0x0C17,0xF5B2,0x0C41} },
	{ {0x0C17,0x0C42,0x0},					{0x0C17,0xF5B2,0x0C42} },
	{ {0x0C17,0x0C43,0x0},					{0x0C17,0xF5B2,0x0C43} },
	{ {0x0C17,0x0C44,0x0},					{0x0C17,0xF5B2,0x0C44} },
	
	{ {0x0C17,0x0C46,0x0},					{0xF5B5,0xF5B0,0xF5B2,0x0} },
	{ {0x0C17,0x0C47,0x0},					{0xF5B5,0xF5B0,0xF5B2,0x0C55,0x0} },
	{ {0x0C17,0x0C48,0x0},					{0x0C56,0xF5B5,0xF5B0,0xF5B2,0x0} },
	
	{ {0x0C17,0x0C4A,0x0},					{0xF5B0,0x0C4A,0x0} },
	{ {0x0C17,0x0C4B,0x0},					{0xF5B0,0x0C4B,0x0} },
	{ {0x0C17,0x0C4C,0x0},					{0xF5B0,0x0C4C,0x0} },
	{ {0x0C17,0x0C4D,0x0},					{0xF5B0,0x0C4D,0xF5B2,0x0} },
	
	// Jha
	{ {0x0C18,0x0},							{0x0C18,0x0} },
	{ {0x0C18,0x0C3E,0x0}, 					{0xF61E,0xF5B6,0xF5B7,0xF5BC,0x0C42,0x0} },
	{ {0x0C18,0x0C3F,0x0},					{0xF5C2,0xF5B6,0xF5B7,0xF5BC,0x0C41,0x0} },
	{ {0x0C18,0x0C40,0x0},					{0xF5C3,0xF5B6,0xF5B7,0xF5BC,0x0C41,0x0} },
	{ {0x0C18,0x0C41,0x0},					{0x0C18,0x0C41,0x0} },
	{ {0x0C18,0x0C42,0x0},					{0x0C18,0x0C42,0x0} },
	{ {0x0C18,0x0C43,0x0},					{0x0C18,0x0C43,0x0} },
	{ {0x0C18,0x0C44,0x0},					{0x0C18,0x0C44,0x0} },
	
	{ {0x0C18,0x0C46,0x0},					{0xF5C4,0xF5B6,0xF5B7,0xF5BC,0x0C41,0x0} },
	{ {0x0C18,0x0C47,0x0},					{0xF5C5,0xF5B6,0xF5B7,0xF5BC,0x0C41,0x0} },
	{ {0x0C18,0x0C48,0x0},					{0x0C56,0xF5C4,0xF5B6,0xF5B7,0xF5BC,0x0C41,0x0} },
	
	{ {0x0C18,0x0C4A,0x0},					{0xF5C4,0xF5B6,0xF5B7,0xF5BC,0x0C41,0x0C41,0x0} },
	{ {0x0C18,0x0C4B,0x0},					{0xF5C4,0xF5B6,0xF5B7,0xF5BC,0x0C42,0x0} },
	{ {0x0C18,0x0C4C,0x0},					{0xF61E,0xF5B6,0xF5B7,0xF5BC,0xF5D5,0x0} },
	{ {0x0C18,0x0C4D,0x0},					{0xF5C9,0xF5B6,0xF5B7,0xF5BC,0x0C41,0x0} },

		
	 // NEED TO CHECK THIS 
	{ {0x0C19,0x0},							{0x0C19,0xF5E6,0x0} },
	{ {0x0C19,0x0C3E,0x0}, 					{0x0C19,0xF5F8,0x0} },
	{ {0x0C19,0x0C3F,0x0},					{0x0C19,0xF5F3,0xF5E6,0x0 } },
	{ {0x0C19,0x0C40,0x0},					{0x0C19,0xF5F4,0xF5E6,0x0 } },
	{ {0x0C19,0x0C41,0x0},					{0x0C19,0xF5E6,0x0C41,0x0} },
	{ {0x0C19,0x0C42,0x0},					{0x0C19,0xF5E6,0x0C42,0x0} },
	{ {0x0C19,0x0C43,0x0},					{0x0C19,0xF5E6,0x0C43,0x0} },
	{ {0x0C19,0x0C44,0x0},					{0x0C19,0xF5E6,0x0C44,0x0} },
	
	{ {0x0C19,0x0C46,0x0},					{0xF5FB,0x0C19,0xF5E6,0x0} },
	{ {0x0C19,0x0C47,0x0},					{0xF5FB,0x0C19,0xF5E6,0x0C55,0x0} },
	{ {0x0C19,0x0C48,0x0},					{0x0C56,0xF5FB,0x0C19,0xF5E6,0x0} },
	
	{ {0x0C19,0x0C4A,0x0},					{0x0C19,0xF5FC,0x0} },
	{ {0x0C19,0x0C4B,0x0},					{0x0C19,0xF5FC,0x0C55,0x0} },
	{ {0x0C19,0x0C4C,0x0},					{0x0C19,0xF5FD,0x0} },
	{ {0x0C19,0x0C4D,0x0},					{0x0C19,0xF5FE } },


	{ {0x0C1A,0x0},							{0x0C1A,0xF5E6,0x0} },
	{ {0x0C1A,0x0C3E,0x0}, 					{0xF5E4,0xF5C1,0x0} },
	{ {0x0C1A,0x0C3F,0x0},					{0xF5E5,0xF5E6,0x0} },
	{ {0x0C1A,0x0C40,0x0},					{0xF5E5,0xF5AD,0xF5E6,0x0} },
	{ {0x0C1A,0x0C41,0x0},					{0x0C1A,0xF5E6,0x0C41,0x0} },
	{ {0x0C1A,0x0C42,0x0},					{0x0C1A,0xF5E6,0x0C42,0x0} },
	{ {0x0C1A,0x0C43,0x0},					{0x0C1A,0xF5E6,0x0C43,0x0} },
	{ {0x0C1A,0x0C44,0x0},					{0x0C1A,0xF5E6,0x0C44,0x0} },
	
	{ {0x0C1A,0x0C46,0x0},					{0xF5E7,0xF5E4,0xF5E6,0x0} },
	{ {0x0C1A,0x0C47,0x0},					{0xF5E7,0xF5E4,0xF5E6,0x0C55,0x0} },
	{ {0x0C1A,0x0C48,0x0},					{0x0C56,0xF5E7,0xF5E4,0xF5E6,0x0} },
	
	{ {0x0C1A,0x0C4A,0x0},					{0xF5E4,0x0C4A,0x0} },
	{ {0x0C1A,0x0C4B,0x0},					{0xF5E4,0x0C4B,0x0} },
	{ {0x0C1A,0x0C4C,0x0},					{0xF5E4,0x0C4C,0x0} },
	{ {0x0C1A,0x0C4D,0x0},					{0xF5E4,0x0C4D,0xF5E6,0x0} },


	{ {0x0C1B,0x0},							{0x0C1B,0xF5E6,0x0} },
	{ {0x0C1B,0x0C3E,0x0}, 					{0xF5E4,0xF5D8,0xF5C1,0x0} },
	{ {0x0C1B,0x0C3F,0x0},					{0xF5E5,0xF5D8,0xF5E6,0x0} },
	{ {0x0C1B,0x0C40,0x0},					{0xF5E5,0xF5D8,0xF5AD,0xF5E6,0x0} },
	{ {0x0C1B,0x0C41,0x0},					{0x0C1B,0xF5E6,0x0C41,0x0} },
	{ {0x0C1B,0x0C42,0x0},					{0x0C1B,0xF5E6,0x0C42,0x0} },
	{ {0x0C1B,0x0C43,0x0},					{0x0C1B,0xF5E6,0x0C43,0x0} },
	{ {0x0C1B,0x0C44,0x0},					{0x0C1B,0xF5E6,0x0C44,0x0} },
	
	{ {0x0C1B,0x0C46,0x0},					{0xF5E7,0xF5E4,0xF5D8,0xF5E6,0x0} },
	{ {0x0C1B,0x0C47,0x0},					{0xF5E7,0xF5E4,0xF5D8,0xF5E6,0x0C55,0x0} },
	{ {0x0C1B,0x0C48,0x0},					{0x0C56,0xF5E7,0xF5E4,0xF5D8,0xF5E6,0x0} },
	
	{ {0x0C1B,0x0C4A,0x0},					{0xF5E4,0xF5D8,0x0C4A,0x0} },
	{ {0x0C1B,0x0C4B,0x0},					{0xF5E4,0xF5D8,0x0C4B,0x0} },
	{ {0x0C1B,0x0C4C,0x0},					{0xF5E4,0xF5D8,0x0C4C,0x0} },
	{ {0x0C1B,0x0C4D,0x0},					{0xF5E4,0xF5D8,0x0C4D,0xF5E6,0x0} },

	{ {0x0C1C,0x0},							{0x0C1C,0xF5E6,0x0} },
	{ {0x0C1C,0x0C3E,0x0}, 					{0x0C1C,0xF5F8,0x0} },
	{ {0x0C1C,0x0C3F,0x0},					{0xF5F1,0xF5E6,0x0} },
	{ {0x0C1C,0x0C40,0x0},					{0xF5F2,0xF5E6,0x0} },
	{ {0x0C1C,0x0C41,0x0},					{0x0C1C,0xF5E6,0xF5F9,0x0} },
	{ {0x0C1C,0x0C42,0x0},					{0x0C1C,0xF5E6,0xF5FA,0x0} },
	{ {0x0C1C,0x0C43,0x0},					{0x0C1C,0xF5E6,0x0C43,0x0} },
	{ {0x0C1C,0x0C44,0x0},					{0x0C1C,0xF5E6,0x0C44,0x0} },
	
	{ {0x0C1C,0x0C46,0x0},					{0xF5FB,0x0C1C,0xF5E6,0x0} },
	{ {0x0C1C,0x0C47,0x0},					{0xF5FB,0x0C1C,0xF5E6,0x0C55,0x0} },
	{ {0x0C1C,0x0C48,0x0},					{0x0C56,0xF5FB,0x0C1C,0xF5E6,0x0} },
	
	{ {0x0C1C,0x0C4A,0x0},					{0x0C1C,0xF5FC,0x0} },
	{ {0x0C1C,0x0C4B,0x0},					{0x0C1C,0xF5FC,0x0C55,0x0} },
	{ {0x0C1C,0x0C4C,0x0},					{0x0C1C,0xF5FD,0x0} },
	{ {0x0C1C,0x0C4D,0x0},					{0x0C1C,0xF5Fe,0x0} },


	{ {0x0C1D,0x0},							{0x0C1D,0x0} },
	{ {0x0C1D,0x0C3E,0x0}, 					{0x0C30,0xF5B2,0xF59F,0x0C42,0x0} },
	{ {0x0C1D,0x0C3F,0x0},					{0xF5A5,0xF5B3,0xF5B2,0xF59F,0x0C41,0x0} } ,
	{ {0x0C1D,0x0C40,0x0},					{0xF5A5,0xF5B4,0xF5B2,0xF59F,0x0C41,0x0} } ,
	{ {0x0C1D,0x0C41,0x0},					{0x0C30,0xF5B2,0xF59F,0x0C41,0x0C41,0x0} },
	{ {0x0C1D,0x0C42,0x0},					{0x0C30,0xF5B2,0xF59F,0x0C41,0x0C42,0x0} },
	{ {0x0C1D,0x0C43,0x0},					{0x0C30,0xF5B2,0xF59F,0x0C41,0x0C43,0x0} },
	{ {0x0C1D,0x0C44,0x0},					{0x0C30,0xF5B2,0xF59F,0x0C41,0x0C44,0x0} },
	
	{ {0x0C1D,0x0C46,0x0},					{0xF5B5,0xF5A5,0xF5B2,0xF59F,0x0C41,0x0} },
	{ {0x0C1D,0x0C47,0x0},					{0xF5B5,0xF5A5,0xF5B2,0x0C55,0xF59F,0x0C41,0x0} },
	{ {0x0C1D,0x0C48,0x0},					{0x0C56,0xF5B5,0xF5A5,0xF5B2,0xF59F,0x0C41,0x0} },
	
	{ {0x0C1D,0x0C4A,0x0},					{0xF5B5,0xF5A5,0xF5B2,0xF59F,0x0C41,0x0C41,0x0} },
	{ {0x0C1D,0x0C4B,0x0},					{0xF5B5,0xF5A5,0xF5B2,0xF59F,0x0C42,0x0} },
	{ {0x0C1D,0x0C4C,0x0},					{0x0C30,0xF5B2,0xF59F,0xF5D5,0x0} } ,
	{ {0x0C1D,0x0C4D,0x0},					{0xF5A5,0x0C4D,0xF5B2,0xF59F,0x0C41,0x0} },


	{ {0x0C1E,0x0},							{0x0C1E,0xF5E6,0x0} },
	{ {0x0C1E,0x0C3E,0x0}, 					{0x0C1E,0xF5F8,0x0} },
	{ {0x0C1E,0x0C3F,0x0},					{0x0C1E,0xF5F3,0xF5E6,0x0} },
	{ {0x0C1E,0x0C40,0x0},					{0x0C1E,0xF5F4,0xF5E6,0x0} },
	{ {0x0C1E,0x0C41,0x0},					{0x0C1E,0xF5E6,0x0C41,0x0} },
	{ {0x0C1E,0x0C42,0x0},					{0x0C1E,0xF5E6,0x0C42,0x0} },
	{ {0x0C1E,0x0C43,0x0},					{0x0C1E,0xF5E6,0x0C43,0x0} },
	{ {0x0C1E,0x0C44,0x0},					{0x0C1E,0xF5E6,0x0C44,0x0} },
	
	{ {0x0C1E,0x0C46,0x0},					{0xF5D3,0x0C1E,0xF5E6,0x0} },
	{ {0x0C1E,0x0C47,0x0},					{0xF5D3,0x0C1E,0xF5E6,0x0C55,0x0} },
	{ {0x0C1E,0x0C48,0x0},					{0x0C56,0xF5D3,0x0C1E,0xF5E6,0x0} },
	
	{ {0x0C1E,0x0C4A,0x0},					{0x0C1E,0xF5FC,0x0} },
	{ {0x0C1E,0x0C4B,0x0},					{0x0C1E,0xF5FC,0x0C55,0x0} },
	{ {0x0C1E,0x0C4C,0x0},					{0x0C1E,0xF5FD,0x0} },
	{ {0x0C1E,0x0C4D,0x0},					{0x0C1E,0xF5FE,0x0} },


	{ {0x0C1F,0x0},							{0x0C1F,0xF5E6,0x0} },
	{ {0x0C1F,0x0C3E,0x0}, 					{0xF5EB,0xF5F8,0x0} },
	{ {0x0C1F,0x0C3F,0x0},					{0xF5EB,0xF5F3,0x0} },
	{ {0x0C1F,0x0C40,0x0},					{0xF5EB,0xF5F4,0x0} },
	{ {0x0C1F,0x0C41,0x0},					{0x0C1F,0xF5E6,0x0C41,0x0} },
	{ {0x0C1F,0x0C42,0x0},					{0x0C1F,0xF5E6,0x0C42,0x0} },
	{ {0x0C1F,0x0C43,0x0},					{0x0C1F,0xF5E6,0x0C43,0x0} },
	{ {0x0C1F,0x0C44,0x0},					{0x0C1F,0xF5E6,0x0C44,0x0} },
	
	{ {0x0C1F,0x0C46,0x0},					{0xF5FB,0xF5EB,0xF5E6,0x0} },
	{ {0x0C1F,0x0C47,0x0},					{0xF5FB,0xF5EB,0xF5E6,0x0C55,0x0} },
	{ {0x0C1F,0x0C48,0x0},					{0x0C56,0xF5FB,0xF5EB,0xF5E6,0x0} },
	
	{ {0x0C1F,0x0C4A,0x0},					{0xF5EB,0xF5FC,0x0} },
	{ {0x0C1F,0x0C4B,0x0},					{0xF5EB,0xF5FC,0x0C55,0x0} },
	{ {0x0C1F,0x0C4C,0x0},					{0xF5EB,0xF5FD,0x0} },
	{ {0x0C1F,0x0C4D,0x0},					{0xF5EB,0xF5FE,0x0} },


	{ {0x0C20,0x0},							{0x0C20,0xF5B2,0x0} },
	{ {0x0C20,0x0C3E,0x0}, 					{0xF5B1,0x0C3E,0x0} },
	{ {0x0C20,0x0C3F,0x0},					{0xF5B1,0xF5B3,0x0} },
	{ {0x0C20,0x0C40,0x0},					{0xF5B1,0xF5B4,0x0} },
	{ {0x0C20,0x0C41,0x0},					{0x0C20,0xF5B2,0x0C41,0x0} },
	{ {0x0C20,0x0C42,0x0},					{0x0C20,0xF5B2,0x0C42,0x0} },
	{ {0x0C20,0x0C43,0x0},					{0x0C20,0xF5B2,0x0C43,0x0} },
	{ {0x0C20,0x0C44,0x0},					{0x0C20,0xF5B2,0x0C44,0x0} },
	
	{ {0x0C20,0x0C46,0x0},					{0xF5B5,0xF5B1,0xF5B2,0x0} },
	{ {0x0C20,0x0C47,0x0},					{0xF5B5,0xF5B1,0xF5B2,0x0C55,0x0} },
	{ {0x0C20,0x0C48,0x0},					{0x0C56,0xF5B5,0xF5B1,0xF5B2,0x0} },
	
	{ {0x0C20,0x0C4A,0x0},					{0xF5B1,0x0C4A,0x0} },
	{ {0x0C20,0x0C4B,0x0},					{0xF5B1,0x0C4B,0x0} },
	{ {0x0C20,0x0C4C,0x0},					{0xF5B1,0x0C4C,0x0} },
	{ {0x0C20,0x0C4D,0x0},					{0xF5B1,0x0C4D,0xF5B2,0x0} },


	{ {0x0C21,0x0},							{0x0C21,0xF5DB,0x0} },
	{ {0x0C21,0x0C3E,0x0}, 					{0xF5D7,0xF5C1,0x0} },
	{ {0x0C21,0x0C3F,0x0},					{0xF5D7,0x0C3F,0xF5DB,0x0} },
	{ {0x0C21,0x0C40,0x0},					{0xF5D7,0x0C40,0xF5DB,0x0} },
	{ {0x0C21,0x0C41,0x0},					{0x0C21,0xF5DB,0x0C41,0x0} },
	{ {0x0C21,0x0C42,0x0},					{0x0C21,0xF5DB,0x0C42,0x0} },
	{ {0x0C21,0x0C43,0x0},					{0x0C21,0xF5DB,0x0C43,0x0} },
	{ {0x0C21,0x0C44,0x0},					{0x0C21,0xF5DB,0x0C44,0x0} },

	{ {0x0C21,0x0C46,0x0},					{0xF5D3,0xF5D7,0xF5DB,0x0} },
	{ {0x0C21,0x0C47,0x0},					{0xF5D3,0xF5D7,0xF5DB,0x0C55,0x0} },
	{ {0x0C21,0x0C48,0x0},					{0x0C56,0xF5D3,0xF5D7,0xF5DB,0x0} },

	{ {0x0C21,0x0C4A,0x0},					{0xF5D7,0x0C4A,0x0} },
	{ {0x0C21,0x0C4B,0x0},					{0xF5D7,0x0C4B,0x0} },
	{ {0x0C21,0x0C4C,0x0},					{0xF5D7,0x0C4C,0x0} },
	{ {0x0C21,0x0C4D,0x0},					{0xF5D7,0x0C4D,0xF5DB,0x0} },

	{ {0x0C22,0x0},							{0x0C22,0xF5DB,0x0} },
	{ {0x0C22,0x0C3E,0x0}, 					{0xF5D7,0xF5D8,0xF5C1,0x0} },
	{ {0x0C22,0x0C3F,0x0},					{0xF5D7,0xF5D8,0x0C3F,0xF5DB,0x0} },
	{ {0x0C22,0x0C40,0x0},					{0xF5D7,0xF5D8,0x0C40,0xF5DB,0x0} },
	{ {0x0C22,0x0C41,0x0},					{0x0C22,0xF5DB,0x0C41,0x0} },
	{ {0x0C22,0x0C42,0x0},					{0x0C22,0xF5DB,0x0C42,0x0} },
	{ {0x0C22,0x0C43,0x0},					{0x0C22,0xF5DB,0x0C43,0x0} },
	{ {0x0C22,0x0C44,0x0},					{0x0C22,0xF5DB,0x0C44,0x0} },
	
	{ {0x0C22,0x0C46,0x0},					{0xF5D3,0xF5D7,0xF5D8,0xF5DB,0x0} },
	{ {0x0C22,0x0C47,0x0},					{0xF5D3,0xF5D7,0xF5D8,0xF5DB,0x0C55,0x0} },
	{ {0x0C22,0x0C48,0x0},					{0x0C56,0xF5D3,0xF5D7,0xF5D8,0xF5DB,0x0} },
	
	{ {0x0C22,0x0C4A,0x0},					{0xF5D7,0xF5D8,0x0C4A,0x0} },
	{ {0x0C22,0x0C4B,0x0},					{0xF5D7,0xF5D8,0x0C4B,0x0} },
	{ {0x0C22,0x0C4C,0x0},					{0xF5D7,0xF5D8,0x0C4C,0x0} },
	{ {0x0C22,0x0C4D,0x0},					{0xF5D7,0xF5D8,0x0C4D,0xF5DB,0x0} },

	{ {0x0C23,0x0},							{0x0C23,0xF600,0x0} },
	{ {0x0C23,0x0C3E,0x0}, 					{0x0C23,0xF5F8,0x0} },
	{ {0x0C23,0x0C3F,0x0},					{0x0C23,0xF5F3,0xF600,0x0} },
	{ {0x0C23,0x0C40,0x0},					{0x0C23,0xF5F4,0xF600,0x0} },
	{ {0x0C23,0x0C41,0x0},					{0x0C23,0xF600,0x0C41,0x0} },
	{ {0x0C23,0x0C42,0x0},					{0x0C23,0xF600,0x0C42,0x0} },
	{ {0x0C23,0x0C43,0x0},					{0x0C23,0xF600,0x0C43,0x0} },
	{ {0x0C23,0x0C44,0x0},					{0x0C23,0xF600,0x0C44,0x0} },

	{ {0x0C23,0x0C46,0x0},					{0xF601,0xF5FF,0xF600,0x0} },
	{ {0x0C23,0x0C47,0x0},					{0x0C23,0xF600,0x0C41,0x0C55,0x0} },
	{ {0x0C23,0x0C48,0x0},					{0x0C56,0x0C23,0xF600,0x0C41,0x0} },

	{ {0x0C23,0x0C4A,0x0},					{0x0C23,0xF602,0xF5FC,0x0} },
	{ {0x0C23,0x0C4B,0x0},					{0x0C23,0xF602,0xF5FC,0x0C55,0x0} },
	{ {0x0C23,0x0C4C,0x0},					{0x0C23,0xF5FD,0x0} },
	{ {0x0C23,0x0C4D,0x0},					{0x0C23,0xF602,0xF5FE,0x0} },


	{ {0x0C24,0x0},							{0x0C24,0xF5E2,0x0} },
	{ {0x0C24,0x0C3E,0x0}, 					{0xF5E0,0xF5C1,0x0} },
	{ {0x0C24,0x0C3F,0x0},					{0xF5E1,0xF5E2,0x0} },
	{ {0x0C24,0x0C40,0x0},					{0xF5E1,0xF5AD,0xF5E2,0x0} },
	{ {0x0C24,0x0C41,0x0},					{0x0C24,0xF5E2,0x0C41,0x0} },
	{ {0x0C24,0x0C42,0x0},					{0x0C24,0xF5E2,0x0C42,0x0} },
	{ {0x0C24,0x0C43,0x0},					{0x0C24,0xF5E2,0x0C43,0x0} },
	{ {0x0C24,0x0C44,0x0},					{0x0C24,0xF5E2,0x0C44,0x0} },

	{ {0x0C24,0x0C46,0x0},					{0xF5E3,0xF5E0,0xF5E2,0x0} },
	{ {0x0C24,0x0C47,0x0},					{0xF5E3,0xF5E0,0xF5E2,0x0C55,0x0} },
	{ {0x0C24,0x0C48,0x0},					{0x0C56,0xF5E3,0xF5E0,0xF5E2,0x0} },

	{ {0x0C24,0x0C4A,0x0},					{0xF5E0,0x0C4A,0x0} },
	{ {0x0C24,0x0C4B,0x0},					{0xF5E0,0x0C4B,0x0} },
	{ {0x0C24,0x0C4C,0x0},					{0xF5E0,0xF5AF,0x0} },
	{ {0x0C24,0x0C4D,0x0},					{0xF5E0,0x0C4D,0xF5E2,0x0} },


	{ {0x0C25,0x0},							{0x0C25,0xF5DB,0x0} },
	{ {0x0C25,0x0C3E,0x0}, 					{0xF5D9,0xF5C1,0x0} },
	{ {0x0C25,0x0C3F,0x0},					{0xF5D9,0xF5B3,0xF5DB,0x0} },
	{ {0x0C25,0x0C40,0x0},					{0xF5D9,0xF5B4,0xF5DB,0x0} },
	{ {0x0C25,0x0C41,0x0},					{0x0C25,0xF5DB,0x0C41,0x0} },
	{ {0x0C25,0x0C42,0x0},					{0x0C25,0xF5DB,0x0C42,0x0} },
	{ {0x0C25,0x0C43,0x0},					{0x0C25,0xF5DB,0x0C43,0x0} },
	{ {0x0C25,0x0C44,0x0},					{0x0C25,0xF5DB,0x0C44,0x0} },

	{ {0x0C25,0x0C46,0x0},					{0xF5D3,0xF5D9,0xF5DB,0x0} },
	{ {0x0C25,0x0C47,0x0},					{0xF5D3,0xF5D9,0xF5DB,0x0C55,0x0} },
	{ {0x0C25,0x0C48,0x0},					{0x0C56,0xF5D3,0xF5D9,0xF5DB,0x0} },

	{ {0x0C25,0x0C4A,0x0},					{0xF5D9,0x0C4A,0x0} },
	{ {0x0C25,0x0C4B,0x0},					{0xF5D9,0x0C4B,0x0} },
	{ {0x0C25,0x0C4C,0x0},					{0xF5D9,0x0C4C,0x0} },
	{ {0x0C25,0x0C4D,0x0},					{0xF5D9,0x0C4D,0xF5DB,0x0} },


	{ {0x0C26,0x0},							{0x0C26,0xF5DB,0x0} },
	{ {0x0C26,0x0C3E,0x0}, 					{0xF5DA,0xF5C1,0x0} },
	{ {0x0C26,0x0C3F,0x0},					{0xF5DA,0xF5B3,0xF5DB,0x0} },
	{ {0x0C26,0x0C40,0x0},					{0xF5DA,0xF5B4,0xF5DB,0x0} },
	{ {0x0C26,0x0C41,0x0},					{0x0C26,0xF5DB,0x0C41,0x0} },
	{ {0x0C26,0x0C42,0x0},					{0x0C26,0xF5DB,0x0C42,0x0} },
	{ {0x0C26,0x0C43,0x0},					{0x0C26,0xF5DB,0x0C43,0x0} },
	{ {0x0C26,0x0C44,0x0},					{0x0C26,0xF5DB,0x0C44,0x0} },

	{ {0x0C26,0x0C46,0x0},					{0xF5D3,0xF5DA,0xF5DB,0x0} },
	{ {0x0C26,0x0C47,0x0},					{0xF5D3,0xF5DA,0xF5DB,0x0C55,0x0} },
	{ {0x0C26,0x0C48,0x0},					{0x0C56,0xF5D3,0xF5DA,0xF5DB,0x0} },

	{ {0x0C26,0x0C4A,0x0},					{0xF5DA,0x0C4A,0x0} },
	{ {0x0C26,0x0C4B,0x0},					{0xF5DA,0x0C4B,0x0} },
	{ {0x0C26,0x0C4C,0x0},					{0xF5DA,0x0C4C,0x0} },
	{ {0x0C26,0x0C4D,0x0},					{0xF5DA,0x0C4D,0xF5DB,0x0} },


	{ {0x0C27,0x0},							{0x0C27,0xF5DB,0x0} },
	{ {0x0C27,0x0C3E,0x0}, 					{0xF5DA,0xF5D8,0xF5C1,0x0} },
	{ {0x0C27,0x0C3F,0x0},					{0xF5DA,0xF5D8,0xF5B3,0xF5DB,0x0} },
	{ {0x0C27,0x0C40,0x0},					{0xF5DA,0xF5D8,0xF5B4,0xF5DB,0x0} },
	{ {0x0C27,0x0C41,0x0},					{0x0C27,0xF5DB,0x0C41,0x0} },
	{ {0x0C27,0x0C42,0x0},					{0x0C27,0xF5DB,0x0C42,0x0} },
	{ {0x0C27,0x0C43,0x0},					{0x0C27,0xF5DB,0x0C43,0x0} },
	{ {0x0C27,0x0C44,0x0},					{0x0C27,0xF5DB,0x0C44,0x0} },

	{ {0x0C27,0x0C46,0x0},					{0xF5D3,0xF5DA,0xF5D8,0xF5DB,0x0} },
	{ {0x0C27,0x0C47,0x0},					{0xF5D3,0xF5DA,0xF5D8,0xF5DB,0x0C55,0x0} },
	{ {0x0C27,0x0C48,0x0},					{0x0C56,0xF5D3,0xF5DA,0xF5D8,0xF5DB,0x0} },

	{ {0x0C27,0x0C4A,0x0},					{0xF5DA,0xF5D8,0x0C4A,0x0} },
	{ {0x0C27,0x0C4B,0x0},					{0xF5DA,0xF5D8,0x0C4B,0x0} },
	{ {0x0C27,0x0C4C,0x0},					{0xF5DA,0xF5D8,0x0C4C,0x0} },
	{ {0x0C27,0x0C4D,0x0},					{0xF5DA,0xF5D8,0x0C4D,0xF5DB,0x0} },


	{ {0x0C28,0x0},							{0x0C28,0xF5BC,0x0} },
	{ {0x0C28,0x0C3E,0x0}, 					{0xF5CF,0xF5C1,0x0} },
	{ {0x0C28,0x0C3F,0x0},					{0xF5D1,0xF5BC,0x0} },
	{ {0x0C28,0x0C40,0x0},					{0xF5D1,0xF5AD,0xF5BC,0x0} },
	{ {0x0C28,0x0C41,0x0},					{0x0C28,0xF5BC,0x0C41,0x0 } },
	{ {0x0C28,0x0C42,0x0},					{0x0C28,0xF5BC,0x0C42,0x0 } },
	{ {0x0C28,0x0C43,0x0},					{0x0C28,0xF5BC,0x0C43,0x0 } },
	{ {0x0C28,0x0C44,0x0},					{0x0C28,0xF5BC,0x0C44,0x0 } },

	{ {0x0C28,0x0C46,0x0},					{0xF5D3,0xF5CF,0xF5BC,0x0} },
	{ {0x0C28,0x0C47,0x0},					{0xF5D3,0xF5CF,0xF5BC,0x0C55,0x0} },
	{ {0x0C28,0x0C48,0x0},					{0x0C56,0xF5D3,0xF5CF,0xF5BC,0x0} },

	{ {0x0C28,0x0C4A,0x0},					{0xF5CF,0xF5D4,0x0} },
	{ {0x0C28,0x0C4B,0x0},					{0xF5CF,0xF5D4,0x0C55,0x0} },
	{ {0x0C28,0x0C4C,0x0},					{0xF5CF,0x0C4C,0x0} },
	{ {0x0C28,0x0C4D,0x0},					{0xF5CF,0xF5D6,0xF5BC,0x0} },

	{ {0x0C2A,0x0},							{0x0C2A,0xF5BC,0x0} },
	{ {0x0C2A,0x0C3E,0x0}, 					{0x0C0E,0xF5C1,0x0} },
	{ {0x0C2A,0x0C3F,0x0},					{0xF5C2,0xF5B6,0xF5BC,0x0} },
	{ {0x0C2A,0x0C40,0x0},					{0xF5C3,0xF5B6,0xF5BC,0x0} },
	{ {0x0C2A,0x0C41,0x0},					{0x0C2A,0xF5BC,0xF5B8,0x0} },
	{ {0x0C2A,0x0C42,0x0},					{0x0C2A,0xF5BC,0xF5B9,0x0} },
	{ {0x0C2A,0x0C43,0x0},					{0x0C2A,0xF5BC,0x0C43,0x0} },
	{ {0x0C2A,0x0C44,0x0},					{0x0C2A,0xF5BC,0x0C44,0x0} },

	{ {0x0C2A,0x0C46,0x0},					{0xF5C4,0xF5B6,0xF5BC,0x0} },
	{ {0x0C2A,0x0C47,0x0},					{0xF5C5,0xF5B6,0xF5BC,0x0} },
	{ {0x0C2A,0x0C48,0x0},					{0x0C56,0xF5C4,0xF5B6,0xF5BC,0x0} },

	{ {0x0C2A,0x0C4A,0x0},					{0x0C0E,0xF5C6,0x0} },
	{ {0x0C2A,0x0C4B,0x0},					{0x0C0E,0xF5C7,0x0} },
	{ {0x0C2A,0x0C4C,0x0},					{0x0C0E,0xF5C8,0x0} },
	{ {0x0C2A,0x0C4D,0x0},					{0xF5C9,0xF5B6,0xF5BC,0x0} },


	{ {0x0C2B,0x0},							{0x0C2B,0xF5BC,0x0} },
	{ {0x0C2B,0x0C3E,0x0}, 					{0x0C0E,0xF5B7,0xF5C1,0x0} },
	{ {0x0C2B,0x0C3F,0x0},					{0xF5C2,0xF5B6,0xF5B7,0xF5BC,0x0} },
	{ {0x0C2B,0x0C40,0x0},					{0xF5C3,0xF5B6,0xF5B7,0xF5BC,0x0} },
	{ {0x0C2B,0x0C41,0x0},					{0x0C2B,0xF5BC,0xF5B8,0x0} },
	{ {0x0C2B,0x0C42,0x0},					{0x0C2B,0xF5BC,0xF5B9,0x0} },
	{ {0x0C2B,0x0C43,0x0},					{0x0C2B,0xF5BC,0x0C43,0x0} },
	{ {0x0C2B,0x0C44,0x0},					{0x0C2B,0xF5BC,0x0C44,0x0} },

	{ {0x0C2B,0x0C46,0x0},					{0xF5C4,0xF5B6,0xF5B7,0xF5BC,0x0} },
	{ {0x0C2B,0x0C47,0x0},					{0xF5C5,0xF5B6,0xF5B7,0xF5BC,0x0} },
	{ {0x0C2B,0x0C48,0x0},					{0x0C56,0xF5C4,0xF5B6,0xF5B7,0xF5BC,0x0} },

	{ {0x0C2B,0x0C4A,0x0},					{0x0C0E,0xF5B7,0xF5C6,0x0} },
	{ {0x0C2B,0x0C4B,0x0},					{0x0C0E,0xF5B7,0xF5C7,0x0} },
	{ {0x0C2B,0x0C4C,0x0},					{0x0C0E,0xF5B7,0xF5C8,0x0} },
	{ {0x0C2B,0x0C4D,0x0},					{0xF5C9,0xF5B6,0xF5B7,0xF5BC,0x0} },


	{ {0x0C2C,0x0},							{0x0C2C,0xF5E6,0x0} },
	{ {0x0C2C,0x0C3E,0x0}, 					{0xF5EC,0xF5F8,0x0} },
	{ {0x0C2C,0x0C3F,0x0},					{0xF5F5,0xF5E6,0x0} },
	{ {0x0C2C,0x0C40,0x0},					{0xF5F5,0xF5AD,0xF5E6,0x0} },
	{ {0x0C2C,0x0C41,0x0},					{0x0C2C,0xF5E6,0x0C41,0x0} },
	{ {0x0C2C,0x0C42,0x0},					{0x0C2C,0xF5E6,0x0C42,0x0} },
	{ {0x0C2C,0x0C43,0x0},					{0x0C2C,0xF5E6,0x0C43,0x0} },
	{ {0x0C2C,0x0C44,0x0},					{0x0C2C,0xF5E6,0x0C44,0x0} },

	{ {0x0C2C,0x0C46,0x0},					{0xF5FB,0xF5EC,0xF5E6,0x0} },
	{ {0x0C2C,0x0C47,0x0},					{0xF5FB,0xF5EC,0xF5E6,0x0C55,0x0} },
	{ {0x0C2C,0x0C48,0x0},					{0x0C56,0xF5FB,0xF5EC,0xF5E6,0x0} },

	{ {0x0C2C,0x0C4A,0x0},					{0xF5EC,0xF5FC,0x0} },
	{ {0x0C2C,0x0C4B,0x0},					{0xF5EC,0xF5FC,0x0C55,0x0} },
	{ {0x0C2C,0x0C4C,0x0},					{0xF5EC,0xF5FD,0x0} },
	{ {0x0C2C,0x0C4D,0x0},					{0xF5EC,0xF5FE,0x0} },


	{ {0x0C2D,0x0},							{0x0C2D,0xF5E6,0x0} },
	{ {0x0C2D,0x0C3E,0x0}, 					{0xF5EC,0xF5D8,0xF5F8,0x0} },
	{ {0x0C2D,0x0C3F,0x0},					{0xF5F5,0xF5D8,0xF5E6,0x0} },
	{ {0x0C2D,0x0C40,0x0},					{0xF5F5,0xF5AD,0xF5D8,0xF5E6,0x0} },
	{ {0x0C2D,0x0C41,0x0},					{0x0C2D,0xF5E6,0x0C41,0x0} },
	{ {0x0C2D,0x0C42,0x0},					{0x0C2D,0xF5E6,0x0C42,0x0} },
	{ {0x0C2D,0x0C43,0x0},					{0x0C2D,0xF5E6,0x0C43,0x0} },
	{ {0x0C2D,0x0C44,0x0},					{0x0C2D,0xF5E6,0x0C44,0x0} },

	{ {0x0C2D,0x0C46,0x0},					{0xF5FB,0xF5EC,0xF5D8,0xF5E6,0x0} },
	{ {0x0C2D,0x0C47,0x0},					{0xF5FB,0xF5EC,0xF5D8,0xF5E6,0x0C55,0x0} },
	{ {0x0C2D,0x0C48,0x0},					{0x0C56,0xF5FB,0xF5EC,0xF5D8,0xF5E6,0x0} },

	{ {0x0C2D,0x0C4A,0x0},					{0xF5EC,0xF5D8,0xF5FC,0x0} },
	{ {0x0C2D,0x0C4B,0x0},					{0xF5EC,0xF5D8,0xF5FC,0x0C55,0x0} },
	{ {0x0C2D,0x0C4C,0x0},					{0xF5EC,0xF5D8,0xF5FD,0x0} },
	{ {0x0C2D,0x0C4D,0x0},					{0xF5EC,0xF5D8,0xF5FE,0x0} },

	{ {0x0C2E,0x0},							{0x0C2E,0x0} },
	//{ {0x0C2E,0x0C3E,0x0}, 					{0x}}
	{ {0x0C2E,0x0C3F,0x0},					{0xF5D2,0xF5BC,0x0C41,0x0} },
	{ {0x0C2E,0x0C40,0x0},					{0xF5D2,0xF5AD,0xF5BC,0x0C41,0x0} },
	{ {0x0C2E,0x0C41,0x0},					{0x0C2E,0x0C41,0x0} },
	{ {0x0C2E,0x0C42,0x0},					{0x0C2E,0x0C42,0x0} },
	{ {0x0C2E,0x0C43,0x0},					{0x0C2E,0x0C43,0x0} },
	{ {0x0C2E,0x0C44,0x0},					{0x0C2E,0x0C44,0x0} },

	{ {0x0C2E,0x0C46,0x0},					{0xF5D3,0xF5D0,0xF5BC,0x0C41,0x0} },
	{ {0x0C2E,0x0C47,0x0},					{0xF5D3,0xF5D0,0xF5BC,0x0C55,0x0C41,0x0} },
	{ {0x0C2E,0x0C48,0x0},					{0x0C56,0xF5D3,0xF5D0,0xF5BC,0x0C41,0x0} },

	{ {0x0C2E,0x0C4A,0x0},					{0xF5D3,0xF5D0,0xF5BC,0x0C41,0x0C41,0x0} },
	{ {0x0C2E,0x0C4B,0x0},					{0xF5D3,0xF5D0,0xF5BC,0x0C42,0x0} },
	//{ {0x0C2E,0x0C4C,0x0},					{0x}}
	{ {0x0C2E,0x0C4D,0x0},					{0xF5D0,0xF5D6,0xF5BC,0x0C41,0x0} },


	{ {0x0C2F,0x0},							{0x0C2F,0x0} },
//	{ {0x0C2F,0x0C3E,0x0}, 					{0x}}
	{ {0x0C2F,0x0C3F,0x0},					{0xF5A5,0xF5B2,0x0C41,0x0C41,0x0} },
	{ {0x0C2F,0x0C40,0x0},					{0xF5A5,0xF5B2,0x0C41,0x0C42,0x0} },
	{ {0x0C2F,0x0C41,0x0},					{0x0C2F,0x0C41,0x0} },
	{ {0x0C2F,0x0C42,0x0},					{0x0C2F,0x0C42,0x0} },
	{ {0x0C2F,0x0C43,0x0},					{0x0C2F,0x0C43,0x0} },
	{ {0x0C2F,0x0C44,0x0},					{0x0C2F,0x0C44,0x0} },

	{ {0x0C2F,0x0C46,0x0},					{0xF5DE,0xF5DD,0xF5DB,0x0C41,0x0} },
	{ {0x0C2F,0x0C47,0x0},					{0xF5DE,0xF5DD,0xF5DB,0x0C55,0x0C41,0x0} },
	{ {0x0C2F,0x0C48,0x0},					{0x0C56,0xF5DE,0xF5DD,0xF5DB,0x0C41,0x0} },

	{ {0x0C2F,0x0C4A,0x0},					{0xF5DE,0xF5DD,0xF5DB,0x0C41,0x0C41,0x0} },
	{ {0x0C2F,0x0C4B,0x0},					{0xF5DE,0xF5DD,0xF5DB,0x0C42,0x0} },
	//{ {0x0C2F,0x0C4C,0x0},					{0x}}
	{ {0x0C2F,0x0C4D,0x0},					{0xF5DD,0xF5DF,0xF5DB,0x0C41,0x0} },


	{ {0x0C30,0x0},							{0x0C30,0xF5B2,0x0} },
	{ {0x0C30,0x0C3E,0x0}, 					{0xF5A5,0x0C3E,0x0} },
	{ {0x0C30,0x0C3F,0x0},					{0xF5A5,0xF5B3,0xF5B2,0x0} },
	{ {0x0C30,0x0C40,0x0},					{0xF5A5,0xF5B4,0xF5B2,0x0} },
	{ {0x0C30,0x0C41,0x0},					{0x0C30,0xF5B2,0x0C41,0x0} },
	{ {0x0C30,0x0C42,0x0},					{0x0C30,0xF5B2,0x0C42,0x0} },
	{ {0x0C30,0x0C43,0x0},					{0x0C30,0xF5B2,0x0C43,0x0} },
	{ {0x0C30,0x0C44,0x0},					{0x0C30,0xF5B2,0x0C44,0x0} },

	{ {0x0C30,0x0C46,0x0},					{0xF5B5,0xF5A5,0xF5B2,0x0} },
	{ {0x0C30,0x0C47,0x0},					{0xF5B5,0xF5A5,0xF5B2,0x0C55,0x0} },
	{ {0x0C30,0x0C48,0x0},					{0x0C56,0xF5B5,0xF5A5,0xF5B2,0x0} },

	{ {0x0C30,0x0C4A,0x0},					{0xF5A5,0x0C4A,0x0} },
	{ {0x0C30,0x0C4B,0x0},					{0xF5A5,0x0C4B,0x0} },
	{ {0x0C30,0x0C4C,0x0},					{0xF5A5,0x0C4C,0x0} },
	{ {0x0C30,0x0C4D,0x0},					{0xF5A5,0x0C4D,0xF5B2,0x0} },


	{ {0x0C31,0x0},							{0x0C31,0xF600,0x0} },
	{ {0x0C31,0x0C3E,0x0}, 					{0x0C31,0xF5F8,0x0} },
	{ {0x0C31,0x0C3F,0x0},					{0x0C31,0xF5F3,0xF600,0x0} },
	{ {0x0C31,0x0C40,0x0},					{0x0C31,0xF5F4,0xF600,0x0} },
	{ {0x0C31,0x0C41,0x0},					{0x0C31,0xF600,0x0C41,0x0} },
	{ {0x0C31,0x0C42,0x0},					{0x0C31,0xF600,0x0C42,0x0} },
	{ {0x0C31,0x0C43,0x0},					{0x0C31,0xF600,0x0C43,0x0} },
	{ {0x0C31,0x0C44,0x0},					{0x0C31,0xF600,0x0C44,0x0} },

	{ {0x0C31,0x0C46,0x0},					{0xF601,0x0C31,0xF600,0x0} },
	{ {0x0C31,0x0C47,0x0},					{0xF601,0x0C31,0xF600,0x0C55,0x0} },
	{ {0x0C31,0x0C48,0x0},					{0x0C56,0xF601,0x0C31,0xF600,0x0} },

	{ {0x0C31,0x0C4A,0x0},					{0x0C31,0xF602,0xF5FC,0x0} },
	{ {0x0C31,0x0C4B,0x0},					{0x0C31,0xF602,0xF5FC,0x0C55,0x0} },
	{ {0x0C31,0x0C4C,0x0},					{0x0C31,0xF5FD,0x0} },
	{ {0x0C31,0x0C4D,0x0},					{0x0C31,0xF602,0xF5FE,0x0} },


	{ {0x0C32,0x0},							{0x0C32,0xF5E6,0x0} },
	{ {0x0C32,0x0C3E,0x0}, 					{0xF5EE,0xF5F8,0x0} },
	{ {0x0C32,0x0C3F,0x0},					{0xF5F6,0xF5E6,0x0} },
	{ {0x0C32,0x0C40,0x0},					{0xF5F6,0xF5AD,0xF5E6,0x0} },
	{ {0x0C32,0x0C41,0x0},					{0x0C32,0xF5E6,0x0C41,0x0} },
	{ {0x0C32,0x0C42,0x0},					{0x0C32,0xF5E6,0x0C42,0x0} },
	{ {0x0C32,0x0C43,0x0},					{0x0C32,0xF5E6,0x0C43,0x0} },
	{ {0x0C32,0x0C44,0x0},					{0x0C32,0xF5E6,0x0C44,0x0} },

	{ {0x0C32,0x0C46,0x0},					{0xF5FB,0xF5EE,0xF5E6,0x0} },
	{ {0x0C32,0x0C47,0x0},					{0xF5FB,0xF5EE,0xF5E6,0x0C55,0x0} },
	{ {0x0C32,0x0C48,0x0},					{0x0C56,0xF5FB,0xF5EE,0xF5E6,0x0} },

	{ {0x0C32,0x0C4A,0x0},					{0xF5EE,0xF5FC,0x0} },
	{ {0x0C32,0x0C4B,0x0},					{0xF5EE,0xF5FC,0x0C55,0x0} },
	{ {0x0C32,0x0C4C,0x0},					{0xF5EE,0xF5FD,0x0} },
	{ {0x0C32,0x0C4D,0x0},					{0xF5EE,0xF5FE,0x0} },


	{ {0x0C33,0x0},							{0x0C33,0xF5E6,0x0} },
	{ {0x0C33,0x0C3E,0x0}, 					{0xF5EF,0xF5F8,0x0} },
	{ {0x0C33,0x0C3F,0x0},					{0xF5F7,0xF5E6,0x0} },
	{ {0x0C33,0x0C40,0x0},					{0xF5F7,0xF5AD,0xF5E6,0x0} },
	{ {0x0C33,0x0C41,0x0},					{0x0C33,0xF5E6,0x0C41,0x0} },
	{ {0x0C33,0x0C42,0x0},					{0x0C33,0xF5E6,0x0C42,0x0} },
	{ {0x0C33,0x0C43,0x0},					{0x0C33,0xF5E6,0x0C43,0x0} },
	{ {0x0C33,0x0C44,0x0},					{0x0C33,0xF5E6,0x0C44,0x0} },

	{ {0x0C33,0x0C46,0x0},					{0xF5FB,0xF5EF,0xF5E6,0x0} },
	{ {0x0C33,0x0C47,0x0},					{0xF5FB,0xF5EF,0xF5E6,0x0C55,0x0} },
	{ {0x0C33,0x0C48,0x0},					{0x0C56,0xF5FB,0xF5EF,0xF5E6,0x0} },

	{ {0x0C33,0x0C4A,0x0},					{0xF5EF,0xF5FC,0x0} },
	{ {0x0C33,0x0C4B,0x0},					{0xF5EF,0xF5FC,0x0C55,0x0} },
	{ {0x0C33,0x0C4C,0x0},					{0xF5EF,0xF5FD,0x0} },
	{ {0x0C33,0x0C4D,0x0},					{0xF5EF,0xF5Fe,0x0} },

	{ {0x0C35,0x0},							{0x0C35,0xF5Bc,0x0} },		// Va 
	{ {0x0C35,0x0C3E,0x0}, 					{0xF5D0,0xF5C1,0x0} },
	{ {0x0C35,0x0C3F,0x0},					{0xF5D2,0xF5BC,0x0} },
	{ {0x0C35,0x0C40,0x0},					{0xF5D2,0xF5AD,0xF5BC,0x0} },
	{ {0x0C35,0x0C41,0x0},					{0x0C35,0xF5BC,0xF5B8,0x0} },
	{ {0x0C35,0x0C42,0x0},					{0x0C35,0xF5BC,0xF5B9,0x0} },
	{ {0x0C35,0x0C43,0x0},					{0x0C35,0xF5BC,0x0C43,0x0} },
	{ {0x0C35,0x0C44,0x0},					{0x0C35,0xF5BC,0x0C44,0x0} },

	{ {0x0C35,0x0C46,0x0},					{0xF5D3,0xF5D0,0xF5BC,0x0} },
	{ {0x0C35,0x0C47,0x0},					{0xF5D3,0xF5D0,0xF5BC,0x0C55,0x0} },
	{ {0x0C35,0x0C48,0x0},					{0x0C56,0xF5D3,0xF5D0,0xF5BC,0x0} },

	{ {0x0C35,0x0C4A,0x0},					{0xF5D0,0xF5D4,0x0} },
	{ {0x0C35,0x0C4B,0x0},					{0xF5D0,0xF5D4,0x0C55,0x0} },
	{ {0x0C35,0x0C4C,0x0},					{0xF5D0,0x0C4C,0x0} },
	{ {0x0C35,0x0C4D,0x0},					{0xF5D0,0xF5D6,0xF5BC,0x0} },		// Va halant

	{ {0x0C36,0x0},							{0x0C36,0xF5A8,0x0} },		// Sha
	{ {0x0C36,0x0C3E,0x0}, 					{0xF5AB,0x0C3E,0x0} },
	{ {0x0C36,0x0C3F,0x0},					{0xF5AC,0xF5A8,0x0} },
	{ {0x0C36,0x0C40,0x0},					{0xF5AC,0xF5AD,0xF5A8,0x0} },
	{ {0x0C36,0x0C41,0x0},					{0x0C36,0xF5A8,0x0C41,0x0} },
	{ {0x0C36,0x0C42,0x0},					{0x0C36,0xF5A8,0x0C42,0x0} },
	{ {0x0C36,0x0C43,0x0},					{0x0C36,0xF5A8,0x0C43,0x0} },
	{ {0x0C36,0x0C44,0x0},					{0x0C36,0xF5A8,0x0C44,0x0} },

	{ {0x0C36,0x0C46,0x0},					{0x0C46,0xF5AB,0xF5A8,0x0} },
	{ {0x0C36,0x0C47,0x0},					{0x0C46,0xF5AB,0xF5A8,0x0C55,0x0} },
	{ {0x0C36,0x0C48,0x0},					{0x0C56,0x0C46,0xF5AB,0xF5A8,0x0} },

	{ {0x0C36,0x0C4A,0x0},					{0xF5AB,0x0C4A,0x0} },
	{ {0x0C36,0x0C4B,0x0},					{0xF5AB,0x0C4B,0x0} },
	{ {0x0C36,0x0C4C,0x0},					{0xF5AB,0xF5AF,0x0} },
	{ {0x0C36,0x0C4D,0x0},					{0xF5AB,0x0C4D,0xF5A8,0x0} },		// Sha halant


	{ {0x0C37,0x0},							{0x0C37,0xF5A9,0x0} },		// sha	
	{ {0x0C37,0x0C3E,0x0}, 					{0xF5Be,0xF5C1,0x0} },
	{ {0x0C37,0x0C3F,0x0},					{0xF5C2,0xF5BD,0xF5BC,0xF5A9,0x0} },
	{ {0x0C37,0x0C40,0x0},					{0xF5C3,0xF5BD,0xF5BC,0xF5A9,0x0} },
	{ {0x0C37,0x0C41,0x0},					{0x0C37,0xF5BC,0xF5BF,0x0} },
	{ {0x0C37,0x0C42,0x0},					{0x0C37,0xF5BC,0xF5C0,0x0} },
	{ {0x0C37,0x0C43,0x0},					{0x0C37,0xF5BC,0xF5A9,0x0C43,0x0} },
	{ {0x0C37,0x0C44,0x0},					{0x0C37,0xF5BC,0xF5A9,0x0C44,0x0} },

	{ {0x0C37,0x0C46,0x0},					{0xF5C4,0xF5BD,0xF5BC,0xF5A9,0x0} },
	{ {0x0C37,0x0C47,0x0},					{0xF5C5,0xF5BD,0xF5BC,0xF5A9,0x0} },
	{ {0x0C37,0x0C48,0x0},					{0x0C56,0xF5C4,0xF5BD,0xF5BC,0xF5A9,0x0} },

	{ {0x0C37,0x0C4A,0x0},					{0xF5BE,0xF5C6,0x0} } ,
	{ {0x0C37,0x0C4B,0x0},					{0xF5BE,0xF5C7,0x0} } ,
	{ {0x0C37,0x0C4C,0x0},					{0xF5BE,0xF5C8,0x0} } ,
	{ {0x0C37,0x0C4D,0x0},					{0xF5C9,0xF5BD,0xF5BC,0xF5A9,0x0} },		// sha halant


	{ {0x0C38,0x0},							{0x0C38,0xF5BC,0x0} },		// sa
	{ {0x0C38,0x0C3E,0x0}, 					{0xF5BB,0xF5C1,0x0} },
	{ {0x0C38,0x0C3F,0x0},					{0xF5C2,0xF5BA,0xF5BC,0x0} },
	{ {0x0C38,0x0C40,0x0},					{0xF5C3,0xF5BA,0xF5BC,0x0} },
	{ {0x0C38,0x0C41,0x0},					{0x0C38,0xF5BC,0x0C41,0x0} },
	{ {0x0C38,0x0C42,0x0},					{0x0C38,0xF5BC,0x0C42,0x0} },
	{ {0x0C38,0x0C43,0x0},					{0x0C38,0xF5BC,0x0C43,0x0} },
	{ {0x0C38,0x0C44,0x0},					{0x0C38,0xF5BC,0x0C44,0x0} },

	{ {0x0C38,0x0C46,0x0},					{0xF5C4,0xF5BA,0xF5BC,0x0} },
	{ {0x0C38,0x0C47,0x0},					{0xF5C5,0xF5BA,0xF5BC,0x0} },
	{ {0x0C38,0x0C48,0x0},					{0x0C56,0xF5C4,0xF5BA,0xF5BC,0x0} },

	{ {0x0C38,0x0C4A,0x0},					{0xF5BB,0xF5C6,0x0} },
	{ {0x0C38,0x0C4B,0x0},					{0xF5BB,0xF5C7,0x0} },
	{ {0x0C38,0x0C4C,0x0},					{0xF5BB,0xF5C8,0x0} },
	{ {0x0C38,0x0C4D,0x0},					{0xF5C9,0xF5BA,0xF5BC,0x0} },

	{ {0x0C39,0x0},							{0x0C39,0xF5CC,0xF5CC,0xF5CC,0x0} },		// ha 
	{ {0x0C39,0x0C3E,0x0}, 					{0xF5CB,0xF5CC,0xF5CC,0xF5CC,0x0} },
	{ {0x0C39,0x0C3F,0x0},					{0xF5C2,0xF5CA,0xF5CC,0xF5CC,0xF5CC,0x0} },
	{ {0x0C39,0x0C40,0x0},					{0xF5C3,0xF5CA,0xF5CC,0xF5CC,0xF5CC,0x0} },
	{ {0x0C39,0x0C41,0x0},					{0x0C39,0xF5CC,0xF5CC,0xF5CC,0xF5CD,0x0} },
	{ {0x0C39,0x0C42,0x0},					{0x0C39,0xF5CC,0xF5CC,0xF5CC,0xF5CE,0x0} },
	{ {0x0C39,0x0C43,0x0},					{0x0C39,0xF5CC,0xF5CC,0xF5CC,0x0C43,0x0} },
	{ {0x0C39,0x0C44,0x0},					{0x0C39,0xF5CC,0xF5CC,0xF5CC,0x0C44,0x0} },

	{ {0x0C39,0x0C46,0x0},					{0xF5C4,0xF5CA,0xF5CC,0xF5CC,0xF5CC,0x0} },
	{ {0x0C39,0x0C47,0x0},					{0xF5C5,0xF5CA,0xF5CC,0xF5CC,0xF5CC,0x0} },
	{ {0x0C39,0x0C48,0x0},					{0x0C56,0xF5C4,0xF5CA,0xF5CC,0xF5CC,0xF5CC,0x0} },

	{ {0x0C39,0x0C4A,0x0},					{0x0C39,0xF5CC,0xF5FC,0x0} },
	{ {0x0C39,0x0C4B,0x0},					{0x0C39,0xF5CC,0xF5FC,0x0C55,0x0} },
	{ {0x0C39,0x0C4C,0x0},					{0x0C39,0xF5CC,0xF5FD,0x0} },
	{ {0x0C39,0x0C4D,0x0},					{0xF5C9,0xF5Ca,0xF5CC,0xF5CC,0xF5CC,0x0} },		// ha halant

	// VOWEL SIGNS ( MATRAS)
	{ {0x0C3E,0x0},							{0x0C3E,0x0} },
	{ {0x0C3F,0x0},							{0x0C3F,0x0} },
	{ {0x0C40,0x0},							{0x0C40,0x0} },
	{ {0x0C41,0x0},							{0x0C41,0x0} },
	{ {0x0C42,0x0},							{0x0C42,0x0} },
	{ {0x0C43,0x0},							{0x0C43,0x0} },
	{ {0x0C44,0x0},							{0x0C44,0x0} },
	{ {0x0C46,0x0},							{0x0C46,0x0} },
	{ {0x0C47,0x0},							{0x0C47,0x0} },
	{ {0x0C48,0x0},							{0x0C48,0x0} },
	{ {0x0C4A,0x0},							{0x0C4A,0x0} },
	{ {0x0C4B,0x0},							{0x0C4B,0x0} },
	{ {0x0C4C,0x0},							{0x0C4C,0x0} },

	// HALANT + CONSONANTS
	{ {0x0C4D,0x0},							{0x0C4D,0x0} },
	{ {0x0C4D,0x0C15,0x0},					{0xF603,0x0} },
	{ {0x0C4D,0x0C16,0x0},					{0xF604,0x0} },
	{ {0x0C4D,0x0C17,0x0},					{0xF605,0x0} },
	{ {0x0C4D,0x0C18,0x0},					{0xF606,0x0} },
	//{ {0x0C4D,0x0C19,0x0},				 special case }
	{ {0x0C4D,0x0C1A,0x0},					{0xF607,0x0} },
	{ {0x0C4D,0x0C1B,0x0},					{0xF607,0xF608,0x0} },
	{ {0x0C4D,0x0C1C,0x0},					{0xF609,0x0} },
	{ {0x0C4D,0x0C1D,0x0},					{0xF60A,0x0} },
	{ {0x0C4D,0x0C1E,0x0},					{0xF60B,0x0} },
	{ {0x0C4D,0x0C1F,0x0},					{0xF60C,0x0} },
	{ {0x0C4D,0x0C20,0x0},					{0xF60D,0x0} },
	{ {0x0C4D,0x0C21,0x0},					{0xF60E,0x0} },
	{ {0x0C4D,0x0C22,0x0},					{0xF60E,0xF60F,0x0} },
	{ {0x0C4D,0x0C23,0x0},					{0xF610,0x0} },
	{ {0x0C4D,0x0C24,0x0},					{0xF611,0x0} },
	{ {0x0C4D,0x0C25,0x0},					{0xF612,0x0} },
	{ {0x0C4D,0x0C26,0x0},					{0xF613,0x0} },
	{ {0x0C4D,0x0C27,0x0},					{0xF613,0xF60F,0x0} },
	{ {0x0C4D,0x0C28,0x0},					{0xF614,0x0} },
	{ {0x0C4D,0x0C2A,0x0},					{0xF615,0x0} },
	{ {0x0C4D,0x0C2B,0x0},					{0xF615,0xF608,0x0} },
	{ {0x0C4D,0x0C2C,0x0},					{0xF616,0x0} },
	{ {0x0C4D,0x0C2D,0x0},					{0xF616,0xF608,0x0} },
	{ {0x0C4D,0x0C2E,0x0},					{0xF617,0x0} },
	{ {0x0C4D,0x0C2F,0x0},					{0xF618,0x0} },
	{ {0x0C4D,0x0C30,0x0},					{0xF61A,0x0} },
	{ {0x0C4D,0x0C31,0x0},					{0xF61B,0x0} },
	{ {0x0C4D,0x0C32,0x0},					{0xF61C,0x0} },
	{ {0x0C4D,0x0C33,0x0},					{0xF61D,0x0} },
	{ {0x0C4D,0x0C35,0x0},					{0xF5A0,0x0} },
	{ {0x0C4D,0x0C36,0x0},					{0xF5A1,0x0} },
	{ {0x0C4D,0x0C37,0x0},					{0xF5A2,0x0} },
	{ {0x0C4D,0x0C38,0x0},					{0xF5A3,0x0} },
	{ {0x0C4D,0x0C39,0x0},					{0xF5A4,0x0} },
	
	// MISC
	{ {0x0C55,0x0},							{0x0C55,0x0} },
	{ {0x0C56,0x0},							{0x0C56,0x0} },

	
	// DIGITS 
	{ {0x0C66,0x0},							{0x0C66,0x0} },
	{ {0x0C67,0x0},							{0x0C67,0x0} },
	{ {0x0C68,0x0},							{0x0C68,0x0} },
	{ {0x0C69,0x0},							{0x0C69,0x0} },
	{ {0x0C6A,0x0},							{0x0C6A,0x0} },
	{ {0x0C6B,0x0},							{0x0C6B,0x0} },
	{ {0x0C6C,0x0},							{0x0C6C,0x0} },
	{ {0x0C6D,0x0},							{0x0C6D,0x0} },
	{ {0x0C6E,0x0},							{0x0C6E,0x0} },
	{ {0x0C6F,0x0},							{0x0C6F,0x0} },
	
};


int TlgVattuTypeTbl[] =
{
/* 15*/	V_SIDE,
/* 16*/	V_BOT,
/* 17*/ V_BOT,
/* 18*/	V_BOT,
/* 19*/ V_BOT,
/* 1A*/ V_SIDE,
/* 1B*/ V_SIDE,
/* 1C*/ V_BOT,
/* 1D*/ V_BOT,
/* 1E*/ V_BOT,
/* 1F*/ V_BOT,
/* 20*/ V_BOT,
/* 21*/ V_BOT,
/* 22*/ V_BOT,
/* 23*/ V_BOT,
/* 24*/ V_BOT,
/* 25*/ V_BOT,
/* 26*/ V_BOT,
/* 27*/ V_BOT,
/* 28*/ V_SIDE,
/* 29*/ 0,
/* 2A*/ V_SIDE,
/* 2B*/ V_SIDE,
/* 2C*/	V_SIDE,
/* 2D*/ V_SIDE,
/* 2E*/ V_SIDE,
/* 2F*/ V_SIDE,
/* 30*/ V_SIDE,
/* 31*/ V_BOT,
/* 32*/ V_BOT,
/* 33*/ V_SIDE,
/* 34*/ 0,
/* 35*/ V_SIDE,
/* 36*/ V_SIDE,
/* 37*/ V_BOT,
/* 38*/ V_SIDE,
/* 39*/ V_BOT,
};


static const MlGlyphEntry MlGlyphTbl[MAP_ML_SIZE] = 
{
	// Vowel modifiers

	{ {0x0D02,0x0},									{0x0D02,0x0} },
	{ {0x0D03,0x0},									{0x0D03,0x0} },
													
	// Vowels										
	{ {0x0D05,0x0},									{0x0D05,0x0} },
	{ {0x0D06,0x0},									{0x0D06,0x0} },
	{ {0x0D07,0x0},									{0x0D07,0x0} },
	{ {0x0D08,0x0},									{0x0D08,0x0} },
	{ {0x0D09,0x0},									{0x0D09,0x0} },
	{ {0x0D0A,0x0},									{0x0D0A,0x0} },
	{ {0x0D0B,0x0},									{0x0D0B,0x0} },
	{ {0x0D0C,0x0},									{0x0D0C,0x0} },
	{ {0x0D0E,0x0},									{0x0D0E,0x0} },
	{ {0x0D0F,0x0},									{0x0D0F,0x0} },
	{ {0x0D10,0x0},									{0x0D10,0x0} },
	{ {0x0D12,0x0},									{0x0D12,0x0} },
	{ {0x0D13,0x0},									{0x0D13,0x0} },
	{ {0x0D14,0x0},									{0x0D14,0x0} },
	{ {0x0D60,0x0},									{0x0D60,0x0} },
	{ {0x0D61,0x0},									{0x0D61,0x0} },
													
	// Consonants									
	// Ka											
	{ {0x0D15,0x0},									{0x0D15,0x0} },
//	{ {0x0D15,0x0D4D,0x0},						{0x0D15,0x0D4D,0x0} },
//	{ {0x0D15,0x0D4D,0x0D2f,0x0},					{0xF481,0x0} },
	{ {0x0D15,0x0D4D,0x0D15,0x0},					{0xF484,0x0} },
	{ {0x0D15,0x0D4D,0x0D32,0x0},					{0xF485,0x0} },
	{ {0x0D15,0x0D4D,0x0D33,0x0},					{0xF485,0x0} },
	{ {0x0D15,0x0D4D,0x0D37,0x0},					{0xF486,0x0} },
	//{ {0x0D15,0x0D4D,0x0D19,0x0},					{0xF489,0x0} },
	{ {0x0D15,0x0D4D,0x0D1F,0x0},					{0xF4B1,0x0} },
	{ {0x0D15,0x0D4D,0x0D24,0x0},					{0xF4C3,0x0} },
													
	// Kha											
	{ {0x0D16,0x0},									{0x0D16,0x0} },
													
	// ga											
	{ {0x0D17,0x0},									{0x0D17,0x0} },
	//{ {0x0D17,0x0D4D,0x0},						{0x0D17,0x0D4D,0x0} },
	{ {0x0D17,0x0D4D,0x0D17,0x0},					{0xF487,0x0} },
	{ {0x0D17,0x0D4D,0x0D33,0x0},					{0xF488,0x0} },
	{ {0x0D17,0x0D4D,0x0D2E,0x0},					{0xF4BF,0x0} },
	{ {0x0D17,0x0D4D,0x0D28,0x0},					{0xF4C4,0x0} },

													
	// gha											
	{ {0x0D18,0x0},									{0x0D18,0x0} },
													
	//Nga											
	{ {0x0D19,0x0},									{0x0D19,0x0} },
	{ {0x0D19,0x0D4D,0x0D19,0x0},					{0xF48A,0x0} },
	{ {0x0D19,0x0D4D,0x0D15,0x0},					{0xF489,0x0} },
													
	// cha											
	{ {0x0D1A,0x0},									{0x0D1A,0x0} },
	//{ {0x0D1A,0x0D4D,0x0},									{0x0D1A,0x0D4D,0x0} },
	{ {0x0D1A,0x0D4D,0x0D1A,0x0},					{0xF48B,0x0} },
	{ {0x0D1A,0x0D4D,0x0D1B,0x0},					{0xF4B4,0x0} },
													
	//Cha											
	{ {0x0D1B,0x0},									{0x0D1B,0x0} },
													
	// ja											
	{ {0x0D1C,0x0},									{0x0D1C,0x0} },
	//{ {0x0D1C,0x0D4D,0x0},									{0x0D1C,0x0D4D,0x0} },
	{ {0x0D1C,0x0D4D,0x0D1C,0x0},					{0xF4B9,0x0} },
	{ {0x0D1C,0x0D4D,0x0D1E,0x0},					{0xF4BD,0x0} },
													
	// jha											
	{ {0x0D1D,0x0},									{0x0D1D,0x0} },
													
	// jna											
	{ {0x0D1E,0x0},									{0x0D1E,0x0} },
	//{ {0x0D1E,0x0D4D,0x0},									{0x0D1E,0x0D4D,0x0} },
	{ {0x0D1E,0x0D4D,0x0D1A,0x0},					{0xF48C,0x0} },
	{ {0x0D1E,0x0D4D,0x0D1E,0x0},					{0xF48D,0x0} },
													
	// ta											
	{ {0x0D1F,0x0},									{0x0D1F,0x0} },
	{ {0x0D1F,0x0D4D,0x0D1F,0x0},					{0xF48E,0x0} },
													
	// tha											
	{ {0x0D20,0x0},									{0x0D20,0x0} },
													
	// da											
	{ {0x0D21,0x0},									{0x0D21,0x0} },
	//{ {0x0D21,0x0D4D,0x0},									{0x0D21,0x0D4D,0x0} },
	{ {0x0D21,0x0D4D,0x0D21,0x0},					{0xF4B0,0x0} },
													
	// dha											
	{ {0x0D22,0x0},									{0x0D22,0x0} },
													
	// na											
	{ {0x0D23,0x0},									{0x0D23,0x0} },
	//{ {0x0D23,0x0D4D,0x0},									{0x0D23,0x0D4D,0x0} },
//	{ {0x0D23,0x0D4D,0x0},							{0xF48F,0x0} },
	{ {0x0D23,0x0D4D,0x0D1F,0x0},					{0xF490,0x0} },
	{ {0x0D23,0x0D4D,0x0D23,0x0},					{0xF491,0x0} },
	{ {0x0D23,0x0D4D,0x0D2E,0x0},					{0xF4BA,0x0} },
	{ {0x0D23,0x0D4D,0x0D21,0x0},					{0xF4C1,0x0} },
													
	// Ta											
	{ {0x0D24,0x0},									{0x0D24,0x0} },
	//{ {0x0D24,0x0D4D,0x0},									{0x0D24,0x0D4D,0x0} },
	{ {0x0D24,0x0D4D,0x0D24,0x0},					{0xF492,0x0} },
	{ {0x0D24,0x0D4D,0x0D25,0x0},					{0xF493,0x0} },
	{ {0x0D24,0x0D4D,0x0D38,0x0},					{0xF4B8,0x0} },
	{ {0x0D24,0x0D4D,0x0D2D,0x0},					{0xF4BE,0x0} },
	{ {0x0D24,0x0D4D,0x0D2E,0x0},					{0xF4C2,0x0} },
													
	//Tha											
	{ {0x0D25,0x0},									{0x0D25,0x0} },
													
	// Da											
	{ {0x0D26,0x0},									{0x0D26,0x0} },
	//{ {0x0D26,0x0D4D,0x0},									{0x0D26,0x0D4D,0x0} },
	{ {0x0D26,0x0D4D,0x0D26,0x0},					{0xF494,0x0} },
	{ {0x0D26,0x0D4D,0x0D27,0x0},					{0xF495,0x0} },
													
	// Dha											
	{ {0x0D27,0x0},									{0x0D27,0x0} },
													
	// Na											
	{ {0x0D28,0x0},									{0x0D28,0x0} },
	//{ {0x0D28,0x0D4D,0x0},									{0x0D28,0x0D4D,0x0} },
//	{ {0x0D28,0x0D4D,0x0},							{0xF496,0x0} },
	{ {0x0D28,0x0D4D,0x0D24,0x0},					{0xF497,0x0} },
	{ {0x0D28,0x0D4D,0x0D26,0x0},					{0xF498,0x0} },
	{ {0x0D28,0x0D4D,0x0D28,0x0},					{0xF499,0x0} },
	{ {0x0D28,0x0D4D,0x0D2E,0x0},					{0xF49A,0x0} },
	{ {0x0D28,0x0D4D,0x0D2A,0x0},					{0xF49F,0x0} },
	{ {0x0D28,0x0D4D,0x0D27,0x0},					{0xF4B7,0x0} },
	{ {0x0D28,0x0D4D,0x0D25,0x0},					{0xF4BC,0x0} },
	{ {0x0D28,0x0D4D,0x0D1A,0x0},					{0xF48C,0x0} },
	{ {0x0D28,0x0D41,0x0},							{0xF4C8,0x0} },
													
	// Pa											
	{ {0x0D2A,0x0},									{0x0D2A,0x0} },
	//{ {0x0D2A,0x0D4D,0x0},									{0x0D2A,0x0D4D,0x0} },
	{ {0x0D2A,0x0D4D,0x0D2A,0x0},					{0xF49B,0x0} },
	{ {0x0D2A,0x0D4D,0x0D33,0x0},					{0xF49C,0x0} },
													
	// Pha											
	{ {0x0D2B,0x0},									{0x0D2B,0x0} },
													
	// ba											
	{ {0x0D2C,0x0},									{0x0D2C,0x0} },
	//{ {0x0D2C,0x0D4D,0x0},									{0x0D2C,0x0D4D,0x0} },
	{ {0x0D2C,0x0D4D,0x0D2C,0x0},					{0xF49D,0x0} },
	{ {0x0D2C,0x0D4D,0x0D33,0x0},					{0xF49E,0x0} },
	{ {0x0D2C,0x0D4D,0x0D27,0x0},					{0xF4B2,0x0} },
	{ {0x0D2C,0x0D4D,0x0D26,0x0},					{0xF4B3,0x0} },
	{ {0x0D2C,0x0D4D,0x0D2A,0x0},					{0xF49F,0x0} },
													
	// bha											
	{ {0x0D2D,0x0},									{0x0D2D,0x0} },
													
	// Ma											
	{ {0x0D2E,0x0},									{0x0D2E,0x0} },
	//{ {0x0D2E,0x0D4D,0x0},									{0x0D2E,0x0D4D,0x0} },
	{ {0x0D2E,0x0D4D,0x0D2E,0x0},					{0xF4A0,0x0} },
	{ {0x0D2E,0x0D4D,0x0D33,0x0},					{0xF4A1,0x0} },
													
	// ya											
	{ {0x0D2F,0x0},									{0x0D2F,0x0} },
	//{ {0x0D2F,0x0D4D,0x0},									{0x0D2F,0x0D4D,0x0} },
	{ {0x0D2F,0x0D4D,0x0D2F,0x0},					{0xF4A2,0x0} },
	{ {0x0D2F,0x0D4D,0x0D2F,0x0D4D,0x0},			{0xF4A2,0x0D4D,0x0} },
													
	//ra												
	{ {0x0D30,0x0},									{0x0D30,0x0} },
//	{ {0x0D30,0x0D4D,0x0},							{0xF4A3,0x0} },
//  {0x0D30,0x0D4D,0x0},							{0x0D30,0x0D4D,0x0} },
	{ {0x0D30,0x0D4D,0x0D30,0x0},					{0x0D30,0x0D4D,0x0D30,0x0} },
	{ {0x0D30,0x0D4D,0x0D30,0x0D4D,0x0D30,0x0},			{0x0D30,0x0D4D,0x0D30,0x0D4D,0x0D30,0x0} },
													
	//Ra Nukta										
	{ {0x0D31,0x0},									{0x0D31,0x0} },
	{ {0x0D31,0x0D4D,0x0D31,0x0},							{0xF4C7,0x0} },
													
	// la											
	{ {0x0D32,0x0},									{0x0D32,0x0} },
	//{ {0x0D32,0x0D4D,0x0},									{0x0D32,0x0D4D,0x0} },
//	{ {0x0D32,0x0D4D,0x0},							{0xF4A5,0x0} },
	{ {0x0D32,0x0D4D,0x0D32,0x0},					{0xF4A6,0x0} },
													
	// La											
	{ {0x0D33,0x0},									{0x0D33,0x0} },
	//{ {0x0D33,0x0D4D,0x0},									{0x0D33,0x0D4D,0x0} },
//	{ {0x0D33,0x0D4D,0x0},							{0xF4A7,0x0} },
	{ {0x0D33,0x0D4D,0x0D33,0x0},					{0xF4A8,0x0} },
													
	// LLa											
	{ {0x0D34,0x0},									{0x0D34,0x0} },
													
	// Va											
	{ {0x0D35,0x0},									{0x0D35,0x0} },
	//{ {0x0D35,0x0D4D,0x0},						{0x0D35,0x0D4D,0x0} },
	{ {0x0D35,0x0D4D,0x0D35,0x0},					{0xF4A9,0x0} },
	{ {0x0D35,0x0D4D,0x0D35,0x0D4D,0x0},			{0xF4A9,0x0D4D,0x0} },
													
	//sha											
	{ {0x0D36,0x0},									{0x0D36,0x0} },
	//{ {0x0D36,0x0D4D,0x0},									{0x0D36,0x0D4D,0x0} },
	{ {0x0D36,0x0D4D,0x0D32,0x0},					{0xF4AA,0x0} },
	{ {0x0D36,0x0D4D,0x0D33,0x0},					{0xF4AA,0x0} },
	{ {0x0D36,0x0D4D,0x0D36,0x0},					{0xF4AB,0x0} },
	{ {0x0D36,0x0D4D,0x0D1A,0x0},					{0xF4C0,0x0} },

													
	//Sha											
	{ {0x0D37,0x0},									{0x0D37,0x0} },
	//{ {0x0D37,0x0D4D,0x0},									{0x0D37,0x0D4D,0x0} },
	{ {0x0D37,0x0D4D,0x0D1F,0x0},					{0xF4C6,0x0} },
													
	// Sa											
	{ {0x0D38,0x0},									{0x0D38,0x0} },
	//{ {0x0D38,0x0D4D,0x0},									{0x0D38,0x0D4D,0x0} },
	{ {0x0D38,0x0D4D,0x0D33,0x0},					{0xF4AC,0x0} },
	{ {0x0D38,0x0D4D,0x0D38,0x0},					{0xF4AD,0x0} },
	{ {0x0D38,0x0D4D,0x0D31,0x0D4D,0x0D31,0x0},		{0xF4AF,0x0} },
	//{ {0x0D38,0x0D4D,0x0D2E,0x0},					{0xF4BA,0x0} },
	{ {0x0D38,0x0D4D,0x0D25,0x0},					{0xF4BB,0x0} },


	// Ha
	{ {0x0D39,0x0},									{0x0D39,0x0} },
	//{ {0x0D39,0x0D4D,0x0},									{0x0D39,0x0D4D,0x0} },
	{ {0x0D39,0x0D4D,0x0D33,0x0},					{0xF4AE,0x0} },
	{ {0x0D39,0x0D4D,0x0D2E,0x0},					{0xF4B5,0x0} },
	{ {0x0D39,0x0D4D,0x0D28,0x0},					{0xF4B6,0x0} },
													
													
	// matras										
	{ {0x0D3E,0x0},									{0x0D3E,0x0} },
	{ {0x0D3F,0x0},									{0x0D3F,0x0} },
	{ {0x0D40,0x0},									{0x0D40,0x0} },
	{ {0x0D41,0x0},									{0x0D41,0x0} },
	{ {0x0D42,0x0},									{0x0D42,0x0} },
	{ {0x0D43,0x0},									{0x0D43,0x0} },
	{ {0x0D46,0x0},									{0x0D46,0x0} },
	{ {0x0D47,0x0},									{0x0D47,0x0} },
	{ {0x0D48,0x0},									{0x0D48,0x0} },
	{ {0x0D4A,0x0},									{0x0D4A,0x0} },
	{ {0x0D4B,0x0},									{0x0D4B,0x0} },
	{ {0x0D4C,0x0},									{0x0D4C,0x0} },
													
	// Halant										
	{ {0x0D4D,0x0},									{0x0D4D,0x0} },
/*	{ {0x0D4D,0x0D2F,0x0},							{0xF481,0x0} },
	{ {0x0D4D,0x0D35,0x0},							{0xF482,0x0} },
	{ {0x0D4D,0x0D30,0x0},							{0xF483,0x0} },
*/													
	{ {0x0D66,0x0},									{0x0D66,0x0} },
	{ {0x0D67,0x0},									{0x0D67,0x0} },
	{ {0x0D68,0x0},									{0x0D68,0x0} },
	{ {0x0D6A,0x0},									{0x0D6A,0x0} },
	{ {0x0D6B,0x0},									{0x0D6B,0x0} },
	{ {0x0D6C,0x0},									{0x0D6C,0x0} },
	{ {0x0D6D,0x0},									{0x0D6D,0x0} },
	{ {0x0D6E,0x0},									{0x0D6E,0x0} },
	{ {0x0D6F,0x0},									{0x0D6F,0x0} },

};


static const PnGlyphEntry PnGlyphTbl [MAP_PN_SIZE] = 
{
	
	// Vowel modifiers
	{ {0x0A02,0x0},									{0x0A02,0x0} },
	
	// Vowels	15
	{ {0x0A05,0x0},									{0x0A05,0x0} },
	{ {0x0A05,0x0A02,0x0},							{0x0A05,0x0A70,0x0} },
	{ {0x0A06,0x0},									{0x0A06,0x0} },
	{ {0x0A06,0x0A02,0x0},							{0x0A05,0xF327,0x0} },
	{ {0x0A07,0x0},									{0x0A07,0x0} },
	{ {0x0A07,0x0A02,0x0},							{0x0A07,0x0A70,0x0} },
	{ {0x0A08,0x0},									{0x0A08,0x0} },
	{ {0x0A08,0x0A02,0x0},							{0x0A72,0xF328,0x0} },
	{ {0x0A09,0x0},									{0x0A09,0x0} },
	{ {0x0A0A,0x0},									{0x0A0A,0x0} },
	{ {0x0A0F,0x0},									{0x0A0F,0x0} },
	{ {0x0A10,0x0},									{0x0A10,0x0} },
	{ {0x0A13,0x0},									{0x0A13,0x0} },
	{ {0x0A14,0x0},									{0x0A14,0x0} },

	// Consonants	44
	
	// Ka
	{ {0x0A15,0x0},									{0x0A15,0x0} },
	{ {0x0A15,0x0A4D,0x0},							{0x0A15,0x0} },
	{ {0x0A15,0x0A02,0x0},							{0x0A15,0x0A70,0x0} },
	{ {0x0A15,0x0A3C,0x0},							{0xF33C,0x0} },
	{ {0x0A15,0x0A4D,0x0A30,0x0},					{0xF321,0x0} },
	{ {0x0A15,0x0A4D,0x0A15,0x0},					{0x0A71,0x0A15,0x0} },
	{ {0x0A15,0x0A3C,0x0A4D,0x0A15,0x0A3C,0x0},		{0x0A71,0xF33C,0x0} },

	// Kha
	{ {0x0A16,0x0},									{0x0A16,0x0} },
	{ {0x0A16,0x0A3C,0x0},							{0x0A59,0x0} },
	{ {0x0A16,0x0A4D,0x0A16,0x0},					{0x0A71,0x0A16,0x0} },
	{ {0x0A16,0x0A3C,0x0A4D,0x0A16,0x0A3C,0x0},		{0x0A71,0x0A59,0x0} },


	// ga
	{ {0x0A17,0x0},									{0x0A17,0x0} },	
	{ {0x0A17,0x0A3C,0x0},							{0x0A5A,0x0} },
	{ {0x0A17,0x0A4D,0x0A17,0x0},					{0x0A71,0x0A17,0x0} },
	{ {0x0A17,0x0A3C,0x0A4D,0x0A17,0x0A3C,0x0},		{0x0A71,0x0A5A,0x0} },

	// gha
	{ {0x0A18,0x0},									{0x0A18,0x0} },
	{ {0x0A18,0x0A4D,0x0A18,0x0},					{0x0A71,0x0A18,0x0} },

	// nga
	{ {0x0A19,0x0},									{0x0A19,0x0} },
	{ {0x0A19,0x0A4D,0x0A19,0x0},					{0x0A71,0x0A19,0x0} },

	// cha
	{ {0x0A1A,0x0},									{0x0A1A,0x0} },
	{ {0x0A1A,0x0A4D,0x0A1A,0x0},					{0x0A71,0x0A1A,0x0} },

	// Cha
	{ {0x0A1B,0x0},									{0x0A1B,0x0} },
	{ {0x0A1B,0x0A4D,0x0A1B,0x0},					{0x0A71,0x0A1B,0x0} },

	// ja
	{ {0x0A1C,0x0},									{0x0A1C,0x0} },
	{ {0x0A1C,0x0A3C,0x0},							{0x0A5B,0x0} },
	{ {0x0A1C,0x0A4D,0x0A1C,0x0},					{0x0A71,0x0A1C,0x0} },
	{ {0x0A1C,0x0A3C,0x0A4D,0x0A1C,0x0A3C,0x0},		{0x0A71,0x0A5B,0x0} },

	// jha
	{ {0x0A1D,0x0},									{0x0A1D,0x0} },
	{ {0x0A1D,0x0A4D,0x0A1D,0x0},					{0x0A71,0x0A1D,0x0} },

	// jna
	{ {0x0A1E,0x0},									{0x0A1E,0x0} },
	{ {0x0A1E,0x0A4D,0x0A1E,0x0},					{0x0A71,0x0A1E,0x0} },

	// ta
	{ {0x0A1F,0x0},									{0x0A1F,0x0} },
	{ {0x0A1F,0x0A4D,0x0A1F,0x0},					{0x0A71,0x0A1F,0x0} },

	// tha
	{ {0x0A20,0x0},									{0x0A20,0x0} },
	{ {0x0A20,0x0A4D,0x0A20,0x0},					{0x0A71,0x0A20,0x0} },

	{ {0x0A20,0x0A47,0x0},							{0x0A20,0xF32D,0x0} },
	{ {0x0A20,0x0A48,0x0},							{0x0A20,0xF330,0x0} },
	{ {0x0A20,0x0A4B,0x0},							{0x0A20,0xF333,0x0} },
	{ {0x0A20,0x0A41,0x0A02,0x0},					{0x0A20,0x0A41,0xF32C,0x0} },
	{ {0x0A20,0x0A42,0x0A02,0x0},					{0x0A20,0x0A42,0xF32C,0x0} },
	{ {0x0A20,0x0A47,0x0A02,0x0},					{0x0A20,0xF32F,0x0} },
	{ {0x0A20,0x0A48,0x0A02,0x0},					{0x0A20,0xF332,0x0} },
	{ {0x0A20,0x0A4B,0x0A02,0x0},					{0x0A20,0xF335,0x0} },
	{ {0x0A20,0x0A4C,0x0A02,0x0},					{0x0A20,0xF33E,0x0} },

	// da
	{ {0x0A21,0x0},									{0x0A21,0x0} },
	{ {0x0A21,0x0A3C,0x0},							{0x0A5C,0x0} },
	{ {0x0A21,0x0A4D,0x0A21,0x0},					{0x0A71,0x0A21,0x0} },
	{ {0x0A21,0x0A3C,0x0A4D,0x0A21,0x0A3C,0x0},		{0x0A71,0x0A5C,0x0} },
	{ {0x0A21,0x0A4D,0x0A30,0x0},					{0xF322,0x0} },

	// dha
	{ {0x0A22,0x0},									{0x0A22,0x0} },
	{ {0x0A22,0x0A3C,0x0},							{0xF33D,0x0} },
	{ {0x0A22,0x0A4D,0x0A22,0x0},					{0x0A71,0x0A22,0x0} },
	{ {0x0A22,0x0A3C,0x0A4D,0x0A22,0x0A3C,0x0},		{0x0A71,0xF33D,0x0} },
	{ {0x0A22,0x0A4D,0x0A30,0x0},					{0xF323,0x0} },

	// na
	{ {0x0A23,0x0},									{0x0A23,0x0} },
	{ {0x0A23,0x0A4D,0x0A23,0x0},					{0x0A71,0x0A23,0x0} },

	// Ta
	{ {0x0A24,0x0},									{0x0A24,0x0} },
	{ {0x0A24,0x0A4D,0x0A24,0x0},					{0x0A71,0x0A24,0x0} },

	// Tha
	{ {0x0A25,0x0},									{0x0A25,0x0} },
	{ {0x0A25,0x0A4D,0x0A25,0x0},					{0x0A71,0x0A25,0x0} },


	// Da
	{ {0x0A26,0x0},									{0x0A26,0x0} },
	{ {0x0A26,0x0A4D,0x0A26,0x0},					{0x0A71,0x0A26,0x0} },

	// Dha
	{ {0x0A27,0x0},									{0x0A27,0x0} },
	{ {0x0A27,0x0A4D,0x0A27,0x0},					{0x0A71,0x0A27,0x0} },

	// Na
	{ {0x0A28,0x0},									{0x0A28,0x0} },
	{ {0x0A28,0x0A4D,0x0A28,0x0},					{0x0A71,0x0A28,0x0} },

	{ {0x0A28,0x0A47,0x0},							{0x0A28,0xF32D,0x0} },
	{ {0x0A28,0x0A48,0x0},							{0x0A28,0xF330,0x0} },
	{ {0x0A28,0x0A4B,0x0},							{0x0A28,0xF333,0x0} },
	{ {0x0A28,0x0A41,0x0A02,0x0},					{0x0A28,0x0A41,0xF32C,0x0} },
	{ {0x0A28,0x0A42,0x0A02,0x0},					{0x0A28,0x0A42,0xF32C,0x0} },
	{ {0x0A28,0x0A47,0x0A02,0x0},					{0x0A28,0xF32F,0x0} },
	{ {0x0A28,0x0A48,0x0A02,0x0},					{0x0A28,0xF332,0x0} },
	{ {0x0A28,0x0A4B,0x0A02,0x0},					{0x0A28,0xF335,0x0} },
	{ {0x0A28,0x0A4C,0x0A02,0x0},					{0x0A28,0xF33E,0x0} },

	// Pa
	{ {0x0A2A,0x0},									{0x0A2A,0x0} },
	{ {0x0A2A,0x0A4D,0x0A2A,0x0},					{0x0A71,0x0A2A,0x0} },

	// Pha
	{ {0x0A2B,0x0},									{0x0A2B,0x0} },
	{ {0x0A2B,0x0A3C,0x0},							{0x0A5D,0x0} },
	{ {0x0A2B,0x0A4D,0x0A2B,0x0},					{0x0A71,0x0A2B,0x0} },
	{ {0x0A2B,0x0A3C,0x0A4D,0x0A2B,0x0A3C,0x0},		{0x0A71,0x0A5D,0x0} },

	// Ba
	{ {0x0A2C,0x0},									{0x0A2C,0x0} },
	{ {0x0A2C,0x0A4D,0x0A2C,0x0},					{0x0A71,0x0A2C,0x0} },

	// Bha
	{ {0x0A2D,0x0},									{0x0A2D,0x0} },
	{ {0x0A2D,0x0A4D,0x0A30,0x0},					{0xF324,0x0} },
	{ {0x0A2D,0x0A4D,0x0A2D,0x0},					{0x0A71,0x0A2D,0x0} },

	// Ma
	{ {0x0A2E,0x0},									{0x0A2E,0x0} },
	{ {0x0A2E,0x0A4D,0x0A2E,0x0},					{0x0A71,0x0A2E,0x0} },

	// Ya
	{ {0x0A2F,0x0},									{0x0A2F,0x0} },
	{ {0x0A2F,0x0A4D,0x0A2F,0x0},					{0x0A71,0x0A2F,0x0} },

	// Ra
	{ {0x0A30,0x0},									{0x0A30,0x0} },
	{ {0x0A30,0x0A4D,0x0A30,0x0},					{0x0A71,0x0A30,0x0} },
	// la
	{ {0x0A32,0x0},									{0x0A32,0x0} },
	{ {0x0A32,0x0A4D,0x0A32,0x0},					{0x0A71,0x0A32,0x0} },

	// La
	{ {0x0A33,0x0},									{0x0A33,0x0} },
	{ {0x0A33,0x0A4D,0x0A33,0x0},					{0x0A71,0x0A33,0x0} },

	// Va
	{ {0x0A35,0x0},									{0x0A35,0x0} },
	{ {0x0A35,0x0A4D,0x0A35,0x0},					{0x0A71,0x0A35,0x0} },

	// Sha
	{ {0x0A36,0x0},									{0x0A36,0x0} },
	{ {0x0A36,0x0A4D,0x0A36,0x0},					{0x0A71,0x0A36,0x0} },

	// sa
	{ {0x0A38,0x0},									{0x0A38,0x0} },
	{ {0x0A38,0x0A4D,0x0A38,0x0},					{0x0A71,0x0A38,0x0} },

	// Ha
	{ {0x0A39,0x0},									{0x0A39,0x0} },
	{ {0x0A39,0x0A4D,0x0A39,0x0},					{0x0A71,0x0A39,0x0} },

	// Nukta
	{ {0x0A3C,0x0},									{0x0A3C,0x0} },


	// Matra

	{ {0x0A3E,0x0},									{0x0A3E,0x0} },
	{ {0x0A3E,0x0A02,0x0},							{0xF327,0x0} },

	{ {0x0A3F,0x0},									{0x0A3F,0x0} },
	{ {0x0A40,0x0},									{0x0A40,0x0} },
	{ {0x0A40,0x0A02,0x0},							{0xF328,0x0} },

	{ {0x0A41,0x0},									{0x0A41,0x0} },
	{ {0x0A41,0x0A02,0x0},							{0x0A41,0x0A70,0x0} },
	{ {0x0A42,0x0},									{0x0A42,0x0} },
	{ {0x0A42,0x0A02,0x0},							{0x0A42,0x0A70,0x0} },

	{ {0x0A47,0x0},									{0x0A47,0x0} },
	{ {0x0A47,0x0A02,0x0},							{0xF32E,0x0} },

	{ {0x0A48,0x0},									{0x0A48,0x0} },
	{ {0x0A48,0x0A02,0x0},							{0xF331,0x0} },
	

	{ {0x0A4B,0x0},									{0x0A4B,0x0} },
	{ {0x0A4B,0x0A02,0x0},							{0xF334,0x0} },

	{ {0x0A4C,0x0},									{0x0A4C,0x0} },
	{ {0x0A4C,0x0A02,0x0},							{0xF33B,0x0} },

	// Halant
	{ {0x0A4D,0x0},									{0x0} },
	{ {0x0A4D,0x0A30,0x0},							{0xF337,0x0} },
	{ {0x0A4D,0x0A30,0x0A41,0x0},					{0xF337,0xF329,0x0} },
	{ {0x0A4D,0x0A30,0x0A42,0x0},					{0xF337,0xF32A,0x0} },

	{ {0x0A4D,0x0A39,0x0},							{0xF336,0x0} },
	{ {0x0A4D,0x0A39,0x0A41,0x0},					{0xF336,0xF329,0x0} },
	{ {0x0A4D,0x0A39,0x0A42,0x0},					{0xF336,0xF32A,0x0} },

	{ {0x0A4D,0x0A35,0x0},							{0xF338,0x0} },
	{ {0x0A4D,0x0A35,0x0A41,0x0},					{0xF338,0xF329,0x0} },
	{ {0x0A4D,0x0A35,0x0A42,0x0},					{0xF338,0xF32A,0x0} },

	{ {0x0A4D,0x0A2F,0x0},							{0xF325,0x0} },
	
	// Nukta consonants 5
	{ {0x0A59,0x0},									{0x0A59,0x0} },
	{ {0x0A5A,0x0},									{0x0A5A,0x0} },
	{ {0x0A5B,0x0},									{0x0A5B,0x0} },
	{ {0x0A5C,0x0},									{0x0A5C,0x0} },
	{ {0x0A5E,0x0},									{0x0A5E,0x0} },
	
	// Digits 10
	{ {0x0A66,0x0},									{0x0A66,0x0} },
	{ {0x0A67,0x0},									{0x0A67,0x0} },
	{ {0x0A68,0x0},									{0x0A68,0x0} },
	{ {0x0A69,0x0},									{0x0A69,0x0} },
	{ {0x0A6A,0x0},									{0x0A6A,0x0} },
	{ {0x0A6B,0x0},									{0x0A6B,0x0} },
	{ {0x0A6C,0x0},									{0x0A6C,0x0} },
	{ {0x0A6D,0x0},									{0x0A6D,0x0} },
	{ {0x0A6E,0x0},									{0x0A6E,0x0} },
	{ {0x0A6F,0x0},									{0x0A6F,0x0} },

	// misc 5
	{ {0x0A70,0x0},									{0x0A70,0x0} },
	{ {0x0A71,0x0},									{0x0A71,0x0} },
	{ {0x0A72,0x0},									{0x0A72,0x0} },
	{ {0x0A73,0x0},									{0x0A73,0x0} },
	{ {0x0A74,0x0},									{0x0A74,0x0} },

};


typedef enum {
	FileCode,	/* UTF-8 */
	ProcessCode	/* wchar_t (UCS4) */
} data_type_t;

typedef struct	_Submap
{
  int	is_leaf;
  union {
    void		(*func)(LayoutObj);
    void		(**funcs)(LayoutObj);
  } d;
} Submap;

typedef	enum {
  ISOLATED,
  INITIAL,
  MIDDLE,
  FINAL
} ArabicShapeState;

typedef	enum {
  Normal,
  LamAlif
} ClusterType;

typedef struct _TransformFROMInfoRec {         

	/* CSI */
	void			*MbUnitLen;
	void			*MbToWcUnit;
	int			maxbytes;

	Submap			maps[256];
	MappingTable		TableMap;
	void			(*func_ucs4)(LayoutObj);

	/* Character Class Tables */
	const thai_cls_t	*ThaiChrClsTbl;
	const hebrew_cls_t	*HebrewChrClsTbl;
	const arabic_cls_t	*ArabicChrClsTbl;
	
	const deva_cls_t	*DevaChrClsTbl;
	const tamil_cls_t	*TamilChrClsTbl;
	const kann_cls_t	*KannChrClsTbl;
	const guja_cls_t	*GujaChrClsTbl;
	const beng_cls_t	*BengChrClsTbl;
	const tlg_cls_t		*TlgChrClsTbl;
	const ml_cls_t		*MlChrClsTbl;
	const pn_cls_t		*PnChrClsTbl;

	/* Character Type Tables */
	const int		*ThaiChrTypeTbl;
	const int		*HebrewChrTypeTbl;
	const int		*ArabicChrTypeTbl;
	
	const int		*DevaChrTypeTbl;
	const int		*TamilChrTypeTbl;
	const int		*KannChrTypeTbl;
	const int		*GujaChrTypeTbl;
	const int		*BengChrTypeTbl;
	const int		*TlgChrTypeTbl;
	const int		*MlChrTypeTbl;
	const int		*PnChrTypeTbl;

	/* Character Compose Tables */
	const int		**ThaiComposeTbl;
	const int		**HebrewComposeTbl;
	const int		**ArabicComposeTbl;
	const int		**DevaComposeTbl;
	const int		**TamilComposeTbl;
	const int		**KannComposeTbl;
	const int		**GujaComposeTbl;
	const int		**BengComposeTbl;
	const int		**TlgComposeTbl;
	const int		**TlgVattuComposeTbl;
	const int		**MlComposeTbl;
	const int		**PnComposeTbl;
	
	/* Glyph Tables */
	const ThaiGlyphTable	*ThaiGlyphTbl;
	const ArabicGlyphTable	*ArabicGlyphTbl;
	const HebrewGlyphTable	*HebrewGlyphTbl;
/*
	const DevaGlyphTable	*DevaGlyphTbl;
*/

} TransformFROMInfoRec;

typedef	struct	_ShpChrsetGrpRec {
	char			Name[BUFSIZ];
	int			ShpNumBytes;
	TransformFROMInfoRec	TransformFROM;
} ShpChrsetGrpRec;

typedef	struct	_CurTransformRec {
	int			CurShpGrpIdx;
	data_type_t		LogicalStrmType;

	char			mbInpBuf_cache[UMLE_CACHE_SIZE];
	char			*mbInpBuf;
	int			inpmb_cz;

	wchar_t			wcInpBuf_cache[UMLE_CACHE_SIZE];
	wchar_t			*wcInpBuf;
	int			inpwc_cz;

	int			width;
	int			i;
	int			o_idx;
	int			tmp_o_idx;
	int			first_o_idx;
	int			n_chrs_in_cluster;
	ArabicShapeState	arabic_state;
	ClusterType		arabic_current_cluster_type;
	ClusterType		hebrew_current_cluster_type;
	int			j;
	int			n_c;
	int			end_hdrt;

	int			num_deva_cons;
	int			dev_j;
	StateTypeID		deva_cluster_type_state;

	int			num_tamil_cons;
	int			tamil_j;
	StateTypeID		tamil_cluster_type_state;

	int			num_kann_cons;
	int			kan_j;
	StateTypeID		kann_cluster_type_state;

	int			num_guja_cons;
	int			guj_j;
	StateTypeID		guja_cluster_type_state;

	int			num_beng_cons;
	int			bng_j;
	StateTypeID		beng_cluster_type_state;

	int			num_tlg_cons;
	int			tlg_j;
	StateTypeID	tlg_cluster_type_state;
	int			tlg_halant_count;		// Only for Telugu
	int			tlg_vattu_count;		// Only for Telugu
	int			tlg_vattu_previous;		// Only for Telugu
	int			tlg_vattu_index;		// Only for Telugu
	int			tlg_vattu_list[3];
	wchar_t			wcTlgInpBuf_cache[UMLE_CACHE_SIZE];
	wchar_t			*wcTlgInpBuf;

	int			num_ml_cons;
	int			ml_j;
	StateTypeID	ml_cluster_type_state;

	int			num_pn_cons;
	int			pn_j;
	StateTypeID	pn_cluster_type_state;
	int			pn_halant_count;		// Only for Punjabi

	int			prop_cz;
	int			i2o_cz;
	int			o2i_cz;
	int			num_unit;
	int			outsize;
	int			out_cz;
	int			unit_cz;
	int			*embeddings;
	int			embeddings_cache[UMLE_CACHE_SIZE];
	bidi_type_t		*dirs;
	bidi_type_t		dirs_cache[UMLE_CACHE_SIZE];
	int			*v2l;
	int			v2l_cache[UMLE_CACHE_SIZE];
	int			*l2v;
	int			l2v_cache[UMLE_CACHE_SIZE];
	int			*visual_blocks;
	int			visual_blocks_cache[UMLE_CACHE_SIZE];
	int			*unit_to_mb_idx;
	int			unit_to_mb_idx_cache[UMLE_CACHE_SIZE];
	char			*OutBuf;
	char			OutBuf_cache[UMLE_CACHE_SIZE];
	size_t			*OutToInp;
	size_t			OutToInp_cache[UMLE_CACHE_SIZE];
	size_t			*TmpOutToInp;
	size_t			TmpOutToInp_cache[UMLE_CACHE_SIZE];
	size_t			*InpToOut;
	size_t			InpToOut_cache[UMLE_CACHE_SIZE];
	size_t			*TmpInpToOut;
	size_t			TmpInpToOut_cache[UMLE_CACHE_SIZE];
	unsigned char		*Property;
	unsigned char		Property_cache[UMLE_CACHE_SIZE];

} CurTransformRec, *CurTransform;

typedef	struct	_UMLERec	*UMLEInfo;

typedef	struct	_UMLERec {
	ShpChrsetGrpRec		ShpChrsetGrp[MAX_SHPDCHRSETS];
	CurTransform		CurInfo;
	void			*private_data;
} UMLERec;

/* List of all real glyph tables */
/* size should be depend on ShapeCharsetSize */

static 	LayoutObject		_LSUMLECreate();
static 	int			_LSUMLEDestroy();
static 	int			_LSUMLEGetValues();
static 	int			_LSUMLESetValues();
static	int			_UMLETransform();
static 	int			_UMLETransform_mb();
static 	int			_UMLETransform_wc();

static	void			func_thai();
static	void			func_hebrew();
static	void			func_arabic();
static	void			func_devanagari();
static	void			func_tamil();
static	void			func_kannada();
static	void			func_gujarati();
static	void			func_bengali();
static	void			func_telugu();
static	void			func_malayalam();
static	void			func_punjabi();

static	void			func_ucs4();

/*
 * Frame Method Declaration
 *
 * int __MbUnitLen(char *)
 *	: return "Number of Bytes of Unit which is char"
 *	: return 0 if input pointer is NULL or 1st data input is 0
 *	: return -1 if input sequence is illegal
 *                     
 * int __MbToWcUnit(wchar_t *, char *)
 *	: return "Number of Bytes of MbUnit
 *	: return 0 if either input pointer is NULL or 1st data input is 0
 *	: return -1 if input sequence is illegal
 *
 */

static	int			__UTF8(char *);
static	int			__UTF8ToUCS4(wchar_t *, char *);

/* Methods for Unicode locale support */
static LayoutMethodsRec _UMLELayoutMethods = {
	_LSUMLECreate,
	_LSUMLEDestroy,
	_LSUMLEGetValues,
	_LSUMLESetValues,
	_UMLETransform_mb,
	_UMLETransform_wc,
};

static int
__UTF8(char *utf8)
{
    int	c;
    int remaining_bytes;
    int byte_count;

    if (! utf8)
	return (0);
    c = (UCHAR)*utf8++;
    if (UTF_8_ONE(c))
	return ((c) ? 1 : 0);

    remaining_bytes = remaining_bytes_tbl[c];

    if (remaining_bytes != 0) {
	byte_count = remaining_bytes + 1;

	for (; remaining_bytes > 0; remaining_bytes--, utf8++)
	    if ((UCHAR)*utf8 < 0x80 || (UCHAR)*utf8 > 0xbf)
		goto _MBTOWC_ILLEGAL_UTF_8;

	return (byte_count);
    }

_MBTOWC_ILLEGAL_UTF_8:
    return(-1);
}

static	int
__UTF8ToUCS4(ucs4, utf8)
    wchar_t     *ucs4;
    char        *utf8;
{
    int c;
    int remaining_bytes;
    int byte_count;

    if (! utf8 || ! ucs4)
        return (0);

    c = (UCHAR)*utf8++;

    if (UTF_8_ONE(c))
        return((*ucs4 = c) ? 1 : 0);

    remaining_bytes = remaining_bytes_tbl[c];

    if (remaining_bytes != 0) {
        byte_count = remaining_bytes + 1;

        *ucs4 = c & masks_tbl[remaining_bytes];

        for (; remaining_bytes > 0; remaining_bytes--) {
             c = (UCHAR)*utf8++;
             if (c < 0x80 || c > 0xbf)
                 goto _MBTOWC_ILLEGAL_UTF_8;
             *ucs4 = (*ucs4 << BIT_SHIFT) | (c & BIT_MASK);
        }

        return(byte_count);
    }

_MBTOWC_ILLEGAL_UTF_8:
    return(-1);
}

static char *
UMLEMalloc(size)
    unsigned size;
{
    char *ptr;
    if ((ptr = UMLEmalloc(size)) == NULL)
        fprintf(stderr, "UMLEMalloc");

    return(ptr);
}

static char *
UMLERealloc(ptr, size)
    char     *ptr;
    unsigned size;
{
   if (ptr == NULL) return(UMLEMalloc(size));
   else if ((ptr = UMLErealloc(ptr, size)) == NULL)
        fprintf(stderr, "UMLERealloc");

   return(ptr);
}

static void UMLEFree(ptr)
    char *ptr;
{
   if (ptr != NULL) free(ptr);
}

static void *
CacheRealloc(old_size, new_size, ptr, cache)
    unsigned old_size;
    unsigned new_size;
    void *ptr;
    void *cache;
{
    char *new_ptr;

    if (ptr == NULL) return (UMLEMalloc(new_size));
    if (old_size >= new_size) return ptr;
    new_ptr = UMLEMalloc(new_size);
    if (new_ptr == NULL) {
	fprintf(stderr, "\nCacheRealloc");
	return ptr;
    }
    memcpy(new_ptr, (char *)ptr, old_size);
    if (ptr != cache)
	UMLEFree(ptr);
    return ((void *)new_ptr);
}

static	void
map_add_routine (obj, from, to, func)
  LayoutObj	obj;
  int		from;
  int		to;
  void		(*func)(LayoutObj);
{
  int i, submap, start, end;

  for (submap = from/256; submap <= to/256; submap++)
    {
      start = (submap == from/256 ? from%256 : 0);
      end = (submap == to/256 ? to%256 : 255);
      if (MAPPS(obj, submap).is_leaf &&
	  start == 0 && end == 255)
	{
	  MAPPS(obj, submap).is_leaf = 0;
	  MAPPS(obj, submap).d.func = func;
	}
      else
        {
          if (MAPPS(obj, submap).is_leaf)
            {
              MAPPS(obj, submap).is_leaf = 0;
	      MAPPS(obj, submap).d.funcs = malloc(256*sizeof(void (**)));
	      for (i=0; i<256; i++)
		MAPPS(obj, submap).d.funcs[i] = NULL;
	    }
	  for (i=start; i<=end; i++)
            MAPPS(obj, submap).d.funcs[i] = func;
	}
    }
}

static	void
table_map_set_routine (obj, entry_idx, from, to, func)
  LayoutObj	obj;
  int		entry_idx;
  int		from;
  int		to;
  void		(*func)(LayoutObj);
{
  START(obj, entry_idx) = from;
  END(obj, entry_idx) = to;
  FUNC(obj, entry_idx) = func;
}

static void *
GetFuncPointer(obj, wc)
  LayoutObj	obj;
  wchar_t	wc;
{
  int i;

  for (i=0; i < MAX_NUM_ENTRIES(obj); i++)
    if (((u_int)wc >= START(obj, i)) && ((u_int)wc <= END(obj, i)))
      return ((void *)(FUNC(obj, i)));
  return NULL;
}

/*
 *  Entry point of to create and initialize the default layout object.
 *
 */
LayoutObject
_LayoutObjectInit(locale_name)
    char                *locale_name;
{
    LayoutObj           obj = (LayoutObj)malloc(sizeof(LayoutObjectRec));
    int			i;

    
    if (obj) {
        memset(obj, 0, sizeof(LayoutObjectRec));
        
        obj->methods = (&_UMLELayoutMethods);
        
        if (obj->core.locale_name = (char *)malloc(strlen(locale_name) + 1))  {
            (void *)strcpy(obj->core.locale_name, locale_name);
        } else
            goto NoMemory;
        
        obj->core.orientation = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!obj->core.orientation)
            goto NoMemory;
        obj->core.orientation->inp = ORIENTATION_LTR;
        obj->core.orientation->out = ORIENTATION_LTR;
        
        obj->core.context = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!obj->core.context)
            goto NoMemory;
        obj->core.context->inp = CONTEXT_LTR;
        obj->core.context->out = CONTEXT_LTR;
        
        obj->core.type_of_text = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!obj->core.type_of_text)
            goto NoMemory;            
        obj->core.type_of_text->inp = TEXT_EXPLICIT;
        obj->core.type_of_text->out = TEXT_VISUAL;
        
        obj->core.implicit_alg = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!obj->core.implicit_alg)
            goto NoMemory;                        
        obj->core.implicit_alg->inp = ALGOR_IMPLICIT;
        obj->core.implicit_alg->out = ALGOR_BASIC;

        obj->core.swapping = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!obj->core.swapping)
            goto NoMemory;                                    
        obj->core.swapping->inp = SWAPPING_NO;
        obj->core.swapping->out = SWAPPING_NO;
        
        obj->core.numerals = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!obj->core.numerals)
            goto NoMemory;                                                
        obj->core.numerals->inp = NUMERALS_NOMINAL;
        obj->core.numerals->out = NUMERALS_NOMINAL;
        
        obj->core.text_shaping = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!obj->core.text_shaping)
            goto NoMemory;
        obj->core.text_shaping->inp = TEXT_NOMINAL;
        obj->core.text_shaping->out = TEXT_SHAPED;
        
        obj->core.active_dir = TRUE;

        obj->core.active_shape_editing = TRUE;
        obj->core.shape_charset_size = 4;

        obj->core.in_out_text_descr_mask = AllTextDescriptors;

        obj->core.in_only_text_descr = 0;
        obj->core.out_only_text_descr = 0;
        
        obj->core.check_mode = MODE_STREAM;
        
        obj->core.shape_context_size = 
            (LayoutEditSize)malloc(sizeof(LayoutEditSizeRec));
        if (!obj->core.shape_context_size)
            goto NoMemory;   
        obj->core.shape_context_size->front = 
            obj->core.shape_context_size->back = 4;

	obj->core.shape_charset = (char *)strdup("tis620.2533=E4;8859-8=E5;8859-6=E6;sun.unicode.india=E7;");

	if (!obj->core.shape_charset)
	    goto NoMemory;	 

	/* Static Data Structure */
	obj->private_data =
	    (UMLEInfo)malloc(sizeof(UMLERec));
	if (!obj->private_data)
	    goto NoMemory;
	    
	CURINFO(obj) =
	    (CurTransform)malloc(sizeof(CurTransformRec));
	if (CURINFO(obj) == NULL)
	    goto NoMemory;

	/* Dynamic Data Structure */
	CURSHPGRP_IDX(obj) = 0;

	MBINPUNITLEN_F(obj) = (void *)__UTF8;
	MBTOWC(obj) = (void *)__UTF8ToUCS4;
	MAXBYTES(obj) = 6;
	SHPNUMBYTES(obj) = 4;

	for (i=0; i<256; i++)
	  {
	    MAPPS(obj, i).is_leaf = 1;
	    MAPPS(obj, i).d.func = NULL;
	  }
/*
	map_add_routine(obj, 0x0E01, 0x0E4E, func_thai);
	map_add_routine(obj, 0x0590, 0x05F4, func_hebrew);
	map_add_routine(obj, 0x0600, 0x0670, func_arabic);
*/
	FuncUCS4(obj) = func_ucs4;
	MAX_NUM_ENTRIES(obj) = MAX_NUM_RANGES;
	table_map_set_routine(obj, 0, 0x0E01, 0x0E4E, func_thai);
/*
	table_map_set_routine(obj, 1, 0x0590, 0x05F4, func_hebrew);
	table_map_set_routine(obj, 2, 0x0600, 0x0670, func_arabic);
	table_map_set_routine(obj, 3, 0x0900, 0x097F, func_devanagari);
*/
	table_map_set_routine(obj, 1, 0x0901, 0x0903, func_devanagari);
	table_map_set_routine(obj, 2, 0x0905, 0x0939, func_devanagari);
	table_map_set_routine(obj, 3, 0x093C, 0x094D, func_devanagari);
	table_map_set_routine(obj, 4, 0x0950, 0x0954, func_devanagari);
	table_map_set_routine(obj, 5, 0x0958, 0x0970, func_devanagari);

	table_map_set_routine(obj, 6, 0x0591, 0x05A1, func_hebrew);
	table_map_set_routine(obj, 7, 0x05A3, 0x05B9, func_hebrew);
	table_map_set_routine(obj, 8, 0x05BB, 0x05C4, func_hebrew);
	table_map_set_routine(obj, 9, 0x05D0, 0x05EA, func_hebrew);
	table_map_set_routine(obj,10, 0x05F0, 0x05F4, func_hebrew);
	table_map_set_routine(obj,11, 0x060C, 0x060C, func_arabic);
	table_map_set_routine(obj,12, 0x061B, 0x061B, func_arabic);
	table_map_set_routine(obj,13, 0x061F, 0x061F, func_arabic);
	table_map_set_routine(obj,14, 0x0621, 0x063A, func_arabic);
	table_map_set_routine(obj,15, 0x0640, 0x0655, func_arabic);
	table_map_set_routine(obj,16, 0x0660, 0x066D, func_arabic);
	table_map_set_routine(obj,17, 0x0670, 0x0670, func_arabic);

	table_map_set_routine(obj,18, 0x0c82, 0x0c83, func_kannada);
	table_map_set_routine(obj,19, 0x0c85, 0x0c8c, func_kannada);
	table_map_set_routine(obj,20, 0x0c8e, 0x0c90, func_kannada);
	table_map_set_routine(obj,21, 0x0c92, 0x0ca8, func_kannada);
	table_map_set_routine(obj,22, 0x0caa, 0x0cb3, func_kannada);
	table_map_set_routine(obj,23, 0x0cb5, 0x0cb9, func_kannada);
	table_map_set_routine(obj,24, 0x0cbe, 0x0cc4, func_kannada);
	table_map_set_routine(obj,25, 0x0cc6, 0x0cc8, func_kannada);
	table_map_set_routine(obj,26, 0x0cca, 0x0ccd, func_kannada);
	table_map_set_routine(obj,27, 0x0cd5, 0x0cd6, func_kannada);
	table_map_set_routine(obj,28, 0x0cde, 0x0cde, func_kannada);
	table_map_set_routine(obj,29, 0x0ce0, 0x0ce1, func_kannada);
	table_map_set_routine(obj,30, 0x0ce6, 0x0cef, func_kannada);

	table_map_set_routine(obj,31, 0x0a81, 0x0a83, func_gujarati);
	table_map_set_routine(obj,32, 0x0a85, 0x0a8b, func_gujarati);
	table_map_set_routine(obj,33, 0x0a8d, 0x0a8d, func_gujarati);
	table_map_set_routine(obj,34, 0x0a8f, 0x0a91, func_gujarati);
	table_map_set_routine(obj,35, 0x0a93, 0x0aa8, func_gujarati);
	table_map_set_routine(obj,36, 0x0aaa, 0x0ab0, func_gujarati);
	table_map_set_routine(obj,37, 0x0ab2, 0x0ab3, func_gujarati);
	table_map_set_routine(obj,38, 0x0ab5, 0x0ab9, func_gujarati);
	table_map_set_routine(obj,39, 0x0abc, 0x0ac5, func_gujarati);
	table_map_set_routine(obj,40, 0x0ac7, 0x0ac9, func_gujarati);
	table_map_set_routine(obj,41, 0x0acb, 0x0acd, func_gujarati);
	table_map_set_routine(obj,42, 0x0ad0, 0x0ad0, func_gujarati);
	table_map_set_routine(obj,43, 0x0ae0, 0x0ae0, func_gujarati);
	table_map_set_routine(obj,44, 0x0ae6, 0x0aef, func_gujarati);

	table_map_set_routine(obj, 45, 0x0981, 0x0983, func_bengali);
	table_map_set_routine(obj, 46, 0x0985, 0x098C, func_bengali);
	table_map_set_routine(obj, 47, 0x098f, 0x0990, func_bengali);
	table_map_set_routine(obj, 48, 0x0993, 0x09A8, func_bengali);
	table_map_set_routine(obj, 49, 0x09AA, 0x09B0, func_bengali);
	table_map_set_routine(obj, 50, 0x09B2, 0x09B2, func_bengali);
	table_map_set_routine(obj, 51, 0x09B6, 0x09B9, func_bengali);
	table_map_set_routine(obj, 52, 0x09BC, 0x09BC, func_bengali);
	table_map_set_routine(obj, 53, 0x09BE, 0x09C4, func_bengali);
	table_map_set_routine(obj, 54, 0x09C7, 0x09C8, func_bengali);
	table_map_set_routine(obj, 55, 0x09CB, 0x09CD, func_bengali);
	table_map_set_routine(obj, 56, 0x09D7, 0x09D7, func_bengali);
	table_map_set_routine(obj, 57, 0x09DC, 0x09DD, func_bengali);
	table_map_set_routine(obj, 58, 0x09DF, 0x09E3, func_bengali);
	table_map_set_routine(obj, 59, 0x09E6, 0x09FA, func_bengali);

	table_map_set_routine(obj, 60, 0x0B82, 0x0B83,func_tamil);
	table_map_set_routine(obj, 61, 0x0B85, 0x0B8A,func_tamil);
	table_map_set_routine(obj, 62, 0x0B8E, 0x0B90,func_tamil);
	table_map_set_routine(obj, 63, 0x0B92, 0x0B95,func_tamil);
	table_map_set_routine(obj, 64, 0x0B99, 0x0B9A,func_tamil);
	table_map_set_routine(obj, 65, 0x0B9C, 0x0B9C,func_tamil);
	table_map_set_routine(obj, 66, 0x0B9E, 0x0B9F,func_tamil);
	table_map_set_routine(obj, 67, 0x0BA3, 0x0BA4,func_tamil);
	table_map_set_routine(obj, 68, 0x0BA8, 0x0BAA,func_tamil);
	table_map_set_routine(obj, 69, 0x0BAE, 0x0BB5,func_tamil);
	table_map_set_routine(obj, 70, 0x0BB7, 0x0BB9,func_tamil);
	table_map_set_routine(obj, 71, 0x0BBE, 0x0BC2,func_tamil);
	table_map_set_routine(obj, 72, 0x0BC6, 0x0BC8,func_tamil);
	table_map_set_routine(obj, 73, 0x0BCA, 0x0BCD,func_tamil);
	table_map_set_routine(obj, 74, 0x0BD7, 0x0BD7,func_tamil);
	table_map_set_routine(obj, 75, 0x0BE7, 0x0BF2,func_tamil);

	table_map_set_routine(obj, 76, 0x0C01, 0x0C03,func_telugu);
	table_map_set_routine(obj, 77, 0x0C05, 0x0C0C,func_telugu);
	table_map_set_routine(obj, 78, 0x0C0E, 0x0C10,func_telugu);
	table_map_set_routine(obj, 79, 0x0C12, 0x0C28,func_telugu);
	table_map_set_routine(obj, 80, 0x0C2A, 0x0C33,func_telugu);
	table_map_set_routine(obj, 81, 0x0C35, 0x0C39,func_telugu);
	table_map_set_routine(obj, 82, 0x0C3E, 0x0C44,func_telugu);
	table_map_set_routine(obj, 83, 0x0C46, 0x0C48,func_telugu);
	table_map_set_routine(obj, 84, 0x0C4A, 0x0C4D,func_telugu);
	table_map_set_routine(obj, 85, 0x0C55, 0x0C56,func_telugu);
	table_map_set_routine(obj, 86, 0x0C60, 0x0C61,func_telugu);
	table_map_set_routine(obj, 87, 0x0C66, 0x0C6F,func_telugu);

	table_map_set_routine(obj, 88, 0x0D02, 0x0D03,func_malayalam);
	table_map_set_routine(obj, 89, 0x0D05, 0x0D0C,func_malayalam);
	table_map_set_routine(obj, 90, 0x0D0E, 0x0D10,func_malayalam);
	table_map_set_routine(obj, 91, 0x0D12, 0x0D28,func_malayalam);
	table_map_set_routine(obj, 92, 0x0D2A, 0x0D39,func_malayalam);
	table_map_set_routine(obj, 93, 0x0D3E, 0x0D43,func_malayalam);
	table_map_set_routine(obj, 94, 0x0D46, 0x0D48,func_malayalam);
	table_map_set_routine(obj, 95, 0x0D4A, 0x0D4D,func_malayalam);
	table_map_set_routine(obj, 96, 0x0D57, 0x0D57,func_malayalam);
	table_map_set_routine(obj, 97, 0x0D60, 0x0D61,func_malayalam);
	table_map_set_routine(obj, 98, 0x0D66, 0x0D6F,func_malayalam);
	
	table_map_set_routine(obj, 99 , 0x0A02, 0x0A02,func_punjabi);
	table_map_set_routine(obj, 100, 0x0A05, 0x0A0A,func_punjabi);
	table_map_set_routine(obj, 101, 0x0A0F, 0x0A10,func_punjabi);
	table_map_set_routine(obj, 102, 0x0A13, 0x0A28,func_punjabi);
	table_map_set_routine(obj, 103, 0x0A2A, 0x0A30,func_punjabi);
	table_map_set_routine(obj, 104, 0x0A32, 0x0A33,func_punjabi);
	table_map_set_routine(obj, 105, 0x0A35, 0x0A36,func_punjabi);
	table_map_set_routine(obj, 106, 0x0A38, 0x0A39,func_punjabi);
	table_map_set_routine(obj, 107, 0x0A3C, 0x0A3C,func_punjabi);
	table_map_set_routine(obj, 108, 0x0A3E, 0x0A42,func_punjabi);
	table_map_set_routine(obj, 109, 0x0A47, 0x0A48,func_punjabi);
	table_map_set_routine(obj, 110, 0x0A4B, 0x0A4D,func_punjabi);
	table_map_set_routine(obj, 111, 0x0A59, 0x0A5C,func_punjabi);
	table_map_set_routine(obj, 112, 0x0A5E, 0x0A5E,func_punjabi);
	table_map_set_routine(obj, 113, 0x0A66, 0x0A6F,func_punjabi);
	table_map_set_routine(obj, 114, 0x0A70, 0x0A74,func_punjabi);
	
	THAI_CHRTYPE_TBL(obj) = ThaiChrTypeTbl;
	HEBREW_CHRTYPE_TBL(obj) = HebrewChrTypeTbl;
	ARABIC_CHRTYPE_TBL(obj) = ArabicChrTypeTbl;
	DEVA_CHRTYPE_TBL(obj) = DevaChrTypeTbl;
	TAMIL_CHRTYPE_TBL(obj) = TamilChrTypeTbl;
	KANN_CHRTYPE_TBL(obj) = KannChrTypeTbl;
	GUJA_CHRTYPE_TBL(obj) = GujaChrTypeTbl;
	BENG_CHRTYPE_TBL(obj) = BengChrTypeTbl;
	TLG_CHRTYPE_TBL(obj) = TlgChrTypeTbl;
	ML_CHRTYPE_TBL(obj) = MlChrTypeTbl;
	PN_CHRTYPE_TBL(obj) = PnChrTypeTbl;

	THAI_CHRCLS_TBL(obj) = ThaiChrClsTbl;
	HEBREW_CHRCLS_TBL(obj) = HebrewChrClsTbl;
	ARABIC_CHRCLS_TBL(obj) = ArabicChrClsTbl;
	DEVA_CHRCLS_TBL(obj) = DevaChrClsTbl;
	TAMIL_CHRCLS_TBL(obj) = TamilChrClsTbl;
	KANN_CHRCLS_TBL(obj) = KannChrClsTbl;
	GUJA_CHRCLS_TBL(obj) = GujaChrClsTbl;
	BENG_CHRCLS_TBL(obj) = BengChrClsTbl;
	TLG_CHRCLS_TBL(obj) = TlgChrClsTbl;
	ML_CHRCLS_TBL(obj) = MlChrClsTbl;
	PN_CHRCLS_TBL(obj) = PnChrClsTbl;

/*
	THAI_COMPOSE_TBL(obj) = ThaiComposeTbl;
	HEBREW_COMPOSE_TBL(obj) = HebrewComposeTbl;
	ARABIC_COMPOSE_TBL(obj) = ArabicComposeTbl;
	DEVA_COMPOSE_TBL(obj) = DevaComposeTbl;
*/

	THAI_GLYPH_TBL(obj) = &ThaiGlyphTbl;
	HEBREW_GLYPH_TBL(obj) = &HebrewGlyphTbl;
	ARABIC_GLYPH_TBL(obj) = &ArabicGlyphTbl;
/*
	DEVA_GLYPH_TBL(obj) = &DevaGlyphTbl;
*/

	INPMBCZ(obj) = INPWCCZ(obj) = OUTCZ(obj) = UMLE_CACHE_SIZE;
	O2ICZ(obj) = UNITCZ(obj) = UMLE_CACHE_SIZE;
	I2OCZ(obj) = UMLE_CACHE_SIZE*sizeof(size_t);
	PROPCZ(obj) = UMLE_CACHE_SIZE*sizeof(unsigned char);
	MBINPBUF(obj) = MBINPBUF_CACHE(obj);
	WCINPBUF(obj) = WCINPBUF_CACHE(obj);
	WCTLGINPBUF(obj) = WCTLGINPBUF_CACHE(obj);		// Added for Telugu
	UNIT_TO_MB_IDX(obj) = UNIT_TO_MB_IDX_CACHE(obj);
	LOGICAL_EMBEDDINGS(obj) = LOGICAL_EMBEDDINGS_CACHE(obj);
	DIRS(obj) = DIRS_CACHE(obj);
	VISUAL_EMBEDDINGS(obj) = VISUAL_EMBEDDINGS_CACHE(obj);
	INPTOOUT(obj) = INPTOOUT_CACHE(obj);
	TMPINPTOOUT(obj) = TMPINPTOOUT_CACHE(obj);
	MAPPING_V2L(obj) = MAPPING_V2L_CACHE(obj);
	PROPERTY(obj) = PROPERTY_CACHE(obj);
	OUTBUF(obj) = OUTBUF_CACHE(obj);
	OUTTOINP(obj) = OUTTOINP_CACHE(obj);
	MAPPING_L2V(obj) = MAPPING_L2V_CACHE(obj);

    	return ((LayoutObject)obj);
    }
    
NoMemory:
    errno = ENOMEM;
    
    if (obj) {
        if(obj->core.locale_name) {
            free(obj->core.locale_name);
            obj->core.locale_name = NULL;
        }
        
        if(obj->core.orientation) {
            free(obj->core.orientation);
            obj->core.orientation = (LayoutTextDescriptor)NULL;
        }
        
        if(obj->core.context) {
            free(obj->core.context);
            obj->core.context = (LayoutTextDescriptor)NULL;
        }
        
        if(obj->core.type_of_text) {
            free(obj->core.type_of_text);
            obj->core.type_of_text = (LayoutTextDescriptor)NULL;
        }
        
        if(obj->core.implicit_alg) {
            free(obj->core.implicit_alg);
            obj->core.implicit_alg = (LayoutTextDescriptor)NULL;
        }
        
        if(obj->core.swapping) {
            free(obj->core.swapping);
            obj->core.swapping = (LayoutTextDescriptor)NULL;
        }
        
        if(obj->core.numerals) {
            free(obj->core.numerals);
            obj->core.numerals = (LayoutTextDescriptor)NULL;
        }

        if(obj->core.text_shaping) {
            free(obj->core.text_shaping);
            obj->core.text_shaping = (LayoutTextDescriptor)NULL;
        }
        
        if(obj->core.shape_context_size) {
            free(obj->core.shape_context_size);
            obj->core.shape_context_size = (LayoutEditSize)NULL;
        }
        
	if(obj->core.shape_charset) {
	    free(obj->core.shape_charset);
	    obj->core.shape_charset = NULL;
	}
	
	if (obj->private_data) {
	    if (CURINFO(obj)) {
		CacheFree((char *) MBINPBUF(obj), MBINPBUF_CACHE(obj));
		CacheFree((char *) WCINPBUF(obj), WCINPBUF_CACHE(obj));
		CacheFree((char *) WCTLGINPBUF(obj), WCTLGINPBUF_CACHE(obj));		// Added for telugu
		CacheFree((char *) UNIT_TO_MB_IDX(obj), UNIT_TO_MB_IDX_CACHE(obj));
		CacheFree((char *) LOGICAL_EMBEDDINGS(obj), LOGICAL_EMBEDDINGS_CACHE(obj));
		CacheFree((char *) DIRS(obj), DIRS_CACHE(obj));
		CacheFree((char *) VISUAL_EMBEDDINGS(obj), VISUAL_EMBEDDINGS_CACHE(obj));
		CacheFree((char *) INPTOOUT(obj), INPTOOUT_CACHE(obj));
		CacheFree((char *) TMPINPTOOUT(obj), TMPINPTOOUT_CACHE(obj));
		CacheFree((char *) MAPPING_L2V(obj), MAPPING_L2V_CACHE(obj));
		CacheFree((char *) PROPERTY(obj), PROPERTY_CACHE(obj));
		CacheFree((char *) OUTBUF(obj), OUTBUF_CACHE(obj));
		CacheFree((char *) OUTTOINP(obj), OUTTOINP_CACHE(obj));
		CacheFree((char *) MAPPING_V2L(obj), MAPPING_V2L_CACHE(obj));
		free(CURINFO(obj));
		CURINFO(obj) = NULL;
	    }
	    free(obj->private_data);
	    obj->private_data = NULL;
	}
    }
    return((LayoutObject)NULL);
}

static LayoutObject
_LSUMLECreate(obj, layout_values)
    LayoutObj		obj;
    LayoutValues 	layout_values;
{
    int                 result = -1;
    int                 index_returned;
    LayoutValues        temp_ptr = layout_values;

    if (layout_values) {
        result = obj->methods->setvalues(obj, layout_values, &index_returned);

	/*
	 * The reason to free ShapeCharset is because it is
	 * malloced from the modifier string.
	 */
        while (temp_ptr->name) {
             if ((temp_ptr->name == ShapeCharset) &&
                 (temp_ptr->value)) {
                 free(temp_ptr->value);
             }
             temp_ptr++;
        }

    }

    return(obj);
}


static int
_LSUMLEDestroy(obj)
    LayoutObj		obj;
{
    int			result = 0;

    if (obj == NULL) {
        errno = EFAULT;
        result = -1;
        return (result);
    }

    if (obj->core.orientation) {
        free(obj->core.orientation);
        obj->core.orientation = (LayoutTextDescriptor)NULL;
    }
        
    if (obj->core.context) {
        free(obj->core.context);
        obj->core.context = (LayoutTextDescriptor)NULL;
    }
        
    if (obj->core.type_of_text) {
        free(obj->core.type_of_text);
        obj->core.type_of_text = (LayoutTextDescriptor)NULL;
    }
        
    if (obj->core.implicit_alg) {
        free(obj->core.implicit_alg);
        obj->core.implicit_alg = (LayoutTextDescriptor)NULL;
    }
    
    if (obj->core.swapping) {
        free(obj->core.swapping);
        obj->core.swapping = (LayoutTextDescriptor)NULL;
    }
        
    if (obj->core.numerals) {
        free(obj->core.numerals);
        obj->core.numerals = (LayoutTextDescriptor)NULL;
    }
        
    if (obj->core.text_shaping) {
        free(obj->core.text_shaping);
        obj->core.text_shaping = (LayoutTextDescriptor)NULL;
    }
                
    if(obj->core.shape_context_size) {
        free(obj->core.shape_context_size);
        obj->core.shape_context_size = (LayoutEditSize)NULL;
    }
        
    if(obj->core.shape_charset) {
        free(obj->core.shape_charset);
        obj->core.shape_charset = NULL;
    }
        
    if(obj->core.locale_name) {
        free(obj->core.locale_name);
        obj->core.locale_name = NULL;
    }

    if (obj->private_data) {
	if (CURINFO(obj)) {
	    CacheFree((char *) MBINPBUF(obj), MBINPBUF_CACHE(obj));
	    CacheFree((char *) WCINPBUF(obj), WCINPBUF_CACHE(obj));
		CacheFree((char *) WCTLGINPBUF(obj), WCTLGINPBUF_CACHE(obj));	// Added for Telugu
	    CacheFree((char *) UNIT_TO_MB_IDX(obj), UNIT_TO_MB_IDX_CACHE(obj));
	    CacheFree((char *) LOGICAL_EMBEDDINGS(obj), LOGICAL_EMBEDDINGS_CACHE(obj));
	    CacheFree((char *) DIRS(obj), DIRS_CACHE(obj));
	    CacheFree((char *) VISUAL_EMBEDDINGS(obj), VISUAL_EMBEDDINGS_CACHE(obj));
	    CacheFree((char *) INPTOOUT(obj), INPTOOUT_CACHE(obj));
	    CacheFree((char *) TMPINPTOOUT(obj), TMPINPTOOUT_CACHE(obj));
	    CacheFree((char *) MAPPING_L2V(obj), MAPPING_L2V_CACHE(obj));
	    CacheFree((char *) PROPERTY(obj), PROPERTY_CACHE(obj));
	    CacheFree((char *) OUTBUF(obj), OUTBUF_CACHE(obj));
	    CacheFree((char *) OUTTOINP(obj), OUTTOINP_CACHE(obj));
	    CacheFree((char *) MAPPING_V2L(obj), MAPPING_V2L_CACHE(obj));
	    free(CURINFO(obj));
	    CURINFO(obj) = NULL;
	}
	free(obj->private_data);
	obj->private_data = NULL;
    }
    free(obj);    
    
    return(result);
}

static int
_LSUMLEGetValues(obj, values, index_returned)
    LayoutObj		obj;
    LayoutValues 	values;
    int 		*index_returned;
{
    int			result = 0;
    int			i = 0;
    unsigned long	descr_mask = 0;
    
    if (!values)
        return(result);
        
    while (values[i].name) {
    
        if (values[i].value == NULL)
             break; /* Error case */
        
        if ((values[i].name & AllTextDescriptors) &&
            (values[i].name & QueryValueSize)) {
            unsigned long		*dummy = values[i].value;
                
            *dummy = sizeof(LayoutTextDescriptorRec);

        } else if (values[i].name == AllTextDescriptors) {
            LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
            
            (*dummy)->inp |= obj->core.orientation->inp;
            (*dummy)->out |= obj->core.orientation->out;
            
            (*dummy)->inp |= obj->core.context->inp;
            (*dummy)->out |= obj->core.context->out;
            
            (*dummy)->inp |= obj->core.type_of_text->inp;
            (*dummy)->out |= obj->core.type_of_text->out;
            
            (*dummy)->inp |= obj->core.implicit_alg->inp;
            (*dummy)->out |= obj->core.implicit_alg->out;
            
            (*dummy)->inp |= obj->core.swapping->inp;
            (*dummy)->out |= obj->core.swapping->out;
            
            (*dummy)->inp |= obj->core.numerals->inp;
            (*dummy)->out |= obj->core.numerals->out;
            
            (*dummy)->inp |= obj->core.text_shaping->inp;
            (*dummy)->out |= obj->core.text_shaping->out;
            
        } else if (values[i].name & AllTextDescriptors) {
                        
            if (values[i].name & Orientation) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = obj->core.orientation->inp;
                (*dummy)->out = obj->core.orientation->out;
            } else if  (values[i].name & Context) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = obj->core.context->inp;
                (*dummy)->out = obj->core.context->out;
            } else if (values[i].name & TypeOfText) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = obj->core.type_of_text->inp;
                (*dummy)->out = obj->core.type_of_text->out;
            } else if (values[i].name & ImplicitAlg) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = obj->core.implicit_alg->inp;
                (*dummy)->out = obj->core.implicit_alg->out;
            } else if (values[i].name & Swapping) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = obj->core.swapping->inp;
                (*dummy)->out = obj->core.swapping->out;
            } else if (values[i].name & Numerals) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = obj->core.numerals->inp;
                (*dummy)->out = obj->core.numerals->out;

            } else if (values[i].name & TextShaping) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = obj->core.text_shaping->inp;
                (*dummy)->out = obj->core.text_shaping->out;
            }
        } else if (values[i].name & ActiveDirectional) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(obj->core.active_dir);
            } else {
                 BooleanValue		*dummy = values[i].value;

                *dummy = (BooleanValue) obj->core.active_dir;
            }
        }  else if (values[i].name & ActiveShapeEditing) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(obj->core.active_shape_editing);
            } else {
                 BooleanValue		*dummy = values[i].value;

                *dummy = (BooleanValue) obj->core.active_shape_editing;
            }
        } else if (values[i].name & ShapeCharset) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = strlen(obj->core.shape_charset) + 1;
            } else {
                 char			**dummy = values[i].value;

                strcpy(*dummy, obj->core.shape_charset);
            }
        }  else if (values[i].name & ShapeCharsetSize) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(obj->core.shape_charset_size);
            } else {
                 int		*dummy = values[i].value;

                *dummy = obj->core.shape_charset_size;
            }
        } else if (values[i].name & InOutTextDescrMask) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(obj->core.in_out_text_descr_mask);
            } else {
                 unsigned long		*dummy = values[i].value;

                *dummy = obj->core.in_out_text_descr_mask;
            }
        } else if (values[i].name & InOnlyTextDescr) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                 
                *dummy = sizeof(obj->core.in_only_text_descr);
            } else {
                 unsigned long		*dummy = values[i].value;
                 
                 *dummy = 0;
                 
                 if (obj->core.in_out_text_descr_mask & Orientation)
                     *dummy |= obj->core.orientation->inp;
                 if (obj->core.in_out_text_descr_mask & Context)
                     *dummy |= obj->core.context->inp;
                 if (obj->core.in_out_text_descr_mask & TypeOfText)
                     *dummy |= obj->core.type_of_text->inp;
                 if (obj->core.in_out_text_descr_mask & ImplicitAlg)
                     *dummy |= obj->core.implicit_alg->inp;
                 if (obj->core.in_out_text_descr_mask & Swapping)
                     *dummy |= obj->core.swapping->inp;
                 if (obj->core.in_out_text_descr_mask & Numerals)
                     *dummy |= obj->core.numerals->inp;                     
                 if (obj->core.in_out_text_descr_mask & TextShaping)
                     *dummy |= obj->core.text_shaping->inp;
            }
        } else if (values[i].name & OutOnlyTextDescr) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                 
                *dummy = sizeof(obj->core.out_only_text_descr);
            } else {
                 unsigned long		*dummy = values[i].value;
                 
                 *dummy = 0;
                 
                 if (obj->core.in_out_text_descr_mask & Orientation)
                     *dummy |= obj->core.orientation->out;
                 if (obj->core.in_out_text_descr_mask & Context)
                     *dummy |= obj->core.context->out;
                 if (obj->core.in_out_text_descr_mask & TypeOfText)
                     *dummy |= obj->core.type_of_text->out;
                 if (obj->core.in_out_text_descr_mask & ImplicitAlg)
                     *dummy |= obj->core.implicit_alg->out;
                 if (obj->core.in_out_text_descr_mask & Swapping)
                     *dummy |= obj->core.swapping->out;
                 if (obj->core.in_out_text_descr_mask & Numerals)
                     *dummy |= obj->core.numerals->out;                     
                 if (obj->core.in_out_text_descr_mask & TextShaping)
                     *dummy |= obj->core.text_shaping->out;
            }
        } else if (values[i].name & CheckMode) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(obj->core.check_mode);
            } else {
                 int			*dummy = values[i].value;

                *dummy = obj->core.check_mode;
            }
        } else if (values[i].name & ShapeContextSize) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(LayoutEditSizeRec);
            } else {
                 LayoutEditSize	*dummy = (LayoutEditSize*)values[i].value;

                 (*dummy)->front = obj->core.shape_context_size->front;
                 (*dummy)->back = obj->core.shape_context_size->back;
            }

        } else
            break; /* Error condition */

        i++;
    
    }
    
    if (values[i].name) {
        result = -1;
        *index_returned = i;
        errno = EINVAL;
    }
    
    return(result);

}

static int 
FoundInvalidValue(values, index_returned)
    LayoutValues	values;
    int 		*index_returned;
{
    int			i = 0;
    unsigned long	in_out_text_descr_mask = 0;
    			
    while (values[i].name) {
             
        if (values[i].name & AllTextDescriptors) {
        
            if (values[i].value == NULL)
                break; /* Error case */
                        
            if (values[i].name & Orientation) {
                LayoutTextDescriptor dummy = (LayoutTextDescriptor)values[i].value;
                
                if ((dummy->inp & ~ORIENTATION_LTR) || (dummy->out & ~ORIENTATION_LTR))
                    goto InvalidValue;

#ifdef notdef                    
                if ((*dummy)->inp & ~MaskOrientation)
                    goto InvalidValue;
                    
                if ((*dummy)->out & ~MaskOrientation)
                    goto InvalidValue;
#endif                    
                                    
            } else if  (values[i].name & Context) {
                LayoutTextDescriptor dummy = (LayoutTextDescriptor)values[i].value;

                if ((dummy->inp & ~CONTEXT_LTR) || (dummy->out & ~CONTEXT_LTR))
                    goto InvalidValue;
                
                if (dummy->inp & ~MaskContext)
                    goto InvalidValue;
                    
                if (dummy->out & ~MaskContext)
                    goto InvalidValue;
               
                                
            } else if (values[i].name & TypeOfText) {
                LayoutTextDescriptor dummy = (LayoutTextDescriptor)values[i].value;
                
                if ((dummy->inp & ~TEXT_IMPLICIT) || (dummy->out & ~TEXT_VISUAL))                
                    goto InvalidValue;

#ifdef notdef                     
                if (dummy->inp & ~MaskTypeOfText)
                    goto InvalidValue;
                    
                if (dummy->out & ~MaskTypeOfText)
                    goto InvalidValue;
#endif                    
                
                
            } else if (values[i].name & ImplicitAlg) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;

                if ((dummy->inp & ~ALGOR_IMPLICIT) || (dummy->out & ~ALGOR_IMPLICIT))                
                    goto InvalidValue;

                if (dummy->inp & ~MaskImplicitAlg)
                    goto InvalidValue;
                    
                if (dummy->out & ~MaskImplicitAlg)
                    goto InvalidValue;
                
            } else if (values[i].name & Swapping) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;

                if ((dummy->inp & ~SWAPPING_NO) || (dummy->out & ~SWAPPING_YES))                
                    goto InvalidValue;

                if (dummy->inp & ~MaskSwapping)
                    goto InvalidValue;
                    
                if (dummy->out & ~MaskSwapping)
                    goto InvalidValue;
                                    
            } else if (values[i].name & Numerals) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;

                if (dummy->inp & ~MaskNumerals)
                    goto InvalidValue;
                    
                if (dummy->out & ~MaskNumerals)
                    goto InvalidValue;
                
                    
            } else if (values[i].name & TextShaping) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                if (dummy->inp & ~MaskTextShaping)
                    goto InvalidValue;
                    
                if (dummy->out & ~MaskTextShaping)
                    goto InvalidValue;
                                   
            }
        } else if (values[i].name & ShapeCharset) {
            char			*dummy = (char *)values[i].value;
            
            if ((dummy == NULL) || (strlen(dummy) <= 0))
                goto InvalidValue;
            
        } else if (values[i].name & InOutTextDescrMask) {
            in_out_text_descr_mask = (unsigned long)values[i].value;
            
            if (in_out_text_descr_mask & ~AllTextDescriptors)
                goto InvalidValue;
                
        } else if ((values[i].name & InOnlyTextDescr) ||
                   (values[i].name & OutOnlyTextDescr)) {
            unsigned long		dummy = (unsigned long)values[i].value;
                 
            if (dummy & ~MaskAllTextDescriptors)
                goto InvalidValue;
                
            if (in_out_text_descr_mask & Orientation) {
                if ((dummy & MaskOrientation) != ORIENTATION_LTR)
                     goto InvalidValue;

            } 
                    
            if (in_out_text_descr_mask & TypeOfText) {
                if ((dummy & MaskTypeOfText) != TEXT_VISUAL)
                     goto InvalidValue;

            } 
                
                
        } else if (values[i].name & CheckMode) {
            int				dummy = (int)values[i].value;
            if ((dummy != MODE_STREAM) && (dummy != MODE_EDIT))
                goto InvalidValue;

        } else
         /*
    	  *  ActiveDirectional, ActiveShapeEditing, ShapeCharsetSize and 
    	  *  ShapeContextSize are readonly and should not be set.
          */   
            goto InvalidValue; /* Error condition */

        i++;
    }
    
    return (FALSE);
    
InvalidValue:
    *index_returned = i;
    return (TRUE);

}	

static int
_LSUMLESetValues(obj, values, index_returned)
    LayoutObj		obj;
    LayoutValues 	values;
    int 		*index_returned;
{
    int			result = 0;
    int			i = 0;
    
    if (!values)
        return(0);
        
          
    if (FoundInvalidValue(values, index_returned)) {
        errno = EINVAL;
        return(-1);
    }
                
    while (values[i].name) {
        
        if (values[i].name == AllTextDescriptors) {
             LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                 
             obj->core.orientation->inp = ORIENTATION_LTR;
	     obj->core.context->inp = CONTEXT_LTR;
	     obj->core.type_of_text->inp = TEXT_EXPLICIT;
	     obj->core.implicit_alg->inp = ALGOR_IMPLICIT;
	     obj->core.swapping->inp = SWAPPING_NO;
	     obj->core.numerals->inp = NUMERALS_NOMINAL;
	     obj->core.text_shaping->inp = TEXT_NOMINAL;
	     
	     if (((dummy->out & MaskOrientation) == ORIENTATION_LTR) ||
		 ((dummy->out & MaskOrientation) == ORIENTATION_CONTEXTUAL))
	         obj->core.orientation->out = (dummy->out & MaskOrientation);
	     obj->core.context->out = CONTEXT_LTR;
	     obj->core.type_of_text->out = TEXT_VISUAL;
	     obj->core.implicit_alg->out = ALGOR_BASIC;
	     obj->core.swapping->out = SWAPPING_NO;
	     obj->core.numerals->out = NUMERALS_NOMINAL;
	     obj->core.text_shaping->out = TEXT_SHAPED;
                 
        } else if (values[i].name & AllTextDescriptors) {
        
            if (values[i].value == NULL)
                break; /* Error case */
                        
            if (values[i].name & Orientation) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                    
                obj->core.orientation->inp = ORIENTATION_LTR;
		if ((dummy->out == ORIENTATION_LTR) || (dummy->out == ORIENTATION_CONTEXTUAL))
		    obj->core.orientation->out = dummy->out;
                
            } else if  (values[i].name & Context) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                obj->core.context->inp = CONTEXT_LTR;
                obj->core.context->out = CONTEXT_LTR;
                
            } else if (values[i].name & TypeOfText) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                obj->core.type_of_text->inp = TEXT_EXPLICIT;
                obj->core.type_of_text->out = TEXT_VISUAL;
                
            } else if (values[i].name & ImplicitAlg) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                obj->core.implicit_alg->inp = ALGOR_IMPLICIT;
                obj->core.implicit_alg->out = ALGOR_BASIC;
                
            } else if (values[i].name & Swapping) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                obj->core.swapping->inp = SWAPPING_NO;
                obj->core.swapping->out = SWAPPING_NO;
                    
            } else if (values[i].name & Numerals) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                obj->core.numerals->inp = NUMERALS_NOMINAL;
                obj->core.numerals->out = NUMERALS_NOMINAL;
                    
            } else if (values[i].name & TextShaping) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                obj->core.text_shaping->inp = TEXT_NOMINAL;
                obj->core.text_shaping->out = TEXT_SHAPED;
                    
            }
        } else if (values[i].name & ShapeCharset) {
        
/*
            if (obj->core.shape_charset)
                 free(obj->core.shape_charset);
*/
            
        } else if (values[i].name & InOutTextDescrMask) {
            obj->core.in_out_text_descr_mask = (unsigned long)values[i].value;
        } else if (values[i].name & InOnlyTextDescr) {
             unsigned long		dummy = (unsigned long)values[i].value;
                 
             if (obj->core.in_out_text_descr_mask & Orientation)
                 obj->core.orientation->inp = ORIENTATION_LTR;
             if (obj->core.in_out_text_descr_mask & Context)
                 obj->core.context->inp = CONTEXT_LTR;
             if (obj->core.in_out_text_descr_mask & TypeOfText)
                 obj->core.type_of_text->inp = TEXT_EXPLICIT;
             if (obj->core.in_out_text_descr_mask & ImplicitAlg)
                 obj->core.implicit_alg->inp = ALGOR_IMPLICIT;
             if (obj->core.in_out_text_descr_mask & Swapping)
                 obj->core.swapping->inp = SWAPPING_NO;
             if (obj->core.in_out_text_descr_mask & Numerals)
                 obj->core.numerals->inp = NUMERALS_NOMINAL;
             if (obj->core.in_out_text_descr_mask & TextShaping)
                 obj->core.text_shaping->inp = TEXT_NOMINAL;
                 
        } else if (values[i].name & OutOnlyTextDescr) {
             unsigned long		dummy = (unsigned long)values[i].value;
                 
             if (obj->core.in_out_text_descr_mask & Orientation) {
		 if (((dummy & MaskOrientation) == ORIENTATION_LTR) ||
		     ((dummy & MaskOrientation) == ORIENTATION_CONTEXTUAL))
                     obj->core.orientation->out = (dummy & MaskOrientation);
             } else if (obj->core.in_out_text_descr_mask & Context)
                 obj->core.context->out = CONTEXT_LTR;
             if (obj->core.in_out_text_descr_mask & TypeOfText)
                 obj->core.type_of_text->out = TEXT_VISUAL;
             if (obj->core.in_out_text_descr_mask & ImplicitAlg)
                 obj->core.implicit_alg->out = ALGOR_BASIC;
             if (obj->core.in_out_text_descr_mask & Swapping)
                 obj->core.swapping->out = SWAPPING_NO;
             if (obj->core.in_out_text_descr_mask & Numerals)
                 obj->core.numerals->out = NUMERALS_NOMINAL;
             if (obj->core.in_out_text_descr_mask & TextShaping)
                 obj->core.text_shaping->out = TEXT_SHAPED;

        } else if (values[i].name & CheckMode) {
             obj->core.check_mode = MODE_STREAM;


        } else
            break; /* Error condition */

        i++;
    
    }
    
    if (values[i].name) {
        result = -1;
        *index_returned = i;
        errno = EINVAL;
    }

    return(result);
}

static bidi_type_t
BiDiType(ucs4)
    wchar_t			ucs4;
{
    if ((ucs4 == 0x0040) ||
    	((ucs4 >= 0x0041) && (ucs4 <= 0x005A)) ||
    	((ucs4 >= 0x0061) && (ucs4 <= 0x007A)) ||
    	(ucs4 == 0x00AA) ||
    	(ucs4 == 0x00B5) ||
    	(ucs4 == 0x00BA) ||
    	((ucs4 >= 0x00C0) && (ucs4 <= 0x00D6)) ||
    	((ucs4 >= 0x00D8) && (ucs4 <= 0x00F6)) ||
    	((ucs4 >= 0x00F8) && (ucs4 <= 0x00FF)) ||
    	((ucs4 >= 0x0100) && (ucs4 <= 0x02C6)) ||
    	((ucs4 >= 0x02D0) && (ucs4 <= 0x02D1)) ||
    	((ucs4 >= 0x02E0) && (ucs4 <= 0x02E4)) ||
    	((ucs4 >= 0x02EE) && (ucs4 <= 0x02FF)) ||
    	((ucs4 >= 0x0300) && (ucs4 <= 0x036F)) ||
    	((ucs4 >= 0x0370) && (ucs4 <= 0x0373)) ||
    	((ucs4 >= 0x0376) && (ucs4 <= 0x037D)) ||
    	((ucs4 >= 0x037F) && (ucs4 <= 0x0383)) ||
    	(ucs4 == 0x0386) ||
    	((ucs4 >= 0x0388) && (ucs4 <= 0x0589)) ||
    	((ucs4 >= 0x058B) && (ucs4 <= 0x058F)) ||
	((ucs4 >= 0x07C0) && (ucs4 <= 0x08FF)) ||
    	((ucs4 >= 0x0900) && (ucs4 <= 0x09F1)) ||
    	((ucs4 >= 0x09F4) && (ucs4 <= 0x0E3E)) ||
    	((ucs4 >= 0x0E40) && (ucs4 <= 0x0F39)) ||
    	((ucs4 >= 0x0F3E) && (ucs4 <= 0x11FF)) ||
    	((ucs4 >= 0x1E00) && (ucs4 <= 0x1FBC)) ||
    	(ucs4 == 0x1FBE) ||
    	((ucs4 >= 0x1FC2) && (ucs4 <= 0x1FCC)) ||
    	((ucs4 >= 0x1FD0) && (ucs4 <= 0x1FDC)) ||
    	((ucs4 >= 0x1FE0) && (ucs4 <= 0x1FEC)) ||
    	((ucs4 >= 0x1FF0) && (ucs4 <= 0x1FFC)) ||
    	(ucs4 == 0x1FFF) ||
	((ucs4 >= 0x1200) && (ucs4 <= 0x1DFF)) ||
	(ucs4 == 0x200E) ||
	(ucs4 == 0x2047) ||
	((ucs4 >= 0x204E) && (ucs4 <= 0x2069)) ||
	((ucs4 >= 0x2071) && (ucs4 <= 0x2073)) ||
	(ucs4 == 0x207F) ||
	((ucs4 >= 0x208F) && (ucs4 <= 0x209F)) ||
    	((ucs4 >= 0x20D0) && (ucs4 <= 0x20FF)) ||
    	(ucs4 == 0x2102) ||
    	(ucs4 == 0x2107) ||
    	((ucs4 >= 0x210A) && (ucs4 <= 0x2113)) ||
    	(ucs4 == 0x2115) ||
    	((ucs4 >= 0x2119) && (ucs4 <= 0x211D)) ||
    	(ucs4 == 0x2124) ||
    	(ucs4 == 0x2126) ||
    	(ucs4 == 0x2128) ||
    	((ucs4 >= 0x212A) && (ucs4 <= 0x212D)) ||
    	((ucs4 >= 0x212F) && (ucs4 <= 0x2131)) ||
	((ucs4 >= 0x2133) && (ucs4 <= 0x2139)) ||
	((ucs4 >= 0x213B) && (ucs4 <= 0x2152)) ||
    	((ucs4 >= 0x2160) && (ucs4 <= 0x2182)) ||
	((ucs4 >= 0x2183) && (ucs4 <= 0x218F)) ||
	((ucs4 >= 0x22F2) && (ucs4 <= 0x22FF)) ||
	((ucs4 >= 0x2336) && (ucs4 <= 0x237A)) ||
	(ucs4 == 0x237C) ||
	(ucs4 == 0x2395) ||
	((ucs4 >= 0x239B) && (ucs4 <= 0x23FF)) ||
	((ucs4 >= 0x2427) && (ucs4 <= 0x243F)) ||
	((ucs4 >= 0x244B) && (ucs4 <= 0x245F)) ||
	((ucs4 >= 0x24EB) && (ucs4 <= 0x24FF)) ||
	((ucs4 >= 0x2596) && (ucs4 <= 0x259F)) ||
	((ucs4 >= 0x25F8) && (ucs4 <= 0x25FF)) ||
	((ucs4 >= 0x2614) && (ucs4 <= 0x2619)) ||
	((ucs4 >= 0x2670) && (ucs4 <= 0x2700)) ||
	(ucs4 == 0x2705) ||
	((ucs4 >= 0x270A) && (ucs4 <= 0x270B)) ||
	(ucs4 == 0x2728) ||
	(ucs4 == 0x274C) ||
	(ucs4 == 0x274E) ||
	((ucs4 >= 0x2753) && (ucs4 <= 0x2755)) ||
	(ucs4 == 0x2757) ||
	((ucs4 >= 0x275F) && (ucs4 <= 0x2760)) ||
	((ucs4 >= 0x2768) && (ucs4 <= 0x2775)) ||
	((ucs4 >= 0x2795) && (ucs4 <= 0x2797)) ||
	(ucs4 == 0x27B0) ||
	((ucs4 >= 0x27BF) && (ucs4 <= 0x27FF)) ||
	((ucs4 >= 0x2900) && (ucs4 <= 0x2E7F)) ||
	((ucs4 >= 0x2FFC) && (ucs4 <= 0x2FFF)) ||
	((ucs4 >= 0x3005) && (ucs4 <= 0x3007)) ||
	((ucs4 >= 0x3021) && (ucs4 <= 0x3029)) ||
	((ucs4 >= 0x3031) && (ucs4 <= 0x3035)) ||
	((ucs4 >= 0x3038) && (ucs4 <= 0x303A)) ||
    	((ucs4 >= 0x3040) && (ucs4 <= 0x309A)) ||
    	((ucs4 >= 0x309D) && (ucs4 <= 0x9FFF)) ||
	((ucs4 >= 0xA000) && (ucs4 <= 0xA48F)) ||
	((ucs4 >= 0xA4C7) && (ucs4 <= 0xABFF)) ||
	((ucs4 >= 0xAC00) && (ucs4 <= 0xD7AF)) ||
	((ucs4 >= 0xF900) && (ucs4 <= 0xFA2D)) ||
	((ucs4 >= 0xFA2E) && (ucs4 <= 0xFA7F)) ||
	((ucs4 >= 0xFB00) && (ucs4 <= 0xFB17)) ||
	((ucs4 >= 0xFE00) && (ucs4 <= 0xFE1F)) ||
	(ucs4 == 0xFF00) ||
	((ucs4 >= 0xFF21) && (ucs4 <= 0xFF3A)) ||
	((ucs4 >= 0xFF41) && (ucs4 <= 0xFF5A)) ||
	((ucs4 >= 0xFF5F) && (ucs4 <= 0xFF60)) ||
	((ucs4 >= 0xFF66) && (ucs4 <= 0xFFDF)) ||
	(ucs4 == 0xFFE7) ||
	((ucs4 >= 0xFFEF) && (ucs4 <= 0xFFF8)) ||
	((ucs4 >= 0xFFFE) && (ucs4 <= 0xFFFF)) )
    	return LR;
    	
    if (ucs4 == 0x202A)
	return LRE;

    if (ucs4 == 0x202D)
	return LRO;

    if (((ucs4 >= 0x0590) && (ucs4 <= 0x05FF)) ||
	(ucs4 == 0x200F) ||
	((ucs4 >= 0xFB1D) && (ucs4 <= 0xFB28)) ||
	((ucs4 >= 0xFB2A) && (ucs4 <= 0xFB4F)) )
	return RL;

    if (((ucs4 >= 0x0600) && (ucs4 <= 0x060B)) ||
    	((ucs4 >= 0x060D) && (ucs4 <= 0x065F)) ||
	((ucs4 >= 0x066D) && (ucs4 <= 0x06E8)) ||
	((ucs4 >= 0x06EA) && (ucs4 <= 0x06FF)) ||
	((ucs4 >= 0x06FA) && (ucs4 <= 0x070E)) ||
	((ucs4 >= 0x0710) && (ucs4 <= 0x07BF)) ||
    	((ucs4 >= 0xFB50) && (ucs4 <= 0xFD3D)) ||
    	((ucs4 >= 0xFD40) && (ucs4 <= 0xFDFF)) ||
	((ucs4 >= 0xFE70) && (ucs4 <= 0xFEFE)) )
	return AL;

    if (ucs4 == 0x202B)
    	return RLE;
    	
    if (ucs4 == 0x202E)
	return RLO;

    if (ucs4 == 0x202C)
	return PDF;

    if (((ucs4 >= 0x0030) && (ucs4 <= 0x0039)) ||
    	((ucs4 >= 0x00B2) && (ucs4 <= 0x00B3)) ||
    	(ucs4 == 0x00B9) ||
    	((ucs4 >= 0x06F0) && (ucs4 <= 0x06F9)) ||
    	(ucs4 == 0x2070) ||
    	((ucs4 >= 0x2074) && (ucs4 <= 0x2079)) ||
	((ucs4 >= 0x2080) && (ucs4 <= 0x2089)) ||
	((ucs4 >= 0x2460) && (ucs4 <= 0x249B)) ||
	(ucs4 == 0x24EA) ||
    	((ucs4 >= 0xFF10) && (ucs4 <= 0xFF19)) )
    	return EN;
    	
    if ((ucs4 == 0x002F) ||
	(ucs4 == 0xFF0F))
	return ES;

    if ((ucs4 == 0x0023) ||
    	(ucs4 == 0x0024) ||
    	(ucs4 == 0x0025) ||
    	(ucs4 == 0x002B) ||
    	(ucs4 == 0x002D) ||
    	((ucs4 >= 0x00A2) && (ucs4 <= 0x00A5)) ||
    	(ucs4 == 0x00B0) ||
    	(ucs4 == 0x00B1) ||
    	(ucs4 == 0x066A) ||
    	((ucs4 >= 0x09F2) && (ucs4 <= 0x09F3)) ||
    	(ucs4 == 0x0E3F) ||
    	((ucs4 == 0x2030) && (ucs4 == 0x2034)) ||
    	(ucs4 == 0x207A) ||
    	(ucs4 == 0x207B) ||
    	(ucs4 == 0x208A) ||
    	(ucs4 == 0x208B) ||
    	((ucs4 >= 0x20A0) && (ucs4 <= 0x20CF)) ||
    	(ucs4 == 0x212E) ||
    	(ucs4 == 0x2212) ||
    	(ucs4 == 0x2213) ||
    	(ucs4 == 0xFB29) ||
    	(ucs4 == 0xFE5F) ||
    	((ucs4 >= 0xFE62) && (ucs4 <= 0xFE63)) ||
    	((ucs4 >= 0xFE69) && (ucs4 <= 0xFE6A)) ||
	((ucs4 >= 0xFF03) && (ucs4 <= 0xFF05)) ||
	(ucs4 == 0xFF0B) ||
	(ucs4 == 0xFF0D) ||
	((ucs4 >= 0xFFE0) && (ucs4 <= 0xFFE1)) ||
	((ucs4 >= 0xFFE5) && (ucs4 <= 0xFFE6)) )
    	return ET;
    	
    if (((ucs4 >= 0x0660) && (ucs4 <= 0x0669)) ||
        (ucs4 == 0x066B) ||
        (ucs4 == 0x066C) )
        return AN;
        
    if ((ucs4 == 0x002C) ||
	(ucs4 == 0x002E) ||
        (ucs4 == 0x003A) ||
    	(ucs4 == 0x00A0) ||
    	(ucs4 == 0x060C) ||
    	(ucs4 == 0xFE52) ||
    	(ucs4 == 0xFE55) ||
	(ucs4 == 0xFF0C) ||
	(ucs4 == 0xFF0E) ||
	(ucs4 == 0xFF1A) )
        return CS;
        
    if ((ucs4 == 0x000A) ||
    	((ucs4 >= 0x001C) && (ucs4 <= 0x001E)) ||
    	(ucs4 == 0x0085) ||
    	(ucs4 == 0x2029) )
        return PS;
        
    if ((ucs4 == 0x0009) ||
    	(ucs4 == 0x000B) )
    	return SS;

    if ((ucs4 == 0x000C) ||
    	(ucs4 == 0x001F) ||
    	(ucs4 == 0x0020) ||
        ((ucs4 >= 0x2000) && (ucs4 <= 0x2006)) ||
    	(ucs4 == 0x2007) ||
        ((ucs4 >= 0x2008) && (ucs4 <= 0x200A)) ||
        (ucs4 == 0x202F) ||
	(ucs4 == 0x2028) ||
        (ucs4 == 0x3000) )
        return WS;

    if (((ucs4 >= 0x0000) && (ucs4 <= 0x0008)) ||
	((ucs4 >= 0x000D) && (ucs4 <= 0x001B)) ||
	((ucs4 >= 0x007F) && (ucs4 <= 0x0084)) ||
	((ucs4 >= 0x0086) && (ucs4 <= 0x009F)) ||
	(ucs4 == 0x070F) ||
	((ucs4 >= 0x180B) && (ucs4 <= 0x180E)) ||
	((ucs4 >= 0x200B) && (ucs4 <= 0x200D)) ||
	((ucs4 >= 0x206A) && (ucs4 <= 0x206F)) ||
	(ucs4 == 0xFEFF) ||
	((ucs4 >= 0xFFF9) && (ucs4 <= 0xFFFB)) )
	return BN;

    return ON;
}

static int
ApplyUnicodeBidi(obj)
    LayoutObject		obj;
{
    int				j;
    int				limit;
    int				jj;
    int				jjj;
    BooleanValue		IsBaseLTR = TRUE;
    int				baselevel;
    int				level;
    int				newlevel;
    int				levelStack[NUMLEVELS];
    int				charStack[NUMLEVELS];
    int				s = 0;
    int				skip = 0;
    BooleanValue		lastStrongWasArabic;
    bidi_type_t			sc;
    bidi_type_t			dc;
    bidi_type_t			prev;
    bidi_type_t			next;
    bidi_type_t			ncur;
    bidi_type_t			dir;
    bidi_type_t			tempBaseDir;
    int				tempBaseLevel;
    int				eot;
    int				nexti;
    bidi_type_t			last;
    bidi_type_t			nval;
    int				nlevel;
    int				olevel;
    int				prevlevel;
    bidi_type_t			cur;

     
     /* Step 1: The Base Level */
    if ((obj->core.orientation->out == ORIENTATION_LTR) ||
	(obj->core.orientation->out == ORIENTATION_TTBLR))
	IsBaseLTR = TRUE;
    else if ((obj->core.orientation->out == ORIENTATION_RTL) ||
	     (obj->core.orientation->out == ORIENTATION_TTBRL))
	IsBaseLTR = FALSE;
    else {
    IsBaseLTR = (obj->core.context->out == CONTEXT_LTR) ? TRUE : FALSE;
    for (j=0; j < NUM_UNIT(obj); j++) {
	if (IsBiDiType(obj, WCINPBUF_AT(obj, j), LR|LRO|LRE)) {
	    IsBaseLTR = TRUE;
	    break;
	}
	if (IsBiDiType(obj, WCINPBUF_AT(obj, j), RL|AL)) {
	    IsBaseLTR = FALSE;
	    break;
	}
    }
    }
    /*
    Step 2 - 4: The Explicit Levels and Directions.
    		The Explicit Overrides.
    		The Terminating Embeddings and Overrides.
    */
    /* EMBEDDING */
    baselevel = level = (IsBaseLTR ? 0 : 1);
    for (j = 0; j < NUM_UNIT(obj); j++) {
	 if (IsBiDiType(obj, WCINPBUF_AT(obj, j), LRE|LRO)) {
	     if (skip > 0)
	         ++skip;
	     else {
	         newlevel = ((level & 0x0e) + 2);
	         if (newlevel >= NUMLEVELS)
	             ++skip;
	         else {
	             charStack[s] = j;
	             levelStack[s++] = LOGICAL_EMBEDDINGS_AT(obj, j) = level;
	             level = (IsBiDiType(obj, WCINPBUF_AT(obj, j), LRO) ?
	             			(newlevel + 0x10) : (newlevel));
	             continue;
	         }
	     }
	 } else if (IsBiDiType(obj, WCINPBUF_AT(obj, j), RLO|RLE)) {
	     if (skip > 0)
		 ++skip;
	     else {
	         newlevel = ((level & 0x0f) + 1) | 0x01;
	         if (newlevel >= NUMLEVELS) {
	             ++skip;
	         } else {
	             charStack[s] = j;
		     levelStack[s++] = LOGICAL_EMBEDDINGS_AT(obj, j) = level;
		     level = (IsBiDiType(obj, WCINPBUF_AT(obj, j), RLO) ?
		 			(newlevel + 0x10) : (newlevel));
		     continue;
		 }
	     }
	 } else if (IsBiDiType(obj, WCINPBUF_AT(obj, j), PDF)) {
	     if (skip > 0)
		 --skip;
	     else if (s > 0) {
	         /* lookahead to coalesce level pairs */
	         if ((j < NUM_UNIT(obj) - 1) &&
		     (WCINPBUF_AT(obj, j+1) == WCINPBUF_AT(obj, charStack[s-1])) ) {
	             LOGICAL_EMBEDDINGS_AT(obj, j) = level;
	             LOGICAL_EMBEDDINGS_AT(obj, j + 1) = level;
	             j++;
	             
	             continue;
	         }
		 level = levelStack[--s];
	     }
	 }
	 LOGICAL_EMBEDDINGS_AT(obj, j) = level;
    }
    
    /* DIRECTION */
    sc = ON;
    olevel = -1;
    for (j = 0; j < NUM_UNIT(obj); j++) {
	 if (LOGICAL_EMBEDDINGS_AT(obj, j) != olevel) {
	     sc = ON;
	     olevel = LOGICAL_EMBEDDINGS_AT(obj, j);
	 }
	 dc = BiDiType(WCINPBUF_AT(obj, j));
	 if (IsBiDiType(obj, WCINPBUF_AT(obj, j), LR|RL|AL))
	     sc = dc;
	 else if (IsBiDiType(obj, WCINPBUF_AT(obj, j), PS))
	     sc = ON;
	 else if (IsBiDiType(obj, WCINPBUF_AT(obj, j), NSM))
	     dc = sc;
	 else if (IsBiDiType(obj, WCINPBUF_AT(obj, j), LRE|LRO|RLE|RLO|PDF))
	     dc = ON;
	 DIRS_AT(obj, j) = dc;
    }
	
    for (j = 0; j < NUM_UNIT(obj); j++) {
	if ((LOGICAL_EMBEDDINGS_AT(obj, j) & 0x10) != 0) {
	     LOGICAL_EMBEDDINGS_AT(obj, j) &= 0x0f;
	     DIRS_AT(obj, j) = (LOGICAL_EMBEDDINGS_AT(obj, j) & 0x01);
	     /* L, R */
	}
    }
	
    /*
     * Step 5: Resolving Weak Type
     */
    j = 0;
    limit = 0;
    while (limit < NUM_UNIT(obj)) {
    
    level = LOGICAL_EMBEDDINGS_AT(obj, limit);
    limit++;
    while ((limit < NUM_UNIT(obj)) &&
    	   (LOGICAL_EMBEDDINGS_AT(obj, limit) == level))
    	limit++;
    
    prev = NO;
    lastStrongWasArabic = (DIRS_AT(obj, j) == AL);
    while (j < limit) {
    			
	 jj = j + 1;
	 next = (jj == limit) ? NO : DIRS_AT(obj, jj);	  

	 if (next == EN && lastStrongWasArabic)
	     next = AN;
	     
	 ncur = DIRS_AT(obj, j);
	 
	 switch (DIRS_AT(obj, j)) {
	   case LR:
	   case RL:
	     lastStrongWasArabic = FALSE;
	     break;
	    
	   case AL:
	     lastStrongWasArabic = TRUE;
	     break;
	      
	   case ES:
	     if (prev == EN && next == EN)
		 ncur = EN;
	     else
		 ncur = ON;
	     break;
	      
	   case CS:
	     if (prev == EN && next == EN)
	          ncur = EN;
	     else if (prev == AN && next == AN)
	          ncur = AN;
	     else
	          ncur = ON;
	     break;
	      
	   case ET:
	     if (prev == EN || next == EN)
	          ncur = EN;
	     else if (next == ET && !lastStrongWasArabic) {
	          /* forward scan to handle ET ET EN */
	          for (jjj = jj + 1; jjj < limit; ++jjj) {
	               dir = DIRS_AT(obj, jjj);
	               
	               if (dir == ET)
	                   continue;
	                   
	               if (dir == EN)
	                   nval = EN;
	               else
	                   nval = ON;
	               while (jj < jjj)
	                   DIRS_AT(obj, jj++) = nval;
	               ncur = nval;
	               next = dir;
	                   
	               break;
	          }
	     } else
	         ncur = ON;
	     break;
	      
	   default:
	     break;
	}
	DIRS_AT(obj, j) = ncur;
	j = jj;
	prev = ncur;
	cur = next;
    }
    
    }

    /*
     * Step 6: Resolving Neutral Types
     */
    j = 0;
    while (j < NUM_UNIT(obj)) {
	tempBaseLevel = LOGICAL_EMBEDDINGS_AT(obj, j);
	tempBaseDir = ((tempBaseLevel & 0x1) == 0) ? LR : RL;
	  
	eot = j + 1;
	while (eot < NUM_UNIT(obj) &&
	       LOGICAL_EMBEDDINGS_AT(obj, eot) == tempBaseLevel)
	  eot++;
	    
	last = tempBaseDir;
	nval = tempBaseDir;
	
	nexti = j - 1;
	  
	while (j < eot) {
	  switch (DIRS_AT(obj, j)) {
	    case LR:
	      last = LR;
	      break;
	    case RL:
	    case AL:
	      last = RL;
	      break;
	    case EN:
	    case ES:
	    case ET:
	    case AN:
	    case CS:
	      break;
	    case PS:
	    case SS:
	      last = tempBaseDir;
	      break;
	    case WS:
	    case ON:
	    case NSM:
	      if (j > nexti) {
	          nval = tempBaseDir;
	          nexti = j + 1;
	          while (nexti < eot) {
		    if (DIRS_AT(obj, nexti) == LR) {
	                nval = last == LR ? LR : tempBaseDir;
			break;
		    } else if ((DIRS_AT(obj, nexti) == RL) ||
		    	       (DIRS_AT(obj, nexti) == AL) ||
			       (DIRS_AT(obj, nexti) == AN)) {
	                nval = last == LR ? tempBaseDir : RL;
			break;
		    } else if (DIRS_AT(obj, nexti) == EN) {
		        nval = last;
		        break;
		    } else if (DIRS_AT(obj, nexti) == PS) {
		        nval = tempBaseDir;
		        break;
		    }
	            ++nexti;
	          } /* while */
	      } /* if */
	      DIRS_AT(obj, j) = nval;
	  } /* switch */
	  ++j;
	} /* while */
    }
	
    /*
     * Step 7: Resolving Implicit Levels
     */
    baselevel = (IsBaseLTR ? 0 : 1);
    prevlevel = -1;
    for (j = 0; j < NUM_UNIT(obj); j++) {
        nlevel = level = LOGICAL_EMBEDDINGS_AT(obj, j);
	switch (DIRS_AT(obj, j)) {
	  case LR:
	    nlevel = (level + 1) & 0x1e;
	    break;
	  case AL:
	  case RL:
	    nlevel = (level | 0x1);
	    break;
	  case AN:
	    nlevel = (level & 0xe) + 2;
	    break;
	  case EN:
	    if ((level & 0x1) != 0) {
	         nlevel++;
	    } else if ((j == 0) || (prevlevel != level)) {
	    	/* start of ltr level, leave it as is */
	    } else {
	         if (DIRS_AT(obj, j - 1) == EN) {
	             nlevel = LOGICAL_EMBEDDINGS_AT(obj, j - 1);
	         } else if (DIRS_AT(obj, j - 1) != LR) {
		     nlevel += 2;
	         }
	    }
	    break;
	  case PS:
	  case SS:
	    nlevel = baselevel;
	    /* Set preceeding whitespace to baselevel too */
	    for (jj = j - 1; jj >= 0 &&
	    			DIRS_AT(obj, jj) == WS; --jj)
	         LOGICAL_EMBEDDINGS_AT(obj, jj) = baselevel;
	    break;
	}
	if (nlevel != level)
	    LOGICAL_EMBEDDINGS_AT(obj, j) = nlevel;
	    
	prevlevel = level;
    }
    for (j = NUM_UNIT(obj) - 1; j >= 0 &&
    				DIRS_AT(obj, j) == WS; --j)
	LOGICAL_EMBEDDINGS_AT(obj, j) = baselevel;
}

static int
GenL2V_V2Lmap(obj)
    LayoutObject		obj;
{
    int				j;
    int				lowestOddLevel = NUMLEVELS + 1;
    int				highestLevel = 0;
    int				begin;
    int				end;
    int				level;
    int				temp;
    
    /* Visual To Logical */
    for (j = 0; j < NUM_UNIT(obj); j++) {
	MAPPING_V2L_AT(obj, j) = j;
	
	level = LOGICAL_EMBEDDINGS_AT(obj, j);
	
	if (level > highestLevel)
	    highestLevel = level;
	if ((level & 0x01) != 0 && level < lowestOddLevel)
	    lowestOddLevel = level;
    }
    /*
     * Get Visual Embeddings added by Chookij V.
     * ************ [not part of Java2D's Bidi]
     */
    for (j = 0; j < NUM_UNIT(obj); j++)
	VISUAL_EMBEDDINGS_AT(obj, j) = LOGICAL_EMBEDDINGS_AT(obj, j);
    
    while (highestLevel >= lowestOddLevel) {
	j = 0;

	for (;;) {
	     while (j < NUM_UNIT(obj) &&
	     	    LOGICAL_EMBEDDINGS_AT(obj, j) < highestLevel)
	     	j++;
	     begin = j++;
	     
	     if (begin == NUM_UNIT(obj))
	         break; /* no more runs at this level */
	         
	     while (j < NUM_UNIT(obj) &&
	     	    LOGICAL_EMBEDDINGS_AT(obj, j) >= highestLevel)
	        j++;
	     end = j - 1;
	     
	     while (begin < end) {
	         temp = MAPPING_V2L_AT(obj, begin);
	         MAPPING_V2L_AT(obj, begin) = MAPPING_V2L_AT(obj, end);
	         MAPPING_V2L_AT(obj, end) = temp;

	         /*
		  * Get Visual Embeddings added by Chookij V.
	          * *********** [not part of Java2D's Bidi]
	          */
		 temp = VISUAL_EMBEDDINGS_AT(obj, begin);
		 VISUAL_EMBEDDINGS_AT(obj, begin) = VISUAL_EMBEDDINGS_AT(obj, end);
		 VISUAL_EMBEDDINGS_AT(obj, end) = temp;

	         ++begin;
	         --end;
	     }
	}
	--highestLevel;
    }
    /* Logical To Visual */
    for (j = 0; j < NUM_UNIT(obj); j++)
         MAPPING_L2V_AT(obj, MAPPING_V2L_AT(obj, j)) = j;
}

static int
OutputCacheRealloc(obj)
    LayoutObject		obj;
{
    OUTBUF(obj) = (char *)
    	CacheRealloc(OUTCZ(obj), OUTCZ(obj)+UMLE_CACHE_SIZE,
			OUTBUF(obj), OUTBUF_CACHE(obj));
    OUTCZ(obj) += UMLE_CACHE_SIZE;
    OUTTOINP(obj) = (size_t *)
	CacheRealloc(O2ICZ(obj)*sizeof(size_t), (O2ICZ(obj)+UMLE_CACHE_SIZE)*sizeof(size_t),
			OUTTOINP(obj), OUTTOINP_CACHE(obj));
    O2ICZ(obj) += UMLE_CACHE_SIZE;
}

static void
ReverseDirection_MB(obj)
    LayoutObj		obj;
{
    int				begin, end, k, len, j;
    wchar_t			temp_out;
    size_t			temp_o2i;
    size_t			*i2o_p_begin;
    size_t			*r_i2o_p_begin;

    /* Reversing OUTBUF
       Reversing OUTTOINP */
    begin = TMP_O_IDX(obj);
    end = O_IDX(obj) - 1;
    while (begin < end)
      {
	temp_out = *((wchar_t *)(&OUTBUF_AT(obj, begin)));
	*((wchar_t *)(&OUTBUF_AT(obj, begin))) = *((wchar_t *)(&OUTBUF_AT(obj, end)));
	*((wchar_t *)(&OUTBUF_AT(obj, end))) = temp_out;

        for (k=0; k < SHPNUMBYTES(obj); k++) {

	temp_o2i = *((size_t *)(&OUTTOINP_AT(obj, begin*SHPNUMBYTES(obj)))+k);
	*((size_t *)(&OUTTOINP_AT(obj, begin*SHPNUMBYTES(obj)))+k) =
	    *((size_t *)(&OUTTOINP_AT(obj, end*SHPNUMBYTES(obj)))+k);
	*((size_t *)(&OUTTOINP_AT(obj, end*SHPNUMBYTES(obj)))+k) = temp_o2i;

        }

	++begin;
	--end;
       }

    /* Reversing INPTOOUT */
    j = END_HDRT(obj) + 1;
    while (j < JJJ(obj))
      {
	i2o_p_begin = &INPTOOUT_AT(obj, OUTTOINP_AT(obj, INPTOOUT_AT(obj,\
	      UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j)))));
	r_i2o_p_begin = &TMPINPTOOUT_AT(obj, \
	      UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j)));
	len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j) + 1) -
	      UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j));
	for (k=0; k < len; k++, r_i2o_p_begin++, i2o_p_begin++)
	  *r_i2o_p_begin = *i2o_p_begin;
	j++;
      }
    j = END_HDRT(obj) + 1;
    while (j < JJJ(obj))
      {
	 i2o_p_begin = &INPTOOUT_AT(obj, \
		UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j)));
	 r_i2o_p_begin = &TMPINPTOOUT_AT(obj,
		UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j)));
	 len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j) + 1) -
	       UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j));
	 for (k=0; k < len; k++, i2o_p_begin++, r_i2o_p_begin++)
	   *i2o_p_begin = *r_i2o_p_begin;
	 j++;
      }
}

static void
ReverseDirection_WC(obj)
    LayoutObj		obj;
{
    int				begin, end, j;
    wchar_t			temp_out;
    size_t			temp_o2i;

    /* Reversing OUTBUF
       Reversing OUTTOINP */
    begin = TMP_O_IDX(obj);
    end = O_IDX(obj) - 1;
    while (begin < end)
      {
	temp_out = *((wchar_t *)(&OUTBUF_AT(obj, begin)));
	*((wchar_t *)(&OUTBUF_AT(obj, begin))) = *((wchar_t *)(&OUTBUF_AT(obj, end)));
	*((wchar_t *)(&OUTBUF_AT(obj, end))) = temp_out;
	temp_o2i = OUTTOINP_AT(obj, begin);
	OUTTOINP_AT(obj, begin) = OUTTOINP_AT(obj, end);
	OUTTOINP_AT(obj, end) = temp_o2i;
	++begin;
	--end;
      }

    /* Reversing INPTOOUT */
    j = END_HDRT(obj) + 1;
    while (j < JJJ(obj))
      {
        TMPINPTOOUT_AT(obj, MAPPING_V2L_AT(obj, j)) =
	  INPTOOUT_AT(obj, OUTTOINP_AT(obj, INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, j))));
	j++;
      }
    j = END_HDRT(obj) + 1;
    while (j < JJJ(obj))
      {
	INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, j)) =
	   TMPINPTOOUT_AT(obj, MAPPING_V2L_AT(obj, j));
	j++;
      }
}

/*
 * Thai script
 */
static	void
GetGlyphsFromCluster_Thai_MB(obj)
  LayoutObj	obj;
{
  int		n_j;
  int		n_jj;
  wchar_t	wc_j;
  wchar_t	wc_jj;
  wchar_t	wc_jjj;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  char		*out_p;
  u_int		dummy;
  int		k, len, len1, len2;

  switch (NChrsInCluster(obj))
    {
      case 1:
	wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
	      UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));

	/* Property, InpToOut */
	prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
	i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
	for (k=0; k < len; k++, prop_p++, i2o_p++)
	  {
	    *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
	    *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	  }

	/* OutBuf, OutToInp */
	if (IsThaiCharCls (obj, wc_j, BelowVowel|BelowDiac|AboveVowel|AboveDiac|Tone))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE400007F;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0xE400007F;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
	    for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
		*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }

	OUTPUT_OUT2INP_CACHE_CHECK
	o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
#ifdef	_BIG_ENDIAN
	*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	out_p = &OUTBUF_AT(obj, O_IDX(obj));
	dummy = (ucs2tis(wc_j) + 0x80) | 0xE4000000;
	for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	  {
	     *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
	     dummy <<= 8;
	  }
#endif
	for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	  *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	O_IDX(obj)++;
	OUTSIZE(obj) = O_IDX(obj);
        break;

      case 2:
	wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
	      UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	n_j = JJJ(obj) + 1;
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
	len1 = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)+1) -
	       UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));

	/* Property */
	prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
	for (k=0; k < len; k++, prop_p++)
          *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
	prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
	for (k=0; k < len1; k++, prop_p++)
          *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

	/* OutBuf, OutToInp, InpToOut */
	if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
	    IsThaiCharCls (obj, wc_jj, SaraAm))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[6] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[6] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000D2;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0xE40000D2;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	 else if (IsThaiCharCls (obj, wc_j, NoTailCons|BotTailCons|SpltTailCons) &&
		  IsThaiCharCls (obj, wc_jj, SaraAm))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
	    for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
              *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000ED;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0xE40000ED;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
	    for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
              *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000D2;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0xE40000D2;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
	    for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
		*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	 else if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
		  IsThaiCharCls (obj, wc_jj, AboveVowel))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_AV[ucs2tis (wc_jj) - 0x51] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = THAI_GLYPH_TBL(obj)->ShiftLeft_AV[ucs2tis (wc_jj) - 0x51] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	 else if (IsThaiCharCls (obj, wc_j, NoTailCons|BotTailCons|SpltTailCons) &&
		  IsThaiCharCls (obj, wc_jj, AboveVowel))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_jj) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_jj) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	 else if (IsThaiCharCls (obj, wc_j, NoTailCons|BotTailCons|SpltTailCons) &&
		  IsThaiCharCls (obj, wc_jj, AboveDiac|Tone))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis (wc_jj) - 0x67] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis (wc_jj) - 0x67] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	 else if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
		  IsThaiCharCls (obj, wc_jj, AboveDiac|Tone))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDownLeft_TONE_AD[ucs2tis (wc_jj) - 0x67] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->ShiftDownLeft_TONE_AD[ucs2tis (wc_jj) - 0x67] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	 else if (IsThaiCharCls (obj, wc_j, NoTailCons|UpTailCons) &&
		  IsThaiCharCls (obj, wc_jj, BelowVowel|BelowDiac))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_jj) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_jj) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	 else if (IsThaiCharCls (obj, wc_j, BotTailCons) &&
		  IsThaiCharCls (obj, wc_jj, BelowVowel|BelowDiac))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_BV_BD[ucs2tis (wc_jj) - 0x58] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy =
		THAI_GLYPH_TBL(obj)->ShiftDown_BV_BD[ucs2tis (wc_jj) - 0x58] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	 else if (IsThaiCharCls (obj, wc_j, SpltTailCons) &&
		  IsThaiCharCls (obj, wc_jj, BelowVowel|BelowDiac))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->TailCutCons[ucs2tis (wc_j) - 0x2D] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy =
		THAI_GLYPH_TBL(obj)->TailCutCons[ucs2tis (wc_j) - 0x2D] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_jj) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis (wc_jj) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
        else
  	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE400007F;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0xE400007F;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
  	break;

      case 3:
	wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
	      UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	n_j = JJJ(obj) + 1;
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
	len1 = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)+1) -
	       UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	n_jj = JJJ(obj) + 2;
	wc_jjj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	len2 = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)+1) -
	       UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));

	/* Property */
	prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
	for (k=0; k < len; k++, prop_p++)
	  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
	prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
	for (k=0; k < len1; k++, prop_p++)
	  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);
	prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)));
	for (k=0; k < len2; k++, prop_p++)
	  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_jj)) & 0x0F);

	/* OutBuf, OutToInp, InpToOut */
	if (IsThaiCharCls (obj, wc_j, NoTailCons|BotTailCons|SpltTailCons) &&
	    IsThaiCharCls (obj, wc_jj, Tone) &&
	    IsThaiCharCls (obj, wc_jjj, SaraAm))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000ED;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0xE40000ED;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	    for (k=0; k < len2; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000D2;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0xE40000D2;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
		*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	else if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
		 IsThaiCharCls (obj, wc_jj, Tone) &&
		 IsThaiCharCls (obj, wc_jjj, SaraAm))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[ucs2tis(wc_jj) - 0x67] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[ucs2tis(wc_jj) - 0x67] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[6] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[6] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	    for (k=0; k < len2; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000D2;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0xE40000D2;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
		*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	else if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
		 IsThaiCharCls (obj, wc_jj, AboveVowel) &&
		 IsThaiCharCls (obj, wc_jjj, AboveDiac|Tone))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_AV[ucs2tis(wc_jj) - 0x51] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->ShiftLeft_AV[ucs2tis(wc_jj) - 0x51] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	    for (k=0; k < len2; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	else if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel) &&
		 IsThaiCharCls (obj, wc_jjj, AboveDiac|Tone))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDownLeft_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->ShiftDownLeft_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	    for (k=0; k < len2; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	else if (IsThaiCharCls (obj, wc_j, NoTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel) &&
		 IsThaiCharCls (obj, wc_jjj, AboveDiac|Tone))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	    for (k=0; k < len2; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	else if (IsThaiCharCls (obj, wc_j, SpltTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel) &&
		 IsThaiCharCls (obj, wc_jjj, AboveDiac|Tone))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->TailCutCons[ucs2tis(wc_j) - 0x2D] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->TailCutCons[ucs2tis(wc_j) - 0x2D] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	    for (k=0; k < len2; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
	else if (IsThaiCharCls (obj, wc_j, BotTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel) &&
		 IsThaiCharCls (obj, wc_jjj, AboveDiac|Tone))
       	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_BV_BD[ucs2tis(wc_jj) - 0x58] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->ShiftDown_BV_BD[ucs2tis(wc_jj) - 0x58] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	    for (k=0; k < len2; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
  	else
  	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif
            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	    for (k=0; k < len; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
	    for (k=0; k < len1; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
	    i2o_p = &INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj));
#ifdef	_BIG_ENDIAN
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jjj) + 0x80) | 0xE4000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = (ucs2tis(wc_jjj) + 0x80) | 0xE4000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	      {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	      }
#endif

            for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	      *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
	    for (k=0; k < len2; k++, i2o_p++)
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
       	  }
      break;
    }
}

static	void
GetGlyphsFromCluster_Thai_WC(obj)
  LayoutObj	obj;
{
  int		n_j;
  int		n_jj;
  wchar_t	wc_j;
  wchar_t	wc_jj;
  wchar_t	wc_jjj;

  switch (NChrsInCluster(obj))
    {
      case 1:
	wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));

	/* Property */
	PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
	  (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;

	OUTPUT_OUT2INP_CACHE_CHECK
	OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);

	/* OutBuf, OutToInp, InpToOut */
	if (IsThaiCharCls (obj, wc_j, BelowVowel|BelowDiac|AboveVowel|AboveDiac|Tone))
	  {
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE400007F;
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }
    
	OUTPUT_OUT2INP_CACHE_CHECK
	*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
	OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	O_IDX(obj)++;
	OUTSIZE(obj) = O_IDX(obj);
	break;

      case 2:
	wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	n_j = JJJ(obj) + 1;
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));

	/* Property */
	PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
	  (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
	PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
	  (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

	/* OutBuf, OutToInp, InpToOut */
	if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
	    IsThaiCharCls (obj, wc_jj, SaraAm))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	    
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[6] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000D2;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }
	else if (IsThaiCharCls (obj, wc_j, NoTailCons|BotTailCons|SpltTailCons) &&
		 IsThaiCharCls (obj, wc_jj, SaraAm))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	    
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000ED;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000D2;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }
	else if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
		 IsThaiCharCls (obj, wc_jj, AboveVowel))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_AV[ucs2tis (wc_jj) - 0x51] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }
	else if (IsThaiCharCls (obj, wc_j, NoTailCons|BotTailCons|SpltTailCons) &&
		 IsThaiCharCls (obj, wc_jj, AboveVowel))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_jj) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }
	else if (IsThaiCharCls (obj, wc_j, NoTailCons|BotTailCons|SpltTailCons) &&
		 IsThaiCharCls (obj, wc_jj, AboveDiac|Tone))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis (wc_jj) - 0x67] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }
	else if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
		 IsThaiCharCls (obj, wc_jj, AboveDiac|Tone))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDownLeft_TONE_AD[ucs2tis (wc_jj) - 0x67] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }
	else if (IsThaiCharCls (obj, wc_j, NoTailCons|UpTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel|BelowDiac))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_jj) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }
	else if (IsThaiCharCls (obj, wc_j, BotTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel|BelowDiac))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_BV_BD[ucs2tis (wc_jj) - 0x58] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }
	else if (IsThaiCharCls (obj, wc_j, SpltTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel|BelowDiac)) 
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->TailCutCons[ucs2tis (wc_j) - 0x2D] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis (wc_jj) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
	  }
        else
          {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE400007F;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
          }
        break;

      case 3:
	wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	n_j = JJJ(obj) + 1;
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
	n_jj = JJJ(obj) + 2;
	wc_jjj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_jj));

	/* Property */
	PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
	  (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
	PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
	  (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);
	PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_jj)) =
	  (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_jj)) & 0x0F);

	/* OutBuf, OutToInp, InpToOut */
	if (IsThaiCharCls (obj, wc_j, NoTailCons|BotTailCons|SpltTailCons) &&
	    IsThaiCharCls (obj, wc_jj, Tone) &&
	    IsThaiCharCls (obj, wc_jjj, SaraAm))
          {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000ED;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_jj);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000D2;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_jj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
          }
	else if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
		 IsThaiCharCls (obj, wc_jj, Tone) &&
		 IsThaiCharCls (obj, wc_jjj, SaraAm))
          {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[ucs2tis(wc_jj) - 0x67] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[6] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_jj);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE40000D2;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_jj);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
          }
	else if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
		 IsThaiCharCls (obj, wc_jj, AboveVowel) &&
		 IsThaiCharCls (obj, wc_jjj, AboveDiac|Tone))
          {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_AV[ucs2tis(wc_jj) - 0x51] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftLeft_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_jj);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
          }
	else if (IsThaiCharCls (obj, wc_j, UpTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel) &&
		 IsThaiCharCls (obj, wc_jjj, AboveDiac|Tone))
          {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDownLeft_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_jj);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
          }
	else if (IsThaiCharCls (obj, wc_j, NoTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel) &&
		 IsThaiCharCls (obj, wc_jjj, AboveDiac|Tone))
          {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_jj);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
          }
	else if (IsThaiCharCls (obj, wc_j, SpltTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel) &&
		 IsThaiCharCls (obj, wc_jjj, AboveDiac|Tone))
          {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->TailCutCons[ucs2tis(wc_j) - 0x2D] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_jj);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
          }
	else if (IsThaiCharCls (obj, wc_j, BotTailCons) &&
		 IsThaiCharCls (obj, wc_jj, BelowVowel) &&
		 IsThaiCharCls (obj, wc_jjj, AboveDiac|Tone))
          {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_BV_BD[ucs2tis(wc_jj) - 0x58] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		THAI_GLYPH_TBL(obj)->ShiftDown_TONE_AD[ucs2tis(wc_jjj) - 0x67] | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_jj);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
          }
        else
          {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_j) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jj) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_j);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);

	    OUTPUT_OUT2INP_CACHE_CHECK
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		(ucs2tis(wc_jjj) + 0x80) | 0xE4000000;
	    OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, n_jj);
	    INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj)) = O_IDX(obj);
	    O_IDX(obj)++;
	    OUTSIZE(obj) = O_IDX(obj);
          }
      break;
    }
}

static void
NextCluster_Thai(obj)
  LayoutObj		obj;
{
  wchar_t	wc_j_ = L'\0';
  wchar_t	wc_j = L'\0';
  wchar_t	wc_jj = L'\0';
  int		n_j;

  if (NEXT_CLUSTER(obj) < END_HDRT(obj))
  {

  if (NChrsInCluster(obj) != -1)
    JJJ(obj) = NEXT_CLUSTER(obj);
  if (JJJ(obj) < END_HDRT(obj))
    wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  else
    wc_j = L'\0';
  n_j = JJJ(obj) + 1;
  if (n_j < END_HDRT(obj))
    wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
  else
    wc_jj = L'\0';
  NChrsInCluster(obj) = 1;
  while (((wc_j != L'\0') && (wc_jj != L'\0') &&
	  IsThaiComposible(obj, wc_j, wc_jj)) ||
	 (NChrsInCluster(obj) == 1 &&
	  (wc_j != L'\0') && (wc_jj != L'\0') &&
	  IsThaiCharCls(obj, wc_j, Cons) &&
	  IsThaiCharCls(obj, wc_jj, SaraAm)) ||
	 (NChrsInCluster(obj) == 2 &&
	  (wc_j_ != L'\0') && (wc_j != L'\0') && (wc_jj != L'\0') &&
	  IsThaiCharCls(obj, wc_j_, Cons) &&
	  IsThaiCharCls(obj, wc_j, Tone) &&
	  IsThaiCharCls(obj, wc_jj, SaraAm)) )
    {
      switch (NChrsInCluster(obj)) {
	case 1:
	  wc_j_ = wc_j;
	  break;

	case 2:
	  wc_j_ = wc_j;
	  break;
      }
      NChrsInCluster(obj)++;
      wc_j = wc_jj;
      n_j++;
      if (n_j < END_HDRT(obj))
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
      else
	wc_jj = L'\0';
    }
  NEXT_CLUSTER(obj) = n_j;

  } else
      NChrsInCluster(obj) = 0;
}

static	void
func_thai(obj)
  LayoutObj		obj;
{
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Thai (obj);
  while (NChrsInCluster(obj) > 0)
    {
      if (LOGICALSTRMTYPE(obj) == FileCode)
	GetGlyphsFromCluster_Thai_MB (obj);
      else
	GetGlyphsFromCluster_Thai_WC (obj);
      NextCluster_Thai (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
					 : ReverseDirection_WC(obj) );
}

/*
 * Hebrew Script
 */
static	void
GetGlyphsFromCluster_Hebrew_MB(obj)
  LayoutObj	obj;
{
  int		i, next_j;
  int		n_j, n_jj;
  wchar_t	wc_j;
  wchar_t	wc_jj;
  wchar_t	wc_jjj;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  int		k, len, len1, len2;
  char		*out_p;
  wchar_t	dummy;

  wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
	UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  n_j = JJJ(obj) - 1;
  if (END_HDRT(obj) < n_j)
    {
      wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
      len1 = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)+1) -
	     UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
    }
  else
    {
      wc_jj = L'\0';
      len1 = 0;
    }
  n_jj = n_j - 1;
  if (END_HDRT(obj) < n_jj)
    {
      wc_jjj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_jj));
      len2 = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)+1) -
	     UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj));
    }
  else
    {
      wc_jjj = L'\0';
      len2 = 0;
    }

#ifdef	HebrewPresentForm
  if (HebrewCurrentClusterType(obj) == LamAlif)
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE500FB4F;
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _SHIN_CN) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _DAGESH) &&
	   (wc_jjj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jjj, _SHIN_DOT|_SIN_DOT))
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)));
      for (k=0; k < len2; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_jj)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	( IsHebrewCharCls(obj, wc_jjj, _SHIN_DOT) ? 0xE500FB2C: 0xE500FB2D );
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_jj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 3;
      next_j = n_jj - 1;
    }
  else if (IsHebrewCharCls(obj, wc_j, _SHIN_CN) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _SHIN_DOT|_SIN_DOT))
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	( IsHebrewCharCls(obj, wc_jj, _SHIN_DOT) ? 0xE500FB2A: 0xE500FB2B );
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _CNDA) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _DAGESH))
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	HEBREW_GLYPH_TBL(obj)->ConsDAGESH[ucs2HebrewCons(wc_j)] | 0xE5000000;
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _BET|_KAF|_PE) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _RAFE))
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	HEBREW_GLYPH_TBL(obj)->ConsRAFE[ucs2HebrewCons(wc_j)] | 0xE5000000;
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _ALEF) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _HIRIQ|_RAFE|_QAMATS))
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	HEBREW_GLYPH_TBL(obj)->ALEF_Vowel[ucs2HebrewHIRIQ(wc_j)] | 0xE5000000;
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _VAV) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _HOLAM))
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE500FB48;
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _DOUB_YOD) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _PATAH))
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE500FB1F;
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _YOD) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _HIRIQ))
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE500FB1D;
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _NS))
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);

      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE5000020;
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wc_j | 0xE5000000;
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 1;
      next_j = n_j;
    }
  else
#endif
/*
    {
*/
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
/*
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wc_j | 0xE5000000;
*/
#ifdef	_BIG_ENDIAN
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = ucs2hebrew_8(wc_j) | 0xE5000000;
#endif

#ifdef	_LITTLE_ENDIAN
      out_p = &OUTBUF_AT(obj, O_IDX(obj));
      dummy = ucs2hebrew_8(wc_j) | 0xE5000000;
      for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
        {
          *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
          dummy <<= 8;
        }
#endif
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 1;
      next_j = n_j;
/*
    }
*/

  while (i < NChrsInCluster(obj))
    {
      if (END_HDRT(obj) < next_j)
	{
          wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, next_j));
	  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)+1) -
		UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j));
        }
      else
        {
	  wc_j = L'\0';
	  len = 0;
	}

      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
#ifdef	_BIG_ENDIAN
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	(( IsHebrewCharCls(obj, WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _WC) ) ?
         (HEBREW_GLYPH_TBL(obj)->PunctForWideCons[ucs2HebrewCantPunct(wc_j)] | 0xE5000000) :
         (HEBREW_GLYPH_TBL(obj)->PunctForNarrowCons[ucs2HebrewCantPunct(wc_j)] | 0xE5000000));
#endif

#ifdef	_LITTLE_ENDIAN
      out_p = &OUTBUF_AT(obj, O_IDX(obj));
      dummy =
	(( IsHebrewCharCls(obj, WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _WC) ) ?
         (HEBREW_GLYPH_TBL(obj)->PunctForWideCons[ucs2HebrewCantPunct(wc_j)] | 0xE5000000) :
         (HEBREW_GLYPH_TBL(obj)->PunctForNarrowCons[ucs2HebrewCantPunct(wc_j)] | 0xE5000000));
      for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
        {
          *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
          dummy <<= 8;
        }
#endif
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);
      next_j--;
      i++;
    }
}

static	void
GetGlyphsFromCluster_Hebrew_WC(obj)
  LayoutObj	obj;
{
  int		i, next_j;
  int		n_j, n_jj;
  wchar_t	wc_j;
  wchar_t	wc_jj;
  wchar_t	wc_jjj;

  wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  n_j = JJJ(obj) - 1;
  if (END_HDRT(obj) < n_j)
    wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
  else
    wc_jj = L'\0';
  n_jj = n_j - 1;
  if (END_HDRT(obj) < n_jj)
    wc_jjj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_jj));
  else
    wc_jjj = L'\0';

#ifdef	HebrewPresentForm
  if (HebrewCurrentClusterType(obj) == LamAlif)
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE500FB4F;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _SHIN_CN) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _DAGESH) &&
	   (wc_jjj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jjj, _SHIN_DOT|_SIN_DOT))
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_jj)) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_jj)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	( IsHebrewCharCls(obj, wc_jjj, _SHIN_DOT) ? 0xE500FB2C: 0xE500FB2D );
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_jj)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 3;
      next_j = n_jj - 1;
    }
  else if (IsHebrewCharCls(obj, wc_j, _SHIN_CN) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _SHIN_DOT|_SIN_DOT))
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	( IsHebrewCharCls(obj, wc_jj, _SHIN_DOT) ? 0xE500FB2A: 0xE500FB2B );
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _CNDA) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _DAGESH))
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	HEBREW_GLYPH_TBL(obj)->ConsDAGESH[ucs2HebrewCons(wc_j)] | 0xE5000000;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _BET|_KAF|_PE) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _RAFE))
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	HEBREW_GLYPH_TBL(obj)->ConsRAFE[ucs2HebrewCons(wc_j)] | 0xE5000000;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _ALEF) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _HIRIQ|_RAFE|_QAMATS))
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	HEBREW_GLYPH_TBL(obj)->ALEF_Vowel[ucs2HebrewHIRIQ(wc_j)] | 0xE5000000;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _VAV) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _HOLAM))
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE500FB48;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _DOUB_YOD) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _PATAH))
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE500FB1F;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _YOD) &&
	   (wc_jj != L'\0') &&
	   IsHebrewCharCls(obj, wc_jj, _HIRIQ))
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE500FB1D;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 2;
      next_j = n_jj;
    }
  else if (IsHebrewCharCls(obj, wc_j, _NS))
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);

      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xE5000020;
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wc_j | 0xE5000000;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 1;
      next_j = n_j;
    }
  else
#endif

/*
    {
*/
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
      (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
/*
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wc_j | 0xE5000000;
*/
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = ucs2hebrew_8(wc_j) | 0xE5000000;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 1;
      next_j = n_j;
/*
    }
*/

  while (i < NChrsInCluster(obj))
    {
      if (END_HDRT(obj) < next_j)
        wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, next_j));
      else
	wc_j = L'\0';

      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, next_j)) =
        (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = ucs2hebrew_8(wc_j) | 0xE5000000;
/*
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	(( IsHebrewCharCls(obj, WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _WC) ) ?
         (HEBREW_GLYPH_TBL(obj)->PunctForWideCons[ucs2HebrewCantPunct(wc_j)] | 0xE5000000) :
         (HEBREW_GLYPH_TBL(obj)->PunctForNarrowCons[ucs2HebrewCantPunct(wc_j)] | 0xE5000000));
*/

      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, next_j);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, next_j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);
      next_j--;
      i++;
    }
}

static void
NextCluster_Hebrew(obj)
    LayoutObj		obj;
{
  wchar_t	wc_j = L'\0';
  wchar_t	wc_jj = L'\0';
  int		n_j;

  if (END_HDRT(obj) < NEXT_CLUSTER(obj))
  {

  HebrewCurrentClusterType(obj) = Normal;
  if (NChrsInCluster(obj) != -1)
    JJJ(obj) = NEXT_CLUSTER(obj);
  if (END_HDRT(obj) < JJJ(obj))
    wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  else
    wc_j = L'\0';
  n_j = JJJ(obj) - 1;
  if (END_HDRT(obj) < n_j)
    wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
  else
    wc_jj = L'\0';
  NChrsInCluster(obj) = 1;
  while ((wc_j != L'\0') && (wc_jj != L'\0') &&
	 IsHebrewComposible(obj, wc_j, wc_jj))
    {
      if (IsHebrewCharCls(obj, wc_j, _ALEF) &&
	  IsHebrewCharCls(obj, wc_jj, _LAM))
	  HebrewCurrentClusterType(obj) = LamAlif;
      NChrsInCluster(obj)++;
      wc_j =  wc_jj;
      n_j--;
      if (END_HDRT(obj) < n_j)
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
      else
	wc_jj = L'\0';
    }
  NEXT_CLUSTER(obj) = n_j;

  } else
      NChrsInCluster(obj) = 0;
}

static	void
func_hebrew(obj)
    LayoutObj		obj;
{
  int	jjj;
  int	end_hdrt = END_HDRT(obj);

  if (IS_REVERSE_AT(obj, III(obj)))
    {
      jjj = JJJ(obj);
      JJJ(obj) = END_HDRT(obj) - 1;
      END_HDRT(obj) = jjj - 1;
    }
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Hebrew (obj);
  while (NChrsInCluster(obj) != 0)
    {
      if (LOGICALSTRMTYPE(obj) == FileCode)
	GetGlyphsFromCluster_Hebrew_MB (obj);
      else
        GetGlyphsFromCluster_Hebrew_WC (obj);
      NextCluster_Hebrew (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
   {
     JJJ(obj) = end_hdrt;
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
    					 : ReverseDirection_WC(obj) );
     JJJ(obj) = END_HDRT(obj) = end_hdrt;
   }
}

/*
 * Arabic Script
 */
static void
GetGlyphsFromCluster_Arabic_MB (obj, base_tbl, combine_tbl, lam_alif_shape)
  LayoutObj		obj;
  int			*base_tbl;
  int			*combine_tbl;
  ArabicShapeState	lam_alif_shape;
{
  int		i, k, len, len1;
  int		j, n_j;
  wchar_t	wc_j;
  wchar_t	wc_jj;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  char		*out_p;
  u_int		dummy;

  wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
	UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  (IS_REVERSE_AT(obj, III(obj)) ? (n_j = JJJ(obj) - 1) : (n_j = JJJ(obj) + 1) );
  if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < n_j)) ||
       (!IS_REVERSE_AT(obj, III(obj)) && (n_j < END_HDRT(obj)))))
    {
      wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
      len1 = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)+1) -
	     UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j));
    }
  else
    {
      wc_jj = L'\0';
      len1 = 0;
    }

  if ((NChrsInCluster(obj) >= 2) &&
      (ArabicCurrentClusterType(obj) == LamAlif))
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      switch (lam_alif_shape)
	{
	  case ISOLATED:
	    dummy = ARABIC_GLYPH_TBL(obj)->IsolatedLamAlifForm[ucs2alif(wc_jj)] | 0xE6000000;
	    break;

	  case FINAL:
	    dummy = ARABIC_GLYPH_TBL(obj)->FinalLamAlifForm[ucs2alif(wc_jj)] | 0xE6000000;
	    break;

	  case INITIAL:
	  case MIDDLE:
	  default:
	    dummy = 0xE400007F;
	    printf("\nInitial/Middle Lam-Alif isn't existed\n");
	    break;
	}
#ifdef	_BIG_ENDIAN
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
      out_p = &OUTBUF_AT(obj, O_IDX(obj));
      for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
        {
          *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
          dummy <<= 8;
        }
#endif
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, n_j)));
      for (k=0; k < len1; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);
      i = 2;
      j = n_j;
    }
  else
    {
      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
#ifdef	_BIG_ENDIAN
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	base_tbl[ucs2iso8859_6(wc_j)] | 0xE6000000;
#endif

#ifdef	_LITTLE_ENDIAN
      out_p = &OUTBUF_AT(obj, O_IDX(obj));
      dummy = base_tbl[ucs2iso8859_6(wc_j)] | 0xE6000000;
      for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
        {
          *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
          dummy <<= 8;
        }
#endif
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);
      i = 1;
      j = JJJ(obj);
    }

  while (i < NChrsInCluster(obj))
    {
      (IS_REVERSE_AT(obj, III(obj)) ? j-- : j++ );
      if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < j)) ||
          (!IS_REVERSE_AT(obj, III(obj)) && (j < END_HDRT(obj)))))
	{
          wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, j));
	  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j)+1) -
		UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j));
	}
      else
	{
	  wc_j = L'\0';
	  len = 0;
	}

      /* Property */
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j)));
      for (k=0; k < len; k++, prop_p++)
	*prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j)));
#ifdef	_BIG_ENDIAN
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	combine_tbl[ucs2iso8859_6(wc_j)] | 0xE6000000;
#endif

#ifdef	_LITTLE_ENDIAN
      out_p = &OUTBUF_AT(obj, O_IDX(obj));
      dummy = combine_tbl[ucs2iso8859_6(wc_j)] | 0xE6000000;
      for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
        {
          *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
          dummy <<= 8;
        }
#endif
      for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, j));
      for (k=0; k < len; k++, i2o_p++)
	*i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);
      i++;
    }
}

static void
GetGlyphsFromCluster_Arabic_WC (obj, base_tbl, combine_tbl, lam_alif_shape)
  LayoutObj		obj;
  int			*base_tbl;
  int			*combine_tbl;
  ArabicShapeState	lam_alif_shape;
{
  int		i;
  int		j, n_j;
  wchar_t	wc_j;
  wchar_t	wc_jj;
  wchar_t	dummy;

  wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  (IS_REVERSE_AT(obj, III(obj)) ? (n_j = JJJ(obj) - 1) : (n_j = JJJ(obj) + 1) );
  if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < n_j)) ||
      (!IS_REVERSE_AT(obj, III(obj)) && (n_j < END_HDRT(obj)))))
    wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
  else
    wc_jj = L'\0';

  if ((NChrsInCluster(obj) >= 2) &&
      (ArabicCurrentClusterType(obj) == LamAlif))
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
	(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, n_j)) =
	(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, n_j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      switch (lam_alif_shape)
	{
	  case ISOLATED:
	    dummy = ARABIC_GLYPH_TBL(obj)->IsolatedLamAlifForm[ucs2alif(wc_jj)] | 0xE6000000;
	    break;

	  case FINAL:
	    dummy = ARABIC_GLYPH_TBL(obj)->FinalLamAlifForm[ucs2alif(wc_jj)] | 0xE6000000;
	    break;

	  case INITIAL:
	  case MIDDLE:
	  default:
	    dummy = 0xE400007F;
	    printf("\nInitial/Middle Lam-Alif isn't existed\n");
	}
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, n_j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);
      i = 2;
      j = n_j;
    }
  else
    {
      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
	(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		base_tbl[ucs2iso8859_6(wc_j)] | 0xE6000000;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);

      i = 1;
      j = JJJ(obj);
    }

  while (i < NChrsInCluster(obj))
    {
      (IS_REVERSE_AT(obj, III(obj)) ? j-- : j++ );
      if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < j)) ||
          (!IS_REVERSE_AT(obj, III(obj)) && (j < END_HDRT(obj)))))
        wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, j));
      else
	wc_j = L'\0';

      /* Property */
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, j)) =
	(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, j)) & 0x0F);

      /* OutBuf, OutToInp, InpToOut */
      OUTPUT_OUT2INP_CACHE_CHECK
      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		combine_tbl[ucs2iso8859_6(wc_j)] | 0xE6000000;
      OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, j);
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, j)) = O_IDX(obj);
      O_IDX(obj)++;
      OUTSIZE(obj) = O_IDX(obj);
      i++;
    }
}

static void
GetGlyphsFromArabicCluster (obj)
  LayoutObj	obj;
{
  wchar_t	next_wc_cluster;
  wchar_t	curr_wc_cluster;

  if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < JJJ(obj))) ||
      (!IS_REVERSE_AT(obj, III(obj)) && (JJJ(obj) < END_HDRT(obj)))))
    curr_wc_cluster = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  else
    curr_wc_cluster = L'\0';

  if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < NEXT_CLUSTER(obj))) ||
      (!IS_REVERSE_AT(obj, III(obj)) && (NEXT_CLUSTER(obj) < END_HDRT(obj)))))
    next_wc_cluster = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, NEXT_CLUSTER(obj)));
  else
    next_wc_cluster = L'\0';

  switch (ARABIC_SHAPING_STATE(obj))
    {
      case ISOLATED:
	if ((curr_wc_cluster != L'\0') &&
	    (next_wc_cluster != L'\0') &&
	    IsArabicCharCls(obj, curr_wc_cluster, _4F|_LM) &&
	    IsArabicCharCls(obj, next_wc_cluster, _4F|_2F|_LM|_AL) &&
	    (ArabicCurrentClusterType(obj) != LamAlif))
	  {
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      GetGlyphsFromCluster_Arabic_MB(obj, ARABIC_GLYPH_TBL(obj)->InitialForm,
		ARABIC_GLYPH_TBL(obj)->ComBaseLineForm, INITIAL);
	    else
	      GetGlyphsFromCluster_Arabic_WC(obj, ARABIC_GLYPH_TBL(obj)->InitialForm,
		ARABIC_GLYPH_TBL(obj)->ComBaseLineForm, INITIAL);
	    ARABIC_SHAPING_STATE(obj) = INITIAL;
	  }
	else
	  {
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      GetGlyphsFromCluster_Arabic_MB(obj, ARABIC_GLYPH_TBL(obj)->IsolatedForm,
		ARABIC_GLYPH_TBL(obj)->ComNoBaseLineForm, ISOLATED);
	    else
	      GetGlyphsFromCluster_Arabic_WC(obj, ARABIC_GLYPH_TBL(obj)->IsolatedForm,
		ARABIC_GLYPH_TBL(obj)->ComNoBaseLineForm, ISOLATED);
	    ARABIC_SHAPING_STATE(obj) = ISOLATED;
	  }
	break;

      case INITIAL:
      case MIDDLE:
	if ((curr_wc_cluster != L'\0') &&
	    (next_wc_cluster != L'\0') &&
	    IsArabicCharCls(obj, curr_wc_cluster, _4F|_LM) &&
	    IsArabicCharCls(obj, next_wc_cluster, _4F|_2F|_LM|_AL) &&
	    (ArabicCurrentClusterType(obj) != LamAlif))
	  {
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      GetGlyphsFromCluster_Arabic_MB(obj, ARABIC_GLYPH_TBL(obj)->MiddleForm,
		ARABIC_GLYPH_TBL(obj)->ComBaseLineForm, MIDDLE);
	    else
	      GetGlyphsFromCluster_Arabic_WC(obj, ARABIC_GLYPH_TBL(obj)->MiddleForm,
		ARABIC_GLYPH_TBL(obj)->ComBaseLineForm, MIDDLE);
	    ARABIC_SHAPING_STATE(obj) = MIDDLE;
	  }
	else
	  {
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      GetGlyphsFromCluster_Arabic_MB(obj, ARABIC_GLYPH_TBL(obj)->FinalForm,
		ARABIC_GLYPH_TBL(obj)->ComNoBaseLineForm, FINAL);
	    else
	      GetGlyphsFromCluster_Arabic_WC(obj, ARABIC_GLYPH_TBL(obj)->FinalForm,
		ARABIC_GLYPH_TBL(obj)->ComNoBaseLineForm, FINAL);
	    ARABIC_SHAPING_STATE(obj) = FINAL;
	  }
	break;

      case FINAL:
	if ((curr_wc_cluster != L'\0') &&
	    (next_wc_cluster != L'\0') &&
	    IsArabicCharCls(obj, curr_wc_cluster, _4F|_LM) &&
	    IsArabicCharCls(obj, next_wc_cluster, _4F|_2F|_LM|_AL) &&
	    (ArabicCurrentClusterType(obj) != LamAlif))
	  {
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      GetGlyphsFromCluster_Arabic_MB(obj, ARABIC_GLYPH_TBL(obj)->InitialForm,
		ARABIC_GLYPH_TBL(obj)->ComBaseLineForm, INITIAL);
	    else
	      GetGlyphsFromCluster_Arabic_WC(obj, ARABIC_GLYPH_TBL(obj)->InitialForm,
		ARABIC_GLYPH_TBL(obj)->ComBaseLineForm, INITIAL);
	    ARABIC_SHAPING_STATE(obj) = INITIAL;
	  }
	else
	  {
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      GetGlyphsFromCluster_Arabic_MB(obj, ARABIC_GLYPH_TBL(obj)->IsolatedForm,
		ARABIC_GLYPH_TBL(obj)->ComNoBaseLineForm, ISOLATED);
	    else
	      GetGlyphsFromCluster_Arabic_WC(obj, ARABIC_GLYPH_TBL(obj)->IsolatedForm,
		ARABIC_GLYPH_TBL(obj)->ComNoBaseLineForm, ISOLATED);
	    ARABIC_SHAPING_STATE(obj) = ISOLATED;
	  }
	break;
   }
}

static void
NextCluster_Arabic(obj)
  LayoutObj		obj;
{
  wchar_t	wc_j = L'\0';
  wchar_t	wc_jj = L'\0';
  int	n_j;

  if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < NEXT_CLUSTER(obj))) ||
       (!IS_REVERSE_AT(obj, III(obj)) && (NEXT_CLUSTER(obj) < END_HDRT(obj)))))
  {

  ArabicCurrentClusterType(obj) = Normal;
  if (NChrsInCluster(obj) != -1)
    JJJ(obj) = NEXT_CLUSTER(obj);
  if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < JJJ(obj))) ||
       (!IS_REVERSE_AT(obj, III(obj)) && (JJJ(obj) < END_HDRT(obj)))))
    wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  else
    wc_j = L'\0';
  (IS_REVERSE_AT(obj, III(obj)) ? (n_j = JJJ(obj) - 1) : (n_j = JJJ(obj) + 1) );
  if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < n_j)) ||
       (!IS_REVERSE_AT(obj, III(obj)) && (n_j < END_HDRT(obj)))))
    wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
  else
    wc_jj = L'\0';
  NChrsInCluster(obj) = 1;
  while ((wc_j != L'\0') && (wc_jj != L'\0') &&
	 IsArabicComposible(obj, wc_j, wc_jj))
    {
      if (IsArabicCharCls(obj, wc_j, _LM) &&
	  IsArabicCharCls(obj, wc_jj, _AL))
	  ArabicCurrentClusterType(obj) = LamAlif;
      NChrsInCluster(obj)++;
      wc_j = wc_jj;
      (IS_REVERSE_AT(obj, III(obj)) ? n_j-- : n_j++ );
      if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < n_j)) ||
          (!IS_REVERSE_AT(obj, III(obj)) && (n_j < END_HDRT(obj)))))
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
      else
	wc_jj = L'\0';
    }
  NEXT_CLUSTER(obj) = n_j;

  } else
      NChrsInCluster(obj) = 0;
}

static	void
func_arabic(obj)
  LayoutObj		obj;
{
  int	jjj;
  int	end_hdrt = END_HDRT(obj);

  if (IS_REVERSE_AT(obj, III(obj)))
    {
      jjj = JJJ(obj);
      JJJ(obj) = END_HDRT(obj) - 1;
      END_HDRT(obj) = jjj - 1;
    }
  ARABIC_SHAPING_STATE(obj) = ISOLATED;
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Arabic (obj);
  while (NChrsInCluster(obj) != 0)
    {
      GetGlyphsFromArabicCluster (obj);
      NextCluster_Arabic (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
   {
     JJJ(obj) = end_hdrt;
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
    					 : ReverseDirection_WC(obj) );
     JJJ(obj) = END_HDRT(obj) = end_hdrt;
   }
}

static int
CheckDevaKernSpaceGlyphs(obj, o_idx)
  LayoutObj	obj;
  int		o_idx;
{
  char		*out_p;
  wchar_t	dummy = L'\0';
  int		k;

  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
#ifdef	_BIG_ENDIAN
    dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));
#endif

#ifdef	_LITTLE_ENDIAN
  out_p = &OUTBUF_AT(obj, o_idx);
  for (k=0; k < sizeof(wchar_t); k++, out_p++)
    {
      dummy <<= 8; 
      dummy |= (0x000000ff & (*out_p));
    }
#endif
    }
  else
    dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));

  return (IsDevaKernSpaceGlyphs(obj, dummy));
}


static int
CheckGujaKernSpaceGlyphs(obj, o_idx)
  LayoutObj	obj;
  int		o_idx;
{
  char		*out_p;
  wchar_t	dummy = L'\0';
  int		k;

  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
#ifdef	_BIG_ENDIAN
    dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));
#endif

#ifdef	_LITTLE_ENDIAN
  out_p = &OUTBUF_AT(obj, o_idx);
  for (k=0; k < sizeof(wchar_t); k++, out_p++)
    {
      dummy <<= 8; 
      dummy |= (0x000000ff & (*out_p));
    }
#endif
    }
  else
    dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));

  return (IsGujaKernSpaceGlyphs(obj, dummy));
}



static int
CheckBengKernSpaceGlyphs(obj, o_idx)
  LayoutObj	obj;
  int		o_idx;
{
  char		*out_p;
  wchar_t	dummy = L'\0';
  int		k;

  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
#ifdef	_BIG_ENDIAN
    dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));
#endif

#ifdef	_LITTLE_ENDIAN
  out_p = &OUTBUF_AT(obj, o_idx);
  for (k=0; k < sizeof(wchar_t); k++, out_p++)
    {
      dummy <<= 8; 
      dummy |= (0x000000ff & (*out_p));
    }
#endif
    }
  else
    dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));

  return (IsBengKernSpaceGlyphs(obj, dummy));
}


static int
CheckTlgKernSpaceGlyphs(obj, o_idx)
  LayoutObj	obj;
  int		o_idx;
{
  char		*out_p;
  wchar_t	dummy = L'\0';
  int		k;

  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
#ifdef	_BIG_ENDIAN
		dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));
#endif

#ifdef	_LITTLE_ENDIAN
	  out_p = &OUTBUF_AT(obj, o_idx);
	  for (k=0; k < sizeof(wchar_t); k++, out_p++)
	  {
		  dummy <<= 8; 
		  dummy |= (0x000000ff & (*out_p));
	  }
#endif
   }
  else
	dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));

  return (IsTlgKernSpaceGlyphs(obj, dummy));

}

static int
CheckTlgMatraAtStemGlyph(obj, o_idx)
  LayoutObj	obj;
  int		o_idx;
{
  char		*out_p;
  wchar_t	dummy = L'\0';
  int		k;

  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
#ifdef	_BIG_ENDIAN
		dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));
#endif

#ifdef	_LITTLE_ENDIAN
	  out_p = &OUTBUF_AT(obj, o_idx);
	  for (k=0; k < sizeof(wchar_t); k++, out_p++)
	  {
		  dummy <<= 8; 
		  dummy |= (0x000000ff & (*out_p));
	  }
#endif
   }
  else
	dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));

  return (IsTlgMatraAtStemGlyph(obj, dummy));

}

static int
CheckIsTlgSideVattu(obj, o_idx)
  LayoutObj	obj;
  int		o_idx;
{
  char		*out_p;
  wchar_t	dummy = L'\0';
  int		k;
  
  if (o_idx < 1 )
	return FALSE;

  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
#ifdef	_BIG_ENDIAN
		dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));
#endif

#ifdef	_LITTLE_ENDIAN
	  out_p = &OUTBUF_AT(obj, o_idx);
	  for (k=0; k < sizeof(wchar_t); k++, out_p++)
	  {
		  dummy <<= 8; 
		  dummy |= (0x000000ff & (*out_p));
	  }
#endif
   }
  else
  	dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));
  
  return (IsOutputTlgSideVattu(obj, dummy) );

}



/* Punjabi */
static int
CheckIsPnValidSequence(obj, o_idx)
  LayoutObj	obj;
  int		o_idx;
{
  char		*out_p;
  
  wchar_t	dummy = L'\0';
  wchar_t	wcCons = L'\0';
  wchar_t	wcMatra = L'\0';

  int		k;
   if (LOGICALSTRMTYPE(obj) == FileCode)
    {
#ifdef	_BIG_ENDIAN
		dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));
#endif

#ifdef	_LITTLE_ENDIAN
	  out_p = &OUTBUF_AT(obj, o_idx);
	  for (k=0; k < sizeof(wchar_t); k++, out_p++)
	  {
		  dummy <<= 8; 
		  dummy |= (0x000000ff & (*out_p));
	  }
#endif
   }
  else
  	dummy = *((wchar_t *)(&OUTBUF_AT(obj, o_idx)));
	// Check if the last character in the output buffer is a vowel or a consonant
  if ( IsValidPnVowel(obj, dummy ) || IsValidPnCons(obj, dummy ) )
  {
	  return TRUE;
  }
  else if ( IsValidPnMatra(obj, dummy ) )	// Else check if the last character in the output buffer is a matra
  {
		// Check if the second last character in the woutput buffer is a consonant 
		if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
			 wcCons = *((wchar_t *)(&OUTBUF_AT(obj, o_idx -1 )));
#endif

#ifdef	_LITTLE_ENDIAN
			  out_p = &OUTBUF_AT(obj, o_idx -1 );
			  for (k=0; k < sizeof(wchar_t); k++, out_p++)
			  {
				  wcCons <<= 8; 
				  wcCons |= (0x000000ff & (*out_p));
			  }
#endif
		}
		else
  			wcCons = *((wchar_t *)(&OUTBUF_AT(obj, o_idx -1 )));

		
		if ( IsValidPnCons(obj, wcCons) )
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	
  }
  else if ( IsValidPnVowelMod(obj, dummy ) )	// Else check if the last character in the output buffer is a vowel modifier 
  {
	  // Check if the second last character in the output buffer is a consonant or a vowel 
	  // Else check if the second last character in the output buffer is a matra
		if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
			 wcCons = *((wchar_t *)(&OUTBUF_AT(obj, o_idx -1 )));
#endif

#ifdef	_LITTLE_ENDIAN
			  out_p = &OUTBUF_AT(obj, o_idx -1 );
			  for (k=0; k < sizeof(wchar_t); k++, out_p++)
			  {
				  wcCons <<= 8; 
				  wcCons |= (0x000000ff & (*out_p));
			  }
#endif
		}
		else
  			wcCons = *((wchar_t *)(&OUTBUF_AT(obj, o_idx -1 )));
		
			wcCons = wcMatra;

		if ( IsValidPnCons(obj, wcCons) ||  IsValidPnVowel(obj, wcCons) )
		{
			return TRUE;
		}
		else if ( IsValidPnMatra(obj, wcMatra) )
		{
			// Check if the third last character in the output buffer is a consonant 
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				wcCons = *((wchar_t *)(&OUTBUF_AT(obj, o_idx - 2 )));
#endif

#ifdef	_LITTLE_ENDIAN
				  out_p = &OUTBUF_AT(obj, o_idx - 2 );
				  for (k=0; k < sizeof(wchar_t); k++, out_p++)
				  {
					  wcCons <<= 8; 
					  wcCons |= (0x000000ff & (*out_p));
				  }
#endif
			}
			else
  				wcCons = *((wchar_t *)(&OUTBUF_AT(obj, o_idx - 2 )));

			if ( IsValidPnCons(obj, wcCons) )
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}

		}
		else
		{
			return FALSE;
		}
	
  }

}

static void
GetCoreConsGlyphs(obj)
  LayoutObj	obj;
{
  int	i, j, k;
  wchar_t temp_out;
  int	delta, nMin, nMaxFocBlockLength, nMapIndex;
  BooleanValue StillMatching;
  char	*out_p;
  u_int	dummy;

  i = 0;
  while (i < NumDevaCoreCons(obj))
    {
       nMaxFocBlockLength = GetDeva_nMaxFocBlockTbl(obj, \
				WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, DevaJJJ(obj) + i)));
       while (nMaxFocBlockLength)
	 {
	    nMin = MIN(nMaxFocBlockLength, NumDevaCoreCons(obj));

		nMapIndex = 0;

		NotFoundEntryYet:

		j = delta = 0;
		StillMatching = FALSE;
		while (((delta < nMin) ||
			 DEVA_GLYPH_TBL(obj)[nMapIndex].strISCII[j]) &&
			(nMapIndex < MAP_SIZE) )
		  {
		    StillMatching = TRUE;
		    if ((delta < nMin) &&
			(j < MAX_CORE_CONS_BUF) &&
			(WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, DevaJJJ(obj) + i + delta)) !=
			 DEVA_GLYPH_TBL(obj)[nMapIndex].strISCII[j]))
		      {
			nMapIndex++;
			goto NotFoundEntryYet;
		      }
		    delta++;
		    j++;
		  }

	    if (StillMatching)
		/* Found */
		break;
	    else
		nMaxFocBlockLength--;
	 }

      i += nMin;

/* What's if can't find entry in the table */
      if ((StillMatching == FALSE) || (nMapIndex >= MAP_SIZE))
	{
	   for (j=0; j < NumDevaCoreCons(obj); j++)
	     {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, DevaJJJ(obj) + j)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, DevaJJJ(obj) + j)) | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
		}
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, DevaJJJ(obj) + j)) | 0xE7000000;
	      O_IDX(obj)++;
	     }
	}
      else if (CheckDevaKernSpaceGlyphs(obj, O_IDX(obj) - 1) &&
	       IsDevaMatraAtStemGlyphs(obj, DEVA_GLYPH_TBL(obj)[nMapIndex].strISFOC[0]) )
	{
	  temp_out = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	  O_IDX(obj)--;

	  for (j=0; DEVA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
	    {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    DEVA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = DEVA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    DEVA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
	      O_IDX(obj)++;
	    }
	  OUTPUT_OUT2INP_CACHE_CHECK
	  if (LOGICALSTRMTYPE(obj) == FileCode)
	    {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = temp_out;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		  }
#endif
	    }
	  else
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out;
	  O_IDX(obj)++;
	}
      else
	{
	  for (j=0; DEVA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
	    {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
	        {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		  DEVA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = DEVA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	        }
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		  DEVA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
	      O_IDX(obj)++;
	    }
	}
    }
}

/* Tamil functions */ 


static void
GetTamilCoreConsGlyphs(obj)
  LayoutObj	obj;
{
  int	i, j, k;
  wchar_t temp_out;
  int	delta, nMin, nMaxFocBlockLength, nMapIndex;
  BooleanValue StillMatching;
  char	*out_p;
  u_int	dummy;

  i = 0;				// i denotes the iscii index 
		

  /* while (i < NumDevaCoreCons(obj))*/ 
	
	while (i < NumTamilCoreCons(obj) )
    {
       
		nMaxFocBlockLength = GetTamil_nMaxFocBlockTbl(obj, \
				WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + i)));

       while (nMaxFocBlockLength)
	 {
	    nMin = MIN(nMaxFocBlockLength, NumTamilCoreCons(obj) );

		nMapIndex = 0;

		NotFoundEntryYet:

		j = delta = 0;
		StillMatching = FALSE;
		while (((delta < nMin) || TAMIL_GLYPH_TBL(obj)[nMapIndex].strISCII[j])
				               && (nMapIndex < MAP_TAMIL_SIZE) )
		{
		    StillMatching = TRUE;
		     if ((delta < nMin) && (j < MAX_CORE_CONS_BUF) && 
				 (WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + i + delta)) != 
				  TAMIL_GLYPH_TBL(obj)[nMapIndex].strISCII[j]))
		     {
					nMapIndex++;
					goto NotFoundEntryYet;
		     }
		    delta++;
		    j++;
		}

	    if (StillMatching)
		/* Found */
		break;
	    else
		nMaxFocBlockLength--;
	 }

      i += nMin;

/* What's if can't find entry in the table */
      if ((StillMatching == FALSE) || (nMapIndex >= MAP_TAMIL_SIZE))
	{
	   for (j=0; j < NumTamilCoreCons(obj); j++)
	   {
				/* OutBuf */
			  OUTPUT_OUT2INP_CACHE_CHECK
			  if (LOGICALSTRMTYPE(obj) == FileCode)
			  {
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + j)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + j)) | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
			  }
			else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + j)) | 0xE7000000;
			O_IDX(obj)++;
	   }
	}
    
	else
	{
		for (j=0; TAMIL_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
	    {
			  /* OutBuf */
			  OUTPUT_OUT2INP_CACHE_CHECK
			  if (LOGICALSTRMTYPE(obj) == FileCode)
			  {
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
					  TAMIL_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = TAMIL_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					  {
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					  }
#endif
			  }
	          else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
					  TAMIL_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
	          O_IDX(obj)++;
		}
	
    
	}

 }

}




static void
GetGlyphsFromTamilCluster (obj)
  LayoutObj	obj;
{
  int		i, next_j;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  int		k, len;
  char		*out_p;
  u_int		dummy;

  FIRST_O_IDX(obj) = O_IDX(obj);
  NumTamilCoreCons(obj) = NChrsInCluster(obj);
  //TamilJJJ(obj) = JJJ(obj);

  /* Property, InpToOut */							// Check this part later 
  if (LOGICALSTRMTYPE(obj) == FileCode)
  {
		  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
				UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
		  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
		  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));

		  for (k=0; k < len; k++, prop_p++, i2o_p++)
		  {
			  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
			  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
		  }
      
		  next_j = JJJ(obj);
		  for (i = 1; i < NChrsInCluster(obj); i++)
		  {
			  next_j++;
			  if (next_j < END_HDRT(obj))
			  {
				  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)+1) -
			  	  	    UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)); 
			  }
			  else
				 len = 0;

			  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
			  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
			  for (k=0; k < len; k++, prop_p++, i2o_p++)
			  {
				  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
				  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
			  }
		  }
  }
  else
  {
		  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
  		  (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
		  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
		  next_j = JJJ(obj);
		  for (i = 1; i < NChrsInCluster(obj); i++)
			{
		  next_j++;

		  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, next_j)) =
			(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
		  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, next_j)) = O_IDX(obj);
			}
  }
 
  switch (TamilClusterTypeState(obj))
  {
	  case St1:
	  case St2:
	  case St3:
	  case St4:
	  case St7:
	  case St9:
		GetTamilCoreConsGlyphs(obj);
	  break;

	  case St5:
	  {
		// This case handles the case of repositioning of the matra.
		// This is achieved by first storing the matra to the output buffer and then calling the GetTamilCoreConsGlyph()
		// to get the glyph of the consonant.
		next_j = JJJ(obj) + 1;
		OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	    {
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j)) | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
				    out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = WCINPBUF_AT(obj,MAPPING_V2L_AT(obj, next_j )) | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j )) | 0xE7000000;

		O_IDX(obj)++;
		NumTamilCoreCons(obj) = NumTamilCoreCons(obj) - 1 ;
		GetTamilCoreConsGlyphs(obj);
		break;
	  }

	  case St6:
		{
			// This case handles the case of split matras
			// This is achieved by first putting the initial part of the matra into the output buffer, then get the glyph 
			// of the consonant and then appending the other half of the matra
			next_j = JJJ(obj)+ 1;
			if ( ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j)) == 0x0BCA) ||
				 ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j)) == 0x0BCC) )
			{ 
			
				OUTPUT_OUT2INP_CACHE_CHECK
				 if (LOGICALSTRMTYPE(obj) == FileCode)
				 {
#ifdef	_BIG_ENDIAN
					 *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000BC6 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
				    out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x00000BC6 | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000BC6 | 0xE7000000;

				O_IDX(obj)++;
			}
		    else if ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j)) == 0x0BCB )
			{
				 OUTPUT_OUT2INP_CACHE_CHECK
				 if (LOGICALSTRMTYPE(obj) == FileCode)
				 {
#ifdef	_BIG_ENDIAN
					 *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000BC7 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
				    out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x00000BC7 | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				 }
				 else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000BC7 | 0xE7000000;
				O_IDX(obj)++;
			}
			NumTamilCoreCons(obj) = NumTamilCoreCons(obj) - 1 ;
			GetTamilCoreConsGlyphs(obj);

			if ( ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j )) == 0x0BCA) ||
			     ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j )) == 0x0BCB) )
			{ 
			
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000BBE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj, O_IDX(obj));
							dummy = 0x00000BBE | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
					}
					else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000BBE | 0xE7000000;
					O_IDX(obj)++;

			}
			else if ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j)) == 0x0BCC)		
			{
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000BD7 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj, O_IDX(obj));
							dummy = 0x00000BD7 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
					}
					else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000BD7 | 0xE7000000;
					O_IDX(obj)++;

			}

		 break;
		}		// End of Case St6
		case St8:
		{

				if ( IsTamilCharCls(obj, WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j ) ) , _TMD ) )
				{
					GetTamilCoreConsGlyphs(obj);

				}
				else if ( ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j )) == 0x0BC6) ||
					 ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j )) == 0x0BC7) ||
					 ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,next_j )) == 0x0BC8) )
				{ 

						GetTamilCoreConsGlyphs(obj);
				
						OUTPUT_OUT2INP_CACHE_CHECK
						if (LOGICALSTRMTYPE(obj) == FileCode)
						{
#ifdef	_BIG_ENDIAN
								*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F766 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
								out_p = &OUTBUF_AT(obj, O_IDX(obj));
								dummy = 0x0000F766 | 0xE7000000;
								for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
								{
									*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
									dummy <<= 8;
								}
#endif
						}
						else
								*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F766 | 0xE7000000;
						O_IDX(obj)++;

						
				}
				else 
				{

						OUTPUT_OUT2INP_CACHE_CHECK
						if (LOGICALSTRMTYPE(obj) == FileCode)
						{
#ifdef	_BIG_ENDIAN
								*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F766 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
								out_p = &OUTBUF_AT(obj, O_IDX(obj));
								dummy = 0x0000F766 | 0xE7000000;
								for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
								{
									*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
									dummy <<= 8;
								}
#endif
						}
						else
								*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F766 | 0xE7000000;
						O_IDX(obj)++;

						GetTamilCoreConsGlyphs(obj);

			}
			break;
		}
			  
	}	 // End of Switch Case statement

	   /* OutToInp */
	  if (LOGICALSTRMTYPE(obj) == FileCode)
		{
		  for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
		{
		  o2i_p = &OUTTOINP_AT(obj, i*SHPNUMBYTES(obj));
		  for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
			*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
		}
		}
	  else
		  for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
			OUTTOINP_AT(obj, i) = MAPPING_V2L_AT(obj, JJJ(obj));

	  OUTSIZE(obj) = O_IDX(obj);					
}

/* Get Glyphs Ends here */


static void
NextCluster_Tamil(obj)
  LayoutObj		obj;
{
  wchar_t	wc_j = L'\0';
  wchar_t	wc_jj = L'\0';
  int		n_j;
 int nSriFlag = 0 ;


  if (NEXT_CLUSTER(obj) < END_HDRT(obj))
  {
		TamilClusterTypeState(obj) = St0;
		if (NChrsInCluster(obj) != -1)
			JJJ(obj) = NEXT_CLUSTER(obj);

		if (JJJ(obj) < END_HDRT(obj))
		{
			wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));		// Get the current character
			// Update the State value 
		    TamilClusterTypeState(obj) = UpdateTamilClusterTypeState(obj,TamilClusterTypeState(obj),wc_j);
		}
		else
			wc_j = L'\0';

		n_j = JJJ(obj) + 1;			/* Index to next character */ 
		if (n_j < END_HDRT(obj))
			wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
		else
			wc_jj = L'\0';

		NChrsInCluster(obj) = 1;
    	while ( (wc_j != L'\0') && (wc_jj != L'\0') && IsTamilComposible(obj, wc_j, wc_jj))
		{
			NChrsInCluster(obj)++;

			if ( wc_j == 0x0BB7)
				nSriFlag = 1;
			else
				nSriFlag = 0;

			wc_j = wc_jj;
			TamilClusterTypeState(obj) = UpdateTamilClusterTypeState(obj, TamilClusterTypeState(obj), wc_j);
			n_j++;				/* Increment the index to the next character */
			
			if (n_j < END_HDRT(obj))
				wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
			else
				wc_jj = L'\0';
			
			// To handle Sri condition
			// The first character in the cluster should be 0x0Bb7			---> nSriFlag = 1 
			// The next character should be ( halant ) 0x0BCD				---> wc_j
			// The next character should be 'Ra' ( 0x0BB0 )					---> wc_jj
			// And the next character should be badi Ii matra ( 0x0BC0)		---> value of inp buffer at (n_j + 1)
			if( (nSriFlag) && ( wc_j == 0x0BCD ) && ( wc_jj == 0x0BB0 ) )
			{
				if (n_j < END_HDRT(obj) && ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, ( n_j + 1 ) ) ) == 0x0BC0 ) ) 
				{
					n_j+= 2;
					NChrsInCluster(obj) += 2 ;
					TamilClusterTypeState(obj) = St9;
					nSriFlag = 0 ;
					break;
				}
			}
			else
				nSriFlag = 0 ;

		}
		NEXT_CLUSTER(obj) = n_j;

  }
  else
       NChrsInCluster(obj) = 0;
}



static	void
func_tamil(obj)
  LayoutObj		obj;
{
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Tamil(obj);
  while (NChrsInCluster(obj) > 0)
    {
      GetGlyphsFromTamilCluster (obj);
      NextCluster_Tamil (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
                                         : ReverseDirection_WC(obj) );
}


static void
GetCoreKannConsGlyphs(obj)
  LayoutObj	obj;
{
  int	i, j, k;
  wchar_t temp_out;
  int	delta, nMin, nMaxFocBlockLength, nMapIndex;
  BooleanValue StillMatching;
  char	*out_p;
  u_int	dummy;

  i = 0;
  while (i < NumKannCoreCons(obj))
    {
       nMaxFocBlockLength = GetKann_nMaxFocBlockTbl(obj, \
				WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, KannJJJ(obj) + i)));
       while (nMaxFocBlockLength)
	 {
	    nMin = MIN(nMaxFocBlockLength, NumKannCoreCons(obj));

		nMapIndex = 0;

		NotFoundEntryYet:

		j = delta = 0;
		StillMatching = FALSE;
		while (((delta < nMin) ||
			 KANN_GLYPH_TBL(obj)[nMapIndex].strISCII[j]) &&
			(nMapIndex < MAP_KANN_SIZE) )
		  {
		    StillMatching = TRUE;
		    if ((delta < nMin) &&
			(j < MAX_CORE_CONS_BUF) &&
			(WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, KannJJJ(obj) + i + delta)) !=
			 KANN_GLYPH_TBL(obj)[nMapIndex].strISCII[j]))
		      {
			nMapIndex++;
			goto NotFoundEntryYet;
		      }
		    delta++;
		    j++;
		  }

	    if (StillMatching)
		/* Found */
		break;
	    else
		nMaxFocBlockLength--;
	 }

      i += nMin;

/* What's if can't find entry in the table */
      if ((StillMatching == FALSE) || (nMapIndex >= MAP_KANN_SIZE))
	{
	   for (j=0; j < NumKannCoreCons(obj); j++)
	     {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, KannJJJ(obj) + j)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, KannJJJ(obj) + j)) | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
		}
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, KannJJJ(obj) + j)) | 0xE7000000;
	      O_IDX(obj)++;
	     }
	}
   else
	{
	  for (j=0; KANN_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
	    {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
	        {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		  KANN_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = KANN_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	        }
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		  KANN_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
	      O_IDX(obj)++;
	    }
	}
  }
}


static void
GetCoreGujaConsGlyphs(obj)
  LayoutObj	obj;
{
  int	i, j, k;
  wchar_t temp_out;
  int	delta, nMin, nMaxFocBlockLength, nMapIndex;
  BooleanValue StillMatching;
  char	*out_p;
  u_int	dummy;

  i = 0;
  while (i < NumGujaCoreCons(obj))
    {
       nMaxFocBlockLength = GetGuja_nMaxFocBlockTbl(obj, \
				WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, GujaJJJ(obj) + i)));
       while (nMaxFocBlockLength)
	 {
	    nMin = MIN(nMaxFocBlockLength, NumGujaCoreCons(obj));

		nMapIndex = 0;

		NotFoundEntryYet:

		j = delta = 0;
		StillMatching = FALSE;
		while (((delta < nMin) ||
			 GUJA_GLYPH_TBL(obj)[nMapIndex].strISCII[j]) &&
			(nMapIndex < MAP_GUJA_SIZE) )
		  {
		    StillMatching = TRUE;
		    if ((delta < nMin) &&
			(j < MAX_CORE_CONS_BUF) &&
			(WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, GujaJJJ(obj) + i + delta)) !=
			 GUJA_GLYPH_TBL(obj)[nMapIndex].strISCII[j]))
		      {
			nMapIndex++;
			goto NotFoundEntryYet;
		      }
		    delta++;
		    j++;
		  }

	    if (StillMatching)
		/* Found */
		break;
	    else
		nMaxFocBlockLength--;
	 }

      i += nMin;

/* What's if can't find entry in the table */
      if ((StillMatching == FALSE) || (nMapIndex >= MAP_GUJA_SIZE))
	{
	   for (j=0; j < NumGujaCoreCons(obj); j++)
	     {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, GujaJJJ(obj) + j)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, GujaJJJ(obj) + j)) | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
		}
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, GujaJJJ(obj) + j)) | 0xE7000000;
	      O_IDX(obj)++;
	     }
	}
      else if (CheckGujaKernSpaceGlyphs(obj, O_IDX(obj) - 1) &&
	       IsGujaMatraAtStemGlyphs(obj, GUJA_GLYPH_TBL(obj)[nMapIndex].strISFOC[0]) )
	{
	  temp_out = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	  O_IDX(obj)--;

	  for (j=0; GUJA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
	    {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    GUJA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = GUJA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
		}
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    GUJA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
	      O_IDX(obj)++;
	    }
	  OUTPUT_OUT2INP_CACHE_CHECK
	  if (LOGICALSTRMTYPE(obj) == FileCode)
	    {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = temp_out;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		  }
#endif
	    }
	  else
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out;
	  O_IDX(obj)++;
	}
      else
	{
	  for (j=0; GUJA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
	    {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
	        {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		  GUJA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = GUJA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	        }
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		  GUJA_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
	      O_IDX(obj)++;
	    }
	}
   }
}



static void
GetCoreBengConsGlyphs(obj)
  LayoutObj	obj;
{
  int	i, j, k;
  wchar_t temp_out;
  int	delta, nMin, nMaxFocBlockLength, nMapIndex;
  BooleanValue StillMatching;
  char	*out_p;
  u_int	dummy;

  i = 0;
  while (i < NumBengCoreCons(obj))
    {
       nMaxFocBlockLength = GetBeng_nMaxFocBlockTbl(obj, \
				WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, BengJJJ(obj) + i)));
       while (nMaxFocBlockLength)
	 {
	    nMin = MIN(nMaxFocBlockLength, NumBengCoreCons(obj));

		nMapIndex = 0;

		NotFoundEntryYet:

		j = delta = 0;
		StillMatching = FALSE;
		while (((delta < nMin) ||
			 BENG_GLYPH_TBL(obj)[nMapIndex].strISCII[j]) &&
			(nMapIndex < MAP_BENG_SIZE) )
		  {
		    StillMatching = TRUE;
		    if ((delta < nMin) &&
			(j < MAX_CORE_CONS_BUF) &&
			(WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, BengJJJ(obj) + i + delta)) !=
			 BENG_GLYPH_TBL(obj)[nMapIndex].strISCII[j]))
		      {
			nMapIndex++;
			goto NotFoundEntryYet;
		      }
		    delta++;
		    j++;
		  }

	    if (StillMatching)
		/* Found */
		break;
	    else
		nMaxFocBlockLength--;
	 }

      i += nMin;

/* What's if can't find entry in the table */
      if ((StillMatching == FALSE) || (nMapIndex >= MAP_BENG_SIZE))
	{
	   for (j=0; j < NumBengCoreCons(obj); j++)
	     {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, BengJJJ(obj) + j)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, BengJJJ(obj) + j)) | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
		}
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, BengJJJ(obj) + j)) | 0xE7000000;
	      O_IDX(obj)++;
	     }
	}
      else if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1) &&
	       IsBengMatraAtStemGlyphs(obj, BENG_GLYPH_TBL(obj)[nMapIndex].strISFOC[0]) )
	{
	  temp_out = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	  O_IDX(obj)--;

	  for (j=0; BENG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
	    {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    BENG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = BENG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
		}
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		    BENG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
	      O_IDX(obj)++;
	    }
	  OUTPUT_OUT2INP_CACHE_CHECK
	  if (LOGICALSTRMTYPE(obj) == FileCode)
	    {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = temp_out;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		  }
#endif
	    }
	  else
	    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out;
	  O_IDX(obj)++;
	}
      else
	{
	  for (j=0; BENG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
	    {
	      /* OutBuf */
	      OUTPUT_OUT2INP_CACHE_CHECK
	      if (LOGICALSTRMTYPE(obj) == FileCode)
	        {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		  BENG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = BENG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	        }
	      else
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		  BENG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
	      O_IDX(obj)++;
	    }
	}
   }
}



static void
GetGlyphsFromDevaCluster (obj)
  LayoutObj	obj;
{
  int		i, next_j;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  int		k, len;
  char		*out_p;
  u_int		dummy;

  FIRST_O_IDX(obj) = O_IDX(obj);
  NumDevaCoreCons(obj) = NChrsInCluster(obj);
  DevaJJJ(obj) = JJJ(obj);

  /* Property, InpToOut */
  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
      len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
            UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++, i2o_p++)
	{
	  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
	  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	}
      next_j = JJJ(obj);
      for (i = 1; i < NChrsInCluster(obj); i++)
        {
	  next_j++;
	  if (next_j < END_HDRT(obj))
	      len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)+1) -
		    UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j));
	  else
	      len = 0;
	  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
	  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
	  for (k=0; k < len; k++, prop_p++, i2o_p++)
	    {
	      *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    }
        }
    }
  else
    {
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
	(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      next_j = JJJ(obj);
      for (i = 1; i < NChrsInCluster(obj); i++)
        {
	  next_j++;

	  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, next_j)) =
		(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
	  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, next_j)) = O_IDX(obj);
        }
    }

  /* OutBuf */
  switch (DevaClusterTypeState(obj))
    {
      case St1:
	if (IsDevaCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)+ NChrsInCluster(obj) - 1)), _IM))
	  {
	    GetCoreConsGlyphs(obj);
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F7C0 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F7C0 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F7C0 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F7C0 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F7C0 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F7C0 | 0xE7000000;
	    O_IDX(obj)++;
	    GetCoreConsGlyphs(obj);
	  }
	break;

      case St2:
      case St3:
      case St4:
      case St5:
      case St6:
	GetCoreConsGlyphs(obj);
	break;

      case St7:
	if (IsDevaCharCls(obj, WCINPBUF_AT(obj,MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _UP))
	  {
	    if (IsDevaCharCls(obj,WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,JJJ(obj)+NChrsInCluster(obj)- 2)), _RM))
		NumDevaCoreCons(obj) -= 2;
	    GetCoreConsGlyphs(obj);
	    if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
			MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _RM))
		{
		   if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _II_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F81B | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F81B | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F81B | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _AYE_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F82D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
			dummy = 0x0000F82D | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F82D | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _EE_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F824 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F824 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F824 | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _EY_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F827 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F827 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F827 | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _AI_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82A | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F82A | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82A | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _AWE_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000093E | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
		     O_IDX(obj)++;
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F82D | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82D | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _O_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000093E | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
		     O_IDX(obj)++;
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F824 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F824 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F824 | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _OW1_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000093E | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
		     O_IDX(obj)++;
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F827 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F827 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F827 | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _OW2_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000093E | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
		     O_IDX(obj)++;
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82A | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F82A | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82A | 0xE7000000;
		     O_IDX(obj)++;
		   }
		}
	  }
	else
	  GetCoreConsGlyphs(obj);
	break;

      case St8:
	NumDevaCoreCons(obj)--;
	GetCoreConsGlyphs(obj);
	if (CheckDevaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
		dummy = 
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St9:
	if (IsDevaCharCls(obj,
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _MS))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093F | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000093F | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093F | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F817 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F817 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F817 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	NumDevaCoreCons(obj)--; 
	GetCoreConsGlyphs(obj);
	break;

      case St10:
	if (IsDevaCharCls(obj,
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _MS))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F814 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F814 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F814 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F818 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F818 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F818 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	NumDevaCoreCons(obj) -= 2;
	GetCoreConsGlyphs(obj);
	break;

      case St11:
	GetCoreConsGlyphs(obj);
	break;

      case St12:
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000930 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x00000930 | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000930 | 0xE7000000;
	O_IDX(obj)++;
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F83A | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x0000F83A | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F83A | 0xE7000000;
	O_IDX(obj)++;
	break;

      case St13:
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000930 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x00000930 | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000930 | 0xE7000000;
	O_IDX(obj)++;
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000094D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x0000094D | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000094D | 0xE7000000;
	O_IDX(obj)++;
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F83A | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x0000F83A | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F83A | 0xE7000000;
	O_IDX(obj)++;
	break;

      case St14:
	DevaJJJ(obj) += 2;
	NumDevaCoreCons(obj) -= 2;
	GetCoreConsGlyphs(obj);
	if (CheckDevaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F812 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
		dummy = 0x0000F812 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F812 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F812 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F812 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F812 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St15:
	NumDevaCoreCons(obj)--;
	DevaJJJ(obj) += 2;
	NumDevaCoreCons(obj) -= 2;
	GetCoreConsGlyphs(obj);
	if (CheckDevaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000094D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000094D | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000094D | 0xE7000000;
	    O_IDX(obj)++;
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 2)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 2))) = 0x0000F812 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 2);
		dummy = 0x0000F812 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 2))) = 0x0000F812 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F812 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F812 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F812 | 0xE7000000;
	    O_IDX(obj)++;
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000094D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000094D | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000094D | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St16:
	if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _RM))
	    NumDevaCoreCons(obj)--;
	DevaJJJ(obj) += 2;
	NumDevaCoreCons(obj) -= 2;
	GetCoreConsGlyphs(obj);
	if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), ~(_RM)))
	  {
	    if (CheckDevaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	      {
		OUTPUT_OUT2INP_CACHE_CHECK
		dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & dummy);
		        dummy >>= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F812 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
		    dummy = 0x0000F812 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F812 | 0xE7000000;
		O_IDX(obj)++;
	      }
	    else
	      {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F812 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F812 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F812 | 0xE7000000;
		O_IDX(obj)++;
	      }
	  }
	else
	  {
	    if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _II_M)) {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F81C | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F81C | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F81C | 0xE7000000;
		O_IDX(obj)++;
	    } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _EY_M)) {
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F828 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F828 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F828 | 0xE7000000;
		O_IDX(obj)++;
	    } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _AI_M)) {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82B | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F82B | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82B | 0xE7000000;
		O_IDX(obj)++;
	    } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _OW1_M)) {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000093E | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
		O_IDX(obj)++;
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F828 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F828 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F828 | 0xE7000000;
		O_IDX(obj)++;
	    } else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _OW2_M)) {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000093E | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
		O_IDX(obj)++;
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82B | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F82B | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82B | 0xE7000000;
		O_IDX(obj)++;
	    }
	  }
	break;

      case St17:
	if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _UP))
	  {
	    if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
			MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _RM))
		NumDevaCoreCons(obj) -= 2;
	    else
		NumDevaCoreCons(obj) -= 1;
	    DevaJJJ(obj) += 2;
	    NumDevaCoreCons(obj) -= 2;
	    GetCoreConsGlyphs(obj);
	    if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
			MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), ~(_RM)))
	      {
		if (CheckDevaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
		  {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & dummy);
		            dummy >>= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F813 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
			dummy = 0x0000F813 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F813 | 0xE7000000;
		    O_IDX(obj)++;
		  }
		else
		  {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F813 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F813 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F813 | 0xE7000000;
		    O_IDX(obj)++;
		  }
	      }
	    else
	      {
		if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		    MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _II_M)) {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F81D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F81D | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
	              }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F81D | 0xE7000000;
		    O_IDX(obj)++;
		} else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		    MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _EY_M)) {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F829 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F829 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F829 | 0xE7000000;
		    O_IDX(obj)++;
		} else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		    MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _AI_M)) {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82C | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F82C | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82C | 0xE7000000;
		    O_IDX(obj)++;
		} else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		    MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _OW1_M)) {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000093E | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
		    O_IDX(obj)++;
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F829 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F829 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F829 | 0xE7000000;
		    O_IDX(obj)++;
		} else if (IsDevaCharCls(obj, WCINPBUF_AT(obj,
		    MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _OW2_M)) {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000093E | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000093E | 0xE7000000;
		    O_IDX(obj)++;
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82C | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F82C | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F82C | 0xE7000000;
		    O_IDX(obj)++;
		}
	      }
	  } /* ? else GetCoreConsGlyphs(obj); */
	break;

      case St18:
	NumDevaCoreCons(obj)--;
	DevaJJJ(obj) += 2;
	NumDevaCoreCons(obj) -= 2;
	GetCoreConsGlyphs(obj);
	if (CheckDevaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;

	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F813 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
		dummy = 0x0000F813 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F813 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F813 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F813 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F813 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St19:
	if (IsDevaCharCls(obj,
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _MS))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F815 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F815 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F815 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F819 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F819 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F819 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	NumDevaCoreCons(obj)--;
	DevaJJJ(obj) += 2;
	NumDevaCoreCons(obj) -= 2;
	GetCoreConsGlyphs(obj);
	break;

      case St20:
	if (IsDevaCharCls(obj,
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _MS))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F816 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F816 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F816 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F81A | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F81A | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F81A | 0xE7000000;
	    O_IDX(obj)++;
	  }
	NumDevaCoreCons(obj) -= 2;
	DevaJJJ(obj) += 2;
	NumDevaCoreCons(obj) -= 2;
	GetCoreConsGlyphs(obj);
	break;
    }

  /* OutToInp */
  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
      for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
	{
	  o2i_p = &OUTTOINP_AT(obj, i*SHPNUMBYTES(obj));
	  for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	    *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	}
    }
  else
      for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
	OUTTOINP_AT(obj, i) = MAPPING_V2L_AT(obj, JJJ(obj));
  OUTSIZE(obj) = O_IDX(obj);
}

static void
GetGlyphsFromKannCluster (obj)
  LayoutObj	obj;
{
  int		i, next_j;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  int		k, len;
  char		*out_p;
  u_int		dummy;

  /* Following 5 variables added by Sachin For Case 10 & 11 */
  wchar_t  temp_out_char=0x0;
  wchar_t  temp_in_char=0x0;
  wchar_t  temp_in_char_vm=0x0; // Used for case 11
//  unsigned int  temp_out_char=0x0;
//  unsigned int  temp_in_char=0x0;
  unsigned int test_flag=0;
  unsigned int NumCharsAtStart=0;


  FIRST_O_IDX(obj) = O_IDX(obj);
  NumKannCoreCons(obj) = NChrsInCluster(obj);
  KannJJJ(obj) = JJJ(obj);

  /* Added by Sachin For Case 10 & 11 */
  NumCharsAtStart = NChrsInCluster(obj);


  /* Property, InpToOut */
  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
      len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
            UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++, i2o_p++)
	{
	  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
	  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	}
      next_j = JJJ(obj);
      for (i = 1; i < NChrsInCluster(obj); i++)
        {
	  next_j++;
	  if (next_j < END_HDRT(obj))
	      len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)+1) -
		    UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j));
	  else
	      len = 0;
	  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
	  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
	  for (k=0; k < len; k++, prop_p++, i2o_p++)
	    {
	      *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    }
        }
    }
  else
    {
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
	(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      next_j = JJJ(obj);
      for (i = 1; i < NChrsInCluster(obj); i++)
        {
	  next_j++;

	  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, next_j)) =
		(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
	  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, next_j)) = O_IDX(obj);
        }
    }

  /* OutBuf */
  switch (KannClusterTypeState(obj))
    {
      case St1:
	
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F6C1 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F6C1 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F6C1 | 0xE7000000;
	    O_IDX(obj)++;
	    GetCoreKannConsGlyphs(obj);
	  
	break;

      case St2:
      case St3:
      case St4:
      case St5:
      case St6:
      case St7:
      case St8:
      case St9:
	GetCoreKannConsGlyphs(obj);
	break;

      case St10:

			  temp_in_char = WCINPBUF_AT(obj,1);
			  
			  WCINPBUF_AT(obj,1) = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, KannJJJ(obj) + NChrsInCluster(obj) - 1) ) ) ;

			  NumKannCoreCons(obj) = 2 ;
			  GetCoreKannConsGlyphs(obj);

			  if((*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) == 0x00000cd5 | 0xE7000000) ||
				 (*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) == 0x00000cd6 | 0xE7000000) ||
				 (*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) == 0x0000f687 | 0xE7000000) )

			  {
				  temp_out_char = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) -1)) ) ;
				  O_IDX(obj)--;
				  test_flag=1;
			  }

			  WCINPBUF_AT(obj,1) =  temp_in_char;
			  KannJJJ(obj) = KannJJJ(obj) + 1;
			  NumKannCoreCons(obj) = NumCharsAtStart - 2 ;
			  GetCoreKannConsGlyphs(obj);

			  if( (test_flag = 1) && (temp_out_char != 0x0) )
			  {
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out_char;
					O_IDX(obj)++;
			  }
			  

			  break;

      case St11:
			  temp_in_char_vm  = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, KannJJJ(obj) + NChrsInCluster(obj) - 1) ) ) ;

			  temp_in_char = WCINPBUF_AT(obj,1);

			  WCINPBUF_AT(obj,1) = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, KannJJJ(obj) + NChrsInCluster(obj) - 2) ) ) ;
			
			  NumKannCoreCons(obj) = 2 ;
			  GetCoreKannConsGlyphs(obj);

			  if((*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) == 0x00000cd5 | 0xE7000000) ||
				 (*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) == 0x00000cd6 | 0xE7000000) ||
				 (*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) == 0x0000f687 | 0xE7000000) )
			  {
				  temp_out_char = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) -1)) ) ;
				  O_IDX(obj)--;
				  test_flag=1;
			  }

			  WCINPBUF_AT(obj,1) =  temp_in_char;
			  KannJJJ(obj) = KannJJJ(obj) + 1;
			  NumKannCoreCons(obj) = NumCharsAtStart - 3 ;
			  GetCoreKannConsGlyphs(obj);

			  if( (test_flag = 1) && (temp_out_char != 0x0) )
			  {
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out_char;
					O_IDX(obj)++;
			  }

			  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out_char | 0xE7000000;
			  O_IDX(obj)++;
			  
			  break;

      case St12:
	GetCoreKannConsGlyphs(obj);
	break;

      case St13:
	GetCoreKannConsGlyphs(obj);
	break;


  }
    /* OutToInp */
  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
      for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
	{
	  o2i_p = &OUTTOINP_AT(obj, i*SHPNUMBYTES(obj));
	  for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	    *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	}
    }
  else
      for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
	OUTTOINP_AT(obj, i) = MAPPING_V2L_AT(obj, JJJ(obj));
  OUTSIZE(obj) = O_IDX(obj);

}



static void
GetGlyphsFromGujaCluster (obj)
  LayoutObj	obj;
{
  int		i, next_j;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  int		k, len;
  char		*out_p;
  u_int		dummy;

  FIRST_O_IDX(obj) = O_IDX(obj);
  NumGujaCoreCons(obj) = NChrsInCluster(obj);
  GujaJJJ(obj) = JJJ(obj);

  /* Property, InpToOut */
  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
      len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
            UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++, i2o_p++)
	{
	  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
	  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	}
      next_j = JJJ(obj);
      for (i = 1; i < NChrsInCluster(obj); i++)
        {
	  next_j++;
	  if (next_j < END_HDRT(obj))
	      len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)+1) -
		    UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j));
	  else
	      len = 0;
	  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
	  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
	  for (k=0; k < len; k++, prop_p++, i2o_p++)
	    {
	      *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    }
        }
    }
  else
    {
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
	(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      next_j = JJJ(obj);
      for (i = 1; i < NChrsInCluster(obj); i++)
        {
	  next_j++;

	  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, next_j)) =
		(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
	  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, next_j)) = O_IDX(obj);
        }
    }

  /* OutBuf */
  switch (GujaClusterTypeState(obj))
    {
      case St1:
	if (IsGujaCharCls(obj,
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)+ NChrsInCluster(obj) - 1)), _GJ_I_M))
	  {
	    GetCoreGujaConsGlyphs(obj);
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F4EB | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F4EB | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F4EB | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F4EB | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F4EB | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F4EB | 0xE7000000;
	    O_IDX(obj)++;
	    GetCoreGujaConsGlyphs(obj);
	  }
	break;

      case St2:
      case St3:
      case St4:
      case St5:
      case St6:
		GetCoreGujaConsGlyphs(obj);
		break;


      case St7:
	if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _GJ_SP_VM))
	  {
	    if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
			MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_RM))
		NumGujaCoreCons(obj) -= 2;
	    GetCoreGujaConsGlyphs(obj);
	    if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
			MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_RM))
		{
		   if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_II_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F546 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F546 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F546 | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_AYE_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F556 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
			dummy = 0x0000F556 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F556 | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_EY_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F550 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F550 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F550 | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_AI_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F553 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F553 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F553 | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_AWE_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x00000ABE | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
		     O_IDX(obj)++;
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F556 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F556 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F556 | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_OW1_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x00000ABE | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
		     O_IDX(obj)++;
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F550 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F550 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F550 | 0xE7000000;
		     O_IDX(obj)++;
		   } else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_OW2_M)) {
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x00000ABE | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
		     O_IDX(obj)++;
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F553 | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F553 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F553 | 0xE7000000;
		     O_IDX(obj)++;
		   }
		}
	  }
	else
	  GetCoreGujaConsGlyphs(obj);
	break;

      case St8:
	NumGujaCoreCons(obj)--;
	GetCoreGujaConsGlyphs(obj);
	if (CheckGujaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
		dummy = 
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St9:
	if (IsGujaCharCls(obj,
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _GJ_MS))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABF | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x00000ABF | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABF | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F542 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F817 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F542 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	NumGujaCoreCons(obj)--; 
	GetCoreGujaConsGlyphs(obj);
	break;

      case St10:
	if (IsGujaCharCls(obj,
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _GJ_MS))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53F | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F53F | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53F | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F543 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F543 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F543 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	NumGujaCoreCons(obj) -= 2;
	GetCoreGujaConsGlyphs(obj);
	break;

      case St11:
	GetCoreGujaConsGlyphs(obj);
	break;

      case St12:
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000AB0 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x00000AB0 | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000AB0 | 0xE7000000;
	O_IDX(obj)++;
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F564 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x0000F564 | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F564 | 0xE7000000;
	O_IDX(obj)++;
	break;

      case St13:
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000AB0 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x00000AB0 | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000AB0 | 0xE7000000;
	O_IDX(obj)++;
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ACD | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x00000ACD | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ACD | 0xE7000000;
	O_IDX(obj)++;
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F564 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x0000F564 | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F564 | 0xE7000000;
	O_IDX(obj)++;
	break;

      case St14:
	GujaJJJ(obj) += 2;
	NumGujaCoreCons(obj) -= 2;
	GetCoreGujaConsGlyphs(obj);
	if (CheckGujaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F53D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
		dummy = 0x0000F53D | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F53D | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F53D | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53D | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St15:
	NumGujaCoreCons(obj)--;
	GujaJJJ(obj) += 2;
	NumGujaCoreCons(obj) -= 2;
	GetCoreGujaConsGlyphs(obj);
	if (CheckGujaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ACD | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x00000ACD | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ACD | 0xE7000000;
	    O_IDX(obj)++;
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 2)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 2))) = 0x0000F53D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 2);
		dummy = 0x0000F53D | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 2))) = 0x0000F53D | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F53D | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53D | 0xE7000000;
	    O_IDX(obj)++;
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ACD | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x00000ACD | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ACD | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St16:
	if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _GJ_RM))
	    NumGujaCoreCons(obj)--;
	GujaJJJ(obj) += 2;
	NumGujaCoreCons(obj) -= 2;
	GetCoreGujaConsGlyphs(obj);
	if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), ~(_GJ_RM)))
	  {
	    if (CheckGujaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	      {
		OUTPUT_OUT2INP_CACHE_CHECK
		dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		    *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & dummy);
		        dummy >>= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F53D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
		    dummy = 0x0000F53D | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F53D | 0xE7000000;
		O_IDX(obj)++;
	      }
	    else
	      {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F53D | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53D | 0xE7000000;
		O_IDX(obj)++;
	      }
	  }
	else
	  {
	    if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _GJ_II_M)) {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F547 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F547 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F547 | 0xE7000000;
		O_IDX(obj)++;
	    } else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _GJ_EY_M)) {
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F557 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F557 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F557 | 0xE7000000;
		O_IDX(obj)++;
	    } else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _GJ_AI_M)) {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F554 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F554 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F554 | 0xE7000000;
		O_IDX(obj)++;
	    }else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _GJ_AWE_M)) {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x00000ABE | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
		O_IDX(obj)++;
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F557 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F557 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F557 | 0xE7000000;
		O_IDX(obj)++;
	    }else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _GJ_OW1_M)) {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x00000ABE | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
		O_IDX(obj)++;
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F551 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F551 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F551 | 0xE7000000;
		O_IDX(obj)++;
	    } else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _GJ_OW2_M)) {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x00000ABE | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
		O_IDX(obj)++;
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F554 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F554 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F554 | 0xE7000000;
		O_IDX(obj)++;
	    }
	  }
	break;

      case St17:
	if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _GJ_SP_VM))
	  {
	    if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
			MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_RM))
		NumGujaCoreCons(obj) -= 2;
	    else
		NumGujaCoreCons(obj) -= 1;
	    GujaJJJ(obj) += 2;
	    NumGujaCoreCons(obj) -= 2;
	    GetCoreGujaConsGlyphs(obj);
	    if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
			MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), ~(_GJ_RM)))
	      {
		if (CheckGujaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
		  {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & dummy);
		            dummy >>= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F53E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
			dummy = 0x0000F53E | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F53E | 0xE7000000;
		    O_IDX(obj)++;
		  }
		else
		  {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F53E | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53E | 0xE7000000;
		    O_IDX(obj)++;
		  }
	      }
	    else
	      {
		if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		    MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_II_M)) {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F548 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F548 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
	              }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F548 | 0xE7000000;
		    O_IDX(obj)++;
		} else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		    MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_EY_M)) {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F558 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F558 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F558 | 0xE7000000;
		    O_IDX(obj)++;
		} else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		    MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_AI_M)) {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F555 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F555 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F555 | 0xE7000000;
		    O_IDX(obj)++;
		}else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _GJ_OW1_M)) {
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x00000ABE | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
		O_IDX(obj)++;
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		  {
#ifdef	_BIG_ENDIAN
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F558 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		    out_p = &OUTBUF_AT(obj, O_IDX(obj));
		    dummy = 0x0000F558 | 0xE7000000;
		    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		      {
		        *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		        dummy <<= 8;
		      }
#endif
		  }
		else
		  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F558 | 0xE7000000;
		O_IDX(obj)++;
	    }else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		    MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_OW1_M)) {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x00000ABE | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
		    O_IDX(obj)++;
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F552 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F552 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F552 | 0xE7000000;
		    O_IDX(obj)++;
		} else if (IsGujaCharCls(obj, WCINPBUF_AT(obj,
		    MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _GJ_OW2_M)) {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x00000ABE | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x00000ABE | 0xE7000000;
		    O_IDX(obj)++;
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F555 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F555 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
		      }
		    else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F555 | 0xE7000000;
		    O_IDX(obj)++;
		}
	      }
	  } /* ? else GetCoreConsGlyphs(obj); */
	break;

      case St18:
	NumGujaCoreCons(obj)--;
	GujaJJJ(obj) += 2;
	NumGujaCoreCons(obj) -= 2;
	GetCoreGujaConsGlyphs(obj);
	if (CheckGujaKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;

	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F53E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
		dummy = 0x0000F53E | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F53E | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53E | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F53E | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F53E | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St19:
	if (IsGujaCharCls(obj,
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _GJ_MS))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F540 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F540 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F540 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F544 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F544 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F544 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	NumGujaCoreCons(obj)--;
	GujaJJJ(obj) += 2;
	NumGujaCoreCons(obj) -= 2;
	GetCoreGujaConsGlyphs(obj);
	break;

      case St20:
	if (IsGujaCharCls(obj,
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))), _GJ_MS))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F541 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F541 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F541 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F545 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F545 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		 {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		 }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F545 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	NumGujaCoreCons(obj) -= 2;
	GujaJJJ(obj) += 2;
	NumGujaCoreCons(obj) -= 2;
	GetCoreGujaConsGlyphs(obj);
	break;
    }

  /* OutToInp */
  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
      for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
	{
	  o2i_p = &OUTTOINP_AT(obj, i*SHPNUMBYTES(obj));
	  for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	    *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	}
    }
  else
      for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
	OUTTOINP_AT(obj, i) = MAPPING_V2L_AT(obj, JJJ(obj));
  OUTSIZE(obj) = O_IDX(obj);
}


static void
GetGlyphsFromBengCluster (obj)
  LayoutObj	obj;
{
  int		i, next_j;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  int		k, len;
  char		*out_p;
  u_int		dummy;

  FIRST_O_IDX(obj) = O_IDX(obj);
  NumBengCoreCons(obj) = NChrsInCluster(obj);
  BengJJJ(obj) = JJJ(obj);

  /* Property, InpToOut */
  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
      len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
            UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
      prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
      for (k=0; k < len; k++, prop_p++, i2o_p++)
	{
	  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
	  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	}
      next_j = JJJ(obj);
      for (i = 1; i < NChrsInCluster(obj); i++)
        {
	  next_j++;
	  if (next_j < END_HDRT(obj))
	      len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)+1) -
		    UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j));
	  else
	      len = 0;
	  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
	  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
	  for (k=0; k < len; k++, prop_p++, i2o_p++)
	    {
	      *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
	      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
	    }
        }
    }
  else
    {
      PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
	(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
      next_j = JJJ(obj);
      for (i = 1; i < NChrsInCluster(obj); i++)
        {
	  next_j++;

	  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, next_j)) =
		(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
	  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, next_j)) = O_IDX(obj);
        }
    }

  /* OutBuf */
  switch (BengClusterTypeState(obj))
    {
      case St1:
	if ((IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)+ NChrsInCluster(obj) - 1)), _BN_I_M))||
	 (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)+ NChrsInCluster(obj) - 1)), _BN_EY_M)) ||
	 (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)+ NChrsInCluster(obj) - 1)), _BN_AI_M)) )
	  {
	    GetCoreBengConsGlyphs(obj);
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F360 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F360 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F360 | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else if((IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)+ NChrsInCluster(obj) - 1)), _BN_OW1_M))) 
	{
		OUTPUT_OUT2INP_CACHE_CHECK
            if (LOGICALSTRMTYPE(obj) == FileCode)
              {
#ifdef  _BIG_ENDIAN
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
#endif

#ifdef  _LITTLE_ENDIAN
                out_p = &OUTBUF_AT(obj, O_IDX(obj));
                dummy = 0x0000F36A | 0xE7000000;
                for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
                  {
                    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
                    dummy <<= 8;
                  }
#endif
              }
            else
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
            O_IDX(obj)++;

            if (LOGICALSTRMTYPE(obj) == FileCode)
              {
#ifdef  _BIG_ENDIAN
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F360 | 0xE7000000;
#endif

#ifdef  _LITTLE_ENDIAN
                out_p = &OUTBUF_AT(obj, O_IDX(obj));
                dummy = 0x0000F360 | 0xE7000000;
                for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
                  {
                    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
                    dummy <<= 8;
                  }
#endif
              }
            else
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F360 | 0xE7000000;
            O_IDX(obj)++;

            if (LOGICALSTRMTYPE(obj) == FileCode)
              {
#ifdef  _BIG_ENDIAN
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BE | 0xE7000000;
#endif

#ifdef  _LITTLE_ENDIAN
                out_p = &OUTBUF_AT(obj, O_IDX(obj));
                dummy = 0x000009BE | 0xE7000000;
                for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
                  {
                    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
                    dummy <<= 8;
                  }
#endif
              }
            else
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BE | 0xE7000000;
            O_IDX(obj)++;

	}
       else if((IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)+ NChrsInCluster(obj) - 1)), _BN_OW2_M))) 
        {
                OUTPUT_OUT2INP_CACHE_CHECK
            if (LOGICALSTRMTYPE(obj) == FileCode)
              {
#ifdef  _BIG_ENDIAN
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
#endif

#ifdef  _LITTLE_ENDIAN
                out_p = &OUTBUF_AT(obj, O_IDX(obj));
                dummy = 0x0000F36A | 0xE7000000;
                for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
                  {
                    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
                    dummy <<= 8;
                  }
#endif
              }
            else
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
            O_IDX(obj)++;

            if (LOGICALSTRMTYPE(obj) == FileCode)
              {
#ifdef  _BIG_ENDIAN
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F360 | 0xE7000000;
#endif

#ifdef  _LITTLE_ENDIAN
                out_p = &OUTBUF_AT(obj, O_IDX(obj));
                dummy = 0x0000F360 | 0xE7000000;
                for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
                  {
                    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
                    dummy <<= 8;
                  }
#endif
              }
            else
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F360 | 0xE7000000;
            O_IDX(obj)++;

            if (LOGICALSTRMTYPE(obj) == FileCode)
              {
#ifdef  _BIG_ENDIAN
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009D7 | 0xE7000000;
#endif

#ifdef  _LITTLE_ENDIAN
                out_p = &OUTBUF_AT(obj, O_IDX(obj));
                dummy = 0x000009D7 | 0xE7000000;
                for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
                  {
                    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
                    dummy <<= 8;
                  }
#endif
              }
            else
              *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009D7 | 0xE7000000;
            O_IDX(obj)++;

        }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F360 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F360 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F360 | 0xE7000000;
	    O_IDX(obj)++;
	    GetCoreBengConsGlyphs(obj);
	  }
	break;

      case St2:
      case St3:
      case St4:
      case St5:
      case St6:
		GetCoreBengConsGlyphs(obj);
		break;


      case St7:
	if (IsBengCharCls(obj, WCINPBUF_AT(obj,
		MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _BN_SP_VM))
	  {
	    if (IsBengCharCls(obj, WCINPBUF_AT(obj,
		       MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _BN_II_M))
		{
		    NumBengCoreCons(obj) -= 2;
		    GetCoreBengConsGlyphs(obj);
		     OUTPUT_OUT2INP_CACHE_CHECK
		     if (LOGICALSTRMTYPE(obj) == FileCode)
		       {
#ifdef	_BIG_ENDIAN
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F364 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F364 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
			    dummy <<= 8;
			  }
#endif
		       }
		     else
		       *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F364 | 0xE7000000;
		     O_IDX(obj)++;
		 }
		else
		{
			NumBengCoreCons(obj) -= 1;
		    GetCoreBengConsGlyphs(obj);
		    
			if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
			{
				OUTPUT_OUT2INP_CACHE_CHECK
					dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & dummy);
						dummy >>= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
						JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
					dummy = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
						JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
					JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
				O_IDX(obj)++;
			}
			else
			{
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
						JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj) );
					dummy = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
						JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
					JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
				O_IDX(obj)++;

			}
		}
	}
	else
		GetCoreBengConsGlyphs(obj);
		break;

      case St8:
	NumBengCoreCons(obj)--;
	GetCoreBengConsGlyphs(obj);
	if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
		dummy = 
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
		WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
		JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St9:
	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_I_M))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BF | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x000009BF | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BF | 0xE7000000;
	    O_IDX(obj)++;
	  }
	
	if ((IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_EY_M)) ||
		(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_OW1_M)) ||
		(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_OW2_M)) 
		)
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F36A | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
	    O_IDX(obj)++;
	  }
	
	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_AI_M))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36B | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F36B | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36B | 0xE7000000;
	    O_IDX(obj)++;
	  }
	
	NumBengCoreCons(obj)--; 
	GetCoreBengConsGlyphs(obj);
	
	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_OW1_M))
	{
		if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x000009BE | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BE | 0xE7000000;
		O_IDX(obj)++;
	}
	
	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_OW2_M))
	{
		if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009D7 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x000009D7 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009D7 | 0xE7000000;
		O_IDX(obj)++;
	}
	


	break;

      case St10:
	if (IsBengCharCls(obj, WCINPBUF_AT(obj,MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _BN_I_M))
	{
		if (IsBengCharCls(obj,WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_SP_VM))
		{
			OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F362 | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x0000F362 | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F362 | 0xE7000000;
				O_IDX(obj)++;
				NumBengCoreCons(obj) -= 2;
				GetCoreBengConsGlyphs(obj);
				break;
		}
		else
		{
			OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BF | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x000009BF | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BF | 0xE7000000;
				O_IDX(obj)++;
			NumBengCoreCons(obj) -= 2;
			GetCoreBengConsGlyphs(obj);
			OUTPUT_OUT2INP_CACHE_CHECK
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =   
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy =  WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000; 
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =   
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
			O_IDX(obj)++;
			break;
		}
	}
	else
	{
		if ((IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_EY_M)) ||
			(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_OW1_M))||
			(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_OW2_M)) 
			)
		{
			OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x0000F36A | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
				O_IDX(obj)++;
		}
		if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_AI_M))
		{
			OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36B | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x0000F36B | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36B | 0xE7000000;
				O_IDX(obj)++;
		}

		NumBengCoreCons(obj)-=2; 
		GetCoreBengConsGlyphs(obj);

		if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_OW1_M))
		{
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = 0x000009BE | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BE | 0xE7000000;
			O_IDX(obj)++;
		}
		if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)+ NChrsInCluster(obj)-2)), _BN_OW2_M))
		{
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009D7 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = 0x000009D7 | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009D7 | 0xE7000000;
			O_IDX(obj)++;
		}

		if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
		{
			OUTPUT_OUT2INP_CACHE_CHECK
				dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & dummy);
					dummy >>= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 
							WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
				dummy = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
					JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
				WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
				JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
			O_IDX(obj)++;
		}
		else
		{
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
					JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj) );
				dummy = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
					JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
				WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,
				JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
			O_IDX(obj)++;

		} 
	}

	
	break;

      case St11:
			GetCoreBengConsGlyphs(obj);
			break;

      case St12:
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009B0 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x000009B0 | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009B0 | 0xE7000000;
	O_IDX(obj)++;
	break;

      case St13:
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009B0 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x000009B0 | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009B0 | 0xE7000000;
	O_IDX(obj)++;
	OUTPUT_OUT2INP_CACHE_CHECK
	if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
#ifdef	_BIG_ENDIAN
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009CD | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
	    out_p = &OUTBUF_AT(obj, O_IDX(obj));
	    dummy = 0x000009CD | 0xE7000000;
	    for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
	     {
		*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		dummy <<= 8;
	     }
#endif
	  }
	else
	  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009CD | 0xE7000000;
	O_IDX(obj)++;

	break;

      case St14:
	BengJJJ(obj) += 2;
	NumBengCoreCons(obj) -= 2;
	GetCoreBengConsGlyphs(obj);
	if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	        *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & dummy);
		    dummy >>= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
		dummy = 0x0000F36D | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F36D | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St15:
	NumBengCoreCons(obj)--;
	BengJJJ(obj) += 2;
	NumBengCoreCons(obj) -= 2;
	GetCoreBengConsGlyphs(obj);
	if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009CD | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x000009CD | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009CD | 0xE7000000;
	    O_IDX(obj)++;
	    OUTPUT_OUT2INP_CACHE_CHECK
	    dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 2)));
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
		*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 2))) = 0x0000F36D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj) - 2);
		dummy = 0x0000F36D | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 2))) = 0x0000F36D | 0xE7000000;
	    O_IDX(obj)++;
	  }
	else
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F36D | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
	    O_IDX(obj)++;
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009CD | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x000009CD | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009CD | 0xE7000000;
	    O_IDX(obj)++;
	  }
	break;

      case St16:
	if (IsBengCharCls(obj, WCINPBUF_AT(obj,	MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _BN_II_M))
		NumBengCoreCons(obj)--;
	BengJJJ(obj) += 2;
	NumBengCoreCons(obj) -= 2;
	GetCoreBengConsGlyphs(obj);

	if (IsBengCharCls(obj, WCINPBUF_AT(obj,	MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _BN_II_M)) 
	{
		OUTPUT_OUT2INP_CACHE_CHECK
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F363 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = 0x0000F363 | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F363 | 0xE7000000;
			O_IDX(obj)++;
	} 
	else
	{
		if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
		{
			OUTPUT_OUT2INP_CACHE_CHECK
				dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & dummy);
					dummy >>= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
			
			if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
			dummy = 0x0000F36D | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
		    O_IDX(obj)++;
		  }
		else
		  {
		    OUTPUT_OUT2INP_CACHE_CHECK
		    if (LOGICALSTRMTYPE(obj) == FileCode)
		      {
#ifdef	_BIG_ENDIAN
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F36D | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			  {
			    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		            dummy <<= 8;
			  }
#endif
			}
			else
		      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
		    O_IDX(obj)++;
		}
	}
	
	break;


      case St17:

		if (IsBengCharCls(obj, WCINPBUF_AT(obj,MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 2)), _BN_II_M))
		{
			NumBengCoreCons(obj) -= 4;
			BengJJJ(obj) += 2;
			GetCoreBengConsGlyphs(obj);

			if (IsBengCharCls(obj, WCINPBUF_AT(obj,MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _BN_SP_VM))
			{
				OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F364 | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
						dummy = 0x0000F364 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F364 | 0xE7000000;
					O_IDX(obj)++;
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
						dummy = 0x0000F36D | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
					O_IDX(obj)++;
			}
			else
			{
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F363 | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
					dummy = 0x0000F363 | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F363 | 0xE7000000;
				O_IDX(obj)++;

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
					dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) =
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
				O_IDX(obj)++;

			}

		}
		else
		{
			NumBengCoreCons(obj) -= 3;
			BengJJJ(obj) += 2;
			GetCoreBengConsGlyphs(obj);

			if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
			{
				OUTPUT_OUT2INP_CACHE_CHECK
						dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));

				if (IsBengCharCls(obj, WCINPBUF_AT(obj,MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _BN_SP_VM))
				{
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 
							WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
						dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
					

					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0000F36D | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
					O_IDX(obj)++;


					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif							
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & dummy);
							dummy >>= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
					O_IDX(obj)++;
				}
				else
				{

					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif							
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & dummy);
							dummy >>= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;

					
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1 );
						dummy = 0x0000F36D | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
					O_IDX(obj)++;


					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
							WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
					O_IDX(obj)++;

				}

			}
			else
			{
				OUTPUT_OUT2INP_CACHE_CHECK

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x0000F36D | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
				O_IDX(obj)++;

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
				O_IDX(obj)++;


			}
		}

	break;

      case St18:
		  NumBengCoreCons(obj)--;
		  BengJJJ(obj) += 2;
		  NumBengCoreCons(obj) -= 2;
		  GetCoreBengConsGlyphs(obj);
		  if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
		  {
			  OUTPUT_OUT2INP_CACHE_CHECK
				  dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
			  if (IsBengCharCls(obj, WCINPBUF_AT(obj,MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1)), _BN_SP_VM))
			  {
				  if (LOGICALSTRMTYPE(obj) == FileCode)
				  {
#ifdef	_BIG_ENDIAN
					  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 
						  WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					  out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
					  dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
					  for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					  {
						  *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						  dummy <<= 8;
					  }
#endif
				  }
				  else
					  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 
					  WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
				  
				  if (LOGICALSTRMTYPE(obj) == FileCode)
				  {
#ifdef	_BIG_ENDIAN
					  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					  out_p = &OUTBUF_AT(obj, O_IDX(obj));
					  dummy = 0x0000F36D | 0xE7000000;
					  for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					  {
						  *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						  dummy <<= 8;
					  }
#endif
				  }
				  else
					  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
				  O_IDX(obj)++;

				  if (LOGICALSTRMTYPE(obj) == FileCode)
				  {
#ifdef	_BIG_ENDIAN
					  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif							
#ifdef	_LITTLE_ENDIAN
					  out_p = &OUTBUF_AT(obj, O_IDX(obj));
					  for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					  {
						  *out_p = (0x000000ff & dummy);
						  dummy >>= 8;
					  }
#endif
				  }
				  else
					  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
				  O_IDX(obj)++;
			  }
			  else
			  {

				  if (LOGICALSTRMTYPE(obj) == FileCode)
				  {
#ifdef	_BIG_ENDIAN
					  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif							
#ifdef	_LITTLE_ENDIAN
					  out_p = &OUTBUF_AT(obj, O_IDX(obj));
					  for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					  {
						  *out_p = (0x000000ff & dummy);
						  dummy >>= 8;
					  }
#endif
				  }
				  else
					  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;

				  if (LOGICALSTRMTYPE(obj) == FileCode)
				  {
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1 );
						dummy = 0x0000F36D | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
					O_IDX(obj)++;


					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
							WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
					O_IDX(obj)++;

					
				}

			}
			else
			{
				OUTPUT_OUT2INP_CACHE_CHECK

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x0000F36D | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
				O_IDX(obj)++;

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj,JJJ(obj) + NChrsInCluster(obj) - 1)) | 0xE7000000;
				O_IDX(obj)++;


			}


			break;

      case St19:


	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_I_M))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F361 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F361 | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F361 | 0xE7000000;
	    O_IDX(obj)++;
	
		NumBengCoreCons(obj) -= 3; 
		GetCoreBengConsGlyphs(obj);
	  }
	
	if ((IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_EY_M)) ||
		(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_OW1_M)) ||
		(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_OW2_M)) 
		)
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x0000F36A | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
	    O_IDX(obj)++;
		
		NumBengCoreCons(obj) -= 3; 
		GetCoreBengConsGlyphs(obj);

	  }
	
	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_AI_M))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36B | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F36B | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36B | 0xE7000000;
		O_IDX(obj)++;
		
		NumBengCoreCons(obj) -= 3; 
		GetCoreBengConsGlyphs(obj);
	}

	if ((IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_EY_M)) ||
		(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_AI_M)) 
		)
	{
		OUTPUT_OUT2INP_CACHE_CHECK

		if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
		{
			dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));

			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
				dummy = 0x0000F36D | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
			
			
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = dummy;

			O_IDX(obj)++;
		}
		else
		{
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = 0x0000F36D | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
			O_IDX(obj)++;
		}
	}
			
	
	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_OW1_M))
	{
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x000009BE | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BE | 0xE7000000;
		O_IDX(obj)++;

		if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F36D | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
		O_IDX(obj)++;
	}
	
	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_OW1_M))
	{
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009D7 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x000009D7 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009D7 | 0xE7000000;
		O_IDX(obj)++;

		if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F36D | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
		O_IDX(obj)++;

	}
	
	break;


      case St20:



	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_I_M))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BF | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
		out_p = &OUTBUF_AT(obj, O_IDX(obj));
		dummy = 0x000009BF | 0xE7000000;
		for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
		  {
		    *out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
		    dummy <<= 8;
		  }
#endif
	      }
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BF | 0xE7000000;
	    O_IDX(obj)++;
	
		NumBengCoreCons(obj) -= 4; 
		GetCoreBengConsGlyphs(obj);
	  }
	
	if ((IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_EY_M)) ||
		(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_OW1_M)) ||
		(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_OW2_M)) 
		)
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F36A | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
	    else
	      *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36A | 0xE7000000;
	    O_IDX(obj)++;
		
		NumBengCoreCons(obj) -= 4; 
		GetCoreBengConsGlyphs(obj);

	  }
	
	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_AI_M))
	  {
	    OUTPUT_OUT2INP_CACHE_CHECK
	    if (LOGICALSTRMTYPE(obj) == FileCode)
	      {
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36B | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x0000F36B | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36B | 0xE7000000;
		O_IDX(obj)++;
		
		NumBengCoreCons(obj) -= 4; 
		GetCoreBengConsGlyphs(obj);
	}

	if ((IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_I_M)) ||
		(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_EY_M)) ||
		(IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_AI_M))
		)
	{
		OUTPUT_OUT2INP_CACHE_CHECK

		if ((IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_SP_VM)))
		{
			if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
			{
				dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
					dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));


				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x0000F36D | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
				O_IDX(obj)++;


				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = dummy;
				O_IDX(obj)++;
			}
			else
			{
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
				O_IDX(obj)++;

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x0000F36D | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
				O_IDX(obj)++;
			}
		}
		else
		{
			if (CheckBengKernSpaceGlyphs(obj, O_IDX(obj) - 1))
			{
				dummy = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj) - 1);
					dummy = 0x0000F36D | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = 0x0000F36D | 0xE7000000;
				


				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = dummy;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1))) = dummy;
				O_IDX(obj)++;


				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));

			}
			else
			{
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0x0000F36D | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
				O_IDX(obj)++;

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
						WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
#endif
#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
				O_IDX(obj)++;

			}
		}
	}
		
	
	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_OW1_M))
	{
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BE | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x000009BE | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009BE | 0xE7000000;
		O_IDX(obj)++;

		if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_SP_VM))
		{
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
			O_IDX(obj)++;


			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = 0x0000F36D | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
			O_IDX(obj)++;
		}
		else
		{
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = 0x0000F36D | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
			O_IDX(obj)++;

			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
			O_IDX(obj)++;

		}
	}
	
	if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 2)), _BN_OW2_M))
	{
		OUTPUT_OUT2INP_CACHE_CHECK
		if (LOGICALSTRMTYPE(obj) == FileCode)
		{
#ifdef	_BIG_ENDIAN
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009D7 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj, O_IDX(obj));
			dummy = 0x000009D7 | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x000009D7 | 0xE7000000;
		O_IDX(obj)++;

		if (IsBengCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1)), _BN_SP_VM))
		{
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
			O_IDX(obj)++;


			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = 0x0000F36D | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
			O_IDX(obj)++;
		}
		else
		{
			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = 0x0000F36D | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F36D | 0xE7000000;
			O_IDX(obj)++;

			if (LOGICALSTRMTYPE(obj) == FileCode)
			{
#ifdef	_BIG_ENDIAN
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
#endif
#ifdef	_LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj, O_IDX(obj));
				dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
					*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
					dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 
					WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj)- 1));
			O_IDX(obj)++;

		}
	}
	
	break;

    }

  /* OutToInp */
  if (LOGICALSTRMTYPE(obj) == FileCode)
    {
      for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
	{
	  o2i_p = &OUTTOINP_AT(obj, i*SHPNUMBYTES(obj));
	  for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
	    *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	}
    }
  else
      for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
	OUTTOINP_AT(obj, i) = MAPPING_V2L_AT(obj, JJJ(obj));
  OUTSIZE(obj) = O_IDX(obj);
}



static void
GetTlgCoreConsGlyphs(obj)
  LayoutObj	obj;
{
  int	i, j, k;
  wchar_t temp_out1,temp_out2;
  int	delta, nMin, nMaxFocBlockLength, nMapIndex;
  BooleanValue StillMatching;
  char	*out_p;
  u_int	dummy;
  int nIndex = 0;

  
    

  i = 0;				// i denotes the iscii index 

    while (i < NumTlgCoreCons(obj) )
	{
		if(WCTLGINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + i )) ) 
		 	nMaxFocBlockLength = GetTlg_nMaxFocBlockTbl(obj,\
								 WCTLGINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + i)));
  		else
			return;
		   while (nMaxFocBlockLength)
		   {
				nMin = MIN(nMaxFocBlockLength, NumTlgCoreCons(obj) );
				nMapIndex = 0;

				NotFoundEntryYet:
					j = delta = 0;
					StillMatching = FALSE;
					while (((delta < nMin) || TLG_GLYPH_TBL(obj)[nMapIndex].strISCII[j])
										   && (nMapIndex < MAP_TLG_SIZE) )
					{
						StillMatching = TRUE;
						if ((delta < nMin) && (j < MAX_CORE_CONS_BUF) && 
							 (WCTLGINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + i + delta)) != 
							 TLG_GLYPH_TBL(obj)[nMapIndex].strISCII[j]))
						{
								nMapIndex++;
								goto NotFoundEntryYet;
						}
						delta++;
						j++;
					}

					if (StillMatching)		
						break;			/* Found */
					else
						nMaxFocBlockLength--;
		   }

		  i += nMin;

	/* What's if can't find entry in the table */
		  if ((StillMatching == FALSE) || (nMapIndex >= MAP_TLG_SIZE))
		  {
			   for (j=0; j < NumTlgCoreCons(obj); j++)
			   {
						/* OutBuf */
					  OUTPUT_OUT2INP_CACHE_CHECK
					  if (LOGICALSTRMTYPE(obj) == FileCode)
					  {
#ifdef	_BIG_ENDIAN
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
							WCTLGINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + j)) | 0xE7000000;
							//WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + j)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj, O_IDX(obj));
							dummy = WCTLGINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + j)) | 0xE7000000;
							// dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + j)) | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
					  }
		 			  else
						  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
						  WCTLGINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + j)) | 0xE7000000;
							// WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + j)) | 0xE7000000;
					  O_IDX(obj)++;
				}
		  }
    	  else
		  {
				/*
					Check this case for O_IDX ... Since in cases when O_IDX =0 then O_IDX -1 may give an error.
				*/

			if ( ( IsTlgBottomVattu(obj,TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[0]) ) && 
				      (	 ( CheckTlgKernSpaceGlyphs(obj,O_IDX(obj) - 1) ) ||
					 ( CheckTlgMatraAtStemGlyph(obj,O_IDX(obj) -1 ) ) ) )
				{

					if( O_IDX(obj) >2 &&
					 (	( CheckTlgKernSpaceGlyphs(obj,O_IDX(obj) - 2) ) ||
						( CheckTlgMatraAtStemGlyph(obj,O_IDX(obj) - 2 ) ) ) )
					{
					
						// Logic 
						// The glyph from the TlgGlyphTbl is to be stored in the second last place of the output string 
						// So move the last 2 glyphs   from the output string to 2 temp variable.
						// Copy the glyph from the table to the second last position.
						// Copy the temp variable to the output string 

						temp_out1 = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
						O_IDX(obj)--;					

						temp_out2 = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
						O_IDX(obj)--;					


						for (j=0; TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
						{
	       
							  OUTPUT_OUT2INP_CACHE_CHECK
							  if (LOGICALSTRMTYPE(obj) == FileCode)
 							  {
#ifdef	_BIG_ENDIAN
									*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
									TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
									out_p = &OUTBUF_AT(obj, O_IDX(obj));
									dummy = TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
									for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
									{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
									}
#endif
							}
							else
								*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
								TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
							O_IDX(obj)++;
						}						

						// Now store Temp2
						OUTPUT_OUT2INP_CACHE_CHECK
						if (LOGICALSTRMTYPE(obj) == FileCode)
						{
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = temp_out2;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = temp_out2;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & dummy);
								dummy >>= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out2;
						O_IDX(obj)++;


						// Now store Temp1
						OUTPUT_OUT2INP_CACHE_CHECK
						if (LOGICALSTRMTYPE(obj) == FileCode)
						{
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = temp_out1;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = temp_out1;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & dummy);
								dummy >>= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out1;
						O_IDX(obj)++;
					}
					else
					{
						temp_out1 = *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj) - 1)));
						O_IDX(obj)--;					

						for (j=0; TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
						{
	       
							  OUTPUT_OUT2INP_CACHE_CHECK
							  if (LOGICALSTRMTYPE(obj) == FileCode)
 							  {
#ifdef	_BIG_ENDIAN
									*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
									TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
									out_p = &OUTBUF_AT(obj, O_IDX(obj));
									dummy = TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
									for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
									{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
									}
#endif
							  }
							  else
								*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
							  		TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
							  O_IDX(obj)++;
						}
						
						// Now store Temp1
						OUTPUT_OUT2INP_CACHE_CHECK
						if (LOGICALSTRMTYPE(obj) == FileCode)
						{
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = temp_out1;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = temp_out1;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & dummy);
								dummy >>= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = temp_out1;
						O_IDX(obj)++;

					}	// end of else part for checking if the last 2 character are either a kern space or a matra 
						// at stem.
				}	// end of else part for checking if the last character is either a kern space or a matra at stem and if the character in the table is a bottom vattu
				else if ( ( ( O_IDX(obj) - FIRST_O_IDX(obj) ) > 0 ) && 
					   (   ( IsTlgSideVattu(obj, ( TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[0] ) ) ) &&
						  ( CheckIsTlgSideVattu(obj,*((wchar_t *)(&OUTBUF_AT(obj, (O_IDX(obj) - 1) ) ) ) ) ) )
						)
				{
						OUTPUT_OUT2INP_CACHE_CHECK
						if (LOGICALSTRMTYPE(obj) == FileCode)
						{
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0x0000F5A9 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0x0000F5A9 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F5A9 | 0xE7000000;
						O_IDX(obj)++;	
						

						for (j=0; TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
						{
	       
							  OUTPUT_OUT2INP_CACHE_CHECK
							  if (LOGICALSTRMTYPE(obj) == FileCode)
 							  {
#ifdef	_BIG_ENDIAN
									*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
									TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
									out_p = &OUTBUF_AT(obj, O_IDX(obj));
									dummy = TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
									for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
									{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
									}
#endif
							}
							else
								*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
								TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
							O_IDX(obj)++;
						}

				}	// end of else part for checking if the last character in the output string in a side vattu and the first character in the glyph table is a side vattu
				else
				{
						for (j=0; TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
						{
	       
							  OUTPUT_OUT2INP_CACHE_CHECK
							  if (LOGICALSTRMTYPE(obj) == FileCode)
 							  {
#ifdef	_BIG_ENDIAN
									*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
									TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
									out_p = &OUTBUF_AT(obj, O_IDX(obj));
									dummy = TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
									for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
									{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
									}
#endif
							}
							else
								*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
								TLG_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
							O_IDX(obj)++;
						}
				} // end of else part for checking the presence of bottom or side vattus.
					

		  }	// end of else part for checking if a match was found in the rules or not.

	}	// end of while 

}	// end of function.
				



/* Telugu GetGlyphsFromCluster Starts here */



static void
GetGlyphsFromTlgCluster (obj)
  LayoutObj	obj;
{
  int		i, next_j;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  int		k, len;
  char		*out_p;
  u_int		dummy;
  wchar_t			*wcTempInpBuf;
  // Temp 
  int nIndex = 0;

  FIRST_O_IDX(obj) = O_IDX(obj);
  NumTlgCoreCons(obj) = NChrsInCluster(obj);
  TlgJJJ(obj) = JJJ(obj);

  wcTempInpBuf = WCTLGINPBUF_CACHE(obj);
  memset(WCTLGINPBUF(obj),0,UMLE_CACHE_SIZE );
  WCTLGINPBUF(obj) = WCINPBUF(obj);

  /*for (nIndex =0 ; nIndex < NChrsInCluster(obj);nIndex++ )
  {
	WCTLGINPBUF_AT(obj,nIndex) = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + nIndex ) ) ) ;
  }
  */

  /* Property, InpToOut */							// Check this part later 
  if (LOGICALSTRMTYPE(obj) == FileCode)
  {
		  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
				UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
		  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
		  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));

		  for (k=0; k < len; k++, prop_p++, i2o_p++)
		  {
			  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
			  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
		  }
      
		  next_j = JJJ(obj);
		  for (i = 1; i < NChrsInCluster(obj); i++)
		  {
			  next_j++;
			  if (next_j < END_HDRT(obj))
			  {
				  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)+1) -
			  	  	    UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)); 
			  }
			  else
				 len = 0;

			  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
			  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
			  for (k=0; k < len; k++, prop_p++, i2o_p++)
			  {
				  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
				  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
			  }
		  }
  }
  else
  {
		  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
  		  (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
		  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
		  next_j = JJJ(obj);
		  for (i = 1; i < NChrsInCluster(obj); i++)
			{
		  next_j++;

		  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, next_j)) =
			(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
		  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, next_j)) = O_IDX(obj);
			}
  }
 
  switch (TlgClusterTypeState(obj))
  {
	  case St1:
	  case St2:
	  case St3:
	  case St9:
	  case St10:
	  case St11:
		  {
			  GetTlgCoreConsGlyphs(obj);
			  //WCTLGINPBUF(obj) = WCTLGINPBUF_CACHE(obj);
			  break;
		  }
	  case St4:
		  {		
			  // In this case if the number of characters in the current cluster is 
			  // less than 3 then the GetTlgCoreConsGlyphs() function is called directly
			  // In case the number of characters in the input string is more than 3 then
			  // it has to be handled slightly differently. The first and the last characters
			  // in the input string are first sent ot the GetTlgCoreConsGlyphs() and 
			  // then the remaining characters are sent to the function.

			  if ( NChrsInCluster(obj) < 3 )
			  {
					GetTlgCoreConsGlyphs(obj);
			  }
			  else
			  {
				  WCTLGINPBUF(obj) = wcTempInpBuf;
				  memset(WCTLGINPBUF(obj),0,UMLE_CACHE_SIZE );
				  WCTLGINPBUF_AT(obj,0) = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) ) ) ) ;
				  WCTLGINPBUF_AT(obj,1) = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + NChrsInCluster(obj) - 1) ) ) ;
				  WCTLGINPBUF_AT(obj,2) = 0x0;
				  NumTlgCoreCons(obj) = 2 ;
				  GetTlgCoreConsGlyphs(obj);
					
				  // Changes made on 16/08/02 
				  // WCTLGINPBUF(obj) = WCINPBUF(obj);
				 
				  WCTLGINPBUF(obj) = WCINPBUF(obj);

				  TlgJJJ(obj) = JJJ(obj) + 1; 
				  NumTlgCoreCons(obj) = NChrsInCluster(obj) - 2 ;
				  GetTlgCoreConsGlyphs(obj);
			  }
			break;
		  }
	  case St5:
		  {
			// This case handles the vattu formation.
			// To get the glyphs in this case the glyph of the first consonant is first stored in the output string
			// Then the glyphs of the remaining character are obtained.
			// There are some typical cases.That for consonant 0x0C30 and 0x0C19.
			
			if ( TlgVattuCount(obj) == 1 )
			{
				// If vattu count is 1 and if the second consonant is 0x0C30 then the ra vattu is placed first in the output 
			    // buffer then the glyph of the first consonant is obtained.
				if ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + 2  ) ) ) == 0x0C30  )
				{
					
					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0x0000F619 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = 0x0000F619 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
									*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
									dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F619 | 0xE7000000;
					O_IDX(obj)++;	
					
					NumTlgCoreCons(obj) = 1;
				    GetTlgCoreConsGlyphs(obj);

				}
				else if( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + 2  ) ) ) == 0x0C19 )
				{
					// In case of 0x0C19 , the character does not have  a vattu form.So the glyphs of the first consonant halant and 
					// the glyph of 0x0C19 is placed in the output buffer.

					NumTlgCoreCons(obj) = 2;
				    GetTlgCoreConsGlyphs(obj);
					
				//	memset(WCTLGINPBUFF(obj),0,UMLE_CACHE_SIZE );
				//	WCTLGINPBUF(obj) = WCINPBUF(obj);					
					TlgJJJ(obj) = JJJ(obj) + 2; 
					NumTlgCoreCons(obj) = 1;
				    GetTlgCoreConsGlyphs(obj);
				}
				else
				{
					NumTlgCoreCons(obj) = 1;
					GetTlgCoreConsGlyphs(obj);
					NumTlgCoreCons(obj) = NChrsInCluster(obj) - 1;
					TlgJJJ(obj) = JJJ(obj) + 1;
					GetTlgCoreConsGlyphs(obj);
					
				}
			}
			else
			{
					NumTlgCoreCons(obj) = 1;
				    GetTlgCoreConsGlyphs(obj);
					if( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) -1  ) ) ) == 0x0C19  )
					{
						NumTlgCoreCons(obj) = NChrsInCluster(obj) - 2 ;
					
					}
					else
					{
						NumTlgCoreCons(obj) = NChrsInCluster(obj) - 1 ;
					} 
			
					TlgJJJ(obj) = JJJ(obj) + 1; 
					GetTlgCoreConsGlyphs(obj);
			}

			break;

		  }
	  case St6:
		  {
			  /* This is slightly complicated .....
			  First we need to pass only the consonant and the matra to the GetTlgCoreConsGlyphs.
			  Next we send all the character included in the vattu's....
			  Approach
				1] Using a temp variable, we first copy the first character and the last character to this temp variable 
				and call the function GetTlgCoreConsGlyphs(). Note this function uses this temp variable only and not the
				original input buffer.
				2] After this we copy the second character onwards till the second last character into the temp variable
				and call the function GetTlgCoreConsGlyphs().
				
			  */
			  
			  WCTLGINPBUF(obj) = wcTempInpBuf;
			  memset(WCTLGINPBUF(obj),0,UMLE_CACHE_SIZE );
			  WCTLGINPBUF_AT(obj,0) = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) ) ) ) ;
			  WCTLGINPBUF_AT(obj,1) = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + NChrsInCluster(obj) - 1) ) ) ;
			  WCTLGINPBUF_AT(obj,2 )  = 0x0;
			  NumTlgCoreCons(obj) = 2 ;
			  GetTlgCoreConsGlyphs(obj);
			  
			  
			  WCTLGINPBUF(obj) = WCINPBUF(obj);					
			  TlgJJJ(obj) = JJJ(obj) + 1 ;
			  NumTlgCoreCons(obj) = NChrsInCluster(obj) - 2;
			  GetTlgCoreConsGlyphs(obj);
			  
			  break;
		  }
	  case St7:
		  {
			  // Getting the glyph is divided in 3 parts 
			  // the first part comprises of the first consonant and the matra
			  // the second part comprises of the second character (i.e halant ) till the character before the matra
			  // and the final part consists of the vowel modifier 

			  // First part 
			  
			  WCTLGINPBUF(obj) = wcTempInpBuf;					
			  memset(WCTLGINPBUF(obj),0,UMLE_CACHE_SIZE );
			  WCTLGINPBUF_AT(obj,0) = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) ) ) ) ;
			  WCTLGINPBUF_AT(obj,1) = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) + NChrsInCluster(obj) - 2) ) ) ;
			  WCTLGINPBUF_AT(obj,2 )  = 0x0;
			  NumTlgCoreCons(obj) = 2 ;
			  GetTlgCoreConsGlyphs(obj);
			  
			  // Second part 
			  WCTLGINPBUF(obj) = WCINPBUF(obj);					
			  TlgJJJ(obj) = JJJ(obj) + 1 ;
			  NumTlgCoreCons(obj) = NChrsInCluster(obj) - 3;
			  GetTlgCoreConsGlyphs(obj);

			  // Third part 
			  WCTLGINPBUF(obj) = wcTempInpBuf;					
			  memset(WCTLGINPBUF(obj),0,UMLE_CACHE_SIZE );
			  WCTLGINPBUF_AT(obj,0) = ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj) + NChrsInCluster(obj) - 1) ) ) ;
			  WCTLGINPBUF_AT(obj,1) = 0x0;
			  TlgJJJ(obj)	 = JJJ(obj);
			  NumTlgCoreCons(obj) = 1;
			  GetTlgCoreConsGlyphs(obj);

			  break;
		  }
	  case St8:
		  {
			  NumTlgCoreCons(obj) = 1 ;
			  GetTlgCoreConsGlyphs(obj);
			  
			  TlgJJJ(obj) = JJJ(obj) + 1;
			  NumTlgCoreCons(obj) = NChrsInCluster(obj) - 1;
			  GetTlgCoreConsGlyphs(obj);
			  break;
		  }
	  case St12:
		  {
			 // if (IsDevaCharCls(obj,WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)+ NChrsInCluster(obj) - 1)), _IM))
			if ( IsTlgCharCls(obj, WCTLGINPBUF_AT(obj, MAPPING_V2L_AT(obj, TlgJJJ(obj) ) ),_TLD ) )
			{
				GetTlgCoreConsGlyphs(obj);
	 		    break;
			}

			 OUTPUT_OUT2INP_CACHE_CHECK
			 if (LOGICALSTRMTYPE(obj) == FileCode)
			 {
#ifdef _BIG_ENDIAN
				*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0x0000F59E | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj,O_IDX(obj));
				dummy = 0x0000F59E | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F59E | 0xE7000000;
			O_IDX(obj)++;	

			GetTlgCoreConsGlyphs(obj);
	
 		    break;
		  }

  }	// End of switch case statement
	
  

  WCTLGINPBUF(obj) = wcTempInpBuf;

  /* OutToInp */
	  if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
			  for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
			  {
				  o2i_p = &OUTTOINP_AT(obj, i*SHPNUMBYTES(obj));
				  for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
						*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
			  }
	  }
	  else
		  for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
				OUTTOINP_AT(obj, i) = MAPPING_V2L_AT(obj, JJJ(obj));

	  OUTSIZE(obj) = O_IDX(obj);					

	  
}

/* Ends Here */


static void
NextCluster_Deva(obj)
  LayoutObj		obj;
{
  wchar_t	wc_j = L'\0';
  wchar_t	wc_jj = L'\0';
  int		n_j;

  if (NEXT_CLUSTER(obj) < END_HDRT(obj))
  {
    DevaClusterTypeState(obj) = St0;
    if (NChrsInCluster(obj) != -1)
	JJJ(obj) = NEXT_CLUSTER(obj);

    if (JJJ(obj) < END_HDRT(obj))
      {
	wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
        DevaClusterTypeState(obj) =
		UpdateDevaClusterTypeState(obj, DevaClusterTypeState(obj), wc_j);
      }
    else
	wc_j = L'\0';
    n_j = JJJ(obj) + 1;
    if (n_j < END_HDRT(obj))
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
    else
	wc_jj = L'\0';
    NChrsInCluster(obj) = 1;
    
    while ((wc_j != L'\0') && (wc_jj != L'\0') &&
	   !((NChrsInCluster(obj) == 1) && (IsDevaCharCls(obj, wc_j, _HL))) &&
	   IsDevaComposible(obj, wc_j, wc_jj))
      {
	NChrsInCluster(obj)++;
	wc_j = wc_jj;
        DevaClusterTypeState(obj) =
		UpdateDevaClusterTypeState(obj, DevaClusterTypeState(obj), wc_j);
	n_j++;
	if (n_j < END_HDRT(obj))
	  wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
	else
	  wc_jj = L'\0';
      }
    NEXT_CLUSTER(obj) = n_j;

  } else
       NChrsInCluster(obj) = 0;
}





static void
NextCluster_Kann(obj)
  LayoutObj		obj;
{
  wchar_t	wc_j = L'\0';
  wchar_t	wc_jj = L'\0';
  int		n_j;

  if (NEXT_CLUSTER(obj) < END_HDRT(obj))
  {
    KannClusterTypeState(obj) = St0;
    if (NChrsInCluster(obj) != -1)
	JJJ(obj) = NEXT_CLUSTER(obj);

    if (JJJ(obj) < END_HDRT(obj))
      {
	wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
        KannClusterTypeState(obj) = UpdateKannClusterTypeState(obj, KannClusterTypeState(obj), wc_j);
      }
    else
	wc_j = L'\0';
    n_j = JJJ(obj) + 1;
    if (n_j < END_HDRT(obj))
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
    else
	wc_jj = L'\0';
    NChrsInCluster(obj) = 1;
    
    while ((wc_j != L'\0') && (wc_jj != L'\0') && IsKannComposible(obj, wc_j, wc_jj))
      {
	NChrsInCluster(obj)++;
	wc_j = wc_jj;
        KannClusterTypeState(obj) = UpdateKannClusterTypeState(obj, KannClusterTypeState(obj), wc_j);
	n_j++;
	if (n_j < END_HDRT(obj))
	  wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
	else
	  wc_jj = L'\0';
      }
    NEXT_CLUSTER(obj) = n_j;

  } else
       NChrsInCluster(obj) = 0;
}




static void
NextCluster_Guja(obj)
  LayoutObj		obj;
{
  wchar_t	wc_j = L'\0';
  wchar_t	wc_jj = L'\0';
  int		n_j;

  if (NEXT_CLUSTER(obj) < END_HDRT(obj))
  {
    GujaClusterTypeState(obj) = St0;
    if (NChrsInCluster(obj) != -1)
	JJJ(obj) = NEXT_CLUSTER(obj);

    if (JJJ(obj) < END_HDRT(obj))
      {
	wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
        GujaClusterTypeState(obj) =
		UpdateGujaClusterTypeState(obj, GujaClusterTypeState(obj), wc_j);
      }
    else
	wc_j = L'\0';
    n_j = JJJ(obj) + 1;
    if (n_j < END_HDRT(obj))
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
    else
	wc_jj = L'\0';
    NChrsInCluster(obj) = 1;
    
    while ((wc_j != L'\0') && (wc_jj != L'\0') &&
	   !((NChrsInCluster(obj) == 1) && (IsGujaCharCls(obj, wc_j, _HL))) &&
	   IsGujaComposible(obj, wc_j, wc_jj))
      {
	NChrsInCluster(obj)++;
	wc_j = wc_jj;
        GujaClusterTypeState(obj) =
		UpdateGujaClusterTypeState(obj, GujaClusterTypeState(obj), wc_j);
	n_j++;
	if (n_j < END_HDRT(obj))
	  wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
	else
	  wc_jj = L'\0';
      }
    NEXT_CLUSTER(obj) = n_j;

  } else
       NChrsInCluster(obj) = 0;
}





static void
NextCluster_Beng(obj)
  LayoutObj		obj;
{
  wchar_t	wc_j = L'\0';
  wchar_t	wc_jj = L'\0';
  int		n_j;

  if (NEXT_CLUSTER(obj) < END_HDRT(obj))
  {
    BengClusterTypeState(obj) = St0;
    if (NChrsInCluster(obj) != -1)
	JJJ(obj) = NEXT_CLUSTER(obj);

    if (JJJ(obj) < END_HDRT(obj))
      {
	wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
        BengClusterTypeState(obj) =
		UpdateBengClusterTypeState(obj, BengClusterTypeState(obj), wc_j);
      }
    else
	wc_j = L'\0';
    n_j = JJJ(obj) + 1;
    if (n_j < END_HDRT(obj))
	wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
    else
	wc_jj = L'\0';
    NChrsInCluster(obj) = 1;
    
    while ((wc_j != L'\0') && (wc_jj != L'\0') &&
	   !((NChrsInCluster(obj) == 1) && (IsBengCharCls(obj, wc_j, _HL))) &&
	   IsBengComposible(obj, wc_j, wc_jj))
      {
	NChrsInCluster(obj)++;
	wc_j = wc_jj;
        BengClusterTypeState(obj) =
		UpdateBengClusterTypeState(obj, BengClusterTypeState(obj), wc_j);
	n_j++;
	if (n_j < END_HDRT(obj))
	  wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
	else
	  wc_jj = L'\0';
      }
    NEXT_CLUSTER(obj) = n_j;

  } else
       NChrsInCluster(obj) = 0;
}


static void
NextCluster_Tlg(obj)
  LayoutObj		obj;
{
  wchar_t	wc_j = L'\0';
  wchar_t	wc_jj = L'\0';
  int		n_j;
  int nIndex =0;

BeginState:
	// Initialize the telugu specific variables
	TlgVattuPrevious(obj) = 0;
	TlgHalantCount(obj) = 0 ;
	TlgVattuCount(obj) = 0;
	TlgVattuIndex(obj) = 0 ;
	for(nIndex =0 ;nIndex < 3 ;nIndex++)
	{
		TlgVattuList(obj,nIndex)=0x0;
	}
	
  if (NEXT_CLUSTER(obj) < END_HDRT(obj))
  {
		TlgClusterTypeState(obj) = St0;
		if (NChrsInCluster(obj) != -1)
			JJJ(obj) = NEXT_CLUSTER(obj);
		

		// Update the state of the first character in the cluster 
		if (JJJ(obj) < END_HDRT(obj))
		{
			wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
			TlgClusterTypeState(obj) = UpdateTlgClusterTypeState(obj, TlgClusterTypeState(obj), wc_j);
		}
		else
			wc_j = L'\0';

		n_j = JJJ(obj) + 1;
		if (n_j < END_HDRT(obj))
			wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
		else
			wc_jj = L'\0';

		NChrsInCluster(obj) = 1;
		while ((wc_j != L'\0') && (wc_jj != L'\0') && IsTlgComposible(obj, wc_j, wc_jj))
		{
			
			/* Code for checking the vattu case */ 		
			if( (TlgClusterTypeState(obj) == St4)  &&
				( IsTlgCharCls(obj,wc_j,_TLH) && IsTlgCharCls(obj, wc_jj, _TLC) ) )
			{
				TlgHalantCount(obj) = TlgHalantCount(obj) + 1;
				TlgVattuCount(obj) = TlgVattuCount(obj) + 1 ;

				// The type of the vattu with respect to the consonant is stored in the TlgVattuList for later reference
				TlgVattuList(obj,TlgVattuIndex(obj)) = TLG_VATTU_TYPE(obj)[wc_jj - 0x0C15];
				if(!IsVattuComposible(obj,TlgVattuPrevious(obj),TlgVattuList(obj,TlgVattuIndex(obj) ) ) )
				{
					TlgClusterTypeState(obj) = St4 ;
					NChrsInCluster(obj) = 2;
					NEXT_CLUSTER(obj) = JJJ(obj) + NChrsInCluster(obj);
					return;
				}	
				TlgVattuPrevious(obj) = TlgVattuList(obj,TlgVattuIndex(obj) );
				TlgVattuIndex(obj) = TlgVattuIndex(obj) + 1;
			}
		
			NChrsInCluster(obj)++;
			wc_j = wc_jj;
			TlgClusterTypeState(obj) = UpdateTlgClusterTypeState(obj, TlgClusterTypeState(obj), wc_j);
			n_j++;
			if (n_j < END_HDRT(obj))
				wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
			else
				wc_jj = L'\0';

			// The max number of vattus possible in a cluster is 3 TLG_CHRTYPE_TBL(obj)[ucs2tlg_index((wc1)
			if( ( TlgVattuCount(obj) == 3 )  && (TlgClusterTypeState(obj) == St5) && 
				( TLG_CHRTYPE_TBL(obj)[ucs2tlg_index(wc_jj)] == __TLH ) )
			{
					TlgClusterTypeState(obj) = St4 ;
					NChrsInCluster(obj) = 2;
					NEXT_CLUSTER(obj) = JJJ(obj) + 2;
					GetGlyphsFromTlgCluster (obj);
					goto BeginState;			
			}
		}
		NEXT_CLUSTER(obj) = n_j;

  }
  else
       NChrsInCluster(obj) = 0;


}




static	void
func_devanagari(obj)
  LayoutObj		obj;
{
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Deva (obj);
  while (NChrsInCluster(obj) > 0)
    {
      GetGlyphsFromDevaCluster (obj);
      NextCluster_Deva (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
                                         : ReverseDirection_WC(obj) );
}


static	void
func_kannada(obj)
  LayoutObj		obj;
{
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Kann (obj);
  while (NChrsInCluster(obj) > 0)
    {
      GetGlyphsFromKannCluster (obj);
      NextCluster_Kann (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
                                         : ReverseDirection_WC(obj) );
}



static	void
func_gujarati(obj)
  LayoutObj		obj;
{
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Guja (obj);
  while (NChrsInCluster(obj) > 0)
    {
      GetGlyphsFromGujaCluster (obj);
      NextCluster_Guja (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
                                         : ReverseDirection_WC(obj) );
}


static	void
func_bengali(obj)
  LayoutObj		obj;
{
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Beng (obj);
  while (NChrsInCluster(obj) > 0)
    {
      GetGlyphsFromBengCluster (obj);
      NextCluster_Beng (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
                                         : ReverseDirection_WC(obj) );
}


/* Telugu */
static	void
func_telugu(obj)
  LayoutObj		obj;
{
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Tlg (obj);
  while (NChrsInCluster(obj) > 0)
    {
      GetGlyphsFromTlgCluster (obj);
      NextCluster_Tlg (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
                                         : ReverseDirection_WC(obj) );
}

/* Malayalam */


static void
GetMlCoreConsGlyphs(obj)
  LayoutObj	obj;
{
  int	i, j, k;
  wchar_t temp_out;
  int	delta, nMin, nMaxFocBlockLength, nMapIndex, nNoOfChars;
  BooleanValue StillMatching;
  char	*out_p;
  u_int	dummy;
  int nIndex = 0;
	i = 0;				// i denotes the iscii index 

    while (i < NumMlCoreCons(obj) )
	{
		nIndex = i ;
		if(WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + i )) ) 
		 	nMaxFocBlockLength = GetMl_nMaxFocBlockTbl(obj,\
								 WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + i)));
  		else
			return;
		   while (nMaxFocBlockLength)
		   {
				nMin = MIN(nMaxFocBlockLength, NumMlCoreCons(obj) );
				nMapIndex = 0;

				NotFoundEntryYet:
					j = delta = 0;
					StillMatching = FALSE;
					while (((delta < nMin) || ML_GLYPH_TBL(obj)[nMapIndex].strISCII[j])
										   && (nMapIndex < MAP_ML_SIZE) )
					{
						if ( NumMlCoreCons(obj) == i + delta )
							break;
						StillMatching = TRUE;
						if ((delta < nMin) && (j < MAX_CORE_CONS_BUF) && 
							 (WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + i + delta)) != 
							 ML_GLYPH_TBL(obj)[nMapIndex].strISCII[j]))
						{
								nMapIndex++;
								goto NotFoundEntryYet;
						}
						delta++;
						j++;
						if( delta != nMin)
							StillMatching = FALSE;
					}
					if( (StillMatching) && ( ML_GLYPH_TBL(obj)[nMapIndex].strISCII[j] == 0x0 ) )
						break;			/* Found */
					else
						nMaxFocBlockLength--;
		   }

		  i += nMin;
		/* What's if can't find entry in the table */
		  if ((StillMatching == FALSE) || (nMapIndex >= MAP_ML_SIZE))
		  {
			   for (j=0; j < NumMlCoreCons(obj); j++)
			   {
						/* OutBuf */
					  OUTPUT_OUT2INP_CACHE_CHECK
					  if (LOGICALSTRMTYPE(obj) == FileCode)
					  {
#ifdef	_BIG_ENDIAN
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
							WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + j)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj, O_IDX(obj));
							dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + j)) | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
					  }
		 			  else
						  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
						  WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + j)) | 0xE7000000;
							
					  O_IDX(obj)++;
				}
		  }
		  else
		  {
				for (j=0; ML_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
				{
	       
						  OUTPUT_OUT2INP_CACHE_CHECK
						  if (LOGICALSTRMTYPE(obj) == FileCode)
 						  {
#ifdef	_BIG_ENDIAN
								*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
								ML_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
								out_p = &OUTBUF_AT(obj, O_IDX(obj));
								dummy = ML_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
								for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
								{
									*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
									dummy <<= 8;
								}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
							ML_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
						O_IDX(obj)++;
				}

		  }


    

	}
}

static void
GetGlyphsFromMlCluster (obj)
  LayoutObj	obj;
{
  int		i, next_j;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  int		k, len;
  char		*out_p;
  u_int		dummy;
  wchar_t	wcTemp;
  int nIndex  = 0 ;
  int nSpecialCaseFlag = 0 ;
  int nMatraPos = 1;

  // Temp 
  
  FIRST_O_IDX(obj) = O_IDX(obj);
  NumMlCoreCons(obj) = NChrsInCluster(obj);
  MlJJJ(obj) = JJJ(obj);

  
  /* Property, InpToOut */							// Check this part later 
  if (LOGICALSTRMTYPE(obj) == FileCode)
  {
		  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
				UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
		  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
		  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));

		  for (k=0; k < len; k++, prop_p++, i2o_p++)
		  {
			  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
			  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
		  }
      
		  next_j = JJJ(obj);
		  for (i = 1; i < NChrsInCluster(obj); i++)
		  {
			  next_j++;
			  if (next_j < END_HDRT(obj))
			  {
				  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)+1) -
			  	  	    UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)); 
			  }
			  else
				 len = 0;

			  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
			  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
			  for (k=0; k < len; k++, prop_p++, i2o_p++)
			  {
				  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
				  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
			  }
		  }
  }
  else
  {
		  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
  		  (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
		  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
		  next_j = JJJ(obj);
		  for (i = 1; i < NChrsInCluster(obj); i++)
			{
		  next_j++;

		  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, next_j)) =
			(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
		  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, next_j)) = O_IDX(obj);
			}
  }
 
  switch (MlClusterTypeState(obj))
  {
	  case St1:
	  case St2:
	  case St4:
	  case St9:
	  {
		  GetMlCoreConsGlyphs(obj);
		  break;
	  }
	  case St3:
	  {
		
		if ( ( NChrsInCluster(obj) > 1 ) &&  
		    ( 
			 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 1 ) ) == 0x0D2F) &&
			   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) != 0x0D2F) 
			 ) ||
			 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 1 ) ) == 0x0D35) &&
			   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) != 0x0D35) 
			 ) ||
			 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 1 ) ) == 0x0D30) ||
			  
			 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 1 ) ) == 0x0D31) && 
			   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) != 0x0D31) ) 
			 )
		   )
		{		 
	
				if ( ( NChrsInCluster(obj) > 3 ) && ( 
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 1 ) ) == 0x0D2F ) &&
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D30 ) ||
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 1 ) ) == 0x0D35 ) &&
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D30 ) ) )
				{
					nSpecialCaseFlag = DOUBLE_CONSONANT_SIGN;			
					NumMlCoreCons(obj) = NChrsInCluster(obj) - 4;
				}
				else
				{
					nSpecialCaseFlag = SINGLE_CONSONANT_SIGN;			
					NumMlCoreCons(obj) = NChrsInCluster(obj) - 2;
				}

				GetMlCoreConsGlyphs(obj);
				// the last consonant in the input string is ra then the halant ra glyph shld be placed before the last 
				// glyph in the output buffer
				// For this copy the last element of the output buffer at the current position of O_IDX and then decrement O_IDX 
				// and  copy the glyph of halant ra at this position. Now increment  O_IDX by 2 

				if ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 1 ) ) == 0x0D30 ) || 
				     ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 1 ) ) == 0x0D31 ) )
				{

					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
						 

#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );

					O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )


					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF483 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

					O_IDX(obj) = O_IDX(obj) + 2  ;	

				}
				else if ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 1 ) ) == 0x0D2F)
				{

					if ( nSpecialCaseFlag == DOUBLE_CONSONANT_SIGN )
					{
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
						 
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
	
						O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )

	
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF483 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

						O_IDX(obj) = O_IDX(obj) + 2  ;	

					}


					 OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF481 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF481 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF481 | 0xE7000000;

					O_IDX(obj)++;

				}
				else if ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 1 ) ) == 0x0D35)
				{
					
					if ( nSpecialCaseFlag == DOUBLE_CONSONANT_SIGN )
					{
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
						 
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
	
						O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )

	
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF483 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

						O_IDX(obj) = O_IDX(obj) + 2  ;	

					}

					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF482 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF482 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF482 | 0xE7000000;

					O_IDX(obj)++;

				}
		
		}	
		else
		{
			GetMlCoreConsGlyphs(obj);
		}
		break;
	}
	case St5:
	{ 

		NumMlCoreCons(obj) = NChrsInCluster(obj) - 1;


		// Check if the second last character is ya, ra, va 
		if ( ( NChrsInCluster(obj) > 2 ) &&  
		   ( 
			 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D2F) &&
			   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 4 ) ) != 0x0D2F) ) ||

			 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D35) &&
			   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 4 ) ) != 0x0D35) ) ||

			 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D30) ||		 
			  
			 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D31) && 
			   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 4 ) ) != 0x0D31) ) ) )
		{		
				
				if ( ( NChrsInCluster(obj) > 4 ) &&  
					( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D2F ) &&
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 4 ) ) == 0x0D30 ) ||
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D35 ) &&
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 4 ) ) == 0x0D30 ) ) )
				{
					nSpecialCaseFlag = DOUBLE_CONSONANT_SIGN;			
					NumMlCoreCons(obj) = NChrsInCluster(obj) - 5;
				}
				else
				{
					nSpecialCaseFlag = SINGLE_CONSONANT_SIGN;
					NumMlCoreCons(obj) = NChrsInCluster(obj) - 3;
				}
				GetMlCoreConsGlyphs(obj);
				// the last consonant in the input string is ra then the halant ra glyph shld be placed before the last 
				// glyph in the output buffer
				// For this copy the last element of the output buffer at the current position of O_IDX and then decrement O_IDX 
				// and  copy the glyph of halant ra at this position. Now increment  O_IDX by 2 

				if ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D30) || 
				     ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D31) )
				{

					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );

					O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )


					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = 0xF483 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

					O_IDX(obj) = O_IDX(obj) + 2  ;	

				}
				else if ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D2F)
				{

					if ( nSpecialCaseFlag == DOUBLE_CONSONANT_SIGN )
					{
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
						 
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
	
						O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )

	
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF483 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

						O_IDX(obj) = O_IDX(obj) + 2  ;	

					}

					 OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF481 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF481 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF481 | 0xE7000000;

					O_IDX(obj)++;

				}
				else if ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D35)
				{

					if ( nSpecialCaseFlag == DOUBLE_CONSONANT_SIGN )
					{
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
						 
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
	
						O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )

	
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF483 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

						O_IDX(obj) = O_IDX(obj) + 2  ;	

					}


					 OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF482 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = 0xF482 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF482 | 0xE7000000;

					O_IDX(obj)++;

				}
		}	
		else
		{
			nSpecialCaseFlag = 0;
			GetMlCoreConsGlyphs(obj);
		}
			
		nMatraPos = 1;
		if( ( IsMlCharCls ( obj, WCINPBUF_AT( obj, MAPPING_V2L_AT(obj, MlJJJ(obj)+ NChrsInCluster(obj) - 1)), _M_SM) )	|| 
			( IsMlCharCls ( obj, WCINPBUF_AT( obj, MAPPING_V2L_AT(obj, MlJJJ(obj)+ NChrsInCluster(obj) - 1)), _M_RM) )  )
		{
				// copy last character one position ahead
				OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef _BIG_ENDIAN
					*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );

				O_IDX(obj)-- ;
				nMatraPos++;

			if ( nSpecialCaseFlag )
			{

				// copy 2nd last character one position ahead
				OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef _BIG_ENDIAN
					*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );

				O_IDX(obj)-- ;
				nMatraPos++;

			}

			if ( nSpecialCaseFlag == DOUBLE_CONSONANT_SIGN )
			{

				// copy 2nd last character one position ahead
				OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef _BIG_ENDIAN
					*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );

				O_IDX(obj)-- ;
				nMatraPos++;

			}
			if ( IsMlCharCls ( obj, WCINPBUF_AT( obj, MAPPING_V2L_AT(obj, MlJJJ(obj)+ NChrsInCluster(obj) - 1)), _M_RM) )
			{
					wcTemp = WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + ( NChrsInCluster(obj) -1 ) ) ) | 0xE7000000; 

					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = wcTemp ;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = wcTemp;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wcTemp;

					O_IDX(obj) =  O_IDX(obj) + nMatraPos ;
					nMatraPos = 1;  // initialize the nMatraPos variable
			}
			else	// Split Matra
			{
					if ( ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + ( NChrsInCluster(obj) - 1 ) ) ) == 0x0D4A ) 	||
						 ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + ( NChrsInCluster(obj) - 1 ) ) ) == 0x0D4C )  )
					{
						wcTemp = 0x0D46;
					}
					else
					{
						wcTemp = 0x0D47;
					}
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = wcTemp | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = wcTemp | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
									*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
									dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wcTemp | 0xE7000000;
					O_IDX(obj) = O_IDX(obj) + nMatraPos ;	
					nMatraPos = 1; // initialize the nMatraPos variable
					
					if ( ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + ( NChrsInCluster(obj) - 1 ) ) ) == 0x0D4A ) 	||
						 ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + ( NChrsInCluster(obj) - 1 ) ) ) == 0x0D4B )  )
					{
						wcTemp = 0x0D3e;
					}
					else
					{
						wcTemp = 0x0D57;
					}
					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = wcTemp | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = wcTemp | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
									*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
									dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wcTemp | 0xE7000000;
					O_IDX(obj)++;

			} // End of split matra for putting the remaining part of matra 
		}	  // End of Right and Split matra condition
		else		// Left Matra
		{
				OUTPUT_OUT2INP_CACHE_CHECK

				wcTemp = WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + NChrsInCluster(obj) -1 ) ) ; 

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{

#ifdef _BIG_ENDIAN
					*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = wcTemp | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = wcTemp | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wcTemp | 0xE7000000;

				O_IDX(obj)++ ;
		}
	  break;
	 }
	 case St6:
	 { 

		// Get the glyphs for the input buffer without taking into consideration the matra and the vowel modifier
		NumMlCoreCons(obj) = NChrsInCluster(obj) - 2;

		// Check if the second last character is ya, ra, va 
		if ( ( NChrsInCluster(obj) > 3 ) &&  
		   ( 
			 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D2F) &&
			   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 5 ) ) != 0x0D2F) ) ||

			 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D35) &&
			   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 5 ) ) != 0x0D35) ) ||

			   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D30) ||
			   			  
			 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D31) && 
			   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 5 ) ) != 0x0D31) ) ) )
		{		

				if ( ( NChrsInCluster(obj) > 5 ) && 	
					 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D2F ) &&
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 5 ) ) == 0x0D30 ) ||
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D35 ) &&
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 5 ) ) == 0x0D30 ) ) )
				{
					nSpecialCaseFlag = DOUBLE_CONSONANT_SIGN;			
					NumMlCoreCons(obj) = NChrsInCluster(obj) - 6;
				}
				else
				{
					nSpecialCaseFlag = SINGLE_CONSONANT_SIGN;
					NumMlCoreCons(obj) = NChrsInCluster(obj) - 4;
				}

				GetMlCoreConsGlyphs(obj);
				// the last consonant in the input string is ra then the halant ra glyph shld be placed before the last 
				// glyph in the output buffer
				// For this copy the last element of the output buffer at the current position of O_IDX and then decrement O_IDX 
				// and  copy the glyph of halant ra at this position. Now increment  O_IDX by 2 

				if ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D30) || 
				     ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D31) )
				{

					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );

					O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )


					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = 0xF483 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

					O_IDX(obj) = O_IDX(obj) + 2  ;	

				}
				else if ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D2F)
				{

					if ( nSpecialCaseFlag == DOUBLE_CONSONANT_SIGN )
					{
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
						 
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
	
						O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )

	
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF483 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

						O_IDX(obj) = O_IDX(obj) + 2  ;	

					}
				
					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF481 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF481 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF481 | 0xE7000000;

					O_IDX(obj)++;

				}
				else if ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 3 ) ) == 0x0D35)
				{
					
					if ( nSpecialCaseFlag == DOUBLE_CONSONANT_SIGN )
					{
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
						 

#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
	
						O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )

	
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF483 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

						O_IDX(obj) = O_IDX(obj) + 2  ;	
					}
		
					
					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF482 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = 0xF482 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF482 | 0xE7000000;

					O_IDX(obj)++;

				}
		}	
		else
		{
			nSpecialCaseFlag = 0;
			GetMlCoreConsGlyphs(obj);
		}
			
		nMatraPos = 1;
		if( ( IsMlCharCls ( obj, WCINPBUF_AT( obj, MAPPING_V2L_AT(obj, MlJJJ(obj)+ NChrsInCluster(obj) - 2)), _M_SM) )	|| 
			( IsMlCharCls ( obj, WCINPBUF_AT( obj, MAPPING_V2L_AT(obj, MlJJJ(obj)+ NChrsInCluster(obj) - 2)), _M_RM) )  )
		{
				// copy last character one position ahead
				OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef _BIG_ENDIAN
					*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );

				O_IDX(obj)-- ;
				nMatraPos++;

			if ( nSpecialCaseFlag )
			{

				// copy 2nd last character one position ahead
				OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef _BIG_ENDIAN
					*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );

				O_IDX(obj)-- ;
				nMatraPos++;

			}

			if ( nSpecialCaseFlag == DOUBLE_CONSONANT_SIGN )
			{

				// copy 2nd last character one position ahead
				OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
				{
#ifdef _BIG_ENDIAN
					*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );

				O_IDX(obj)-- ;
				nMatraPos++;

			}

			if ( IsMlCharCls ( obj, WCINPBUF_AT( obj, MAPPING_V2L_AT(obj, MlJJJ(obj)+ NChrsInCluster(obj) - 2)), _M_RM) )
			{
					wcTemp = WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + ( NChrsInCluster(obj) - 2 ) ) ) | 0xE7000000; 

					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = wcTemp ;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = wcTemp;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wcTemp;

					O_IDX(obj) =  O_IDX(obj) + nMatraPos ;
					nMatraPos = 1;  // initialize the nMatraPos variable
			}
			else	// Split Matra
			{
					if ( ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + ( NChrsInCluster(obj) - 2 ) ) ) == 0x0D4A ) 	||
						 ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + ( NChrsInCluster(obj) - 2 ) ) ) == 0x0D4C )  )
					{
						wcTemp = 0x0D46;
					}
					else
					{
						wcTemp = 0x0D47;
					}
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = wcTemp | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = wcTemp | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
									*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
									dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wcTemp | 0xE7000000;
					O_IDX(obj) = O_IDX(obj) + nMatraPos ;	
					nMatraPos = 1; // initialize the nMatraPos variable
					
					if ( ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + ( NChrsInCluster(obj) - 2 ) ) ) == 0x0D4A ) 	||
						 ( WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + ( NChrsInCluster(obj) - 2 ) ) ) == 0x0D4B )  )
					{
						wcTemp = 0x0D3e;
					}
					else
					{
						wcTemp = 0x0D57;
					}
					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
						*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = wcTemp | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = wcTemp | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
									*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
									dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wcTemp | 0xE7000000;
					O_IDX(obj)++;

			} // End of split matra for putting the remaining part of matra 
		}	  // End of Right and Split matra condition
		else		// Left Matra
		{
				OUTPUT_OUT2INP_CACHE_CHECK

				wcTemp = WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + NChrsInCluster(obj) -2 ) ) ; 

				if (LOGICALSTRMTYPE(obj) == FileCode)
				{

#ifdef _BIG_ENDIAN
					*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = wcTemp | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj,O_IDX(obj));
						dummy = wcTemp | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wcTemp | 0xE7000000;

				O_IDX(obj)++ ;
		}

		OUTPUT_OUT2INP_CACHE_CHECK

		wcTemp = WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + NChrsInCluster(obj) -1 ) ) ; 

		if (LOGICALSTRMTYPE(obj) == FileCode)
		{

#ifdef _BIG_ENDIAN
			*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = wcTemp | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj,O_IDX(obj));
			dummy = wcTemp | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wcTemp | 0xE7000000;

		O_IDX(obj)++ ;

	  break;
	 }
	 case St7:
	 {
		
		NumMlCoreCons(obj) = NChrsInCluster(obj) - 1;

		if ( ( NChrsInCluster(obj) > 2 ) &&  
		     ( 
				 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D2F) &&
				   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 4 ) ) != 0x0D2F) 
				 ) ||
				 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D35) &&
				   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 4 ) ) != 0x0D35) 
				 ) ||
				 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D30) ||
				  
				 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D31) && 
				   ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 4 ) ) != 0x0D31) ) 
				 )
			)
		{		 
				
				if ( ( NChrsInCluster(obj) > 4 ) && 	
					 ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D2F ) &&
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 4 ) ) == 0x0D30 ) ||
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D35 ) &&
					 ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 4 ) ) == 0x0D30 ) ) )
				{
					nSpecialCaseFlag = DOUBLE_CONSONANT_SIGN;			
					NumMlCoreCons(obj) = NChrsInCluster(obj) - 5;
				}
				else
				{
					NumMlCoreCons(obj) = NChrsInCluster(obj) - 3;
				}

				GetMlCoreConsGlyphs(obj);
				// the last consonant in the input string is ra then the halant ra glyph shld be placed before the last 
				// glyph in the output buffer
				// For this copy the last element of the output buffer at the current position of O_IDX and then decrement O_IDX 
				// and  copy the glyph of halant ra at this position. Now increment  O_IDX by 2 

				if ( ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D30 ) || 
				     ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D31 ) )
				{

					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
						 

#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );

					O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )


					OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF483 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

					O_IDX(obj) = O_IDX(obj) + 2  ;	

				}
				else if ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D2F)
				{

					if ( nSpecialCaseFlag == DOUBLE_CONSONANT_SIGN )
					{
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
						 

#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
	
						O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )

	
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF483 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

						O_IDX(obj) = O_IDX(obj) + 2  ;	


					}

					 OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF481 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF481 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF481 | 0xE7000000;

					O_IDX(obj)++;

				}
				else if ( WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, MlJJJ(obj) + NChrsInCluster(obj) - 2 ) ) == 0x0D35)
				{

					if ( nSpecialCaseFlag == DOUBLE_CONSONANT_SIGN )
					{
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
						 

#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = *( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) -1 ) ) );
	
						O_IDX(obj)--;	// move to the previous position ..Here we put the glyph for ( halant ra )

	
						OUTPUT_OUT2INP_CACHE_CHECK
						 if (LOGICALSTRMTYPE(obj) == FileCode)
						 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF483 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF483 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF483 | 0xE7000000;

						O_IDX(obj) = O_IDX(obj) + 2  ;	


					}
					 OUTPUT_OUT2INP_CACHE_CHECK
					 if (LOGICALSTRMTYPE(obj) == FileCode)
					 {
#ifdef _BIG_ENDIAN
							*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0xF482 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj,O_IDX(obj));
							dummy = 0xF482 | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
										*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
										dummy <<= 8;
							}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF482 | 0xE7000000;

					O_IDX(obj)++;

				}
		
		}	
		else
		{
			GetMlCoreConsGlyphs(obj);
		}

		OUTPUT_OUT2INP_CACHE_CHECK

		wcTemp = WCINPBUF_AT(obj,MAPPING_V2L_AT(obj,MlJJJ(obj) + NChrsInCluster(obj) -1 ) ) ; 

		if (LOGICALSTRMTYPE(obj) == FileCode)
		{

#ifdef _BIG_ENDIAN
			*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = wcTemp | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
			out_p = &OUTBUF_AT(obj,O_IDX(obj));
			dummy = wcTemp | 0xE7000000;
			for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
			{
				*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
				dummy <<= 8;
			}
#endif
		}
		else
			*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = wcTemp | 0xE7000000;

		O_IDX(obj)++ ;
		 break;
	 }
	 case St8:
	  {
		  
  			 OUTPUT_OUT2INP_CACHE_CHECK
			 if (LOGICALSTRMTYPE(obj) == FileCode)
			 {
#ifdef _BIG_ENDIAN
				*( (wchar_t *) (&OUTBUF_AT(obj,O_IDX( obj) ) ) ) = 0x0000F480 | 0xE7000000;
#endif
#ifdef _LITTLE_ENDIAN
				out_p = &OUTBUF_AT(obj,O_IDX(obj));
				dummy = 0x0000F480 | 0xE7000000;
				for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
				{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
				}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0000F480 | 0xE7000000;
			O_IDX(obj)++;	

		  GetMlCoreConsGlyphs(obj);
		  break;
	  }


  }

  /* OutToInp */
	  if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
			  for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
			  {
				  o2i_p = &OUTTOINP_AT(obj, i*SHPNUMBYTES(obj));
				  for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
						*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
			  }
	  }
	  else
		  for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
				OUTTOINP_AT(obj, i) = MAPPING_V2L_AT(obj, JJJ(obj));

	  OUTSIZE(obj) = O_IDX(obj);					


}

static void
NextCluster_Ml(obj)
  LayoutObj		obj;
{
	  wchar_t	wc_j = L'\0';
	  wchar_t	wc_jj = L'\0';
	  wchar_t	wc_Temp = L'\0';
	  int		n_j;
	  int		nConjunctFlag =0 ;


	  if (NEXT_CLUSTER(obj) < END_HDRT(obj))
	  {
			MlClusterTypeState(obj) = St0;
			if (NChrsInCluster(obj) != -1)
				JJJ(obj) = NEXT_CLUSTER(obj);

			if (JJJ(obj) < END_HDRT(obj))
			{
				wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
				MlClusterTypeState(obj) = UpdateMlClusterTypeState(obj, MlClusterTypeState(obj), wc_j);
			}
			else
				wc_j = L'\0';

			n_j = JJJ(obj) + 1;
			if (n_j < END_HDRT(obj))
				wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
			else
				wc_jj = L'\0';

			NChrsInCluster(obj) = 1;
			while ( (wc_j != L'\0') && (wc_jj != L'\0') && (IsMlComposible(obj, wc_j, wc_jj) ) )
			{
				NChrsInCluster(obj)++;
				wc_Temp = wc_j;
				wc_j = wc_jj;
				MlClusterTypeState(obj) = UpdateMlClusterTypeState(obj, MlClusterTypeState(obj), wc_j);
				n_j++;
				if( (MlClusterTypeState(obj) == St4) && NChrsInCluster(obj) == 8 )
					break;

				if (n_j < END_HDRT(obj))
					wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
				else
					wc_jj = L'\0';

			}
			NEXT_CLUSTER(obj) = n_j;
	  }
	  else
			NChrsInCluster(obj) = 0;
}

static	void
func_malayalam(obj)
  LayoutObj		obj;
{
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Ml (obj);
  while (NChrsInCluster(obj) > 0)
    {
      GetGlyphsFromMlCluster (obj);
      NextCluster_Ml (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
                                         : ReverseDirection_WC(obj) );
}


/* Punjabi */


static void
GetPnCoreConsGlyphs(obj)
  LayoutObj	obj;
{
  int	i, j, k;
  wchar_t temp_out;
  int	delta, nMin, nMaxFocBlockLength, nMapIndex;
  BooleanValue StillMatching;
  char	*out_p;
  u_int	dummy;
  int nIndex = 0;
	i = 0;				// i denotes the iscii index 

    while (i < NumPnCoreCons(obj) )
	{
		nIndex = i ;
		if(WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, PnJJJ(obj) + i )) ) 
		 	nMaxFocBlockLength = GetPn_nMaxFocBlockTbl(obj,\
								 WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, PnJJJ(obj) + i)));
  		else
			return;

		   while (nMaxFocBlockLength)
		   {
				nMin = MIN(nMaxFocBlockLength, NumPnCoreCons(obj) );
				nMapIndex = 0;

				NotFoundEntryYet:
					j = delta = 0;
					StillMatching = FALSE;
					while (((delta < nMin) || PN_GLYPH_TBL(obj)[nMapIndex].strISCII[j])
										   && (nMapIndex < MAP_PN_SIZE) )
					{
						StillMatching = TRUE;
						if ((delta < nMin) && (j < MAX_CORE_CONS_BUF) && 
							 (WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, PnJJJ(obj) + i + delta)) != 
							 PN_GLYPH_TBL(obj)[nMapIndex].strISCII[j]))
						{
								nMapIndex++;
								goto NotFoundEntryYet;
						}
						delta++;
						j++;
						if ( (delta == nMin) && ( PN_GLYPH_TBL(obj)[nMapIndex].strISCII[j]) &&\
						     ( ! WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, PnJJJ(obj) + i + delta))  ) )
						{
							nMapIndex++;
							goto NotFoundEntryYet;
						}
					}

					if (StillMatching)		
						break;			/* Found */
					else
						nMaxFocBlockLength--;
		   }

		  i += nMin;

		/* What's if can't find entry in the table */
		  if ((StillMatching == FALSE) || (nMapIndex >= MAP_PN_SIZE))
		  {
			   for (j=0; j < NumPnCoreCons(obj); j++)
			   {
						/* OutBuf */
					  OUTPUT_OUT2INP_CACHE_CHECK
					  if (LOGICALSTRMTYPE(obj) == FileCode)
					  {
#ifdef	_BIG_ENDIAN
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
							WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, PnJJJ(obj) + j)) | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
							out_p = &OUTBUF_AT(obj, O_IDX(obj));
							dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, PnJJJ(obj) + j)) | 0xE7000000;
							for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
							{
								*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
								dummy <<= 8;
							}
#endif
					  }
		 			  else
						  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
						  WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, PnJJJ(obj) + j)) | 0xE7000000;
							
					  O_IDX(obj)++;
				}
		  }
		  else
		  {

				for (j=0; PN_GLYPH_TBL(obj)[nMapIndex].strISFOC[j]; j++)
				{
	       
						  OUTPUT_OUT2INP_CACHE_CHECK
						  if (LOGICALSTRMTYPE(obj) == FileCode)
 						  {
#ifdef	_BIG_ENDIAN
								*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
								PN_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
								out_p = &OUTBUF_AT(obj, O_IDX(obj));
								dummy = PN_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
								for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
								{
									*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
									dummy <<= 8;
								}
#endif
						}
						else
							*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
							PN_GLYPH_TBL(obj)[nMapIndex].strISFOC[j] | 0xE7000000;
						O_IDX(obj)++;
				}
		  }
	}
}

static void
GetGlyphsFromPnCluster (obj)
  LayoutObj	obj;
{
  int		i, next_j;
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  int		k, len;
  char		*out_p;
  u_int		dummy;
  wchar_t	wcTemp;
  int		nIndex =0;
  int		nCharsCount =0;
  FIRST_O_IDX(obj) = O_IDX(obj);
  NumPnCoreCons(obj) = NChrsInCluster(obj);
  PnJJJ(obj) = JJJ(obj);

  
  /* Property, InpToOut */							// Check this part later 
  if (LOGICALSTRMTYPE(obj) == FileCode)
  {
		  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
				UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
		  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
		  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));

		  for (k=0; k < len; k++, prop_p++, i2o_p++)
		  {
			  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
			  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
		  }
      
		  next_j = JJJ(obj);
		  for (i = 1; i < NChrsInCluster(obj); i++)
		  {
			  next_j++;
			  if (next_j < END_HDRT(obj))
			  {
				  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)+1) -
			  	  	    UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)); 
			  }
			  else
				 len = 0;

			  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
			  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, next_j)));
			  for (k=0; k < len; k++, prop_p++, i2o_p++)
			  {
				  *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
				  *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
			  }
		  }
  }
  else
  {
		  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) =
  		  (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
		  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
		  next_j = JJJ(obj);
		  for (i = 1; i < NChrsInCluster(obj); i++)
			{
		  next_j++;

		  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, next_j)) =
			(LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, next_j)) & 0x0F);
		  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, next_j)) = O_IDX(obj);
			}
  }
  switch (PnClusterTypeState(obj))
  {
	  case St1:
	  case St2:
	  case St11:
	  {
		 GetPnCoreConsGlyphs(obj);
		  break;
	  }
	  case St4:
	  {
				while(PnHalantCount(obj) )
				{
						NumPnCoreCons(obj) = 1;
						GetPnCoreConsGlyphs(obj);
						PnHalantCount(obj) = PnHalantCount(obj) -1 ;
						if( PnHalantCount(obj) > 0 )
							PnJJJ(obj) = PnJJJ(obj) + 2;
						
				}

	  	break;
	  }
	  case St3:
	  case St5:
	  case St6:
	  {
			if( PnHalantCount(obj) == 0 )
			{
				GetPnCoreConsGlyphs(obj);
			}
			else if( PnHalantCount(obj) == 1 )
			{
				// Check if previous character or sequence of character are valid 
				// if yes then
				// GetPnCoreConsGlyphs(obj);
				// else
				// Ignore the halants and get glyphs
				if ( ( O_IDX(obj) > 0 ) &&  CheckIsPnValidSequence(obj, ( O_IDX(obj) - 1 ) ) )
					GetPnCoreConsGlyphs(obj);
				else
				{
						NumPnCoreCons(obj) = 1 ;
						GetPnCoreConsGlyphs(obj);
						PnJJJ(obj) = PnJJJ(obj) + 2 ;
						NumPnCoreCons(obj) = NChrsInCluster(obj) - 2;
						GetPnCoreConsGlyphs(obj);
				}

			}
			else if ( PnHalantCount(obj) > 1 )
			{
				// Ignore the halant and get glyphs
				nIndex = PnHalantCount(obj);
				while(PnHalantCount(obj) )
				{
						NumPnCoreCons(obj) = 1;
						GetPnCoreConsGlyphs(obj);
						PnJJJ(obj) = PnJJJ(obj) + 2;
						PnHalantCount(obj) = PnHalantCount(obj) -1 ;
				}

				NumPnCoreCons(obj) = NChrsInCluster(obj) - ( 2 * nIndex );
				GetPnCoreConsGlyphs(obj);
			}

		break;
	  } 
	  case 7:
	  {
	
		  if( PnHalantCount(obj) == 0 )
		  {
				  OUTPUT_OUT2INP_CACHE_CHECK
				  if (LOGICALSTRMTYPE(obj) == FileCode)
 				  {
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A3F | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
				O_IDX(obj)++;
				
				NumPnCoreCons(obj) = NChrsInCluster(obj) -1 ;
				GetPnCoreConsGlyphs(obj);
	
		  }
		  else if( PnHalantCount(obj) == 1 )
		  {
			  if ( CheckIsPnValidSequence(obj, ( O_IDX(obj) - 1 ) ) )
			  {

				    // First put the adhak
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
 					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A71 | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A71 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A71 | 0xE7000000;
					O_IDX(obj)++;

					// Then put the I matra
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
 					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A3F | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
					O_IDX(obj)++;

					// Now the remainig characters
					NumPnCoreCons(obj) = 1 ;
					GetPnCoreConsGlyphs(obj);
			  }
			  else
			  {
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
 					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A3F | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
					O_IDX(obj)++;
					
					NumPnCoreCons(obj) = 1 ;
					GetPnCoreConsGlyphs(obj);
					PnJJJ(obj) = PnJJJ(obj) + 2;
					NumPnCoreCons(obj) = 1 ;
					GetPnCoreConsGlyphs(obj);

			  }

		  }
		  else if( PnHalantCount(obj) > 1 )
		  {
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
 					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A3F | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
					O_IDX(obj)++;

					nIndex = PnHalantCount(obj);
					while(PnHalantCount(obj) )
					{
							NumPnCoreCons(obj) = 1;
							GetPnCoreConsGlyphs(obj);
							PnJJJ(obj) = PnJJJ(obj) + 2;
							PnHalantCount(obj) = PnHalantCount(obj) -1 ;
					}

					NumPnCoreCons(obj) = NChrsInCluster(obj) - ( 2 * nIndex ) - 1;
					GetPnCoreConsGlyphs(obj);

		  }
		 break;
			
	  }
	  case 8:
	  {
	
		  if( PnHalantCount(obj) == 0 )
		  {
				  OUTPUT_OUT2INP_CACHE_CHECK
				  if (LOGICALSTRMTYPE(obj) == FileCode)
 				  {
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A3F | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
				O_IDX(obj)++;
				
				NumPnCoreCons(obj) = NChrsInCluster(obj) - 2 ;
				GetPnCoreConsGlyphs(obj);

				OUTPUT_OUT2INP_CACHE_CHECK
				  if (LOGICALSTRMTYPE(obj) == FileCode)
 				  {
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A70 | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A70 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A70 | 0xE7000000;
				O_IDX(obj)++;

	
		  }
		  else if( PnHalantCount(obj) == 1 )
		  {
			  if ( CheckIsPnValidSequence(obj, ( O_IDX(obj) - 1 ) ) )
			  {

				    // First put the adhak
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
 					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A71 | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A71 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A71 | 0xE7000000;
					O_IDX(obj)++;

					// Then put the I matra
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
 					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A3F | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
					O_IDX(obj)++;

					// Now the remaining characters
					NumPnCoreCons(obj) = 1 ;
					GetPnCoreConsGlyphs(obj);

					
			  }
			  else
			  {
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
 					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A3F | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
					O_IDX(obj)++;
					
					NumPnCoreCons(obj) = 1 ;
					GetPnCoreConsGlyphs(obj);
					PnJJJ(obj) = PnJJJ(obj) + 2;
					NumPnCoreCons(obj) = 1 ;
					GetPnCoreConsGlyphs(obj);
			  }

			  OUTPUT_OUT2INP_CACHE_CHECK
			  if (LOGICALSTRMTYPE(obj) == FileCode)
			  {
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A70 | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A70 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
			  }
			  else
			 	*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A70 | 0xE7000000;
			  O_IDX(obj)++;

		  }
		  else if( PnHalantCount(obj) > 1 )
		  {
					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
 					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A3F | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A3F | 0xE7000000;
					O_IDX(obj)++;

					nIndex = PnHalantCount(obj);
					while(PnHalantCount(obj) )
					{
							NumPnCoreCons(obj) = 1;
							GetPnCoreConsGlyphs(obj);
							PnJJJ(obj) = PnJJJ(obj) + 2;
							PnHalantCount(obj) = PnHalantCount(obj) -1 ;
					}

					NumPnCoreCons(obj) = NChrsInCluster(obj) - ( 2 * nIndex ) - 2;
					GetPnCoreConsGlyphs(obj);

					OUTPUT_OUT2INP_CACHE_CHECK
					if (LOGICALSTRMTYPE(obj) == FileCode)
					{
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A70 | 0xE7000000;
#endif
#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0x0A70 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
					}
					else
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0x0A70 | 0xE7000000;
					O_IDX(obj)++;
		  }
		 break;
			
	  }
	  case 9:
	  {
	
		  if( PnHalantCount(obj) == 0 )
		  {
				GetPnCoreConsGlyphs(obj);
		  }
		  else if( PnHalantCount(obj) == 1 )
		  {
				if ( CheckIsPnValidSequence(obj, ( O_IDX(obj) - 1 ) ) )
				{
					GetPnCoreConsGlyphs(obj);
				}
				else
				{
						NumPnCoreCons(obj) = 1 ;
						GetPnCoreConsGlyphs(obj);
						PnJJJ(obj) = PnJJJ(obj) + 2 ;
						NumPnCoreCons(obj) = NChrsInCluster(obj) - 2;
						GetPnCoreConsGlyphs(obj);					
				}
		  }
		  else if( PnHalantCount(obj) > 1 )
		  {
				nIndex = PnHalantCount(obj);
				while(PnHalantCount(obj) )
				{
						NumPnCoreCons(obj) = 1;
						GetPnCoreConsGlyphs(obj);
						PnJJJ(obj) = PnJJJ(obj) + 2;
						PnHalantCount(obj) = PnHalantCount(obj) -1 ;
				}

				NumPnCoreCons(obj) = NChrsInCluster(obj) - ( 2 * nIndex );
				GetPnCoreConsGlyphs(obj);
		  }
		 break;
			
	  }
	  case 10:
	  {

		  if(WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, PnJJJ(obj) ) ) == 0x0A3F) 
		  {
				
				GetPnCoreConsGlyphs(obj);
				  OUTPUT_OUT2INP_CACHE_CHECK
				  if (LOGICALSTRMTYPE(obj) == FileCode)
 				  {
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF31F | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0xF31F | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF31F | 0xE7000000;
				O_IDX(obj)++;

		  }
		  /***************************************************************************/
		  else if(WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, PnJJJ(obj) ) ) == 0x0A4D) 
		  {
				
				  OUTPUT_OUT2INP_CACHE_CHECK
				  if (LOGICALSTRMTYPE(obj) == FileCode)
 				  {
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF31F | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0xF31F | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF31F | 0xE7000000;
				O_IDX(obj)++;

				
				OUTPUT_OUT2INP_CACHE_CHECK
				if (LOGICALSTRMTYPE(obj) == FileCode)
 				  {
#ifdef	_BIG_ENDIAN
						*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF339 | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
						out_p = &OUTBUF_AT(obj, O_IDX(obj));
						dummy = 0xF339 | 0xE7000000;
						for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
						{
							*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
							dummy <<= 8;
						}
#endif
				}
				else
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF339 | 0xE7000000;
				O_IDX(obj)++;


		  }	
		  /***************************************************************************/
		  else
		  {
			  OUTPUT_OUT2INP_CACHE_CHECK
			  if (LOGICALSTRMTYPE(obj) == FileCode)
			  {
#ifdef	_BIG_ENDIAN
					*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF31F | 0xE7000000;
#endif

#ifdef	_LITTLE_ENDIAN
					out_p = &OUTBUF_AT(obj, O_IDX(obj));
					dummy = 0xF31F | 0xE7000000;
					for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
					{
						*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
						dummy <<= 8;
					}
#endif
			}
			else
				*((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) = 0xF31F | 0xE7000000;
			O_IDX(obj)++;
			GetPnCoreConsGlyphs(obj);
		  }
		  break;

	  }



	  
  }

  /* OutToInp */
	  if (LOGICALSTRMTYPE(obj) == FileCode)
	  {
			  for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
			  {
				  o2i_p = &OUTTOINP_AT(obj, i*SHPNUMBYTES(obj));
				  for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
						*o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
			  }
	  }
	  else
		  for (i = FIRST_O_IDX(obj); i < O_IDX(obj); i++)
				OUTTOINP_AT(obj, i) = MAPPING_V2L_AT(obj, JJJ(obj));

	  OUTSIZE(obj) = O_IDX(obj);					


}


static void
NextCluster_Punjabi(obj)
  LayoutObj		obj;
{
  wchar_t	wc_j = L'\0';
  wchar_t	wc_jj = L'\0';
  int		n_j;

  PnHalantCount(obj) = 0;

  if (NEXT_CLUSTER(obj) < END_HDRT(obj))
  {
		PnClusterTypeState(obj) = St0;
		if (NChrsInCluster(obj) != -1)
			JJJ(obj) = NEXT_CLUSTER(obj);

		if (JJJ(obj) < END_HDRT(obj))
		{
			wc_j = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
			PnClusterTypeState(obj) = UpdatePnClusterTypeState(obj, PnClusterTypeState(obj), wc_j);
		}
		else
			wc_j = L'\0';

		n_j = JJJ(obj) + 1;
		if (n_j < END_HDRT(obj))
			wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
		else
			wc_jj = L'\0';

		NChrsInCluster(obj) = 1;

		while ( (wc_j != L'\0') && (wc_jj != L'\0') && IsPnComposible(obj, wc_j, wc_jj) )
		{
			if( (PnClusterTypeState(obj) == 3) && (PN_CHRCLS_TBL(obj)[(ucs2pn_index((wc_jj)))] & (_PH) ) )
			{
				PnHalantCount(obj) = PnHalantCount(obj) + 1;							
			}
			NChrsInCluster(obj)++;
			wc_j = wc_jj;
			PnClusterTypeState(obj) = UpdatePnClusterTypeState(obj, PnClusterTypeState(obj), wc_j);
			n_j++;
			if (n_j < END_HDRT(obj))
				wc_jj = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
			else
				wc_jj = L'\0';
		}
		NEXT_CLUSTER(obj) = n_j;

  }
  else
       NChrsInCluster(obj) = 0;
}

static	void
func_punjabi(obj)
  LayoutObj		obj;
{
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextCluster_Punjabi (obj);
  while (NChrsInCluster(obj) > 0)
    {
      GetGlyphsFromPnCluster (obj);
      NextCluster_Punjabi (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
                                         : ReverseDirection_WC(obj) );
}

static void
GetGlyphsFromUCS4Cluster_MB (obj)
  LayoutObj	obj;
{
  unsigned char *prop_p;
  size_t	*o2i_p;
  size_t	*i2o_p;
  char		*out_p;
  u_int		dummy;
  int		k, len;

  len = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))+1) -
	UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));

  /* Property, InpToOut */
  prop_p = &PROPERTY_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
  i2o_p = &INPTOOUT_AT(obj, UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))));
  for (k=0; k < len; k++, prop_p++, i2o_p++)
    {
      *prop_p = (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;
      *i2o_p = O_IDX(obj)*SHPNUMBYTES(obj);
    }

  /* OutBuf, OutToInp */
  OUTPUT_OUT2INP_CACHE_CHECK
#ifdef	_BIG_ENDIAN
  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
#endif

#ifdef	_LITTLE_ENDIAN
  out_p = &OUTBUF_AT(obj, O_IDX(obj));
  dummy = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  for (k=0; k < SHPNUMBYTES(obj); k++, out_p++)
    {
	*out_p = (0x000000ff & ((dummy & 0xff000000) >> 24));
	dummy <<= 8;
    }
#endif
  o2i_p = &OUTTOINP_AT(obj, O_IDX(obj)*SHPNUMBYTES(obj));
  for (k=0; k < SHPNUMBYTES(obj); k++, o2i_p++)
    *o2i_p = UNIT_TO_MB_IDX_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  O_IDX(obj)++;
  OUTSIZE(obj) = O_IDX(obj);
}

static void
GetGlyphsFromUCS4Cluster_WC (obj)
  LayoutObj	obj;
{
  /* Property */
  PROPERTY_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = 
    (LOGICAL_EMBEDDINGS_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) & 0x0F) | 0x80;

  /* OutBuf, OutToInp, InpToOut */
  OUTPUT_OUT2INP_CACHE_CHECK
  *((wchar_t *)(&OUTBUF_AT(obj, O_IDX(obj)))) =
	WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
  OUTTOINP_AT(obj, O_IDX(obj)) = MAPPING_V2L_AT(obj, JJJ(obj));
  INPTOOUT_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj))) = O_IDX(obj);
  O_IDX(obj)++;
  OUTSIZE(obj) = O_IDX(obj);
}

static void
NextUCS4Cluster(obj)
  LayoutObj		obj;
{
  int		n_j;

  if (((IS_REVERSE_AT(obj, III(obj)) && (END_HDRT(obj) < NEXT_CLUSTER(obj))) ||
       (!IS_REVERSE_AT(obj, III(obj)) && (NEXT_CLUSTER(obj) < END_HDRT(obj)))))
  {
  if (NChrsInCluster(obj) != -1)
    JJJ(obj) = NEXT_CLUSTER(obj);
  NChrsInCluster(obj) = 1;
  (IS_REVERSE_AT(obj, III(obj)) ? (n_j = JJJ(obj) - 1) : (n_j = JJJ(obj) + 1) );
  NEXT_CLUSTER(obj) = n_j;
  } else
      NChrsInCluster(obj) = 0;
}

static void
func_ucs4(obj)
  LayoutObj		obj;
{
  int   jjj;
  int   end_hdrt = END_HDRT(obj);

  if (IS_REVERSE_AT(obj, III(obj)))
    {
      jjj = JJJ(obj);
      JJJ(obj) = END_HDRT(obj) - 1;
      END_HDRT(obj) = jjj - 1;
    }
  NChrsInCluster(obj) = -1;
  NEXT_CLUSTER(obj) = JJJ(obj);
  NextUCS4Cluster (obj);
  while (NChrsInCluster(obj) != 0)
    {
      if (LOGICALSTRMTYPE(obj) == FileCode)
	GetGlyphsFromUCS4Cluster_MB (obj);
      else
	GetGlyphsFromUCS4Cluster_WC (obj);
      NextUCS4Cluster (obj);
    }
  if (IS_REVERSE_AT(obj, III(obj)))
   {
     JJJ(obj) = end_hdrt;
    ( (LOGICALSTRMTYPE(obj) == FileCode) ? ReverseDirection_MB(obj)
    					 : ReverseDirection_WC(obj) );
     JJJ(obj) = END_HDRT(obj) = end_hdrt;
   }
}

static void
ApplyShaping(obj)
  LayoutObject	obj;
{
  wchar_t	current_wc;
  wchar_t	next_wc;
  int		n_j;
  int		k;

    while (III(obj) < NUM_UNIT(obj))
      {
        for (k=III(obj)+1, WIDTH(obj)=1; (k < NUM_UNIT(obj)) &&
             (VISUAL_EMBEDDINGS_AT(obj, III(obj)) == VISUAL_EMBEDDINGS_AT(obj, k));
             k++, WIDTH(obj)++);
	JJJ(obj) = III(obj);
	END_HDRT(obj) = III(obj) + WIDTH(obj);

	current_wc = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, JJJ(obj)));
	n_j = JJJ(obj) + 1;
	if (n_j < III(obj) + WIDTH(obj))
	  next_wc = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
	else
	  next_wc = L'\0';

	do
	  {
	    if (obj->core.active_shape_editing)
	      {
		if (FUNC_POINTER(obj, current_wc) != FUNC_POINTER(obj, next_wc))
	          {
		    TMP_O_IDX(obj) = O_IDX(obj);
		    END_HDRT(obj) = n_j;
		    (FUNC_POINTER(obj, CURR_WC_CLUSTER) != NULL ?
			 (ACTIVATE_FUNC(obj, CURR_WC_CLUSTER)) : ACTIVATE_F_UCS4(obj) );
		    JJJ(obj) = END_HDRT(obj);
	          }
	      }
	    else
	      {
		TMP_O_IDX(obj) = O_IDX(obj);
		END_HDRT(obj) = n_j;
		ACTIVATE_F_UCS4(obj);
		JJJ(obj) = END_HDRT(obj);
	      }

	    current_wc = next_wc;
	    n_j++;
	    if (n_j < III(obj) + WIDTH(obj))
	      next_wc = WCINPBUF_AT(obj, MAPPING_V2L_AT(obj, n_j));
/*
 * Must not change next_wc because of FUNC_POINTER will not be correctly used
	    else
	      next_wc = L'\0';
*/
	  }
	while (n_j < III(obj) + WIDTH(obj));
	if (JJJ(obj) < III(obj) + WIDTH(obj))
	  {
	    TMP_O_IDX(obj) = O_IDX(obj);
	    END_HDRT(obj) = III(obj) + WIDTH(obj);

	    (FUNC_POINTER(obj, CURR_WC_CLUSTER) != NULL ?
		 (ACTIVATE_FUNC(obj, CURR_WC_CLUSTER)) : ACTIVATE_F_UCS4(obj) );
	    JJJ(obj) = END_HDRT(obj);
	  }

	III(obj) += WIDTH(obj);
      }
}

static int
_UMLETransform(obj, InpBuf, InpSize, OutBuf, OutSize,
		   InpToOut, OutToInp, Property, InpBufIndex)
    LayoutObj			obj;
    const void			*InpBuf;
    size_t			InpSize;
    void			*OutBuf;
    size_t			*OutSize;
    size_t			*InpToOut;
    size_t			*OutToInp;
    unsigned char		*Property;
    size_t   			*InpBufIndex;
{
    char			*IntOutBuf = (char *) OutBuf;
    int				TempInpBufIndex;
    int				TempInpSize;
    int				TempOutSize;
    int				num_bytes;
    int				nbytes_unit;
    int				(*MbUnitLen)() = (int(*)())(MBINPUNITLEN_F(obj));
    int				(*MbToWc)() = (int(*)())(MBTOWC(obj));
    char			*mbP;
    wchar_t			*wcP;
    int				j;
    int				result = 0;
    BooleanValue                IsOutBufNotEnough = FALSE;
    int				mb_idx = 0;

    errno = 0;
    if (InpBuf == NULL) {
        /* Reset bidi nest level state */
       return(result);
    }
    if (LOGICALSTRMTYPE(obj) == FileCode) {
	TempInpBufIndex = (InpBufIndex ? *InpBufIndex : 0);
	TempInpSize = (InpSize == (size_t)-1) ? 0 : InpSize;
	TempOutSize = (OutSize ? *OutSize : 0);
    } else {
	TempInpBufIndex = (InpBufIndex ? *InpBufIndex : 0);
	TempInpSize = (InpSize == (size_t)-1) ? 0 : InpSize;
	TempOutSize = (OutSize ? (*OutSize)*SHPNUMBYTES(obj) : 0);
    }

    /* Step 1: Get Number of Units */
    NUM_UNIT(obj) = 0;
    num_bytes = 0;
    mbP = (char *)InpBuf;
    wcP = (wchar_t *)InpBuf;
    if (TempInpSize == 0) {
	if (LOGICALSTRMTYPE(obj) == FileCode) {
	    do {
		nbytes_unit = (*MbUnitLen)(mbP+TempInpBufIndex+num_bytes);
		if (nbytes_unit > 0) {
		    num_bytes += nbytes_unit;
		    NUM_UNIT(obj)++;
		} else if (nbytes_unit < 0) {
		    num_bytes++;
		    NUM_UNIT(obj)++;
		}
	    } while (nbytes_unit != 0);
	} else {
	    nbytes_unit = wcslen(wcP + TempInpBufIndex);

	    num_bytes = NUM_UNIT(obj) = nbytes_unit;
	}
    } else {
	if (LOGICALSTRMTYPE(obj) == FileCode) {
	    while (num_bytes < TempInpSize) {
		nbytes_unit = (*MbUnitLen)(mbP+TempInpBufIndex+num_bytes);
		num_bytes += ((nbytes_unit > 0) ? nbytes_unit : 1);
		NUM_UNIT(obj)++;
	    }
	} else {
	    /* ASSERT(TempInpSize > 0); */
	    num_bytes = NUM_UNIT(obj) = TempInpSize;
	}
    }

    /* Step 2: Allocation Internal Cache Buffer */
    if (INPMBCZ(obj) < (NUM_UNIT(obj)+1)*MAXBYTES(obj)) {
	MBINPBUF(obj) = (char *)CacheRealloc(INPMBCZ(obj),
			(NUM_UNIT(obj)+1)*MAXBYTES(obj)+UMLE_CACHE_SIZE,
				MBINPBUF(obj), MBINPBUF_CACHE(obj));
	INPMBCZ(obj) = (NUM_UNIT(obj)+1)*MAXBYTES(obj)+UMLE_CACHE_SIZE;
    }
    if (INPWCCZ(obj) < (NUM_UNIT(obj)+1)) {
	WCINPBUF(obj) = (wchar_t *)CacheRealloc(INPWCCZ(obj) * sizeof(wchar_t),
			(NUM_UNIT(obj) + 1 + UMLE_CACHE_SIZE) * sizeof(wchar_t),
				WCINPBUF(obj), WCINPBUF_CACHE(obj));
	INPWCCZ(obj) = NUM_UNIT(obj) + 1 + UMLE_CACHE_SIZE;
    }
    if (LOGICALSTRMTYPE(obj) == FileCode) {
	if (I2OCZ(obj) < NUM_UNIT(obj)*MAXBYTES(obj)*sizeof(size_t)) {
	    INPTOOUT(obj) = (size_t *)CacheRealloc(I2OCZ(obj),
		(NUM_UNIT(obj)*MAXBYTES(obj)+UMLE_CACHE_SIZE)*sizeof(size_t),
				INPTOOUT(obj), INPTOOUT_CACHE(obj));
	    TMPINPTOOUT(obj) = (size_t *)CacheRealloc(I2OCZ(obj),
		(NUM_UNIT(obj)*MAXBYTES(obj)+UMLE_CACHE_SIZE)*sizeof(size_t),
				TMPINPTOOUT(obj), TMPINPTOOUT_CACHE(obj));
	    I2OCZ(obj) = (NUM_UNIT(obj)*MAXBYTES(obj)+UMLE_CACHE_SIZE) *
				sizeof(size_t);
	}
	if (PROPCZ(obj) < NUM_UNIT(obj)*MAXBYTES(obj)*sizeof(unsigned char)) {
	    PROPERTY(obj) = (unsigned char *)CacheRealloc(PROPCZ(obj),
		(NUM_UNIT(obj)*MAXBYTES(obj)+UMLE_CACHE_SIZE)*sizeof(unsigned char),
				PROPERTY(obj), PROPERTY_CACHE(obj));
	    PROPCZ(obj) = (NUM_UNIT(obj)*MAXBYTES(obj)+UMLE_CACHE_SIZE)*sizeof(unsigned char);
	}
    } else { /* if (LOGICALSTRMTYPE(obj) == ProcessCode) */
	if (I2OCZ(obj) < NUM_UNIT(obj)*sizeof(size_t)) {
	    INPTOOUT(obj) = (size_t *)CacheRealloc(I2OCZ(obj),
		(NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(size_t),
				INPTOOUT(obj), INPTOOUT_CACHE(obj));
	    TMPINPTOOUT(obj) = (size_t *)CacheRealloc(I2OCZ(obj),
		(NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(size_t),
				TMPINPTOOUT(obj), TMPINPTOOUT_CACHE(obj));
	    I2OCZ(obj) = (NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(size_t);
	}
	if (PROPCZ(obj) < NUM_UNIT(obj)*sizeof(unsigned char)) {
	    PROPERTY(obj) = (unsigned char *)CacheRealloc(PROPCZ(obj),
		(NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(unsigned char),
				PROPERTY(obj), PROPERTY_CACHE(obj));
	    PROPCZ(obj) = (NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(unsigned char);
	}
    }
    if (UNITCZ(obj) <= NUM_UNIT(obj)) {
	UNIT_TO_MB_IDX(obj) = (int *)CacheRealloc(UNITCZ(obj)*sizeof(int),
		(NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(int),
				UNIT_TO_MB_IDX(obj), UNIT_TO_MB_IDX_CACHE(obj));
	LOGICAL_EMBEDDINGS(obj) = (int *)CacheRealloc(UNITCZ(obj)*sizeof(int),
		(NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(int),
			LOGICAL_EMBEDDINGS(obj), LOGICAL_EMBEDDINGS_CACHE(obj));
	DIRS(obj) = (bidi_type_t *)CacheRealloc(UNITCZ(obj)*sizeof(bidi_type_t),
		(NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(bidi_type_t),
				DIRS(obj), DIRS_CACHE(obj));
	VISUAL_EMBEDDINGS(obj) = (int *)CacheRealloc(UNITCZ(obj)*sizeof(int),
		(NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(int),
			VISUAL_EMBEDDINGS(obj), VISUAL_EMBEDDINGS_CACHE(obj));
	MAPPING_L2V(obj) = (int *)CacheRealloc(UNITCZ(obj)*sizeof(int),
		(NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(int),
				MAPPING_L2V(obj), MAPPING_L2V_CACHE(obj));
	MAPPING_V2L(obj) = (int *)CacheRealloc(UNITCZ(obj)*sizeof(int),
		(NUM_UNIT(obj)+UMLE_CACHE_SIZE)*sizeof(int),
				MAPPING_V2L(obj), MAPPING_V2L_CACHE(obj));
	UNITCZ(obj) = NUM_UNIT(obj)+UMLE_CACHE_SIZE;
    }
    if (LOGICALSTRMTYPE(obj) == FileCode)
	memcpy(MBINPBUF(obj), mbP+TempInpBufIndex, num_bytes);
    else
	memcpy(WCINPBUF(obj), wcP+TempInpBufIndex, num_bytes*sizeof(wchar_t));

    /* Step 3: Calculate the index boundary of each logical unit */
    if (LOGICALSTRMTYPE(obj) == FileCode)
      {
        /* First: Does Conversion */
	for (j=0,mbP=MBINPBUF(obj),wcP=WCINPBUF(obj);
		j < NUM_UNIT(obj); j++, wcP++) {
		    nbytes_unit = (*MbToWc)(wcP, mbP);
		    if (nbytes_unit > 0)
			mbP += nbytes_unit;
		    else
			mbP++;
	}

	/* Second: Calculate Index */
	for (j=0, mb_idx=0, mbP=MBINPBUF(obj);
    		j < NUM_UNIT(obj); j++) {
	     UNIT_TO_MB_IDX_AT(obj, j) = mb_idx;
    	     nbytes_unit = (*MbUnitLen)(mbP);
	     if (nbytes_unit > 0) {
		 mb_idx += nbytes_unit;
		 mbP += nbytes_unit;
	     } else {
		 mb_idx++;
		 mbP++;
	     }
	}
	UNIT_TO_MB_IDX_AT(obj, NUM_UNIT(obj)) = mb_idx;
      }

    /* Step 4: Does the transformation */
    if (obj->core.active_dir)
	ApplyUnicodeBidi(obj);
    else
        memset(LOGICAL_EMBEDDINGS(obj), 0, NUM_UNIT(obj));
    GenL2V_V2Lmap(obj);
    III(obj) = JJJ(obj) = O_IDX(obj) = END_HDRT(obj) = 0;
    WIDTH(obj) = NUM_UNIT(obj);
    ApplyShaping(obj);

    /* Step 5: Matching the result */
    if (Property)
        memcpy(Property, PROPERTY(obj),
		sizeof(unsigned char)*((LOGICALSTRMTYPE(obj) == FileCode) ? num_bytes : NUM_UNIT(obj)));
    if (InpToOut)
    	memcpy(InpToOut, INPTOOUT(obj),
    		sizeof(size_t)*((LOGICALSTRMTYPE(obj) == FileCode) ? num_bytes : NUM_UNIT(obj)));
    if (OUTSIZE(obj)*SHPNUMBYTES(obj) <= TempOutSize) {
	if (IntOutBuf)
	    memcpy(IntOutBuf, OUTBUF(obj), sizeof(char)*OUTSIZE(obj)*SHPNUMBYTES(obj));
	if (OutToInp) {
	    if (LOGICALSTRMTYPE(obj) == FileCode)
		memcpy(OutToInp, OUTTOINP(obj), sizeof(size_t)*OUTSIZE(obj)*SHPNUMBYTES(obj));
	    else
		memcpy(OutToInp, OUTTOINP(obj), sizeof(size_t)*OUTSIZE(obj));
	}
    } else {
	if (IntOutBuf && (TempOutSize > 0))
	    memcpy(IntOutBuf, OUTBUF(obj), TempOutSize);
	IsOutBufNotEnough = (TempOutSize ? TRUE : FALSE);
    }
    if (IsOutBufNotEnough && (IntOutBuf != NULL)) {
        errno = E2BIG;
        result = -1;
    }
    if (InpBufIndex)
        *InpBufIndex =
                (IsOutBufNotEnough ? *InpBufIndex
				   : ((LOGICALSTRMTYPE(obj) == FileCode) ? (TempInpBufIndex + num_bytes)
									 : (*InpBufIndex + NUM_UNIT(obj)) ) );
    if (OutSize)
        *OutSize = (IsOutBufNotEnough ? *OutSize
				      : ((LOGICALSTRMTYPE(obj) == FileCode) ? OUTSIZE(obj)*SHPNUMBYTES(obj)
									    : OUTSIZE(obj)));
    return (result);
}

static int
_UMLETransform_mb(obj, InpBuf, InpSize, OutBuf, OutSize,
		   InpToOut, OutToInp, Property, InpBufIndex)
    LayoutObj			obj;
    const char			*InpBuf;
    size_t			InpSize;
    char			*OutBuf;
    size_t			*OutSize;
    size_t			*InpToOut;
    size_t			*OutToInp;
    unsigned char		*Property;
    size_t   			*InpBufIndex;
{
    if (obj == NULL) {
        errno = EFAULT;
        return (-1);
    }
    LOGICALSTRMTYPE(obj) = FileCode;
    return _UMLETransform(obj, (void *)InpBuf, InpSize, (void *)OutBuf, OutSize,
    		   InpToOut, OutToInp, Property, InpBufIndex);
}

static int
_UMLETransform_wc(obj, InpBuf, InpSize, OutBuf, OutSize,
		   InpToOut, OutToInp, Property, InpBufIndex)
    LayoutObject		obj;
    const wchar_t		*InpBuf;
    size_t			InpSize;
    wchar_t			*OutBuf;
    size_t			*OutSize;
    size_t			*InpToOut;
    size_t			*OutToInp;
    unsigned char		*Property;
    size_t			*InpBufIndex;		   

{
    if (obj == NULL) {
        errno = EFAULT;
        return (-1);
    }
    LOGICALSTRMTYPE(obj) = ProcessCode;
    return _UMLETransform(obj, (void *)InpBuf, InpSize, (void *)OutBuf, OutSize,
    		   InpToOut, OutToInp, Property, InpBufIndex);
}
