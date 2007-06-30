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
#pragma ident "@(#)LOchar.C	1.11     99/11/19  SMI%"

#include "LOchar.H"
#include "LOexception.H"
/* fix shape and fixcombo have been moved into LOseg.C

void LOchar::fixshape(const CharPtrArray &line,
			     CharPtrArray::const_iterator cur,
			     CharPtrArray::const_iterator prev,
			     CharPtrArray::const_iterator next,
			     text_descriptor_val_t shapeCase)
{

// unless shapeCase is SHAPE_SHAPED, shaping is not context
// sensitive and the prev, next characters are ignored
//
  switch (shapeCase) {
  case SHAPE_BEGIN:
    (*cur)->lo_tobegin();
    break;
  case SHAPE_MIDDLE:
    (*cur)->lo_tomiddle();
    break;
  case SHAPE_END:
    (*cur)->lo_toend();
    break;
  case SHAPE_INDEP:
    (*cur)->lo_toisolated();
    break;
  case SHAPE_SHAPED:
    {
      LOchar dummyChar(0);	// used if there is no prev or next char
      LOchar *prevCharPtr = &dummyChar;
      LOchar *nextCharPtr = &dummyChar;

      // find the previous first non-diacretic character
      if (cur != line.begin()) {
	for (; prev != line.begin() && (*prev)->lo_isavocal(); prev--);
	prevCharPtr = *prev;
      }
      // find the next first non-diacretic character
      for (; next != line.end() && (*next)->lo_isavocal(); next++);
      nextCharPtr = (line.end() == next ? &dummyChar : *next); // danger if passed end()

      if (nextCharPtr->lo_isashape4() || nextCharPtr->lo_isashape2()) {
	if (prevCharPtr->lo_isashape4()) {
	  (*cur)->lo_tomiddle();
      } else {
	(*cur)->lo_tobegin();
      }
      } else if (prevCharPtr->lo_isashape4()) {
	(*cur)->lo_toend();
      } else {
	(*cur)->lo_toisolated();
      }
    }
    break;
  default:
#ifdef DEBUG
    // HDIGH is short for "How (the hell) Did I Get Here!"
    {
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
    }
#endif
    break;
  }
}
*/
/*
LOchar::CharPtrArray::iterator 
LOchar::fixcombo(const CharPtrArray &line,
		 CharPtrArray::iterator cur)
  
{
  // if next is end of input, nothing to combine to
  CharPtrArray::iterator next;
  // this loop continues as long as curent character combines with
  //he next character
  //
  for (next = cur + 1;
       next != line.end();
       next++) {
    val_t combo = (*cur)->combomapGet((*cur)->getval(), (*next)->getval());
    if (0 == combo) {
      break;		// this value and next value not in combo map
    }
    (*cur)->setval(combo);			// set val to new ISOLATED form of combo
    (*next)->setval(0);		// "mark" next for deletion from sortout    
    (*next)->getup()->setcellIndicator(0);       // not a new cell
    (*next)->getup()->setdown(*cur);	// reset next's parent down link to point at this
  }
  return next - 1;		// return pointer to next chracter to be processed
}
*/
