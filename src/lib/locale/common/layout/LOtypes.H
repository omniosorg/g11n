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
#pragma ident "@(#)LOtypes.H	1.11     99/11/19  SMI%"

#ifndef LOTYPES_H
#define LOTYPES_H

#include <stdlib.h>
#include "define_bad_alloc.h"
#include "LOdefs.H"

/** Per character data map management class.
    It is responsible for initializing, managing and
    providing access methods for retreive character attribute,
    shaping and combining values, etc.
*/
class LOtypes : public LOdefs {
public:
  friend class LOcharSeg;
  /** character attribute data type.
  */
  typedef unsigned long lotype_t;
  /** Initialization Data types.
      These types are used for static initialization of
      the various STL maps.
      @name data initialization structs
  */
  //@{
  /** value type for shaping map.
      begine, middle, end and isolated are the glyph value 
      for that form of the nominal input character.
  */
  struct shapeData_t {
    val_t begin;
    val_t middle;
    val_t end;
    val_t isolated;
  };
  //@}
  /// Constructor
  LOtypes() {}

protected:
  /// enumeration type of per-character attribute flags
  enum lo_ctypeflags_t {
  _LTR      = (1L << 1),	/* strong LTR character */
  _RTL      = (1L << 2),	/* strong RTL character */
  _END	    = (1L << 3),	/* end neutral (takes previous char embedding level) */
  _NUM      = (1L << 4),	/* digit */
  _VOC      = (1L << 5),	/* Vocalization Mark */
  _SEP      = (1L << 6),	/* Numeric Separator */
  _BGN      = (1L << 7),	/* begin neutral (takes next embedding level) */
  _AS1      = (1L << 8),	/* Shape1, always independant */
  _AS2      = (1L << 9),	/* Shape2: independant, end */
  _AS4      = (1L << 10),	/* Shape4: independant, begin, middle, end */
  _ACB      = (1L << 11),	/* may Combine with next char */
  _SWAP     = (1L << 12),	/* mirrored character */
  _NEND     = (1L << 13),	/* number (weak type) terminator */
  _CELL     = (1L << 14),       /* New cell indicator */
  _CTRL     = (1L << 15)        /* Data controls*/
  };
  /** Static value maps low level access routines.
      @name Maps Access  Routines
  */
  //@{

 /// returns the index position of the character in an array
  inline const int binSearch(int val, int start, int end, const int* array)const;

  /** character attribute map access routine.
      @param c search key
      @return XORed attribute flags for c, or 0 if not found
  */
  const lotype_t &charmapGet(const val_t &c) const;
  /** swap map access routine.
      @param c search key
      @return mirrored value of c, if found, or c
  */
  const val_t &swapmapGet(const val_t &c) const;
  /** number shape map access routine.
      @param c search key
      @return Indic value for the digit c, if found, or c
  */
  const val_t &nummapGet(const val_t &c) const;
  /** shape map access routine.
      If the search key c is nor found in the map,
      the returned value fields begin, middle, end, etc. 
      are guaranteed to all be initialized to the 
      same value of input character c.
      @param c search key (chracter value)
      @return shapeData_t value for this character.
  */
  shapeData_t shapemapGet(const val_t &c) const;
  /** combo map access routine.
      The map itself contains an unsinged integer as the key
      and val_t is the value mapped to the key. In order to do
      a search, the values of the base character (c1) and next
      character (c2) have to be joined to form an unsigned integer 
      value which is than used as the search key.
      @param c1 base combo character
      @param c2 next combo character
      @return new combo glyph value or 0 if key not found
  */
  const val_t &combomapGet(const val_t &c1, const val_t &c2) const;
  //@}
private:
  /** Static arrays of map initialization data
      @name static initialization data
  */
  //@{
  static const int charDataIndex[];
  static const lotype_t charData[];
  ///
  static const int shapeDataIndex[];
  static const shapeData_t shapeData[];
  ///
  static const int comboDataIndex[];
  static const val_t comboData[];
  ///
  static const int swapDataIndex[];
  static const val_t swapData[];
  ///
  static const val_t numData[]; 
 //@}
};


#endif /* LOTYPES_H */
