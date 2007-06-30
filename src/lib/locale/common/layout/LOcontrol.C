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
/*
 * Copyright (c) 1998, by Sun Microsystems, Inc.
 * All rights reserved.
 */
#pragma ident "@(#)LOcontrol.C	1.11     99/11/19  SMI%"

#include <errno.h> 
#include <wchar.h>
#include <sys/param.h>
#include "LOresolve.H"
#include "LOcontrol.H"
#include "LOexception.H"
#include "CTLfunctions.H"
#include "new.h"
/// methods record to set into layout_obj->methods
static LayoutMethodsRec CTL_ArabicLayoutMethods = {
  ::CTL_createLayout,
  ::CTL_destroyLayout,
  ::CTL_getValuesLayout,
  ::CTL_setValuesLayout,
  ::CTL_transformLayout,
  ::CTL_wtransformLayout,
};


/// handler for catching memory allocation problems
static void CTL_new_handler()
{
  throw LOnewException();
}
void
LOcontrol::initObj(const char *locale_name) 
{
    /* Methods for ar locale support */

  layout_obj->methods = (&CTL_ArabicLayoutMethods);
  setLocaleName(locale_name);

  /* copy address of appropriate LayoutTextDescriptorRec items
   * in the rehashValueMap to the appropriate LayoutTextDescriptor
   * fields in layout_obj
   */
  /*
  layout_obj->core.orientation = &rehashValueMap[ORIENTATION];
  layout_obj->core.type_of_text = &rehashValueMap[TYPE_OF_TEXT];
  layout_obj->core.implicit_alg = &rehashValueMap[IMPLICIT_ALG];
  layout_obj->core.swapping = &rehashValueMap[SWAPPING];
  layout_obj->core.numerals = &rehashValueMap[NUMERALS];
  layout_obj->core.text_shaping = &rehashValueMap[TEXT_SHAPING];

  // set other rehash* values
  layout_obj->core.shape_context_size = &rehashValueShapeContextSize;
  layout_obj->core.context = &rehashValueMap[CONTEXT];
  layout_obj->core.locale_name = (char*)getLocaleName();
  layout_obj->core.active_dir = TRUE;
  layout_obj->core.active_shape_editing = TRUE;
  layout_obj->core.shape_charset_size = rehashValueShapeCharsetSize;
  layout_obj->core.shape_charset = rehashValueShapeCharset;
  */
  layout_obj->core.orientation = rehashObjValue->orientation;
  layout_obj->core.type_of_text = rehashObjValue->type_of_text;
  layout_obj->core.implicit_alg = rehashObjValue->implicit_alg;
  layout_obj->core.swapping = rehashObjValue->swapping;
  layout_obj->core.numerals = rehashObjValue->numerals;
  layout_obj->core.text_shaping = rehashObjValue->text_shaping;
  // set other rehash* values
  layout_obj->core.shape_context_size = rehashObjValue->shape_context_size;
  layout_obj->core.context = rehashObjValue->context;
  layout_obj->core.locale_name = (char*)getLocaleName();
  layout_obj->core.active_dir = TRUE;
  layout_obj->core.active_shape_editing = TRUE;
  layout_obj->core.shape_charset_size = rehashObjValue->shape_charset_size;
  layout_obj->core.shape_charset =(char*)getCharSet();
  // set other fields we don't care about (?)
  layout_obj->core.in_out_text_descr_mask = AllTextDescriptors;
  layout_obj->core.in_only_text_descr = 0;
  layout_obj->core.out_only_text_descr = 0;
  layout_obj->core.check_mode = MODE_STREAM;
}

/* 
 * this routine is almost unmodified - original
 * implementation in the old C engine implementation by
 * LangBox
 */
int
LOcontrol::getValuesLayout(LayoutValues values, int *index_returned) {
    int			result = 0;
    int			i = 0;
/*     unsigned long	descr_mask = 0; */
    
    if (!values)
        return(result);
        
    while (values[i].name) {
    
        if (values[i].value == NULL)
             break; /* Error case */
        
        if ((values[i].name & AllTextDescriptors) &&
            (values[i].name & QueryValueSize)) {
            unsigned long	 *dummy = (unsigned long*)values[i].value;
                
            *dummy = sizeof(LayoutTextDescriptorRec);

        } else if (values[i].name == AllTextDescriptors) {
            LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
            
            dummy->inp |= layout_obj->core.orientation->inp;
            dummy->out |= layout_obj->core.orientation->out;
            
            dummy->inp |= layout_obj->core.context->inp;

            if(layout_obj->core.orientation->out == ORIENTATION_CONTEXTUAL)
            {
//                 if(*((int *)(layout_obj->private_data))&0x0F)
//                     dummy->out |= *((int *)(layout_obj->private_data))&0x0F;
//                 else
                    dummy->out |= layout_obj->core.context->out;
            }
            else
                dummy->out |= layout_obj->core.context->out;
            
            dummy->inp |= layout_obj->core.type_of_text->inp;
            dummy->out |= layout_obj->core.type_of_text->out;
            
            dummy->inp |= layout_obj->core.implicit_alg->inp;
            dummy->out |= layout_obj->core.implicit_alg->out;
            
            dummy->inp |= layout_obj->core.swapping->inp;
            dummy->out |= layout_obj->core.swapping->out;
            
            dummy->inp |= layout_obj->core.numerals->inp;
            dummy->out |= layout_obj->core.numerals->out;
            
            dummy->inp |= layout_obj->core.text_shaping->inp;
            dummy->out |= layout_obj->core.text_shaping->out;
            
        } else if (values[i].name & AllTextDescriptors) {
                        
            if (values[i].name & Orientation) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                    
                dummy->inp = layout_obj->core.orientation->inp;
                dummy->out = layout_obj->core.orientation->out;
            } else if  (values[i].name & Context) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                    
                dummy->inp = layout_obj->core.context->inp;
                if(layout_obj->core.orientation->out == ORIENTATION_CONTEXTUAL)
                {
//                     if(*((int *)(layout_obj->private_data))&0x0F)
//                         dummy->out = *((int *)(layout_obj->private_data))&0x0F;
//                     else
                        dummy->out = layout_obj->core.context->out;
                }
                else
                    dummy->out = layout_obj->core.context->out;

            } else if (values[i].name & TypeOfText) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                    
                dummy->inp = layout_obj->core.type_of_text->inp;
                dummy->out = layout_obj->core.type_of_text->out;
            } else if (values[i].name & ImplicitAlg) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                    
                dummy->inp = layout_obj->core.implicit_alg->inp;
                dummy->out = layout_obj->core.implicit_alg->out;
            } else if (values[i].name & Swapping) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                    
                dummy->inp = layout_obj->core.swapping->inp;
                dummy->out = layout_obj->core.swapping->out;
            } else if (values[i].name & Numerals) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                    
                dummy->inp = layout_obj->core.numerals->inp;
                dummy->out = layout_obj->core.numerals->out;

            } else if (values[i].name & TextShaping) {
                LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                    
                dummy->inp = layout_obj->core.text_shaping->inp;
                dummy->out = layout_obj->core.text_shaping->out;
            }
        } else if (values[i].name & ActiveDirectional) {
            if (values[i].name & QueryValueSize) {
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                
                *dummy = sizeof(layout_obj->core.active_dir);
            } else {
                 BooleanValue	 *dummy = (BooleanValue*)values[i].value;

                *dummy = layout_obj->core.active_dir;
            }
        }  else if (values[i].name & ActiveShapeEditing) {
            if (values[i].name & QueryValueSize) {
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                
                *dummy = sizeof(layout_obj->core.active_shape_editing);
            } else {
                 BooleanValue	 *dummy = (BooleanValue*)values[i].value;

                *dummy = layout_obj->core.active_shape_editing;
            }
        } else if (values[i].name & ShapeCharset) {
            if (values[i].name & QueryValueSize) {
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                
                *dummy = strlen(layout_obj->core.shape_charset) + 1;
            } else {
                 char			**dummy = (char**)values[i].value;

                strcpy(*dummy, layout_obj->core.shape_charset);
            }
        }  else if (values[i].name & ShapeCharsetSize) {
            if (values[i].name & QueryValueSize) {
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                
                *dummy = sizeof(layout_obj->core.shape_charset_size);
            } else {
                 int	 *dummy = (int*)values[i].value;

                *dummy = layout_obj->core.shape_charset_size;
            }
        } else if (values[i].name & InOutTextDescrMask) {
            if (values[i].name & QueryValueSize) {
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                
                *dummy = sizeof(layout_obj->core.in_out_text_descr_mask);
            } else {
                 unsigned long	 *dummy = (unsigned long*)values[i].value;

                *dummy = layout_obj->core.in_out_text_descr_mask;
            }
        } else if (values[i].name & InOnlyTextDescr) {
            if (values[i].name & QueryValueSize) {
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                 
                *dummy = sizeof(layout_obj->core.in_only_text_descr);
            } else {
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                 
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
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                 
                *dummy = sizeof(layout_obj->core.out_only_text_descr);
            } else {
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                 
                 *dummy = 0;
                 
                 if (layout_obj->core.in_out_text_descr_mask & Orientation)
                     *dummy |= layout_obj->core.orientation->out;
                 if (layout_obj->core.in_out_text_descr_mask & Context) {
		   if(layout_obj->core.orientation->out == ORIENTATION_CONTEXTUAL)
                     {
// 		       if(*((int *)(layout_obj->private_data))&0x0F) {
// 			 *dummy |= *((int *)(layout_obj->private_data))&0x0F;
// 		       } else
			 *dummy |= layout_obj->core.context->out;

                     } else {
		       *dummy |= layout_obj->core.context->out;
		     }
		 }

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
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                
                *dummy = sizeof(layout_obj->core.check_mode);
            } else {
                 int		 *dummy = (int*)values[i].value;

                *dummy = layout_obj->core.check_mode;
            }
        } else if (values[i].name & ShapeContextSize) {
            if (values[i].name & QueryValueSize) {
                 unsigned long	 *dummy = (unsigned long*)values[i].value;
                
                *dummy = sizeof(LayoutEditSizeRec);
            } else {
                 LayoutEditSize		dummy = (LayoutEditSize)values[i].value;

                 dummy->front = layout_obj->core.shape_context_size->front;
                 dummy->back = layout_obj->core.shape_context_size->back;
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
/* 
 * this routine is almost unmodified - original
 * implementation in the old C engine implementation by
 * LangBox
 */
int
LOcontrol::setValuesLayout(LayoutValues values, int *index_returned) {
    int			result = 0;
    int			i = 0;
/*     LayoutCoreRec	core_data; */
    
    
    if (!values)
        return(0);
        
          
    if (foundInvalidValue(values, index_returned))
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
	  setCharSet((char *)values[i].value);
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
             layout_obj->core.check_mode = *(int*)values[i].value;


        } else
            break; /* Error condition */

        i++;
    
    }
    
    if (values[i].name) {
        result = -1;
        *index_returned = i;
        errno = EINVAL;
    }
    rehash();
    return(result);

}
/* 
 * this routine is almost unmodified - original
 * implementation in the old C engine implementation by
 * LangBox
 */
int 
LOcontrol::foundInvalidValue(LayoutValues values, int *index_returned)
{
  int			i = 0;
    			
  while (values[i].name) {
             
    if (values[i].name & AllTextDescriptors) {
        
      if (values[i].value == NULL)
	break; /* Error case */
                        
      if (values[i].name & Orientation) {
	LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                    
	if (dummy->inp & ~MaskOrientation)
	  goto InvalidValue;
                    
	if (dummy->out & ~MaskOrientation)
	  goto InvalidValue;
                                    
      } else if  (values[i].name & Context) {
	LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;
                
	if (dummy->inp & ~MaskContext)
	  goto InvalidValue;
                    
	if (dummy->out & ~MaskContext)
	  goto InvalidValue;
               
                                
      } else if (values[i].name & TypeOfText) {
	LayoutTextDescriptor	dummy = (LayoutTextDescriptor)values[i].value;

	if (dummy->inp & ~MaskTypeOfText)
	  goto InvalidValue;
                    
	if (dummy->out & ~MaskTypeOfText)
	  goto InvalidValue;
                
                
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
      unsigned long	dummy = (unsigned long)values[i].value;
            
      if (dummy & ~AllTextDescriptors)
	goto InvalidValue;
                
    } else if ((values[i].name & InOnlyTextDescr) ||
	       (values[i].name & OutOnlyTextDescr)) {
      unsigned long		dummy = (unsigned long)values[i].value;
                 
      if (dummy & ~MaskAllTextDescriptors)
	goto InvalidValue;
                
    } else if (values[i].name & CheckMode) {
      int				dummy = *(int*)values[i].value;
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

LayoutObject
LOcontrol::createLayout(LayoutValues layout_values)
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
  rehash();
  return  (LayoutObject)layout_obj;
}
int 
LOcontrol::transformImplement(const void *InpBuf, 
			      size_t InpSize, void *OutBuf, 
			      size_t *OutSize, size_t *InpToOut,
			      size_t *OutToInp, 
			      unsigned char *Property, 
			      size_t *InpBufIndex,
			      outbufftype_t bufType) 
{
  int startIndex = (InpBufIndex && (*InpBufIndex < InpSize)) ? *InpBufIndex : 0;
  int result = 0;
  if (InpBuf == NULL) {
    /* Reset bidi nest level state */
    return(result);
  }
  size_t num_bytes;
  size_t realOutsize;
  
  if (0 == InpSize) {	// string is null terminated
    switch (bufType) {
    case ISUSHORTPTR:
    case ISCHARPTR:
      num_bytes = strlen((char*)InpBuf);
      break;
    case ISWCHARPTR:
      num_bytes = wcslen((wchar_t*)InpBuf);
      break;
    default:
#ifdef DEBUG
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
      break;
    }
  } else {
    num_bytes = InpSize;
  }

  if (OutSize && (*OutSize == 0)) {
    *OutSize = num_bytes;
    return(result);
  }
  if (OutSize && *OutSize) {
//     realOutsize = (ISUSHORTPTR == bufType ? *OutSize / sizeof(unsigned short) : *OutSize);
    realOutsize = *OutSize;
  } else {
    realOutsize = num_bytes;
  }
  if (num_bytes > realOutsize) {
    num_bytes = realOutsize;
    errno = E2BIG;
    result = -1;
  }
  /* document does not mention a possibility of OutSize being NULL,
   * Until known otherwise, assume size of OutBuf is same as num_bytes
   */
#if 0
  if (!OutSize) {
    // need to do something?
  }
#endif
  /* set the line from appropriate inputtype:
   * unless OutBuf type is ISWCHARPTR, the type of OutBuf is
   * (char *)InpBuf
   */
  switch (bufType) {
  case ISUSHORTPTR:
  case ISCHARPTR:
    line->setLine((unsigned char*)InpBuf, num_bytes);
    break;
  case ISWCHARPTR:
    line->setLine((wchar_t*)InpBuf, num_bytes);
    break;
  default:
#ifdef DEBUG
    throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
    break;
  }
  transformLine();
  copyOutBuff(OutBuf, OutSize, InpBufIndex, bufType,startIndex);
  copyInpToOut(InpToOut,bufType, startIndex);
  copyOutToInp(OutToInp, bufType, startIndex);
  copyProperies(Property,bufType, startIndex);
  ///line.deleteLine();
  return result;
}
/* if no CTL char in the text, no translation needed but fill the corrrect values in the out buffers*/
int 
LOcontrol::asciitransformImplement(const void *InpBuf, 
				   size_t InpSize, void *OutBuf, 
				   size_t *OutSize, size_t *InpToOut,
				   size_t *OutToInp, 
				   unsigned char *Property, 
				   size_t *InpBufIndex,
				   outbufftype_t bufType) 
{
  int startIndex = (InpBufIndex && (*InpBufIndex < InpSize)) ? *InpBufIndex : 0;
  int result = 0;
  if (InpBuf == NULL) {
    /* Reset bidi nest level state */
    return(result);
  }
  size_t num_bytes;
  size_t realOutsize;
  
  if (0 == InpSize) {	// string is null terminated
    switch (bufType) {
    case ISUSHORTPTR:
    case ISCHARPTR:
      num_bytes = strlen((char*)InpBuf);
      break;
    case ISWCHARPTR:
      num_bytes = wcslen((wchar_t*)InpBuf);
      break;
    default:
#ifdef DEBUG
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
      break;
    }
  } else {
    num_bytes = InpSize;
  }

  if (OutSize && (*OutSize == 0)) {
    *OutSize = num_bytes;
    return(result);
  }
  if (OutSize && *OutSize) {
//     realOutsize = (ISUSHORTPTR == bufType ? *OutSize / sizeof(unsigned short) : *OutSize);
    realOutsize = *OutSize;
  } else {
    realOutsize = num_bytes;
  }
  if (num_bytes > realOutsize) {
    num_bytes = realOutsize;
    errno = E2BIG;
    result = -1;
  }
  /* document does not mention a possibility of OutSize being NULL,
   * Until known otherwise, assume size of OutBuf is same as num_bytes
   */
#if 0
  if (!OutSize) {
    // need to do something?
  }
#endif
  asciicopyOutBuff(OutBuf, OutSize, InpBuf, InpSize, InpBufIndex, bufType,startIndex);
  asciicopyInpToOut(InpToOut, bufType, InpSize, startIndex);
  asciicopyOutToInp(OutToInp, bufType, InpSize, startIndex);
  asciicopyProperies(Property,bufType, InpSize, startIndex);
  ///line.deleteLine();
  return result;
}
int 
LOcontrol::transform(const unsigned char *InpBuf, 
		     size_t InpSize, void *OutBuf, 
		     size_t *OutSize, size_t *InpToOut, 
		     size_t *OutToInp, 
		     unsigned char *Property, 
		     size_t *InpBufIndex) 
{
  outbufftype_t bufType; 
  BOOL isCTLchar = false;
  switch (rehashObjValue->shape_charset_size) {
  case 1:
    bufType = ISCHARPTR;
    break;
  case 2:
    bufType = ISUSHORTPTR;
    break;
  case 4:
    bufType = ISUSHORTPTR;
    break;
  default:
#ifdef DEBUG
    {
       throw LOexception("HDIGH:",__FILE__ , __LINE__);
    }
#endif
    break;
  }
   
  for(int i=0; i<InpSize; i++)
    if ((!isascii(InpBuf[i])) ||
	(isdigit(InpBuf[i]) && rehashObjValue->numerals->out != NUMERALS_NOMINAL)){
      isCTLchar=true;     
      break;
    }
  if (isCTLchar)
    return transformImplement((const void*)InpBuf, 
			      InpSize, 
			      OutBuf, 
			      OutSize, 
			      InpToOut, 
			      OutToInp, 
			      Property, 
			      InpBufIndex,
			      bufType);
  else
    return asciitransformImplement((const void*)InpBuf, 
				   InpSize, 
				   OutBuf, 
				   OutSize, 
				   InpToOut, 
				   OutToInp, 
				   Property, 
				   InpBufIndex,
				   bufType); 
}
int 
LOcontrol::wtransform(const wchar_t *InpBuf, 
		      size_t InpSize, wchar_t *OutBuf, 
		      size_t *OutSize, size_t *InpToOut, 
		      size_t *OutToInp, 
		      unsigned char *Property, 
		      size_t *InpBufIndex) 
{ 
  BOOL isCTLchar = false;
  for(int i=0; i<InpSize; i++)
    if ((!isascii(InpBuf[i])) ||
	(isdigit(InpBuf[i]) && rehashObjValue->numerals->out != NUMERALS_NOMINAL)){
      isCTLchar=true;     
      break;
    }
  if (isCTLchar)
    return transformImplement((const void*)InpBuf, 
			      InpSize, 
			      (void*)OutBuf, 
			      OutSize, 
			      InpToOut, 
			      OutToInp, 
			      Property, 
			      InpBufIndex,
			      ISWCHARPTR);
  else
    return asciitransformImplement((const void*)InpBuf, 
				   InpSize, 
				   (void*)OutBuf, 
				   OutSize, 
				   InpToOut, 
				   OutToInp, 
				   Property, 
				   InpBufIndex,
				   ISWCHARPTR);
}
BOOL
LOcontrol::transformLine() {
#if (defined (DEBUG) && defined (PRINT))
  size_t InpSize = line->getinsize();
  printf("\n-------\n");
  printf("transformLine: <");
  for (size_t i = 0; i < InpSize; i++) {
    printf("%c", (unsigned char)line->in[i].getval());
	   }
  printf(">\n");
  printf("InpSize: (%d)\n",InpSize);
  printf("-------\n");
#endif
  LOresolve resolver(*this, *line);
  if (!resolver.resolve()) {
    return false;
  }
#if (defined (DEBUG) && defined (PRINT))
  line->print();
#endif
  return true;
}

void
LOcontrol::copyOutBuff(void*OutBuf, size_t *OutSize, size_t *InpBufIndex, outbufftype_t bufType, int startIndex )
{
  unsigned short glyphIndex;
  //wchar_t wglyphIndex;
  const LOchar::CharPtrArray &sortout = line->getline();
  size_t reali=0; 
  for (size_t i = 0 ; i+startIndex < line->getsortoutsize(); i++) {
    if (NULL != OutBuf) {
      switch (bufType) {
      case ISCHARPTR:
	((unsigned char*)OutBuf)[i] = (unsigned char)sortout[i+startIndex]->getshaped();
	break;
      case ISWCHARPTR:
	if(rehashObjValue->shape_charset_size!=4){	 	  
	  ((wchar_t*)OutBuf)[i] = (wchar_t)sortout[i+startIndex]->getshaped();
	  break;
	}
      case ISUSHORTPTR: //Arabic two bytes font
	if (rehashObjValue->shape_charset_size <= 2)  
	  ((unsigned short*)OutBuf)[i] = (unsigned short)sortout[i+startIndex]->getshaped();

	else { // 4 byte for multi font, insert the font ID in the first byte
	  glyphIndex = (unsigned short)sortout[i+startIndex]->getshaped();	
	  reali=(i*2* sizeof(short));
	  ((unsigned char*) OutBuf)[reali] = (hibyte(glyphIndex)>0)? strtol("E6", (char **)NULL, 16):0x00; //first byte (fontID)
	  ((unsigned char*) OutBuf)[reali + 1] = 0x00;//2nd byte
	  ((unsigned char*) OutBuf)[reali + 2] = hibyte(glyphIndex);//3rd byte
	  ((unsigned char*) OutBuf)[reali + 3] = lobyte(glyphIndex);//4th byte
	}
#if (defined (DEBUG) && defined (XPRINT))
  printf("output buffer for char %d: ", i);
  printf("%2x ", ((unsigned char*) OutBuf)[reali]);
  printf("%2x ", ((unsigned char*) OutBuf)[reali+1]);
  printf("%2x ", ((unsigned char*) OutBuf)[reali+2]);
  printf("%2x \n ", ((unsigned char*) OutBuf)[reali+3]); 
#endif
  break;
      default:
#ifdef DEBUG
	{
	  throw LOexception("HDIGH:",__FILE__ , __LINE__);
	}
#endif
  break;
      }
    }
  }
  if (NULL != OutSize) {
    if (ISUSHORTPTR == bufType) {
      *OutSize = (line->getsortoutsize() * (rehashObjValue->shape_charset_size) )-startIndex;
    } else {
      *OutSize = line->getsortoutsize()-startIndex;
    }
  }

  if (NULL != InpBufIndex) {
    // set to number of input character we processed (which is all of them)
    *InpBufIndex = line->getinsize();
  }
}
void
LOcontrol::asciicopyOutBuff(void*OutBuf, size_t *OutSize, const void *InpBuf, 
			    size_t InpSize, size_t *InpBufIndex, outbufftype_t bufType, int startIndex )
{
  unsigned short glyphIndex;
  size_t reali=0; 
  for (size_t i = 0 ; i+startIndex < InpSize; i++) {
    if (NULL != OutBuf) {
      switch (bufType) {
      case ISCHARPTR:
	((unsigned char*)OutBuf)[i] = ((unsigned char*)InpBuf)[i+startIndex];
	break;
      case ISWCHARPTR:
	if(rehashObjValue->shape_charset_size!=4){	 	  
	  ((wchar_t*)OutBuf)[i]= (wchar_t)(((unsigned char*)InpBuf)[i+startIndex]);
	  break;
	}
      case ISUSHORTPTR: //Arabic two bytes font
	if (rehashObjValue->shape_charset_size <= 2)  
	  ((unsigned short*)OutBuf)[i] = (unsigned short)(((unsigned char*)InpBuf)[i+startIndex]);

	else { // 4 byte for multi font, insert the font ID in the first byte
	  glyphIndex =(unsigned short)(((unsigned char*)InpBuf)[i+startIndex]);	
	  reali=(i*2* sizeof(short));
	  ((unsigned char*) OutBuf)[reali] = (hibyte(glyphIndex)>0)? strtol("E6", (char **)NULL, 16):0x00; //first byte (fontID)
	  ((unsigned char*) OutBuf)[reali + 1] = 0x00;//2nd byte
	  ((unsigned char*) OutBuf)[reali + 2] = hibyte(glyphIndex);//3rd byte
	  ((unsigned char*) OutBuf)[reali + 3] = lobyte(glyphIndex);//4th byte
	}
#if (defined (DEBUG) && defined (XPRINT))
  printf("output buffer for char %d: ", i);
  printf("%2x ", ((unsigned char*) OutBuf)[reali]);
  printf("%2x ", ((unsigned char*) OutBuf)[reali+1]);
  printf("%2x ", ((unsigned char*) OutBuf)[reali+2]);
  printf("%2x \n ", ((unsigned char*) OutBuf)[reali+3]); 
#endif
  break;
      default:
#ifdef DEBUG
	{
	  throw LOexception("HDIGH:",__FILE__ , __LINE__);
	}
#endif
  break;
      }
    }
  }
  if (NULL != OutSize) {
    if (ISUSHORTPTR == bufType) {
     *OutSize = (InpSize *  (rehashObjValue->shape_charset_size)) - startIndex;
    } else {
      *OutSize = InpSize - startIndex;
    }
  }

  if (NULL != InpBufIndex) {
    // set to number of input character we processed (which is all of them)
    *InpBufIndex = InpSize;
  }
}  
void
LOcontrol::copyInpToOut(size_t *InpToOut,outbufftype_t bufType, int startIndex)
{
  size_t i;
  if (!InpToOut) {
    return;
  }
    switch (bufType) {
    case ISUSHORTPTR: 
      for( i = 0; i+startIndex < line->getinsize(); i++) {
	InpToOut[i] = ((rehashObjValue->shape_charset_size)* line->in[i+startIndex].getdown()->getpos())+
	  (rehashObjValue->shape_charset_size - 1);
#if (defined (DEBUG) && defined (PRINT))
  printf("intoout buffer for char %d: ", i);
  printf("%2d \n ",InpToOut[i]); 
#endif

      }
      break;
    default:
      for( i = 0; i+startIndex < line->getinsize(); i++) {
	InpToOut[i] = line->in[i+startIndex].getdown()->getpos();
      }
      break;
    }
}
void
LOcontrol::asciicopyInpToOut(size_t *InpToOut,outbufftype_t bufType, size_t inpSize, int startIndex)
{
 size_t i;
  if (!InpToOut) {
    return;
  }
    switch (bufType) {
    case ISUSHORTPTR: 
      for( i = 0; i+startIndex <inpSize; i++) {
	InpToOut[i] = ((rehashObjValue->shape_charset_size)*(i+startIndex))+
	  (rehashObjValue->shape_charset_size - 1);
#if (defined (DEBUG) && defined (PRINT))
  printf("intoout buffer for char %d: ", i);
  printf("%2d \n ",InpToOut[i]); 
#endif

      }
      break;
    default:
      for( i = 0; i+startIndex < inpSize; i++) {
	InpToOut[i] =i+startIndex;
      }
      break;
    }
}
void
LOcontrol::copyOutToInp(size_t *OutToInp,outbufftype_t bufType, int startIndex)
{
  size_t i;
  if (!OutToInp) {
    return;
  }
    switch (bufType) {
    case ISUSHORTPTR: 
      for( i = 0; i+startIndex < line->getsortoutsize(); i++) {
	if(rehashObjValue->shape_charset_size == 2)
	  OutToInp[i*2]=OutToInp[(i*2)+1] = line->sortout[i+startIndex]->getup()->getpos(); //the output is 2 bytes
	  else
	    OutToInp[i*4]=OutToInp[(i*4)+1]=
	      OutToInp[(i*4)+2]=OutToInp[(i*4)+3] = line->sortout[i+startIndex]->getup()->getpos(); //the output is 4 bytes
#if (defined (DEBUG) && defined (PRINT))
  printf("outtoin buffer for char %d: ", i);
  printf("%2d ", OutToInp[i*4]);
  printf("%2d ", OutToInp[(i*4)+1]);
  printf("%2d ", OutToInp[(i*4)+2]);
  printf("%2d \n ",OutToInp[(i*4)+3]); 
#endif
      }
      break;
    default:   
    for( i = 0; i+startIndex < line->getsortoutsize(); i++) {
    //    OutToInp[i] = line.sortout[i]->getup()->getpos() + inp_buf_index;
	OutToInp[i] = line->sortout[i+startIndex]->getup()->getpos();
      }
    break;
    }
}
void
LOcontrol::asciicopyOutToInp(size_t *OutToInp,outbufftype_t bufType, size_t inpSize, int startIndex)
{
   size_t i;
  if (!OutToInp) {
    return;
  }
    switch (bufType) {
    case ISUSHORTPTR: 
      for( i = 0; i+startIndex < inpSize; i++) {
	if(rehashObjValue->shape_charset_size == 2)
	 	OutToInp[i*2]=OutToInp[(i*2)+1] = i+startIndex; //the output is 2 bytes
	  else
	    OutToInp[i*4]=OutToInp[(i*4)+1]=
	      OutToInp[(i*4)+2]=OutToInp[(i*4)+3] = i+startIndex; //the output is 4 bytes
#if (defined (DEBUG) && defined (PRINT))
  printf("outtoin buffer for char %d: ", i);
  printf("%2d ", OutToInp[i*4]);
  printf("%2d ", OutToInp[(i*4)+1]);
  printf("%2d ", OutToInp[(i*4)+2]);
  printf("%2d \n ",OutToInp[(i*4)+3]); 
#endif
      }
      break;
    default:   
    for( i = 0; i+startIndex < inpSize; i++) {
	OutToInp[i] = i+startIndex;
      }
    break;
    }
}
void
LOcontrol::copyProperies(unsigned char *Property,outbufftype_t bufType, int startIndex)
{
  size_t i;
  if (!Property) {
    return;
  }
   for(i = 0; i+startIndex < line->getinsize(); i++) {
      Property[i] = (line->in[i+startIndex].getproperty() | line->in[i+startIndex].getcellIndicator());
#if (defined (DEBUG) && defined (PRINT))
  printf("property buffer for char %d: ", i);
  printf("%2x \n ",Property[i] ); 
#endif
    }
}
void
LOcontrol::asciicopyProperies(unsigned char *Property,outbufftype_t bufType, size_t inpSize, int startIndex)
{
  size_t i;
  if (!Property) {
    return;
  }
   for(i = 0; i+startIndex < inpSize; i++) {
      Property[i] = 0x10;
    }
} 
/*----------------------------------------------------------------------
  PLS interface functions
  ---------------------------------------------------------------------*/
LayoutObject CTL_createLayout(LayoutObj layout_obj, LayoutValues layout_values)
{
  LayoutObject res = NULL;
  LOcontrolClass *loControl = (LOcontrolClass*)layout_obj->private_data;
  try {
    res = loControl->createLayout(layout_values);
  }
  catch (LOexception) {
    res = NULL;
    CTL_destroyLayout(layout_obj);
  }
  return res;
}

int CTL_destroyLayout(LayoutObj layout_obj)
{
  if (layout_obj) {
    LOcontrolClass *loControl = (LOcontrolClass*)layout_obj->private_data;
    if (loControl) {
      delete loControl;
    }
    delete layout_obj;
  }
  return 0;
}

int CTL_getValuesLayout(LayoutObj layout_obj, LayoutValues values, int *index_returned)
{
  LOcontrolClass *loControl = (LOcontrolClass*)layout_obj->private_data;
  int res = 0;
  try {
    res = loControl->getValuesLayout(values, index_returned);
  }
  catch (LOexception) {
    res = -1;
  }
  return res;
}

int CTL_setValuesLayout(LayoutObj layout_obj, LayoutValues values, int *index_returned)
{
  int res = 0;
  LOcontrolClass *loControl = (LOcontrolClass*)layout_obj->private_data;
  try {
    res = loControl->setValuesLayout(values, index_returned);
  }
  catch (LOexception) {
    res = -1;
  }
  return res;
}

int CTL_transformLayout(LayoutObj layout_obj, 
				const char *InpBuf, 
				size_t InpSize, void *OutBuf, 
				size_t *OutSize, size_t *InpToOut, 
				size_t *OutToInp, 
				unsigned char *Property, 
				size_t *InpBufIndex) 
{
  LOcontrolClass *loControl = (LOcontrolClass*)layout_obj->private_data;
  int res = 0;
  try {
    res = loControl->transform((const unsigned char*)InpBuf, 
			       InpSize, 
			       OutBuf, 
			       OutSize, 
			       InpToOut, 
			       OutToInp, 
			       Property, 
			       InpBufIndex);
  }
  catch (LOexception) {
    res = -1;
  }
  return res;
}  

int 
CTL_wtransformLayout(LayoutObject layout_obj, 
				   const wchar_t *InpBuf, 
				   size_t InpSize, 
				   void *OutBuf, 
				   size_t *OutSize, 
				   size_t *InpToOut, 
				   size_t *OutToInp, 
				   unsigned char *Property, 
				   size_t *InpBufIndex)
{
  LOcontrolClass *loControl = (LOcontrolClass*)layout_obj->private_data;
  int res = 0;
  try {
    res =  loControl->wtransform(InpBuf, 
				 InpSize, 
				 (wchar_t*)OutBuf, 
				 OutSize, 
				 InpToOut, 
				 OutToInp, 
				 Property, 
				 InpBufIndex);
  }
  catch (LOexception) {
    res = -1;
  }
  return res;
}


LayoutObj
_LayoutObjectInit(char *locale_name)
{
  BOOL caughtException = false;
  set_new_handler(&CTL_new_handler);
  LayoutObj layout_obj;
  try {
    layout_obj = new LayoutObjectRec;
    bzero(layout_obj,sizeof(LayoutObjectRec));
    layout_obj->private_data = new LOcontrolClass(layout_obj, locale_name);
  }
  catch (LOexception) {
    caughtException = true;
  }
  if (caughtException) {
    CTL_destroyLayout(layout_obj);
    return NULL;
  }
  return layout_obj;
}
const LOdefs::text_descriptor_val_t
LOcontrol::getDescriptor(textDescriptor_enum_t id,
			 textDescriptor_side_t side) const
{//
  // LayoutValueMap::const_iterator k = rehashValueMap.find(id);
#ifdef DEBUG
  // if (rehashValueMap.end() == k) {
  //  throw LOexception("HDIGH:",__FILE__ , __LINE__);
  // }
#endif
  switch (id){
  case orientation:
    switch (side) {
    case INP:
      //return (text_descriptor_val_t)(*k).second.inp;
      return (text_descriptor_val_t)( rehashObjValue->orientation->inp);
    case OUT:
      //return (text_descriptor_val_t)(*k).second.out;
      return (text_descriptor_val_t)( rehashObjValue->orientation->out);
    default:
#ifdef DEBUG
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
      break;
    }
    break;
  case context:
    switch (side) {
    case INP:
      return (text_descriptor_val_t)( rehashObjValue->context->inp);
    case OUT:
      return (text_descriptor_val_t)( rehashObjValue->context->out);
    default:
#ifdef DEBUG
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
      break;
    }
    break;
  case type_of_text:
    switch (side) {
    case INP:
      return (text_descriptor_val_t)( rehashObjValue->type_of_text->inp);
    case OUT:
      return (text_descriptor_val_t)( rehashObjValue->type_of_text->out);
    default:
#ifdef DEBUG
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
      break;
    }
    break;
  case implicit_alg:
    switch (side) {
    case INP:
    return (text_descriptor_val_t)( rehashObjValue->implicit_alg->inp);
    case OUT:
    return (text_descriptor_val_t)( rehashObjValue->implicit_alg->out);
    default:
#ifdef DEBUG
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
      break;
    }
    break;
  case swapping:
    switch (side) {
    case INP:
    return (text_descriptor_val_t)( rehashObjValue->swapping->inp);
    case OUT:
    return (text_descriptor_val_t)( rehashObjValue->swapping->out);
    default:
#ifdef DEBUG
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
      break;
    }
    break;
  case numerals:
    switch (side) {
    case INP:
    return (text_descriptor_val_t)( rehashObjValue->numerals->inp);
    case OUT:
    return (text_descriptor_val_t)( rehashObjValue->numerals->out);
    default:
#ifdef DEBUG
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
      break;
    }
    break;
  case text_shaping:
    switch (side) {
    case INP:
    return (text_descriptor_val_t)( rehashObjValue->text_shaping->inp);
    case OUT:
    return (text_descriptor_val_t)( rehashObjValue->text_shaping->out);
    default:
#ifdef DEBUG
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
      break;
    }
    break;
  }
}
#if (defined (DEBUG) && defined (PRINT))
void
LOcontrol::initValMap()
{
  if (valueMapInitialized) {
    return;
  }
  /*
  valMap[TYPE_OF_TEXT] = "TYPE_OF_TEXT";
  valMap[NUMERALS] = "NUMERALS";
  valMap[TEXT_SHAPING] = "TEXT_SHAPING";
  valMap[ORIENTATION] = "ORIENTATION";
  valMap[SWAPPING] = "SWAPPING";
  valMap[CONTEXT] = "CONTEXT";
  valMap[IMPLICIT_ALG] = "IMPLICIT_ALG";

  nameMap[ORIENTATION_LTR] = "ORIENTATION_LTR";
  nameMap[ORIENTATION_RTL] = "ORIENTATION_RTL";
  nameMap[ORIENTATION_TTBRL] = "ORIENTATION_TTBRL";
  nameMap[ORIENTATION_TTBLR] = "ORIENTATION_TTBLR";
  nameMap[ORIENTATION_CONTEXTUAL] = "ORIENTATION_CONTEXTUAL";

  /* Possible values for Context 
  nameMap[CONTEXT_LTR] = "CONTEXT_LTR";
  nameMap[CONTEXT_RTL] = "CONTEXT_RTL";
  */
  /* Possible values for TypeOfText
  nameMap[TEXT_IMPLICIT] = "TEXT_IMPLICIT";
  nameMap[TEXT_EXPLICIT] = "TEXT_EXPLICIT";
  nameMap[TEXT_VISUAL] = "TEXT_VISUAL";
  */
  /* Possible values for ImplicitAlg
  nameMap[ALGOR_BASIC] = "ALGOR_BASIC";
  nameMap[ALGOR_IMPLICIT] = "ALGOR_IMPLICIT";
  */
  /* Possible values for Swapping 
  nameMap[SWAPPING_NO] = "SWAPPING_NO";
  nameMap[SWAPPING_YES] = "SWAPPING_YES";
  */
  /* Possible values for Numerals 
  nameMap[NUMERALS_NOMINAL] = "NUMERALS_NOMINAL";
  nameMap[NUMERALS_NATIONAL] = "NUMERALS_NATIONAL";
  nameMap[NUMERALS_CONTEXTUAL] = "NUMERALS_CONTEXTUAL";
  */
  /* Possible values for TextShaping 
  nameMap[TEXT_SHAPED] = "TEXT_SHAPED";
  nameMap[TEXT_NOMINAL] = "TEXT_NOMINAL";
  nameMap[TEXT_SHFORM1] = "TEXT_SHFORM1";
  nameMap[TEXT_SHFORM2] = "TEXT_SHFORM2";
  nameMap[TEXT_SHFORM3] = "TEXT_SHFORM3";
  nameMap[TEXT_SHFORM4] = "TEXT_SHFORM4";
  */
  /* Possible values for CheckMode */
//   nameMap[MODE_STREAM] = "MODE_STREAM";
//   nameMap[MODE_EDIT] = "MODE_EDIT";

  valueMapInitialized = true;
}
void
LOcontrol::print()
{
  initValMap();
  /*
  map<textDescriptor_enum_t, const char *, less<textDescriptor_enum_t> >::const_iterator k;
  printf("Layout Object Setup Status:\n");
  printf("---------------------------\n");
  for (k = valMap.begin(); k != valMap.end(); k++) {
    printf("%s\n", (*k).second);
    printf("\t IN:%-15s\n", nameMap[rehashValueMap[(*k).first].inp]);
    printf("\tOUT:%-15s\n", nameMap[rehashValueMap[(*k).first].out]);
  }
  printf("%-15s:\t%s\n", "ShapeCharset", getCharSet());
  printf("%-15s:\t%d\n", "ShapeCharsetSize", rehashValueShapeCharsetSize);
  */
  printf("Layout Object Setup Status:\n");
  printf("---------------------------\n");
  printf("%s\n","orientation");
  printf("\t IN:%-15d\n",rehashObjValue->orientation->inp);
  printf("\tOUT:%-15d\n",rehashObjValue->orientation->out);
  printf("%s\n","type_of_text");
  printf("\t IN:%-15d\n",rehashObjValue->type_of_text->inp);
  printf("\tOUT:%-15d\n",rehashObjValue->type_of_text->out);
  printf("%s\n","numerals");
  printf("\t IN:%-15d\n",rehashObjValue->numerals->inp);
  printf("\tOUT:%-15d\n",rehashObjValue->numerals->out);
  printf("%s\n","text_shaping");
  printf("\t IN:%-15d\n",rehashObjValue->text_shaping->inp);
  printf("\tOUT:%-15d\n",rehashObjValue->text_shaping->out);
  printf("%s\n","orientation");
  printf("\t IN:%-15d\n",rehashObjValue->orientation->inp);
  printf("\tOUT:%-15d\n",rehashObjValue->orientation->out);
  printf("%s\n","swapping");
  printf("\t IN:%-15d\n",rehashObjValue->swapping->inp);
  printf("\tOUT:%-15d\n",rehashObjValue->swapping->out);
  printf("%s\n","implicit_alg");
  printf("\t IN:%-15d\n",rehashObjValue->implicit_alg->inp);
  printf("\tOUT:%-15d\n",rehashObjValue->implicit_alg->out);
  printf("%s\n","context");
  printf("\t IN:%-15d\n",rehashObjValue->context->inp);
  printf("\tOUT:%-15d\n",rehashObjValue->context->out);
  printf("\t shape_charset_size:%-15d\n",rehashObjValue->shape_charset_size);
  printf("\t shape_charset:%-15s\n",rehashValueShapeCharset);
}

#endif // DEBUG
