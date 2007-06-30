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
#pragma ident "@(#)LOseg.C	1.11     99/11/19  SMI%"

#include "LOseg.H"

const LOchar::CharPtrArray &
LOcharSeg::flatten() 
{
  /* this class is a decendant of LOchar::CharPtrArray
   * therefore simply return a reference to itself
   */
  return  charsegPtr;
}
void LOcharSeg::fixdir()
{
  // reverse characters if this is an Arabic/Hebrew segment
  if (RTL == getdir()) {
    reverse();
  }
}


/*--------------------------------------------------*/

const LOchar::CharPtrArray &
LOsegSeg::flatten() 
{
  int cursegsize=0;
  int curflattensize=0;
  /* "flatten" each contained segment and append result to 
     lochar member fixshape
  */
  for ( int k=0 ; k < segsegPtrSize; k++){ 
    LOchar::CharPtrArray v = segsegPtr[k]->flatten();
    cursegsize=segsegPtr[k]->getflattensize();
    for (int i=0; i < cursegsize;i++)
      lochars[i+curflattensize]=v[i];
    curflattensize+=cursegsize;
  }
  flattenSize+=curflattensize;
  return lochars;
}

void LOsegSeg::fixdir()
{
  // reverse order of contained segments if this is Arabic/Hebrew segment
  if (RTL == getdir()) {
    reverse();
  }
  // call the fixdir() method for each contained segment
  for (int k= 0 ; k < segsegPtrSize ;k++){
    segsegPtr[k]->fixdir();
  }
}
void LOsegSeg::fixshape(text_descriptor_val_t sc)
{
  // call the fixshape() method for each contained segment
  for (int k= 0 ; k < segsegPtrSize ;k++){
    if (RTL == (segsegPtr[k])->getdir())
      segsegPtr[k]->fixshape(sc);
  }
}
void LOcharSeg::fixshape(text_descriptor_val_t shapeCase)
{
  // shaping is done on character in input order
      LOchar dummyChar(0);	// used if there is no prev or next char
      LOchar* nextCharPtr = &dummyChar;
      int  prevShape = LOtypes::_AS1;
      int j= charsegPtrSize;
      for (int i= 0 ; i < j ;i++){
	switch (shapeCase) {
	case SHAPE_BEGIN:
	  charsegPtr[i]->lo_tobegin();
	  break;
	case SHAPE_MIDDLE:
	  charsegPtr[i]->lo_tomiddle();
	  break;
	case SHAPE_END:
	  charsegPtr[i]->lo_toend();
	  break;
	case SHAPE_INDEP:
	  charsegPtr[i]->lo_toisolated();
	  break;
	case SHAPE_SHAPED:
	  {	      
	    int next = i + 1;
	    LOchar* cur = charsegPtr[i]; 
	    for (next = i + 1; next !=j && charsegPtr[next]->lo_isavocal(); next++);
	      nextCharPtr = (j == next) ? &dummyChar : (charsegPtr[next]);
	    if (nextCharPtr->lo_isashape4() || nextCharPtr->lo_isashape2()) {
	      if (prevShape == LOtypes::_AS4)
		cur->lo_tomiddle();
	      else
		cur->lo_tobegin();
	    }
	    else if (prevShape == LOtypes::_AS4)
	      cur->lo_toend();
	    else
	      cur->lo_toisolated();	    
	    prevShape = (cur->lo_isavocal())? prevShape: cur->lo_shape();
	    break;
	  }
	}
      }
}

void LOsegSeg::fixswap()
{
  // call fixswap() method for each contained segment
  for (int k= 0 ; k <  segsegPtrSize ;k++){
    segsegPtr[k]->fixswap();
  }
}
void LOcharSeg::fixswap()
{
  // swapping for mirrored characters is only done in RTL segments
  if (RTL != getdir()) {
    return;			// nothing to do
  }
  for (int k= 0 ; k < charsegPtrSize ;k++){
    charsegPtr[k]->fixswap();
  }
}
void LOsegSeg::fixcombo()
{
  // call fixcombo() method for each contained segment
  for (int k= 0 ; k < segsegPtrSize ;k++){
    segsegPtr[k]->fixcombo();
  }
}
void LOsegSeg::fixnumerals(direction_t inDir,
			   BOOL nationalMode)
{
  // call fixnumerals() method for each contained segment
  for (int k= 0 ; k < segsegPtrSize ;k++){
    segsegPtr[k]->fixnumerals(getdir(), nationalMode);
  }
}

void LOcharSeg::fixnumerals(direction_t inDir,
			    BOOL nationalMode)
{
  /* call the fixnumerals() method in each containd LOchar, 
   * but only if this is a NUM segment. Check the value of 
   * the nationalMode flag - if it is 'true', conversion
   * to nationalMode numbers is forced regardless of
   * the direction tag of the previous segment.
   */
  if (getdir() == NUM && (inDir == RTL || nationalMode)) {
  for (int k= 0 ; k < charsegPtrSize ;k++){
    charsegPtr[k]->fixnumerals();
  }
}
}
void LOcharSeg::fixcombo(){
  // copy chars to arr, because we will need to erase combined chars
  // combo is done on characters in input order (cur and next)
  int j = charsegPtrSize;
  int combo = 0;
  int segptrshift=0;
  for (int i = 0; i < (j-segptrshift); i++) {
    charsegPtr[i]=charsegPtr[i+segptrshift];
    LOchar* cur = charsegPtr[i];
    if(i < (j - (segptrshift+1))){	       
      combo = cur->combomapGet(cur->getval(), charsegPtr[i+1+segptrshift]->getval());
      if(combo != 0){		
	cur->setval(combo);
	(charsegPtr[i+1+segptrshift])->setval(0);
	(charsegPtr[i+1+segptrshift])->getup()->setdown(cur);
	segptrshift++;
	charsegPtrSize--;
	
      }
    }
  }
}
