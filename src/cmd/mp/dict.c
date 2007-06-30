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
 * Copyright (c) 2001 by Sun Microsystems, Inc.
 * All rights reserved.
 */
#pragma ident "@(#)dict.c 1.3 02/11/15 SMI"

#include <stdio.h>
#include "general_header.h"
typedef struct _DICT {
	int          num;
	float           dx;
} Dict;

static Dict             dict[DICTTYPES][DICTSIZE];
static int              num_dict = 0;

int mp_newdict_flg=0;

void
clear_dict()
{
	int	j,k;

	for(j = 0; j < DICTSIZE; j++){
		for(k=0; k<DICTTYPES; k++){
			dict[k][j].num = 0;
			dict[k][j].dx  = 0.0;
		}
	}
	num_dict = 0;
}

int
check_dict(int k, float dx, int pstyle)
{
	int	i, j;
	static int first_dict=1;

	if(pstyle==HDNGFONT) return -1;

	for(j = 0; j < num_dict; j++){
		if (dict[pstyle][j].num == k )
			return(j);
	}

	if(!(num_dict % (DICTSIZE-1))) {
		clear_dict();
		if(first_dict!=1) {
			fprintf(stdout,"\nend");
		}
		first_dict=2;
		mp_newdict_flg=1;
		fprintf(stdout,"\n%d dict begin\n", DICTSIZE);
	}
	/* Need to clear the old dict entries before storing */

	for(i=0; i<DICTTYPES; i++){
		dict[i][num_dict].num = 0;
		dict[i][num_dict].dx  = 0.0;
	}

	dict[pstyle][num_dict].num = k;
	dict[pstyle][num_dict].dx = dx;

	num_dict++;

	return -1;
}

float 
ret_dx(int pstyle, int j)
{
	return(dict[pstyle][j].dx > 0 ? dict[pstyle][j].dx : 0.0);
}
