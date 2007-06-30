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
#pragma ident "@(#)LOdefs.H	1.11     99/11/19  SMI%"

#ifndef LODEF_H
#define LODEF_H

#include "layout.h"
#include "layout_int.h"
#ifndef bool_def
typedef int BOOL;
#define true 1
#define false 0
#define bool_def
#endif


/** a namespace for commonly used types.
    All classes are decendants of this class, so 
    they all share the same types. This class would
    probably be unnecessary if "namespace" was
    supported by both CC and g++.
*/
class LOdefs {
public:
  /// value type to use for internal representation of characters
  //typedef unsigned short val_t;
  typedef int val_t;
  /** enumeration of possible layout text descriptor values.
      The enumerated values receive exactly the same 
      values from the definitions in the layout.h PLS
      library header. The enum name is slightly modified 
      as not to clash with the def. The reason why enumerate in
      the first place is because it is cleaner, usefull during
      debugging and in switch statements (gcc, for example, warn you if
      you have not used all the enumerated types)
  */
  enum  text_descriptor_val_t {
    NUM_NOMINAL = NUMERALS_NOMINAL, 
    NUM_NATIONAL = NUMERALS_NATIONAL,
    NUM_CONTEXTUAL = NUMERALS_CONTEXTUAL,
    SHAPE_SHAPED = TEXT_SHAPED,
    SHAPE_BEGIN = TEXT_SHFORM1, 
    SHAPE_MIDDLE = TEXT_SHFORM2, 
    SHAPE_END = TEXT_SHFORM3, 
    SHAPE_INDEP = TEXT_SHFORM4,
    TXT_IMPLICIT = TEXT_IMPLICIT, 
    TXT_EXPLICIT = TEXT_EXPLICIT,
    TXT_VISUAL = TEXT_VISUAL,
    SWAP_YES = SWAPPING_YES, 
    SWAP_NO = SWAPPING_NO,
    ORIENT_LTR = ORIENTATION_LTR,
    ORIENT_RTL = ORIENTATION_RTL,
    ORIENT_CONTEXTUAL = ORIENTATION_CONTEXTUAL,
    ALG_IMPLICIT = ALGOR_IMPLICIT,
    ALG_BASIC = ALGOR_BASIC,
    CONTXT_LTR = CONTEXT_LTR,
    CONTXT_RTL = CONTEXT_RTL
  };
  /** enumeration of textDescriptor side.
      This flag is used in LOcontrol for getting
      PLS setting values/modifiers.
  */
  enum textDescriptor_side_t {
    INP,
    OUT
  };
  /// enumeration of text descriptor types
  enum textDescriptor_enum_t {
    orientation, 
    context,     
    type_of_text,
    implicit_alg,
    swapping,    
    numerals,    
    text_shaping
  };
  /// enumaration of resolved direction types
  enum direction_t {NONE, LTR, RTL, NUM};
};

#endif // LODEF_H
