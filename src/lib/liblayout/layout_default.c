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
 *      (c) Copyright 1995 Sun Microsystems, Inc. 
 *	All rights reserved.  
 *	See LEGAL NOTICE file for terms of the license.
 */


/* 
 *   This module contains the functions for default C locale
 *   layout services implementation.
 */
#include <errno.h> 
#include <malloc.h>
#include <wchar.h>
#include <stdlib.h>
#include <langinfo.h>
#include "layout.h"
#include "layout_int.h"


static 	LayoutObject		_LSDefaultCreate();
static 	int			_LSDefaultDestroy();
static 	int			_LSDefaultGetValues();
static 	int			_LSDefaultSetValues();
static 	int			_LSDefaultTransform();
static 	int			_LSDefaultWCSTransform();


/* Methods for C locale support */
static LayoutMethodsRec _DefaultLayoutMethods = {
	_LSDefaultCreate,
	_LSDefaultDestroy,
	_LSDefaultGetValues,
	_LSDefaultSetValues,
	_LSDefaultTransform,
	_LSDefaultWCSTransform,
};

   
/* 
 *  Entry point of to create and initialize the default layout object.
 *
 */
LayoutObject
_LayoutObjectDefaultInit(locale_name)
    char		*locale_name;
{
    LayoutObj            layout_obj = (LayoutObj)malloc(sizeof(LayoutObjectRec));
 
    if (layout_obj) {
        memset(layout_obj, 0, sizeof(LayoutObjectRec));
        
        layout_obj->methods = (&_DefaultLayoutMethods);            
        
        if (layout_obj->core.locale_name = (char *)malloc(strlen(locale_name) + 1))  {
            (void *)strcpy(layout_obj->core.locale_name, locale_name);
        } else
            goto NoMemory;
        
        layout_obj->core.orientation = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!layout_obj->core.orientation)
            goto NoMemory;
        layout_obj->core.orientation->inp = ORIENTATION_LTR;
        layout_obj->core.orientation->out = ORIENTATION_LTR;
        
        layout_obj->core.context = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!layout_obj->core.context)
            goto NoMemory;
        layout_obj->core.context->inp = CONTEXT_LTR;
        layout_obj->core.context->out = CONTEXT_LTR;
        
        layout_obj->core.type_of_text = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!layout_obj->core.type_of_text)
            goto NoMemory;            
        layout_obj->core.type_of_text->inp = TEXT_VISUAL;
        layout_obj->core.type_of_text->out = TEXT_VISUAL;
        
        layout_obj->core.implicit_alg = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!layout_obj->core.implicit_alg)
            goto NoMemory;                        
        layout_obj->core.implicit_alg->inp = ALGOR_IMPLICIT;
        layout_obj->core.implicit_alg->out = ALGOR_IMPLICIT;

        layout_obj->core.swapping = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!layout_obj->core.swapping)
            goto NoMemory;                                    
        layout_obj->core.swapping->inp = SWAPPING_NO;
        layout_obj->core.swapping->out = SWAPPING_NO;
        
        layout_obj->core.numerals = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!layout_obj->core.numerals)
            goto NoMemory;                                                
        layout_obj->core.numerals->inp = NUMERALS_NOMINAL;
        layout_obj->core.numerals->out = NUMERALS_NOMINAL;
        
        layout_obj->core.text_shaping = 
            (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        if (!layout_obj->core.text_shaping)
            goto NoMemory;                                                            
        layout_obj->core.text_shaping->inp = TEXT_SHAPED;
        layout_obj->core.text_shaping->out = TEXT_SHAPED;


        
        layout_obj->core.active_dir = FALSE;
        layout_obj->core.active_shape_editing = FALSE;
        
        layout_obj->core.shape_charset_size = MB_CUR_MAX;
        
 	if (strcmp(locale_name, "C") == 0) {       
            layout_obj->core.shape_charset = (char *)malloc(strlen("iso8859-1") + 1);
	    if (!layout_obj->core.shape_charset)
		goto NoMemory;
            (void *)strcpy(layout_obj->core.shape_charset, "iso8859-1");
        } else  {
            layout_obj->core.shape_charset = (char *)malloc(strlen(nl_langinfo(CODESET)) + 1);
	    if (!layout_obj->core.shape_charset)
		goto NoMemory;
            (void *)strcpy(layout_obj->core.shape_charset, nl_langinfo(CODESET));
        }
        
        layout_obj->core.in_out_text_descr_mask = AllTextDescriptors;

        layout_obj->core.in_only_text_descr = 0;
        layout_obj->core.out_only_text_descr = 0;
        
        layout_obj->core.check_mode = MODE_STREAM;
        
        layout_obj->core.shape_context_size = 
            (LayoutEditSize)malloc(sizeof(LayoutEditSizeRec));
        if (!layout_obj->core.shape_context_size)
            goto NoMemory;   
        layout_obj->core.shape_context_size->front = 
            layout_obj->core.shape_context_size->back = 0;
        
    } else {
        goto NoMemory;
    }
 
    return ((LayoutObject)layout_obj);
    
NoMemory:
    errno = ENOMEM;
    
    if (layout_obj) {
        if(layout_obj->core.locale_name) {
            free(layout_obj->core.locale_name);
        }
        
        if(layout_obj->core.orientation) {
            free(layout_obj->core.orientation);
        }
        
        if(layout_obj->core.context) {
            free(layout_obj->core.context);
        }
        
        if(layout_obj->core.type_of_text) {
            free(layout_obj->core.type_of_text);
        }
        
        if(layout_obj->core.implicit_alg) {
            free(layout_obj->core.implicit_alg);
        }
        
        if(layout_obj->core.swapping) {
            free(layout_obj->core.swapping);
        }
        
        if(layout_obj->core.numerals) {
            free(layout_obj->core.numerals);
        }

        if(layout_obj->core.text_shaping) {
            free(layout_obj->core.text_shaping);
        }
        
        if(layout_obj->core.shape_context_size) {
            free(layout_obj->core.shape_context_size);
        }

	if(layout_obj->core.shape_charset) {
	   free(layout_obj->core.shape_charset);
	}
    
        free(layout_obj);
    }
    return((LayoutObject)NULL);    
}


static LayoutObject
_LSDefaultCreate(layout_obj, layout_values)
    LayoutObj		layout_obj;
    LayoutValues 	layout_values;
{
    int			result = -1;
    int			index_returned;
    LayoutValues	temp_ptr = layout_values;
    
    if (layout_values) {
        result = layout_obj->methods->setvalues(layout_obj, layout_values, &index_returned);
        
        /*
         *  The reason to free ShapeCharset is because it is
         *  malloced from the modifier string. 
         */
	while (temp_ptr->name) {
	     if ((temp_ptr->name == ShapeCharset) &&
		 (temp_ptr->value)) {
		 free(temp_ptr->value);
	     }
	     temp_ptr++;
	}

    }
    
    return(layout_obj);
}


static int
_LSDefaultDestroy(layout_obj)
    LayoutObj		layout_obj;
{
    int			result = -1;
    
    if (layout_obj->core.orientation)
        free(layout_obj->core.orientation);
        
    if (layout_obj->core.context)
        free(layout_obj->core.context);
        
    if (layout_obj->core.type_of_text)
        free(layout_obj->core.type_of_text);
        
    if (layout_obj->core.implicit_alg)
        free(layout_obj->core.implicit_alg);
    
    if (layout_obj->core.swapping)
        free(layout_obj->core.swapping);
        
    if (layout_obj->core.numerals)
        free(layout_obj->core.numerals);
        
    if (layout_obj->core.text_shaping)
        free(layout_obj->core.text_shaping);
                
    if(layout_obj->core.shape_context_size) 
        free(layout_obj->core.shape_context_size);
        
    if(layout_obj->core.shape_charset) 
        free(layout_obj->core.shape_charset);
        
    if(layout_obj->core.locale_name) 
        free(layout_obj->core.locale_name);
        
    free(layout_obj);    
    
    return(0);
}

static int
_LSDefaultGetValues(layout_obj, values, index_returned)
    LayoutObj		layout_obj;
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
            
            (*dummy)->inp |= layout_obj->core.orientation->inp;
            (*dummy)->out |= layout_obj->core.orientation->out;
            
            (*dummy)->inp |= layout_obj->core.context->inp;
            (*dummy)->out |= layout_obj->core.context->out;
            
            (*dummy)->inp |= layout_obj->core.type_of_text->inp;
            (*dummy)->out |= layout_obj->core.type_of_text->out;
            
            (*dummy)->inp |= layout_obj->core.implicit_alg->inp;
            (*dummy)->out |= layout_obj->core.implicit_alg->out;
            
            (*dummy)->inp |= layout_obj->core.swapping->inp;
            (*dummy)->out |= layout_obj->core.swapping->out;
            
            (*dummy)->inp |= layout_obj->core.numerals->inp;
            (*dummy)->out |= layout_obj->core.numerals->out;
            
            (*dummy)->inp |= layout_obj->core.text_shaping->inp;
            (*dummy)->out |= layout_obj->core.text_shaping->out;
            
        } else if (values[i].name & AllTextDescriptors) {
                        
            if (values[i].name & Orientation) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = layout_obj->core.orientation->inp;
                (*dummy)->out = layout_obj->core.orientation->out;
            } else if  (values[i].name & Context) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = layout_obj->core.context->inp;
                (*dummy)->out = layout_obj->core.context->out;
            } else if (values[i].name & TypeOfText) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = layout_obj->core.type_of_text->inp;
                (*dummy)->out = layout_obj->core.type_of_text->out;
            } else if (values[i].name & ImplicitAlg) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = layout_obj->core.implicit_alg->inp;
                (*dummy)->out = layout_obj->core.implicit_alg->out;
            } else if (values[i].name & Swapping) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = layout_obj->core.swapping->inp;
                (*dummy)->out = layout_obj->core.swapping->out;
            } else if (values[i].name & Numerals) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = layout_obj->core.numerals->inp;
                (*dummy)->out = layout_obj->core.numerals->out;

            } else if (values[i].name & TextShaping) {
                LayoutTextDescriptor *dummy = (LayoutTextDescriptor*)values[i].value;
                    
                (*dummy)->inp = layout_obj->core.text_shaping->inp;
                (*dummy)->out = layout_obj->core.text_shaping->out;
            }
        } else if (values[i].name & ActiveDirectional) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(layout_obj->core.active_dir);
            } else {
                 BooleanValue		*dummy = values[i].value;

                *dummy = (BooleanValue) layout_obj->core.active_dir;
            }
        }  else if (values[i].name & ActiveShapeEditing) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(layout_obj->core.active_shape_editing);
            } else {
                 BooleanValue		*dummy = values[i].value;

                *dummy = (BooleanValue) layout_obj->core.active_shape_editing;
            }
        } else if (values[i].name & ShapeCharset) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = strlen(layout_obj->core.shape_charset) + 1;
            } else {
                 char			**dummy = values[i].value;

                strcpy(*dummy, layout_obj->core.shape_charset);
            }
        }  else if (values[i].name & ShapeCharsetSize) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(layout_obj->core.shape_charset_size);
            } else {
                 int		*dummy = values[i].value;

                *dummy = layout_obj->core.shape_charset_size;
            }
        } else if (values[i].name & InOutTextDescrMask) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(layout_obj->core.in_out_text_descr_mask);
            } else {
                 unsigned long		*dummy = values[i].value;

                *dummy = layout_obj->core.in_out_text_descr_mask;
            }
        } else if (values[i].name & InOnlyTextDescr) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                 
                *dummy = sizeof(layout_obj->core.in_only_text_descr);
            } else {
                 unsigned long		*dummy = values[i].value;
                 
                 *dummy = 0;
                 
                 if (layout_obj->core.in_out_text_descr_mask & Orientation)
                     *dummy |= layout_obj->core.orientation->inp;
                 if (layout_obj->core.in_out_text_descr_mask & Context)
                     *dummy |= layout_obj->core.context->inp;
                 if (layout_obj->core.in_out_text_descr_mask & TypeOfText)
                     *dummy |= layout_obj->core.type_of_text->inp;
                 if (layout_obj->core.in_out_text_descr_mask & ImplicitAlg)
                     *dummy |= layout_obj->core.implicit_alg->inp;
                 if (layout_obj->core.in_out_text_descr_mask & Swapping)
                     *dummy |= layout_obj->core.swapping->inp;
                 if (layout_obj->core.in_out_text_descr_mask & Numerals)
                     *dummy |= layout_obj->core.numerals->inp;                     
                 if (layout_obj->core.in_out_text_descr_mask & TextShaping)
                     *dummy |= layout_obj->core.text_shaping->inp;
            }
        } else if (values[i].name & OutOnlyTextDescr) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                 
                *dummy = sizeof(layout_obj->core.out_only_text_descr);
            } else {
                 unsigned long		*dummy = values[i].value;
                 
                 *dummy = 0;
                 
                 if (layout_obj->core.in_out_text_descr_mask & Orientation)
                     *dummy |= layout_obj->core.orientation->out;
                 if (layout_obj->core.in_out_text_descr_mask & Context)
                     *dummy |= layout_obj->core.context->out;
                 if (layout_obj->core.in_out_text_descr_mask & TypeOfText)
                     *dummy |= layout_obj->core.type_of_text->out;
                 if (layout_obj->core.in_out_text_descr_mask & ImplicitAlg)
                     *dummy |= layout_obj->core.implicit_alg->out;
                 if (layout_obj->core.in_out_text_descr_mask & Swapping)
                     *dummy |= layout_obj->core.swapping->out;
                 if (layout_obj->core.in_out_text_descr_mask & Numerals)
                     *dummy |= layout_obj->core.numerals->out;                     
                 if (layout_obj->core.in_out_text_descr_mask & TextShaping)
                     *dummy |= layout_obj->core.text_shaping->out;
            }
        } else if (values[i].name & CheckMode) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(layout_obj->core.check_mode);
            } else {
                 int			*dummy = values[i].value;

                *dummy = layout_obj->core.check_mode;
            }
        } else if (values[i].name & ShapeContextSize) {
            if (values[i].name & QueryValueSize) {
                 unsigned long		*dummy = values[i].value;
                
                *dummy = sizeof(LayoutEditSizeRec);
            } else {
                 LayoutEditSize	*dummy = (LayoutEditSize*)values[i].value;

                 (*dummy)->front = layout_obj->core.shape_context_size->front;
                 (*dummy)->back = layout_obj->core.shape_context_size->back;
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
                
                if (dummy->inp & ~MaskContext)
                    goto InvalidValue;
                    
                if (dummy->out & ~MaskContext)
                    goto InvalidValue;
               
                                
            } else if (values[i].name & TypeOfText) {
                LayoutTextDescriptor dummy = (LayoutTextDescriptor)values[i].value;
                
                if ((dummy->inp & ~TEXT_VISUAL) || (dummy->out & ~TEXT_VISUAL))                
                    goto InvalidValue;

#ifdef notdef                     
                if (dummy->inp & ~MaskTypeOfText)
                    goto InvalidValue;
                    
                if (dummy->out & ~MaskTypeOfText)
                    goto InvalidValue;
#endif                    
                
                
            } else if (values[i].name & ImplicitAlg) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;

                if (dummy->inp & ~MaskImplicitAlg)
                    goto InvalidValue;
                    
                if (dummy->out & ~MaskImplicitAlg)
                    goto InvalidValue;
                
            } else if (values[i].name & Swapping) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;

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
_LSDefaultSetValues(layout_obj, values, index_returned)
    LayoutObj		layout_obj;
    LayoutValues 	values;
    int 		*index_returned;
{
    int			result = 0;
    int			i = 0;
    LayoutCoreRec	core_data;
    
    
    if (!values)
        return(0);
        
          
    if (FoundInvalidValue(values, index_returned))
        return(-1);
        
                
    while (values[i].name) {
        
        if (values[i].name == AllTextDescriptors) {
             LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                 
             layout_obj->core.orientation->inp = (dummy->inp & MaskOrientation);
	     layout_obj->core.context->inp = (dummy->inp & MaskContext);
	     layout_obj->core.type_of_text->inp = (dummy->inp & MaskTypeOfText);
	     layout_obj->core.implicit_alg->inp = (dummy->inp & MaskImplicitAlg);
	     layout_obj->core.swapping->inp = (dummy->inp & MaskSwapping);
	     layout_obj->core.numerals->inp = (dummy->inp & MaskNumerals);
	     layout_obj->core.text_shaping->inp = (dummy->inp & MaskTextShaping);
	     
	     layout_obj->core.orientation->out = (dummy->out & MaskOrientation);
	     layout_obj->core.context->out = (dummy->out & MaskContext);
	     layout_obj->core.type_of_text->out = (dummy->out & MaskTypeOfText);
	     layout_obj->core.implicit_alg->out = (dummy->out & MaskImplicitAlg);
	     layout_obj->core.swapping->out = (dummy->out & MaskSwapping);
	     layout_obj->core.numerals->out = (dummy->out & MaskNumerals);
	     layout_obj->core.text_shaping->out = (dummy->out & MaskTextShaping);
                 
        } else if (values[i].name & AllTextDescriptors) {
        
            if (values[i].value == NULL)
                break; /* Error case */
                        
            if (values[i].name & Orientation) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                    
                layout_obj->core.orientation->inp = dummy->inp;
                layout_obj->core.orientation->out = dummy->out;
                
            } else if  (values[i].name & Context) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                layout_obj->core.context->inp = dummy->inp;
                layout_obj->core.context->out = dummy->out;
                
            } else if (values[i].name & TypeOfText) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                layout_obj->core.type_of_text->inp = dummy->inp;
                layout_obj->core.type_of_text->out = dummy->out;
                
            } else if (values[i].name & ImplicitAlg) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                layout_obj->core.implicit_alg->inp = dummy->inp;
                layout_obj->core.implicit_alg->out = dummy->out;
                
            } else if (values[i].name & Swapping) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                layout_obj->core.swapping->inp = dummy->inp;
                layout_obj->core.swapping->out = dummy->out;
                    
            } else if (values[i].name & Numerals) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                layout_obj->core.numerals->inp = dummy->inp;
                layout_obj->core.numerals->out = dummy->out;
                    
            } else if (values[i].name & TextShaping) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
                layout_obj->core.text_shaping->inp = dummy->inp;
                layout_obj->core.text_shaping->out = dummy->out;
                    
            }
        } else if (values[i].name & ShapeCharset) {
        
            if (layout_obj->core.shape_charset)
                 free(layout_obj->core.shape_charset);
            layout_obj->core.shape_charset = (char *)malloc(strlen((char *)values[i].value) + 1);
            strcpy(layout_obj->core.shape_charset, (char *)values[i].value);
            
        } else if (values[i].name & InOutTextDescrMask) {
            layout_obj->core.in_out_text_descr_mask = (unsigned long)values[i].value;

        } else if (values[i].name & InOnlyTextDescr) {
             unsigned long		dummy = (unsigned long)values[i].value;
                 
             if (layout_obj->core.in_out_text_descr_mask & Orientation)
                 layout_obj->core.orientation->inp = (dummy & MaskOrientation);
             if (layout_obj->core.in_out_text_descr_mask & Context)
                 layout_obj->core.context->inp = (dummy & MaskContext);
             if (layout_obj->core.in_out_text_descr_mask & TypeOfText)
                 layout_obj->core.type_of_text->inp = (dummy & MaskTypeOfText);
             if (layout_obj->core.in_out_text_descr_mask & ImplicitAlg)
                 layout_obj->core.implicit_alg->inp = (dummy & MaskImplicitAlg);
             if (layout_obj->core.in_out_text_descr_mask & Swapping)
                 layout_obj->core.swapping->inp = (dummy & MaskSwapping);
             if (layout_obj->core.in_out_text_descr_mask & Numerals)
                 layout_obj->core.numerals->inp = (dummy & MaskNumerals);
             if (layout_obj->core.in_out_text_descr_mask & TextShaping)
                 layout_obj->core.text_shaping->inp = (dummy & MaskTextShaping);
                 
        } else if (values[i].name & OutOnlyTextDescr) {
             unsigned long		dummy = (unsigned long)values[i].value;
                 
             if (layout_obj->core.in_out_text_descr_mask & Orientation)
                 layout_obj->core.orientation->out = (dummy & MaskOrientation);
             if (layout_obj->core.in_out_text_descr_mask & Context)
                 layout_obj->core.context->out = (dummy & MaskContext);
             if (layout_obj->core.in_out_text_descr_mask & TypeOfText)
                 layout_obj->core.type_of_text->out = (dummy & MaskTypeOfText);
             if (layout_obj->core.in_out_text_descr_mask & ImplicitAlg)
                 layout_obj->core.implicit_alg->out = (dummy & MaskImplicitAlg);
             if (layout_obj->core.in_out_text_descr_mask & Swapping)
                 layout_obj->core.swapping->out = (dummy & MaskSwapping);
             if (layout_obj->core.in_out_text_descr_mask & Numerals)
                 layout_obj->core.numerals->out = (dummy & MaskNumerals);
             if (layout_obj->core.in_out_text_descr_mask & TextShaping)
                 layout_obj->core.text_shaping->out = (dummy & MaskTextShaping);

        } else if (values[i].name & CheckMode) {
             layout_obj->core.check_mode = (int) values[i].value;


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
_LSDefaultTransform(layout_obj, InpBuf, InpSize, OutBuf, OutSize,
		   InpToOut, OutToInp, Property, InpBufIndex)
    LayoutObj			layout_obj;
    const char			*InpBuf;
    size_t			InpSize;
    char			*OutBuf;
    size_t			*OutSize;
    size_t			*InpToOut;
    size_t			*OutToInp;
    unsigned char		*Property;
    size_t   			*InpBufIndex;
{
    int				result = 0;
    int				num_bytes, i;
    				/* Should confirm with the spec on
    				 * handling NULL InpBufIndex
    				 */
    int				temp_inp_buf_index = InpBufIndex ? 
    							*InpBufIndex : 0;
    char			*start_mb_char_ptr = (char *)InpBuf+temp_inp_buf_index;

    
    if (InpBuf == NULL) {
        /* Reset bidi nest level state */
       
       return(result);
    }
#ifdef notdef    
    /*
     *  The following code can only support iso8859-1
     *  and charset size is 1.
     */
    
    if (((strcmp(layout_obj->core.shape_charset, "iso8859-1") != 0) &&
         (strcmp(layout_obj->core.shape_charset, "ISO8859-1") != 0)) ||
         (layout_obj->core.shape_charset_size != 1)) {
       /*  Cannot be supported for now */
       return(result);
        
    }
#endif
    
    num_bytes = (InpSize == 0) ? (strlen(InpBuf) - temp_inp_buf_index) : (InpSize );
    
    if (OutSize && (*OutSize == 0)) {
        *OutSize = num_bytes;
        return(result);
    }
    
    if (OutSize && (num_bytes > *OutSize)) {
        num_bytes = *OutSize;
        errno = E2BIG;
        result = -1;
    }
            
    for (i = 0; i < num_bytes; i++) {
        int	start_mb_char;
        
        start_mb_char = (start_mb_char_ptr == (InpBuf+temp_inp_buf_index));
        
        if (OutBuf) {
            OutBuf[i] = InpBuf[temp_inp_buf_index];
        }

        if (InpToOut) {
          /* 
           *  Since OutBuf is just a copy of InpBuf, we can use InpBuf.
           */
            InpToOut[i] =  start_mb_char ?   i : InpToOut[i-1];
        }
        if (OutToInp) {
            OutToInp[i] = start_mb_char ? temp_inp_buf_index : OutToInp[i-1];
        }
        
        if (Property) {
            Property[i] = start_mb_char ? DISPLAYCELL_MASK : 0;
        }
        
        if (start_mb_char)
            start_mb_char_ptr = (char *)(InpBuf + temp_inp_buf_index + mblen(InpBuf+temp_inp_buf_index, MB_CUR_MAX));
            
        (temp_inp_buf_index)++;
    }
    
    if (OutSize)
        *OutSize = num_bytes;
    
    if (InpBufIndex) 
        *InpBufIndex = temp_inp_buf_index;
        
        
        
    return(result);
    

}

static int
_LSDefaultWCSTransform(layout_obj, InpBuf, InpSize, OutBuf, OutSize,
		   InpToOut, OutToInp, Property, InpBufIndex)
    LayoutObject		layout_obj;
    const wchar_t		*InpBuf;
    size_t			InpSize;
    wchar_t			*OutBuf;
    size_t			*OutSize;
    size_t			*InpToOut;
    size_t			*OutToInp;
    unsigned char		*Property;
    size_t			*InpBufIndex;		   

{
    int				result = 0;
    int				num_bytes, i;
    				/* Should confirm with the spec on
    				 * handling NULL InpBufIndex
    				 */
    int				temp_inp_buf_index = InpBufIndex ? 
    							*InpBufIndex : 0;
    
    if (InpBuf == NULL) {
        /* Reset bidi nest level state */
       
       return(result);
    }
    
#ifdef notdef    
    /*
     *  The following code can only support iso8859-1
     *  and charset size is 1.
     */
    
    if (((strcmp(layout_obj->core.shape_charset, "iso8859-1") != 0) &&
         (strcmp(layout_obj->core.shape_charset, "ISO8859-1") != 0)) ||
         (layout_obj->core.shape_charset_size != 1)) {
       /*  Cannot be supported for now */
       return(result);
        
    }
#endif
    
    num_bytes = (InpSize == 0) ? (wslen(InpBuf) - temp_inp_buf_index) : (InpSize );
    
    if (OutSize && (*OutSize == 0)) {
        *OutSize = num_bytes;
        return(result);
    }
    
    if (OutSize && (num_bytes > *OutSize)) {
        num_bytes = *OutSize;
        errno = E2BIG;
        result = -1;
    }
        
    
    for (i = 0; i < num_bytes; i++) {
        if (OutBuf) {
            OutBuf[i] = InpBuf[temp_inp_buf_index];
        }

        if (InpToOut) {
            InpToOut[i] = i;
        }
        if (OutToInp) {
            OutToInp[i] = temp_inp_buf_index;
        }
        if (Property) {
            Property[i] = DISPLAYCELL_MASK;
        }
        (temp_inp_buf_index)++;
    }
    
    if (OutSize)
        *OutSize = num_bytes;
    
    if (InpBufIndex)
        *InpBufIndex = temp_inp_buf_index;
        
    return(result);
    

}		   	
		


