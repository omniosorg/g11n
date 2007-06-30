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

#ifndef CTLFUNCTIONS_H
#define CTLFUNCTIONS_H

#include <stdlib.h>
#include "layout.h"
#include "layout_int.h"

#ifdef ARABIC_TARGET
#define LOcontrolClass LOarControl
#else 
#define LOcontrolClass LOheControl
#endif

/** Functions directly interfaced with liblayout.
    These functions are wrappers with C linking - the
    real job is delegated to pear routines in the LOcontrol
    object attached to the private_data of the layout object 
    parameter
    @name PLS library interface functions
    @see LOcontrol
*/
extern "C" {
//@{
  /** initialize the layout object.
      @param locale_name name of current locale - e.g. "ar"
  */
  LayoutObject _LayoutObjectInit(char *locale_name);
  /** m_create_layout interface.
      @param layout_obj current LayoutObject
      @param layout_values LayoutValues to override default engine's layout settings
  */
  static LayoutObject CTL_createLayout(LayoutObj layout_obj, LayoutValues layout_values);
  /// 
  static int CTL_destroyLayout(LayoutObj layout_obj);
  /// return values of requested fields
  static int CTL_getValuesLayout(LayoutObj layout_obj, LayoutValues values, int *index_returned);
  /// set values of requested fields in layout object 
  static int CTL_setValuesLayout(LayoutObj layout_obj, LayoutValues values, int *index_returned);
  /** transform text for char or short input type.
      the InpBuf type, even though declared as char pointer, will
      be treated as short array If the shape_charset_size of current
      locale is 2 bytes  - i e use 16bit fonts
  */
  static int CTL_transformLayout(LayoutObj layout_obj, 
				 const char *InpBuf, size_t InpSize, 
				 void *OutBuf, 
				 size_t *OutSize, 
				 size_t *InpToOut, 
				 size_t *OutToInp, 
				 unsigned char *Property, 
				 size_t *InpBufIndex);
  /// transform text - input and output of type wchar_t*
  static int CTL_wtransformLayout(LayoutObject layout_obj, 
				   const wchar_t *InpBuf, 
				   size_t InpSize, 
				   void *OutBuf, 
				   size_t *OutSize, 
				   size_t *InpToOut, 
				   size_t *OutToInp, 
				   unsigned char *Property, 
				   size_t *InpBufIndex);
//@}
}

#endif // CTLFUNCTIONS_H
