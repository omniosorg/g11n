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
 * Copyright (c) 1998, by Sun Microsystems, Inc.
 * All rights reserved.
 */

#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)layout_int.h 1.2 94/12/14";
#endif
#endif

/*
 *      (c) Copyright 1994 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */
 
/*
 *  This is the private header file for layout services.
 */
#ifndef _LAYOUT_INT_H_
#define _LAYOUT_INT_H_

/* In case if TRUE or FALSE are defined */
#if     !defined(TRUE) || ((TRUE) != 1)
#define TRUE    1
#endif   
#if     !defined(FALSE) || ((FALSE) != 0)
#define FALSE   0
#endif   


typedef struct _LayoutObject *LayoutObj;		/* Private name */



typedef struct {
    LayoutObject (*create)(
#if NeedFunctionPrototypes
        LayoutObj, LayoutValues
#endif
        );

    int (*destroy)(
#if NeedFunctionPrototypes
        LayoutObj
#endif
        );
        
    int (*getvalues)(
#if NeedFunctionPrototypes
        LayoutObj, LayoutValues, int *
#endif
        );
        
    int (*setvalues)(
#if NeedFunctionPrototypes
        LayoutObj, LayoutValues, int *
#endif
        );
        
    int (*transform)(
#if NeedFunctionPrototypes
        LayoutObj, const char *, size_t , void *,
        size_t *, size_t *, size_t *, unsigned char *, size_t *
#endif
        );
        
    int (*wcstransform)(
#if NeedFunctionPrototypes
        LayoutObj, const wchar_t *, size_t , void *,
        size_t *, size_t *, size_t *, unsigned char *, size_t *
#endif
        );
        
} LayoutMethodsRec, *LayoutMethods;


typedef struct {
    char			*locale_name;
    
    LayoutTextDescriptor	orientation;
    LayoutTextDescriptor	context;
    LayoutTextDescriptor	type_of_text;
    LayoutTextDescriptor	implicit_alg;
    LayoutTextDescriptor	swapping;
    LayoutTextDescriptor	numerals;
    LayoutTextDescriptor	text_shaping;
    BooleanValue		active_dir;
    BooleanValue		active_shape_editing;
    char			*shape_charset;
    int				shape_charset_size;
    unsigned long		in_out_text_descr_mask;
    unsigned long		in_only_text_descr;
    unsigned long		out_only_text_descr;
    int				check_mode;
    LayoutEditSize		shape_context_size;
} LayoutCoreRec, *LayoutCore;

typedef struct _LayoutObject {
    LayoutMethods         methods;         /* methods of this CTL Object */
    LayoutCoreRec         core;                   /* data of this CTL Object */
    void		  *private_data;   /* Private data of locale-dependent object */

} LayoutObjectRec;


#endif /* _LAYOUT_INT_H_ */
