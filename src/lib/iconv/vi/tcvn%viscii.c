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
 * Copyright (c) 2008, by Sun Microsystems, Inc.
 * All rights reserved.
 */
#ident   "@(#)tcvn_to_viscii.c	1.1 08/07/31"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <viscii_tcvn.h>    /* VISCII <-> TCVN mapping table */
#include <vi_combine.h>
#include "common_defs.h"


#define NON_ID_CHAR '?'     /* non-identified character */

typedef struct _icv_state {
    int	_errno;		/* internal errno */
    unsigned short last; 
    boolean  little_endian;
    boolean  bom_written;
} _iconv_st;


static int binsearch(unsigned long x, Combine_map v[], int n);

/*
 * Open; called from iconv_open()
 */
void *
_icv_open()
{
    _iconv_st *st;

    if ((st = (_iconv_st *)malloc(sizeof(_iconv_st))) == NULL) {
        errno = ENOMEM;
	return ((void *) -1);
    }

    st->_errno = 0;
    st->last = 0;
    return ((void *) st);
}


/*
 * Close; called from iconv_close()
 */
void
_icv_close(_iconv_st *st)
{
    if (!st)
        errno = EBADF;
    else
        free(st);
}


/*
 * Actual conversion; called from iconv()
 */
size_t
_icv_iconv(_iconv_st *st, char **inbuf, size_t *inbytesleft,
				char **outbuf, size_t *outbytesleft)
{
    unsigned char   *op = NULL;
    int     unconv = 0;
    int             idx = -1;
#ifdef DEBUG
    fprintf(stderr, "==========     iconv(): TCVN5712 -->UCS-2   ==========\n");
#endif
    if (st == NULL) {
        errno = EBADF;
        return ((size_t) -1);
    }

    if (inbuf == NULL || *inbuf == NULL) { /* Reset request. */
        st->_errno = 0;
        return ((size_t) 0); 
    }

    st->_errno = 0;     /* Reset internal errno */
    errno = 0;          /* Reset external errno */

    /* Convert tcvn encoding to UCS-2 */
    while (*inbytesleft > 0 && *outbytesleft > 0) {
        unsigned char ch = 0;
        unsigned char chout = 0;
        
        if (st->last != 0) {
            if (ISCOMB_TCVN(ch)) {
                /*
                 * Composed characters with combine character
                 */
                idx =  binsearch(st->last, tcvn_comb_data, VOWEL_NUM);
                if (idx >= 0) {
                    ch = vi_comb_data[idx].composed[ch-0xb0];
                } else {
                    errno = EBADF;
                    return ((size_t)-1);
                }
                st->last = 0;
            }
            st->last = 0;
        } else {
            if (ch >= 0x41 && ch <= 0xad
                && ((tcvn_comp_bases_mask0[(ch-0x0040) >> 5] >> (ch & 0x1f)) & 1)) {
                /* 
                 * uni is vowel, it's a possible match with combine character. 
                 * Buffer it. 
                 * */
                st->last = ch;
                (*inbuf)++;
                (*inbytesleft)--;
                continue;
            }
        }

        tcvn_2_viscii(ch, &chout);
        if (**inbuf != 0x0 && chout == 0x0) {
            unconv++;
            chout = NON_ID_CHAR;
        }
        
        *(*outbuf)++ = chout;
        (*outbytesleft) -= 1;
	(*inbuf)++;
        (*inbytesleft)--;

    }   

    if ( *inbytesleft > 0 && *outbytesleft <= 0 ) {
        errno = E2BIG;
        st->last = 0; 
        return ((size_t)-1);
    }    
 
    return ((size_t)unconv);

}

/* binsearch: find x in v[0] <= v[1] <= ... <= v[n-1] */
static int binsearch(unsigned long x, Combine_map v[], int n)
{
    int low = 0; 
    int mid = 0;
    int high = n - 1;

    low = 0;
    while (low <= high) {
        mid = (low + high) / 2;
        if (x < v[mid].base)
            high = mid - 1;
        else if (x > v[mid].base)
            low = mid + 1;
        else    
            /* found match */
            return mid;
    }

    /* no match */
    return (-1);   
}

