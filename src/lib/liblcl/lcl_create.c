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
/* Copyright (c) 2000, Sun Microsystems, Inc. All rights reserved. */
 
#pragma ident	"@(#)lcl_create.c	1.3	00/01/07 SMI"

/*
 *  XXX For now, it should work at least with the followings.
 *
 *  LCTd lclt = 
 *     lct_create(lcld, 
 *		  LctNSourceType, LctNMsgText, buf_header, buf_body,
 *		  LctNSourceForm, LctCInComingStreamForm,
 *		  LctNKeepReference, True,
 *		  NULL);
 */				    

#include <stdio.h>     
#include <string.h>
#include <stdlib.h>
#include <sys/varargs.h>

#include "lcl.h"
#include "lcl_internal.h"

LCTd
lct_create(LCLd lcld,...)
{
    va_list var;
    
    char *attr ;
    LctNEAttribute eattr;
    LctNEAttribute type = LctNUnused;
    LctNEAttribute keep_str = LctNUnused;
    LctNEAttribute form = LctNUnused;

    char *buf1, *buf2;
    size_t	length;

    va_start(var,lcld);

    for (eattr = va_arg(var, LctNEAttribute); eattr; eattr = va_arg(var, LctNEAttribute)) {
	switch(eattr) {
	case LctNSourceType:
	    if(eattr = va_arg(var, LctNEAttribute)){
		switch(type = eattr) {
		case LctNMsgText:
		    if(attr = va_arg(var, char*)){
			buf1 = attr;
			if(attr = va_arg(var, char*)){
			    buf2 = attr;
			} else return NULL;
		    } else return NULL;
		    break;
		case LctNPlainText:
		    if(attr = va_arg(var, char*)){
			buf1 = attr;
		    } else return NULL;
		    break;
		case LctNTaggedText:
		    if(attr = va_arg(var, char*)){
			buf1 = attr;
		    } else return NULL;
		    break;
		case LctNSeparatedTaggedText:
		    if(attr = va_arg(var, char*)){
			buf1 = attr;
			if(attr = va_arg(var, char*)){
			    buf2 = attr;
			} else return NULL;
		    } else return NULL;
		    break;
		case LctNSourceUnknown:
		    if(attr = va_arg(var, char*)){
			buf1 = attr;
			if(attr = va_arg(var, char*)){
			    length = (size_t)attr;
			} else return NULL;
		    } else return NULL;
		    break;
		default:
		    return NULL;
		}
	    } else return NULL ;
	    break;
	case LctNSourceForm:
	    if(eattr = va_arg(var, LctNEAttribute)){
		form = eattr;
	    } else return NULL;
	    break;
	case LctNKeepReference:
	    if(eattr = va_arg(var, LctNEAttribute)){
		keep_str = eattr;
	    } else return NULL; /* error if battr = B_FALSE */
	    break;
	default:
	    return NULL;
	} /* end switch */
    } /* end for */
    
    /* check */
    switch (type) {
    case LctNMsgText:
    case LctNSeparatedTaggedText:
	return _lct_create_msg(lcld, type, buf1, buf2, form, keep_str);
    case LctNPlainText:
	return _lct_create_plain(lcld, type, buf1, form, keep_str);
    case LctNTaggedText:
	return _lct_create_tagged(lcld, type, buf1, form, keep_str);
    case LctNSourceUnknown:
	return _lct_create_buf(lcld, type, buf1, length, form, keep_str);
    default:
	return (LCTd)NULL;
    }
}
    
