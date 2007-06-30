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
#pragma ident "@(#)LOcontrol.H	1.11     99/11/19  SMI%"

#ifndef LOCONTROL_H
#define LOCONTROL_H

#include "LOline.H"
#include "CTLfunctions.H"
#include <string.h>
/** PLS layout object interface class.
    This class interfaces with the global m_*_layout routines.
    It also contains, as a protected data member, the actuall
    LayoutObject object which is passed to each of the m_*_layout routines
    by the client applications. This LayoutObject is kept
    in the layout_obj data member. In turn, the value of
    the private data pointer in layout_obj (layout_obj->private_data),
    contains the value of 'this'. So this class is a little bit like a snake eating
    its own tail....
    Other providing C++ interface to the m_*_layout routines, this class
    also provides other classes with methods to access and query the lyout values,
    such as the value of orientation, swapping, etc.
 */
class LOcontrol : public LOdefs {
public:
  /** Contsruct with preallocated PLS layout object.
      The variable obj is copied to the member variable layout_obj
      this pointer was allocated outside scope of this class and should 
      not be destroyed by *this
      @param obj preallocated layout object
      @param locale_name argument to pass to initLayoutObj
      @see layout_obj
  */
  LOcontrol(LayoutObj obj, const char* locale_name) :
#if (defined (DEBUG) && defined (PRINT))
    valueMapInitialized(false),
#endif
    layout_obj(obj),
    rehashValueShapeCharset(NULL),// defaultShapeCharset(NULL), 
    rehashValueLocaleName(NULL),line(NULL),
    rehashObjValue(NULL), defaultObjValue(NULL)
     {
       rehashObjValue= new LayoutCoreRec[(sizeof(LayoutCoreRec))+1]; 
       //bzero(rehashObjValue,sizeof(rehashObjValue));
       defaultObjValue= new LayoutCoreRec[(sizeof(LayoutCoreRec))+1];
       //bzero(defaultObjValue,sizeof(defaultObjValue));
    initDefaultValues();
  }
  /// 
  //inline void deleteObj(){delete this;}
  ~LOcontrol() {
    if (NULL != rehashValueLocaleName) {
      delete [] rehashValueLocaleName;
    }
    if (NULL != rehashValueShapeCharset) {
      delete [] rehashValueShapeCharset;
    }
    /* 
    if (NULL != defaultShapeCharset) {
      delete [] defaultShapeCharset;
    }
    */
    if (NULL !=rehashObjValue) {
      delete [] rehashObjValue->type_of_text;
      delete [] rehashObjValue->numerals;
      delete [] rehashObjValue->text_shaping;
      delete [] rehashObjValue->orientation;
      delete [] rehashObjValue->swapping;
      delete [] rehashObjValue->implicit_alg;
      delete [] rehashObjValue->context;
      delete [] rehashObjValue->shape_context_size;
      delete [] rehashObjValue;
    }
    if (NULL !=defaultObjValue) {
      delete [] defaultObjValue->type_of_text;
      delete [] defaultObjValue->numerals;
      delete [] defaultObjValue->text_shaping;
      delete [] defaultObjValue->orientation;
      delete [] defaultObjValue->swapping;
      delete [] defaultObjValue->implicit_alg;
      delete [] defaultObjValue->context;
      delete [] defaultObjValue->shape_context_size;
      delete [] defaultObjValue;
    }
    if (NULL !=line) delete line;
  }
  /** Description of layout values set/get mechanisem.
      @name Rehashing
      
      The Layout engine has to keep track of certain values
      in the LayoutObjectRec pointed to by the data member layout_obj.
      These values may be either queried or set by external applications
      using the m_setvalues_layout() or m_getvalues_layout() PLS API routines.
      It is the responsibility of the LOcontrol class to not only provide an
      internal interface to the above mentioned routines, but also to manage memory
      of all text descriptor fields (allocation and freeing of), as well as the locale
      name, shape charset name etc.

      These are the fields of LayoutObjectRec (from layout_int.h):

      \begin{verbatim}
      
      typedef struct {                                            
          char                        *locale_name;               
                                                                  
          LayoutTextDescriptor        orientation;                
          LayoutTextDescriptor        context;                    
          LayoutTextDescriptor        type_of_text;               
          LayoutTextDescriptor        implicit_alg;               
          LayoutTextDescriptor        swapping;                   
          LayoutTextDescriptor        numerals;                   
          LayoutTextDescriptor        text_shaping;               
          BooleanValue                active_dir;                 
          BooleanValue                active_shape_editing;       
          char                        *shape_charset;             
          int                         shape_charset_size;         
          unsigned long               in_out_text_descr_mask;     
          unsigned long               in_only_text_descr;         
          unsigned long               out_only_text_descr;        
          int                         check_mode;                 
          LayoutEditSize              shape_context_size;         
      } LayoutCoreRec, *LayoutCore;                               
                                                                  
      \end{verbatim}
      
      The type LayoutTextDescriptor in turn, is a pointer to:
      
      \begin{verbatim}
      
      typedef struct {
         LayoutDesc		inp;
         LayoutDesc		out;
      } LayoutTextDescriptorRec, *LayoutTextDescriptor;
      \end{verbatim}

      When the LOcontrol object is created (note: the LOcontrol class
      itself is an abstract class, it always initialize via its decendant 
      classes), the LayoutTextDescriptor tyep fields, as well as char* fields are
      not allocated. By calling the initObj() method, we set these pointer fields to 
      point at valid LayoutTextDescriptorRec allocated in the rehashValueMap. The
      rehashValueMap is a map where the key is one of the enumerated types 
      textDescriptor_enum_t, with the value being of the type LayoutTextDescriptorRec.

      This way, insteed of accessing directly the pointer fields in the LayoutCoreRec,
      we use our set/get methods on the rehashValueMap. Since the various LayoutTextDescriptor
      fields are only pointers, we thus ensure exectly the same values are accessible to
      external applications which use the PLS APIs m_getvalues_layout() or m_setvalues_layout().

      We keep too (almost) identical layout value maps:
      \begin{itemize}
      \item rehashValueMap - the LayoutTextDescriptorRec items in this map are "linked" to
            the appropriate fields in the layout_obj.
      \item defaultValueMap - this map's LayoutTextDescriptorRec item values never change. 
            They are used to reset the appropriate values in the rehashValueMap to the engine's
	    internal default settings, when, for example, an m_setvalues_layout() was called 
	    externally and asked set the value for "orientation" to 0.
      \end{itemize}

      In addition to the rehash and default maps of LayoutTextDescriptorRec, we have to 
      also synchronize the rehashValueLocaleName, rehashValueShapeCharset, and 
      rehashValueShapeCharsetSize. Since these values are of a different type, we can not 
      use a common interface like for the LayoutTextDescriptor, rather we have to take
      care of these individually. However, these fields too follow a simillar convention - 
      for each such item we have both the rehash* and the default*. The rehash* and default* 
      values (both for the items managed by the maps, as well as those which don't) 3 way 
      synchronizing between the rehash* default* and layout_obj is done by a call to the
      method rehash(), which is called after each m_setvalues_layout().

      @see rehash
      @see initObj
      @see initDefaultValues
  */

  /** get value of a text descriptor.
      The text descriptors are defined in the PLS public header
      layout.h. All the fields our layout engine is interested in
      are inserted into a the rehashValueMap, with the appropriate
      pointers in the layout_obj's record set to point at
      the appropriate map entry.
      @param id - id of text_descriptor_val_t to fetch
      @param side we are interested in - either INP or OUT
  */
  const text_descriptor_val_t getDescriptor(textDescriptor_enum_t id,
					     textDescriptor_side_t side) const;
  /// interface to m_getvalues_layout
  inline int getValuesLayout( LayoutValues values, int *index_returned);
  /// interface to m_setvalues_layout
  inline int setValuesLayout( LayoutValues values, int *index_returned);
  /// routine to check for valid input query values
  inline int foundInvalidValue(LayoutValues values, int *index_returned);
  /// interface to m_create_layout
  LayoutObject createLayout(LayoutValues layout_values);
  /** interface functionfor m_tranform_layout
   */
  int transform(const unsigned char *InpBuf, 
		size_t InpSize, void *OutBuf, 
		size_t *OutSize, size_t *InpToOut, 
		size_t *OutToInp, 
		unsigned char *Property, 
		size_t *InpBufIndex);
  /** interface functionfor m_wtranform_layout
   */
  int wtransform(const wchar_t *InpBuf, 
		 size_t InpSize, wchar_t *OutBuf, 
		 size_t *OutSize, size_t *InpToOut, 
		 size_t *OutToInp, 
		 unsigned char *Property, 
		 size_t *InpBufIndex);
protected:
  /// enumeration of possible output buffer types
  enum outbufftype_t {ISCHARPTR, ISWCHARPTR, ISUSHORTPTR};
  /// get the locale name string pointer
  inline const char *getLocaleName() const {
    return rehashValueLocaleName;
    //return  (rehashObjValue.locale_name)
  }
  /// routine for setting the locale name variable
  inline void setLocaleName(const char* name) {
    if (NULL == name) {
      return;
    }
   if (NULL != rehashValueLocaleName) {
     delete [] rehashValueLocaleName;
   }
   rehashValueLocaleName = new char[strlen(name) + 1];
    strcpy(rehashValueLocaleName, name);
  }
  /// routine for setting shape_charset
  
  inline void setCharSet(const char* name) {
    if (NULL == name) {
      name= defaultShapeCharset;
    }
    if (NULL != rehashValueShapeCharset) {
      delete [] rehashValueShapeCharset;
    }
    rehashValueShapeCharset = new char[strlen(name) + 1];
    strcpy(rehashValueShapeCharset, name);
    rehashObjValue->shape_charset_size= (strcmp(name,"iso8859-6")==0)? 2: 
      defaultObjValue->shape_charset_size;
    layout_obj->core.shape_charset_size=rehashObjValue->shape_charset_size;
  }
  inline const char *getCharSet() const {
    return rehashValueShapeCharset;
  }
  /** set pointer fields in the layout_obj to the rehash* values.
    @see Rehashing
    */
  inline void initObj(const char *locale_name);
  /** initialize initial layout values.
    This routine can be overloaded by castomized LOcontrol classes
    to modify some of the values
    */
  inline void initDefaultValues() {
    // initialize the default map - fall-back values
    defaultObjValue->type_of_text = new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    defaultObjValue->type_of_text->inp = TXT_IMPLICIT;
    defaultObjValue->type_of_text->out = TXT_VISUAL;
    defaultObjValue->numerals= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    defaultObjValue->numerals->inp = NUM_NOMINAL;
    defaultObjValue->numerals->out = NUM_NOMINAL;
    defaultObjValue->text_shaping= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    defaultObjValue->text_shaping->inp = SHAPE_INDEP;
    defaultObjValue->text_shaping->out = SHAPE_INDEP;
    defaultObjValue->orientation= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    defaultObjValue->orientation->inp = ORIENT_LTR;
    defaultObjValue->orientation->out = ORIENT_LTR;
    defaultObjValue->swapping= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    defaultObjValue->swapping->inp = SWAP_NO;
    defaultObjValue->swapping->out = SWAP_YES;
    defaultObjValue->implicit_alg= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    defaultObjValue->implicit_alg->inp = ALG_IMPLICIT;
    defaultObjValue->implicit_alg->out = ALG_IMPLICIT;
    defaultObjValue->context= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    defaultObjValue->context->inp = CONTXT_LTR;
    defaultObjValue->context->out = CONTXT_LTR;
    defaultObjValue->shape_charset_size = 1;
    defaultObjValue->shape_context_size= new LayoutEditSizeRec[sizeof(LayoutEditSizeRec)+1];
    defaultObjValue->shape_context_size->front=0;
    defaultObjValue->shape_context_size->back=0;   
    //initialize the line
    line = new LOline();
  }
  /** initialize all the rehash* values.
      The rehash* maps/values are initialized from their
      default* counterparts.
      @see Rehashing
  */
  inline void initValues() {
    rehashObjValue->orientation= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    rehashObjValue->orientation->inp = defaultObjValue->orientation->inp;
    rehashObjValue->orientation->out = defaultObjValue->orientation->out;
    rehashObjValue->type_of_text = new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    rehashObjValue->type_of_text->inp = defaultObjValue->type_of_text->inp;
    rehashObjValue->type_of_text->out = defaultObjValue->type_of_text->out;
    rehashObjValue->implicit_alg= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    rehashObjValue->implicit_alg->inp = defaultObjValue->implicit_alg->inp;
    rehashObjValue->implicit_alg->out = defaultObjValue->implicit_alg->out;
    rehashObjValue->swapping= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    rehashObjValue->swapping->inp = defaultObjValue->swapping->inp;
    rehashObjValue->swapping->out = defaultObjValue->swapping->out;
    rehashObjValue->numerals= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    rehashObjValue->numerals->inp = defaultObjValue->numerals->inp;
    rehashObjValue->numerals->out = defaultObjValue->numerals->out;
    rehashObjValue->text_shaping= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    rehashObjValue->text_shaping->inp = defaultObjValue->text_shaping->inp;
    rehashObjValue->text_shaping->out = defaultObjValue->text_shaping->out;
    rehashObjValue->shape_context_size= new LayoutEditSizeRec[sizeof(LayoutEditSizeRec)+1];
    rehashObjValue->shape_context_size->front = defaultObjValue->shape_context_size->front;
    rehashObjValue->shape_context_size->back = defaultObjValue->shape_context_size->back;
    rehashObjValue->context= new LayoutTextDescriptorRec[sizeof(LayoutTextDescriptorRec)+1];
    rehashObjValue->context->inp = defaultObjValue->context->inp;
    rehashObjValue->context->out = defaultObjValue->context->out;
    rehashObjValue->shape_charset_size = defaultObjValue->shape_charset_size;
    setCharSet(defaultShapeCharset);
  }
  /** synchronize control data with layoutObj.
    This function should be called after layoutCreate or layoutSetValues
    @see Rehashing
    */
  inline void rehash() {
    if(!rehashObjValue->orientation->inp)
      rehashObjValue->orientation->inp = defaultObjValue->orientation->inp;
    if(!rehashObjValue->orientation->out)
      rehashObjValue->orientation->out = defaultObjValue->orientation->out;
    if(!rehashObjValue->type_of_text->inp)
      rehashObjValue->type_of_text->inp = defaultObjValue->type_of_text->inp;
    if(!rehashObjValue->type_of_text->out)
      rehashObjValue->type_of_text->out = defaultObjValue->type_of_text->out;
    if(!rehashObjValue->implicit_alg->inp)
      rehashObjValue->implicit_alg->inp = defaultObjValue->implicit_alg->inp;
    if(!rehashObjValue->implicit_alg->out)
      rehashObjValue->implicit_alg->out = defaultObjValue->implicit_alg->out;
    if(!rehashObjValue->swapping->inp)
      rehashObjValue->swapping->inp = defaultObjValue->swapping->inp;
    if(!rehashObjValue->swapping->out)
      rehashObjValue->swapping->out = defaultObjValue->swapping->out;
    if(!rehashObjValue->numerals->inp)
      rehashObjValue->numerals->inp = defaultObjValue->numerals->inp;
    if(!rehashObjValue->numerals->out)
      rehashObjValue->numerals->out = defaultObjValue->numerals->out;
    if(!rehashObjValue->text_shaping->inp)
      rehashObjValue->text_shaping->inp = defaultObjValue->text_shaping->inp;
    if(!rehashObjValue->text_shaping->out)
      rehashObjValue->text_shaping->out = defaultObjValue->text_shaping->out;
    if(!rehashObjValue->shape_context_size->front)
      rehashObjValue->shape_context_size->front = defaultObjValue->shape_context_size->front;
    if(!rehashObjValue->shape_context_size->back)
      rehashObjValue->shape_context_size->back = defaultObjValue->shape_context_size->back;
    if(!rehashObjValue->context->inp)
      rehashObjValue->context->inp = defaultObjValue->context->inp;
    if(!rehashObjValue->context->out)
      rehashObjValue->context->out = defaultObjValue->context->out;
    if(!rehashObjValue->shape_charset_size)
      rehashObjValue->shape_charset_size = defaultObjValue->shape_charset_size;
    layout_obj->core.shape_charset = (char*)getCharSet();    
#if (defined (DEBUG) && defined (PRINT))
    print();
#endif
  }
  /** The function called from transform and wtransform.
      processes the transformation according to the required
      output buffer type
  */
  int transformImplement(const void *InpBuf, 
			 size_t InpSize, void *OutBuf, 
			 size_t *OutSize, size_t *InpToOut, 
			 size_t *OutToInp, 
			 unsigned char *Property, 
			 size_t *InpBufIndex,
			 outbufftype_t bufType);
  /** The function called from transform and wtransform.
      processes no transformation happen but it filles the out buffer with the input values
      according to the required output buffer type
  */
  int asciitransformImplement(const void *InpBuf, 
			      size_t InpSize, void *OutBuf, 
			      size_t *OutSize, size_t *InpToOut, 
			      size_t *OutToInp, 
			      unsigned char *Property, 
			      size_t *InpBufIndex,
			      outbufftype_t bufType);
  /// copies transfom result from line to OutBuf
  inline void copyOutBuff(void *OutBuf, size_t *OutSize, size_t *InpBufIndex, outbufftype_t bufType,int inp_buf_index );
  /// fills the OutToInp buffer with line's transform result
  inline void copyOutToInp(size_t *OutToInp,outbufftype_t bufType, int inp_buf_index);
  /// fills the InpToOut buffer with line's transform result
  inline void copyInpToOut(size_t *InpToOut, outbufftype_t bufType, int inp_buf_index);
  /// fills the Property buffer with line's transform result
  inline void copyProperies(unsigned char *Property,outbufftype_t bufType, int inp_buf_index);
  /// copies input without trasformation to OutBuf
  inline void asciicopyOutBuff(void *OutBuf, size_t *OutSize,const void *InpBuf, size_t InpSize,
			       size_t *InpBufIndex, outbufftype_t bufType,int inp_buf_index );
  /// fills the OutToInp buffer with line's input
  inline void asciicopyOutToInp(size_t *OutToInp,outbufftype_t bufType, size_t inpSize, int inp_buf_index);
  /// fills the InpToOut buffer with line's input
  inline void asciicopyInpToOut(size_t *InpToOut, outbufftype_t bufType, size_t inpSize, int inp_buf_index);
  /// fills the Property buffer with line's input
  inline void asciicopyProperies(unsigned char *Property,outbufftype_t bufType, size_t inpSize, int inp_buf_index);
  /** the "real" transform routine.
      The line must be set first by using line.setLin() in calling routine.
   */
  inline BOOL transformLine();
#if (defined (DEBUG) && defined (PRINT))
  // map for printing values - maybe can be used not only for debugging?
  /** Debug variables and routines.
      (if compiled with DEBUG flag)
   */
  //@{
  /// map of pointers to layout variables and name string (for print())
  //  map<textDescriptor_enum_t, const char *, less<textDescriptor_enum_t> > valMap;
  /// map of layout constant values and name string (for print())
  //  map<int, const char *, less<int> > nameMap;
  /// initialize name maps
  inline void initValMap();
  /// flag
  BOOL valueMapInitialized;
  /// print layout variables values
  inline void print();			// print layout object status
  //@}
#endif
  /** Pointer to PLS layout object.
      As a matter of fact, a pointer to instance of this class
      is copied to layout_obj.core.private_data, therefore
      it should never be deleted or freed during lifetime of this
      instance
  */
  LayoutObj layout_obj;
  /** The layout types (out of the many) we care about.
      @name Important Layout Types
  */
  //@{
  /// name of charset to be used for shaping
  char *rehashValueShapeCharset;
  /// name of charset to be used for shaping
  char *defaultShapeCharset;
  ///
  char *rehashValueLocaleName;
  ///
  LayoutCore defaultObjValue, rehashObjValue;
  //@}
  /// line slave to use for layout operations
  LOline *line;
};

/** Hebrew castomized version of LOcontrol.
    In this class some of the default values are modified.
    everything else is handled by the parent class - LOcontrol.
 */
class LOheControl : public LOcontrol {
public:
  /** Hebrew LOheControl constructor.
      In the constructor, default values (either
      in the defaultValueMap or other "loose" default*
      values have to be override (if needed) BEFORE
      the calls to initValues() and initObj() are made.
  */
  LOheControl(LayoutObj obj, const char* locale_name) : 
    LOcontrol(obj, locale_name) {
      // override some inherited default values
      defaultShapeCharset = (char *)"iso8859-8";// set default charset to that of Hebrew
      initValues();
      initObj(locale_name);
  }
};

/** Arabic castomized version of LOcontrol.
    In this class some of the default values are modified.
    everything else is handled by the parent class - LOcontrol.
 */
class LOarControl : public LOcontrol {
public:
  /** Arabic LOheControl constructor.
      In the constructor, default values (either
      in the defaultValueMap or other "loose" default*
      values have to be override (if needed) BEFORE
      the calls to initValues() and initObj() are made.
  */
  LOarControl(LayoutObj obj, const char* locale_name) : 
    LOcontrol(obj, locale_name) {
     // override some inherited default values
      defaultShapeCharset = (char *)"iso8859-6=E6;";// set default charset to that of Arabic
      defaultObjValue->text_shaping->out = SHAPE_SHAPED;
      defaultObjValue->numerals->out = NUM_CONTEXTUAL;
      defaultObjValue->shape_charset_size = 4;
      initValues();
      initObj(locale_name);
  }
};

#endif // LOCONTROL_H








