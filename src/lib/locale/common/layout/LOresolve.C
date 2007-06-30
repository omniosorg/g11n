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
#pragma ident "@(#)LOresolve.C	1.11     99/11/19  SMI%"

#include "LOresolve.H"

int
LOresolve::flatten() 
{
  if (!topSegPtr->getsegsize()) {
    return 0;			// nothing to do
  }
  const LOchar::CharPtrArray v = topSegPtr->flatten();
  bzero(sortoutPtr,sizeof(sortoutPtr));
  sortoutPtrSize=topSegPtr->getflattensize();
  memcpy(sortoutPtr,v, (sizeof(LOchar*)*sortoutPtrSize));
return sortoutPtrSize;
}

int 
LOresolve::embedOne(LOchar::CharPtrArray &line, int k) 
{

  if (sortoutPtrSize == k) {
    return k;			// end of input
  }
  direction_t dir = line[k]->getdir();
  // contain new char segment in current segment
  if (dir == curSegPtr->getdir() || NUM == dir) {
    return (appendToNewSegment(line, k));			// return next input character to process
  } 
  /*
   * either increment (pushSeg()) or decrement (popSeg()) embeding level
   */
  if (dir == defaultDir) {
    popSeg();
  } else {
    pushSeg(dir,k);
  }
  return k;
}
void LOresolve::fixdir()
{
  // fix direction of segments
  topSegPtr->fixdir(); // conveniently, loseg class does it recursively
  /* topSeg's direction type is LTR by default
   * reverse it if global direction is not LTR
   */
  if (LTR == topSegPtr->getdir() && RTL == defaultDir) {
    topSegPtr->reverse();
  }
}


BOOL LOresolve::resolve()
{
  BOOL doFixDir = false;	// flag to know either need to reverse RTL segments
  if (!sortoutPtrSize) {
    return true;		// nothing to do
  }
  // get text type PLS settings from LOcontrol
  text_descriptor_val_t outtextType = loControl.getDescriptor(type_of_text, OUT);
  text_descriptor_val_t intextType = loControl.getDescriptor(type_of_text, INP);
  /* unlese input type != output type, we have nothing to do.
   * also, unless output type is visual, again, nothing for us to do
   */
  if (intextType != outtextType &&
      intextType != LOcontrol::TXT_VISUAL) {

    /*
       * which bidi algorithm to use
       */
    switch (intextType) {
    case LOcontrol::TXT_VISUAL:
      // nothing to do
      break;
    case LOcontrol::TXT_EXPLICIT:
    case LOcontrol::TXT_IMPLICIT:
      {
	/* fetch PLS settings for text orientation type,
	 * to determine the default direction value
	 */
	text_descriptor_val_t outorientType = loControl.getDescriptor(orientation, OUT);
	switch (outorientType) {
	case LOcontrol::ORIENT_LTR:
	  defaultDir = LTR;
	  break;
	case LOcontrol::ORIENT_RTL:
	  defaultDir = RTL;
	  break;
	case LOcontrol::ORIENT_CONTEXTUAL:
	  /* set defaultDir from the type of first strong
	   * character.
	   */
	  defaultDir = loLine.findFirstStrongChar();
	  break;
	default:
#ifdef DEBUG
	  throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
	  break;
	}
	if (LOcontrol::TXT_EXPLICIT == outorientType) {
	  // use escape sequence tags for embeding
#ifdef notyet
	  embedExplicit();	// explicit embeding
#endif
	} else {
	  embedImplicit();	// implicit embedding
	  doFixDir = true;	// we need to call fixdir()
	}
      }
      break;
    default:
#ifdef DEBUG
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
      break;
    }
#ifdef ARABIC_TARGET
    /*
     * which shaping algorithm to us
     */
    text_descriptor_val_t inshapingType = loControl.getDescriptor(text_shaping, INP);
    text_descriptor_val_t outshapingType = loControl.getDescriptor(text_shaping, OUT);
    if (LOcontrol::SHAPE_SHAPED != inshapingType &&
	LOcontrol::SHAPE_INDEP != outshapingType) {
      // fixcombo() must come before fixshape()!
      fixcombo();
      fixshape();
    }
    /*
     * do we treat numerals or not
     */
    text_descriptor_val_t innumberType = loControl.getDescriptor(numerals, INP);
    text_descriptor_val_t outnumberType = loControl.getDescriptor(numerals, OUT);
    if (innumberType != outnumberType) {
      switch (outnumberType) {
      case LOcontrol::NUM_NATIONAL:
	fixnumerals(true);
	break;
      case LOcontrol::NUM_CONTEXTUAL:
	fixnumerals();
	break;
      case LOcontrol::NUM_NOMINAL:
	break;			// don't know what to do yet
      default:
#ifdef DEBUG
	throw LOexception("HDIGH:",__FILE__ , __LINE__);
#endif
	break;
      }
    }
#endif
    /*
     * do we fix mirrored characters?
     */
    text_descriptor_val_t inswappingType = loControl.getDescriptor(swapping, INP);
    text_descriptor_val_t outswappingType = loControl.getDescriptor(swapping, OUT);
    if (inswappingType != outswappingType &&
	outswappingType == LOcontrol::SWAP_YES) {
      fixswap();
    }
    if (doFixDir) {
      fixdir();
    }
    int finalsize= flatten();
    loLine.setsortoutsize(finalsize);			// create output in visual ordering
  }
  loLine.fixpos();		/* set the pos property for each 
				 * input, output character in LOline,
				 * this will later be used for the
				 * OutToInp, InpToOut arrays
				 */
  return true;
}
void 
LOresolve::fixshape() {
  // get the shaping directive from the LOcontrol object
  text_descriptor_val_t shapingType = loControl.getDescriptor(text_shaping, OUT);
  topSegPtr->fixshape(shapingType);
}

void
LOresolve::embedStackInit(direction_t dir) {
  segStack= new LOsegSeg*[sizeof(LOsegSeg*)*sortoutPtrSize+1 ];
  segStackPtr=0;

  switch (defaultDir) {
  case LTR:
    globalEmbedLevel = 0;	// LTR always even
    break;
  case RTL:
    globalEmbedLevel = 1;	// RTL always odd
    break;
  default:
#ifdef DEBUG
    {
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
    }
#endif
    break;
  }
  if (NULL != topSegPtr) {
    delete topSegPtr;
  }

  currentEmbedLevel = globalEmbedLevel;	// set initial embeding level
  topSegPtr = new LOtopSeg(defaultDir, currentEmbedLevel, sortoutPtrSize ); // create top segment
  curSegPtr = topSegPtr;	// set topSegPtr to be current "active" segment
}

void
LOresolve::pushSeg(direction_t dir, int k) {
  LOsegSeg *segPtr;
  // increment embeding level
  currentEmbedLevel++;
  /* create new segment of segments,
   * which is embeded at the currentEmbedLevel and
   * has the direction type dir
   */
  segPtr = new LOsegSeg(dir, currentEmbedLevel,sortoutPtrSize-k);
  // add the new segment to current segment
  curSegPtr->addseg(segPtr);
  // place newly create segment at top of segment stack
  segStack[++segStackPtr]=segPtr;
  /* the newly created segment now becomes the
   * current segment - any new LOcharSeg segment,
   * or new LOsegSeg segments will be added into this
   * segment, untill it is "poped" from stack.
   */
  curSegPtr = segPtr;
}
void
LOresolve::popSeg() {
  if (segStackPtr==0){
    return;			// already at bottom level
  }	
	// throw away already used embeded segment pointer
  segStackPtr--;
  currentEmbedLevel--;		// decrement embeding level
  if (segStackPtr==0){
    /* already at bottom embeding level, restore topSegPtr
     * as the current "active" segment for embeding
     */
    curSegPtr = topSegPtr;
  } else {
    // stack not empty - previous segment becoms "active" segment
    curSegPtr = segStack[segStackPtr];
  }
}
void
LOresolve::embedImplicit() {
  /*
   * initialize the embeding stack.
   * defaultDir is a value received from the LOcontrol reference,
   * if it is NONE, the call to the LOline:: resolveDirectionType() method
   * will return the direction type of the first STRONG character in the 
   * input line.
   */
  defaultDir = loLine.resolveDirectionType(defaultDir);

  // initialize the embedding data structure
  embedStackInit(defaultDir);

  // embed all input chracters, using service routine embedOne()
  int j = sortoutPtrSize;
  int k=0;
  while (j > k) k=(embedOne(sortoutPtr, k));
}
int
LOresolve::appendToNewSegment(LOchar::CharPtrArray &line, int k) {
  if (sortoutPtrSize == k) {
    return k;			// no more input
  }
  /* direction of first character will determine the
   * direction of the new LOcharSeg segment.
   */
  direction_t dir = line[k]->getdir(); 
  /* base embeding level is that of current segment,
   * to which we are going to append the new LOcharSeg segment.
   */
  int embeding = curSegPtr->getlevel();
  if (NUM == dir) {
    // NUM segment inside an RTL segment get level incremented by 1
    switch(curSegPtr->getdir()) {
    case RTL:
      embeding++;
      break;
    case LTR:
      /* numbers embeded in LTR segment get same embedding level
       * so nothing to do
       */
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
  // create a new segment of LOchar pointers
  LOcharSeg* segp = new LOcharSeg(dir, embeding, sortoutPtrSize - k) ;
  /* add new segment to current LOsegSeg at top
   * of stack.
   */
  curSegPtr->addseg(segp);
  /* add characters from input line, for
   * as long as the direction type is the same as
   * the new segment's direction type
   */
  for (; k !=sortoutPtrSize && dir == (line[k])->getdir(); k++) {
    segp->addchar(line[k]);
  }
return k;
}
