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
 
#pragma ident	"@(#)lcl_args.c	1.2	00/01/06 SMI"

#include <stdio.h>

#include "lcl.h"
#include "lcl_internal.h"
#include "lcl_args.h"

typedef struct {
    LctNEAttribute	name;
    void		*value;
} LclArg;


static int
_LclNestedListToNestedList(nlist, list)
    LclArg *nlist;   /* This is the new list */
    LclArg *list;    /* The original list */
{
    register LclArg *ptr = list;

    while (ptr->name) {
	if (ptr->name == LclNVaNextedList) {
	    nlist += _LclNestedListToNestedList(nlist, (LclArg *)ptr->value);
	} else {
	    nlist->name = ptr->name;
	    nlist->value = ptr->value;
	    ptr++;
	    nlist++;
	}
    }
    return ptr - list;
}

static void
_LclCountNestedList(args, total_count)
    LclArg *args;
    int *total_count;
{
    for (; args->name; args++) {
	if (args->name == LclNVaNextedList)
	    _LclCountNestedList((LclArg *)args->value, total_count);
	else
	    ++(*total_count);
    }
}

static void
_LclCountVaList(va_list var, int *total_count)
{
    LctNEAttribute attr;

    *total_count = 0;

    for (attr = va_arg(var, LctNEAttribute); attr; attr = va_arg(var, LctNEAttribute)) {
	if (attr == LclNVaNextedList) {
	    _LclCountNestedList(va_arg(var, LclArg*), total_count);
	} else {
	    va_arg(var, LclArg*);
	    ++(*total_count);
	}
    }
}

static void
_LclVaToNestedList(va_list var, int max_count, LclArg **args_return)
{
    LclArg *args;
    LctNEAttribute   attr;

    if (max_count <= 0) {
	*args_return = (LclArg *)NULL;
	return;
    }

    args = (LclArg *)malloc((unsigned)(max_count + 1) * sizeof(LclArg));
    *args_return = args;
    if (!args) return;

    for (attr = va_arg(var, LctNEAttribute); attr; attr = va_arg(var, LctNEAttribute)) {
	if (attr == LclNVaNextedList) {
	    args += _LclNestedListToNestedList(args, va_arg(var, LclArg*));
	} else {
	    args->name = attr;
	    args->value = va_arg(var, void *);
	    args++;
	}
    }
    args->name = LctNUnused;
}

void *
LclVaCreateNestedList(int dummy, ...)
{
    va_list		var;
    LclArg		*args = NULL;
    int			total_count;

    Va_start(var, dummy);
    _LclCountVaList(var, &total_count);
    va_end(var);

    Va_start(var, dummy);
    _LclVaToNestedList(var, total_count, &args);
    va_end(var);

    return (void *)args;
}

static LclError *
create_err(LctNEAttribute attr, int err_code)
{
	LclError	*err;

	err = (LclError *)malloc(sizeof(LclError));
	if(err){
		err->attribute = attr;
		err->error_code = err_code;
	}
	else{
		err = (LclError *)-1;
	}

	return err;
}

static LclError *
set_values(LCTd lctd, LctNEAttribute form, LclArg *args)
{
    LclArg	*p;

    for(p = args; p && p->name; p++){
	switch(p->name) {
	case LctNHeaderCharset:
	case LctNBodyCharset:
	case LctNTaggedTextCharset:
	case LctNPlainTextCharset:
	    _lct_set_context_string_attribute(lctd, form, p->name, p->value);
	    break;
	case LctNHeaderEncoding:
	case LctNBodyEncoding:
	case LctNTaggedTextEncoding:
	case LctNPlainTextEncoding:
	case LctNMailType:
	    _lct_set_string_attribute(lctd, form, p->name, p->value);
	    break;
	case LctNAddHeader:
	    {
		int ret;
		ret = _lct_add_header_buffer(lctd, form, p->value);
		if(ret != LctErrorNone){
		    return(create_err(p->name, ret));
		}
	    }
	break;
	default:
	    return(create_err(p->name, LctErrorInvalidAttribute));
	}
    }
    return (LclError *)NULL;
}

static LclError *
get_values(LCTd lctd, LctNEAttribute form, LclArg *args)
{
    LclArg	*p;

    for(p = args; p && p->name; p++){
	switch(p->name) {
	case LctNHeaderCharset:
	case LctNBodyCharset:
	case LctNTaggedTextCharset:
	case LctNPlainTextCharset:
	case LctNHeaderEncoding:
	case LctNBodyEncoding:
	case LctNTaggedTextEncoding:
	case LctNPlainTextEncoding:
	case LctNMailType:
	    {
		char	*str = _lct_get_context_string_attribute(lctd, form, p->name);
		if(str == (char *)NULL)
		    *((char **)(p->value)) = (char *)NULL;
		else
		    *((char **)(p->value)) = _LclCreateStr(str);
	    }
	break;
	case LctNHeaderCharsetEncoding:
	case LctNBodyCharsetEncoding:
	    {
		char	*str = _lct_get_charset_encoding(lctd, form, p->name);
		
		if(str == (char *)NULL)
		    *((char **)(p->value)) = (char *)NULL;
		else
		    *((char **)(p->value)) = _LclCreateStr(str);
	    }
	break;
	case LctNBodyCharsetMailName:
	    {
		char	*str = _lct_get_mail_string_attribute(lctd, form, p->name);
		if(str == (char *)NULL)
		    *((char **)(p->value)) = (char *)NULL;
		else
		    *((char **)(p->value)) = _LclCreateStr(str);
	    }
	break;
	case LctNHeaderCharsetList:
	case LctNBodyCharsetList:
	case LctNTaggedTextCharsetList:
	case LctNPlainTextCharsetList:
	    *((LclCharsetList **)(p->value)) = _lct_get_context_string_list_attribute(lctd, form, p->name);
	    break;
	case LctNHeaderPossibleCharsetList:
	case LctNBodyPossibleCharsetList:
	case LctNTaggedTextPossibleCharsetList:
	case LctNPlainTextPossibleCharsetList:
	    *((LclCharsetList **)(p->value)) = _lct_get_possible_string_list(lctd, form, p->name);
	    break;
	case LctNHeaderSegment:
	case LctNBodySegment:
	case LctNTaggedTextSegment:
	case LctNPlainTextSegment:
	case LctNContentOfHeaderSegment:
	case LctNContentOfTaggedTextSegment:
	    {
		int ret;
		ret = _lct_get_convert(lctd, form, p->name, (LclCharsetSegmentSet **)p->value);
		if(ret != LctErrorNone){
		    return(create_err(p->name, ret));
		}
	    }
	break;
	case LctNQuerySourceType:
	    {
		LclContentInfo	*info;
		info = _lct_get_query_content_type(lctd, form);
		if(!info){
		    return(create_err(p->name, LctErrorNotEnoughResource));
		}
		*((LclContentInfo **)(p->value)) = info;
		break;
	    }
	default:
	    return(create_err(p->name, LctErrorInvalidAttribute));
	}
    }
    return (LclError *)NULL;
}

LclError *
lct_setvalues(LCTd lctd, LctNEAttribute target, ...)
{
    va_list var;
    int     total_count;
    LclArg *args;
    LclError	*err;

    /*
     * so count the stuff dangling here
     */
    Va_start(var, lctd);
    _LclCountVaList(var, &total_count);
    va_end(var);

    /*
     * now package it up so we can send it along
     */
    Va_start(var, lctd);
    _LclVaToNestedList(var, total_count, &args);
    va_end(var);

    err = set_values(lctd, target, args);
    if (args) free((char *)args);

    return err;
}

LclError *
lct_getvalues(LCTd lctd, LctNEAttribute target, ...)
{
    va_list var;
    int     total_count;
    LclArg *args;
    LclError	*err;

    /*
     * so count the stuff dangling here
     */
    Va_start(var, lctd);
    _LclCountVaList(var, &total_count);
    va_end(var);

    /*
     * now package it up so we can send it along
     */
    Va_start(var, lctd);
    _LclVaToNestedList(var, total_count, &args);
    va_end(var);

    err = get_values(lctd, target, args);
    if (args) free((char *)args);

    return err;
}

void
lcl_destroy_error(LclError *lct_error)
{
	if(lct_error){
		free(lct_error);
	}
}
