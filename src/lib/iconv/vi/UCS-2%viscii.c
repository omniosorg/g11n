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
#ident   "@(#)UCS2_to_viscii.c	1.1 08/07/31"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unicode_viscii.h> /* Unicode to VISCII  mapping table */
#include "common_defs.h"
#define NON_ID_CHAR '?'     /* non-identified character */

typedef struct _icv_state {
    char    keepc[4];   /* maximum # byte of UCS code */
    int     _errno;     /* internal errno */
    boolean little_endian;
    boolean bom_written;
} _iconv_st;


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
    st->little_endian = false;
    st->bom_written = false;
#if defined(UCS_2LE)
    st->little_endian = true;
    st->bom_written = true;
#endif
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
    unsigned char   c1, c2;
    unsigned char   *op = NULL;
    int             no_id_char_num = 0;
#ifdef DEBUG
    fprintf(stderr, "==========     iconv(): UCS-2 --> viscii  ==========\n");
#endif
    if (st == NULL) {
        errno = EBADF;
        return ((size_t) -1);
    }

    if (inbuf == NULL || *inbuf == NULL) { /* Reset request. */
        st->_errno = 0;
        return ((size_t) 0); 
    }

    st->_errno = 0;		/* reset internal errno */
    errno = 0;		/* reset external errno */

    /* convert UCS-2 encoding to viscii */
    while (*inbytesleft > 0 && *outbytesleft > 0) {
        unsigned long uni = 0;
        unsigned char c1 = 0, c2 = 0;
        unsigned char ch = 0;

        c1 = **inbuf;
        (*inbuf)++;
        (*inbytesleft) -= 1;
        if (*inbytesleft == 0) {
            errno = EILSEQ;
            return ((size_t)-1);
        }
        c2 = **inbuf;
        (*inbuf)++;
        (*inbytesleft) -= 1;

        if (st->little_endian) {
            uni |= (unsigned long)c1;
            uni |= (unsigned long)c2 << 8;
        }else {
            uni |= (unsigned long)c1 << 8;
            uni |= (unsigned long)c2;
        }
        if ( *inbytesleft > 0 && *outbytesleft <= 0 ) {
             errno = E2BIG; 
             return ((size_t)-1);
        } 
 
        if (uni_2_viscii(uni, &ch) == 1) {
            **outbuf = ch;
        } else {
            **outbuf = NON_ID_CHAR;
            no_id_char_num += 1;
        }
        (*outbuf) += 1;
        (*outbytesleft) -= 1;
    }

    return ((size_t)no_id_char_num);
}

