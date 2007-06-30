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
 * Copyright (c) 1999 by Sun Microsystems, Inc.
 * All rights reserved.
 */
#ifndef _PRINT_PREPROCESS_H
#define _PRINT_PREPROCESS_H

#pragma ident   "@(#)print_preprocess.h	1.2 99/07/25 SMI"

#include "tp1.h"
#include "pcf.h"
#include "ttf.h"

typedef union {
	ucs4_t ucs4val;
	struct { unsigned char c0, c1, c2, c3; } ucs4_equi;
}ucs4val_u;

typedef struct {
	int line_pos;
	ucs4_t val;
	int font_style;
	int print_style;
	int scale_fact;
	int disp_var;

	union {
		struct {
				pcf_SCcharmet_t *scCmet;
				pcf_charmet_t   *Cmet;
				pcf_bm_t	*pcfbm;
			} pcf_st;

		struct {
				ttffont_t	*ttfont;
				tt_OutlineData_t	*ttodata;
			} ttf_st;

		struct {
				tp1font_t	* tp1_font;
				ucs4_t		*target_char;
			} tp1_st;
		} font_u;

} print_info;

#endif /* _PRINT_PREPROCESS_H */
