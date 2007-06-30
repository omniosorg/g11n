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
#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)layout_wrap.c 1.10 95/04/06";
#endif
#endif

/*
 *      (c) Copyright 1995 Sun Microsystems, Inc. 
 *	All rights reserved.  
 *	See LEGAL NOTICE file for terms of the license.
 */

/* 
 *   This module contains the functions defined in the Layout Services API.
 */

#include <errno.h>
#include <malloc.h>
#include <wchar.h>
#include <locale.h>
#include <assert.h>
#include "ctl_threads.h"
#include "layout.h"
#include "layout_int.h"


extern LayoutObj		_LayoutObjectGetHandle();
extern LayoutValues		_ModifierToLayoutValues();

static ctl_mutex_t 			*layout_lock = NULL;
static ctl_mutexattr_t			mattr;

LayoutObject
m_create_layout(attrobj, modifier)
    const void		*attrobj; /* This should be type Attobj, but DISS header is not available */
    const char		*modifier;		
{
    char		*locale_name;
    LayoutObj		layout_obj = (LayoutObj)NULL;
    LayoutValues	layout_values = (LayoutValues)NULL;


    if (!layout_lock) {
        layout_lock = (ctl_mutex_t *)malloc(sizeof(ctl_mutex_t));
	ctl_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_PRIVATE);
        ctl_mutex_init(layout_lock, &mattr); 
    }
       
    ctl_mutex_lock(layout_lock);

    if (attrobj == NULL) {
         locale_name = (char *)setlocale(LC_CTYPE, (char *)NULL);
    }
    else
      locale_name = (char*)attrobj;

    layout_obj = _LayoutObjectGetHandle(locale_name);
    
    if (layout_obj)  {
        
        if (modifier) 
            layout_values = _ModifierToLayoutValues(layout_obj, modifier);
        
        layout_obj = layout_obj->methods->create(layout_obj, layout_values);
        
        if (layout_values)
            free(layout_values);
    }

    assert(layout_obj != 0);

    ctl_mutex_unlock(layout_lock);
              
    return((LayoutObject) layout_obj);
}


int
m_destroy_layout(layout_object)
    const LayoutObject 	layout_object;
{
    LayoutObj		layout_obj = (LayoutObj) layout_object;
    int			result = -1;
    
    ctl_mutex_lock(layout_lock);
    
    if (layout_obj) {
        result = layout_obj->methods->destroy(layout_obj);
        
    } else {
        errno = EFAULT;
    }
    ctl_mutex_unlock(layout_lock);    
    return(result);
}

int
m_getvalues_layout(layout_object, values, index_returned)
    LayoutObject 	layout_object;
    LayoutValues 	values;
    int 		*index_returned;
{
    LayoutObj		layout_obj = (LayoutObj) layout_object;
    int			result = -1;
    
    ctl_mutex_lock(layout_lock);

    if (layout_obj) {
        result = layout_obj->methods->getvalues(layout_obj, values, index_returned);
    } else {
        errno = EINVAL;
        *index_returned = -1;
    }
    
    ctl_mutex_unlock(layout_lock);
        
    return(result);

}	

int
m_setvalues_layout(layout_object, values, index_returned)
    LayoutObject 	layout_object;
    LayoutValues 	values;
    int 		*index_returned;
{
    LayoutObj		layout_obj = (LayoutObj) layout_object;
    int			result = -1;
    
    ctl_mutex_lock(layout_lock);
    
    if (layout_obj) {
        result = layout_obj->methods->setvalues(layout_obj, values, index_returned);
    } else {
        errno = EINVAL;
    }
    
    ctl_mutex_unlock(layout_lock);
        
    return(result);

}	

int
m_transform_layout(layout_object, InpBuf, InpSize, OutBuf, OutSize,
		   InpToOut, OutToInp, Property, InpBufIndex)
    LayoutObject		layout_object;
    const char			*InpBuf;
    size_t			InpSize;
    void			*OutBuf;
    size_t			*OutSize;
    size_t			*InpToOut;
    size_t			*OutToInp;
    unsigned char		*Property;
    size_t			*InpBufIndex;		   
{
    LayoutObj			layout_obj = (LayoutObj) layout_object;
    int				result = -1;
    
    ctl_mutex_lock(layout_lock);
    
    if (layout_obj) {
        
        result = layout_obj->methods->transform(layout_obj, InpBuf, InpSize,
        				        OutBuf, OutSize, InpToOut, 
        				        OutToInp, Property, InpBufIndex);
         				        
    } else
        errno = EBADF;   				    
        				        
    ctl_mutex_unlock(layout_lock);    				        
    return(result);   				        

}

int
m_wtransform_layout(layout_object, InpBuf, InpSize, OutBuf, Outsize,
		   InpToOut, OutToInp, Property, InpBufIndex)
    LayoutObject		layout_object;
    const wchar_t		*InpBuf;
    size_t			InpSize;
    void			*OutBuf;
    size_t			*Outsize;
    size_t			*InpToOut;
    size_t			*OutToInp;
    unsigned char		*Property;	
    size_t			*InpBufIndex;	   
{
    LayoutObj			layout_obj = (LayoutObj) layout_object;
    int				result = -1;
    
    ctl_mutex_lock(layout_lock);
    
    if (layout_obj) {
        
        result = layout_obj->methods->wcstransform(layout_obj, InpBuf,
        					   InpSize, OutBuf, Outsize,
		   				   InpToOut, OutToInp, 
		   				   Property, InpBufIndex);
    } else
        errno = EBADF;
        	   				   
    ctl_mutex_unlock(layout_lock);	   				   
		   				   
        
    return(result);

}
		   	
		
		
