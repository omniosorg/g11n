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
#pragma ident "@(#)LOseg.H	1.11     99/11/19  SMI%"

#ifndef LOSEG_H
#define LOSEG_H
#include "define_bad_alloc.h"
#include "LOchar.H"
#include <strings.h>

class LOchar;

/** A class to resolve bidi embeding.
    This is an abstract class for either a segment of LOchar::CharPtrArray,
    or a segment containing pointers to other segments. Many or the routines in
    this class are virtual, because the action which should be taken depands on the
    contained data type (i.e. CharPtrArray or SegPtrArray).
*/
class LOseg : public LOdefs {
public:
  /// 
  ///typedef vector<LOseg*> SegPtrArray;
  typedef LOseg**  SegPtrArray;
  /** The constructor
      @param in_dir direction type of this segment
      @param in_embedLevel embeding level of this segment
  */
  LOseg(const direction_t in_dir=NONE, 
	int in_embedLevel=0, int in_segSize=0): 
    dir(in_dir), 
    embedLevel(in_embedLevel), 
    segSize(in_segSize){}
  virtual ~LOseg() {}
  /// resolve embeded character shaping
  virtual void fixshape(text_descriptor_val_t sc)=0;
  ///resolve embeded character numerals
  virtual void fixnumerals(direction_t dir,
			   BOOL nationalMode)=0;
  /// resolve embeded character combinations
  virtual void fixcombo()=0;
  /// resolve embeded character mirroring
  virtual void fixswap()=0;
  /** recursivly resolve direction of all segments
   *
   *  Fix direction of segment (recursive)
   *  check The direction type of this segment
   *  2. do reverse() (either of losegs, or lochars data members), if RTL
   *  3. if segtype == SEGSEG, recursvely call fixdir() for each one
   *
   *  @see reverse
   */
  virtual void fixdir()=0;
  /// erase all members
  virtual void erase()=0;
  /** return a "flat" array of all lochars it, 
      or its subsegment contain (recursively)
  */
  virtual const LOchar::CharPtrArray &flatten()=0;
  /// reverses all items in array
  virtual void reverse()=0;
  /// get direction type of this segment
  inline direction_t getdir() const {return dir;}
  /// get embeding level of this segment
  inline int getlevel() const {return embedLevel;}
  /// get the real size
  virtual int getflattensize()=0;

protected:
  /// direction type of this segment
  direction_t dir; 
  /// embeding level of this segment
  int embedLevel;
  /// segment size
  int segSize;
};

/// a class of LOseg to hold an array of LOchar*
//class LOcharSeg : public LOseg, public vector<LOchar*> {
class LOcharSeg : public LOseg {
public:
  /** The constructor
      @param in_dir direction type of this segment
      @param in_embedLevel embeding level of this segment
  */
  LOcharSeg(const direction_t in_dir=NONE, 
	    int in_embedLevel=0 , int in_segSize=0) :
    LOseg(in_dir, in_embedLevel, in_segSize){initCharSeg();}
  /// adds a new LOchar pointer to array
  inline void addchar(LOchar *ot) {
    // add to array
    charsegPtr[charsegPtrSize++]= ot;
    // set the upchar embedding to same as this segment
    ot->getup()->setproperty(getlevel());
  }
  /** calls fixshape() foreach containt LOchar object
   * @param sc shaping directives to be used
   */
  void fixshape(text_descriptor_val_t sc);
  /// calls fixcombo() foreach containt LOchar object
  void fixcombo();
  /** calls fixnumerals() foreach containt character.
   * This function will do nothing unless the direction type
   * for this segment instance is NUM, in which case, it will iterate
   * on each character and call its fixnumerals() method.
   * the parameter dir gives the direction of the PREVIOUS segment.
   * the direction tag of the previous segment is needed as to 
   * decide either numbers should be transformed to Indic or not,
   * when the PLS or default layout settings are numbers=contextual.
   * in such a case, numbers will only be "fixed" to Indic digits IF the 
   * previous segment is Arabic (i.e. dir == RTL).
   * 
   * @param dir the direction tag of the PREVIOUS segment
   * @param nationalMode a flag either Indic transformation should be forced
   */
  void fixnumerals(direction_t dir,
		   BOOL nationalMode);
  /// calls fixswap() foreach containt LOchar object
  void fixswap();
  /// erase items from data array
  inline void erase() {
    if (NULL !=charsegPtr)
      delete [] charsegPtr;
    charsegPtrSize=0;
  }
  /** Reverse the characters in array, if needed.
      If this segment is a segment of Hebrew/Arabic characters
      (i.e. segment direction tag is RTL), all the items in
      the segments are reversed, as to produce the visual RTL effect.
   */
  void fixdir();
  /** returns an array of LOchar pointers.
      It simply returns itself.
  */
  inline const LOchar::CharPtrArray &flatten();
  /// get the size of the charseg array
  inline int getcharsegptr() {return charsegPtrSize;}
  inline int getsegsize() {return charsegPtrSize;}
  inline int getflattensize() {return charsegPtrSize;}
  inline void initCharSeg(){
    charsegPtrSize=0;
    charsegPtr= new LOchar*[(sizeof(LOchar*)*segSize)+1];
  }
protected:
  /// reverses all items in data ar
  inline void reverse() {
    //::reverse(begin(),end());
    int i,j;
    LOchar* tmp;
    for( i = 0, j =charsegPtrSize-1; i<j; i++, j--){
      tmp = charsegPtr[i];
      charsegPtr[i] = charsegPtr[j];
      charsegPtr[j] = tmp;
    }
  }
  ///charsegment pointer
  LOchar::CharPtrArray charsegPtr;
  /// char segment size
  int charsegPtrSize;
};

/** A class able to recursively contain its own type
    This class is used for resolving embeding levels, but
    Also to segment together segments of similar character types,
    for example - NUM, RTL, LTR etc
*/
//class LOsegSeg :  public LOseg, public vector<LOseg*>  {
class LOsegSeg :  public LOseg {
public:

  /** The constructor
      @param in_dir direction type of this segment
      @param in_embedLevel embeding level of this segment
  */
  LOsegSeg(const direction_t in_dir=NONE, 
	   int in_embedLevel=0, int in_segSize=0) :
    LOseg(in_dir, in_embedLevel, in_segSize) {initSegSeg();}
  inline void initSegSeg(){
    flattenSize=0;
    segsegPtrSize=0;
    segsegPtr= new LOseg*[(sizeof(LOseg*)*segSize)+1];
    lochars = new LOchar*[(sizeof(LOchar*)*segSize)+1];
  }
  /// calls fixshape() foreach containt LOseg object
  void fixshape(text_descriptor_val_t sc);
  /// calls fixcombo() foreach containt LOseg object
  void fixcombo();
  ///calls fixnumerals() foreach containt LOseg object
  void fixnumerals(direction_t di,
			   BOOL nationalMode);
  /// calls fixswap() foreach containt LOseg object
  void fixswap();
  /** free allocated memory.
      The LOsegSeg instances are allocated memory
      by calling routine before added to the array.
      When the top most segment (LOtopSeg) is destroyed,
      it should call this method which free all allocated 
      memory.
  */
  inline void erase() {
    // tell each contained segment to erase its segment (i.e. recursive)/
    if (NULL !=segsegPtr) {
      if (segsegPtrSize>0)
	for (int k = 0; (k < segsegPtrSize && (NULL !=segsegPtr[k])) ; k++) {	
	  segsegPtr[k]->erase();
	  delete segsegPtr[k];
	}  
      delete [] segsegPtr;
    }
    if (NULL !=lochars)
      delete [] lochars;
  }
    
  /// adds a new segment
  inline LOseg *addseg(LOseg *ot) {
    //push_back(ot);
    segsegPtr[segsegPtrSize++]=ot;
    return ot;
  }
  /** reverse order of contained segments (recursive).
   * This routine also calls the fixdir() method for each contained
   * segment. Segments in the array will be reversed only if the direction
   * tag of this segment is RTL.
   */
  void fixdir();
  /** returns a "flat" array of LOchar::CharPtrArray.
      This routine is the final routine to be called, after all 
      bidi and shaping processing is done. The return value of a call
      to this routine in the top most segment is than used to replace
      the 'sortout' array of character pointers.
  */
  const LOchar::CharPtrArray &flatten();
  /// reverses all contained segments
  inline void reverse() {
    int i,j;
    LOseg* tmp;
    for(i = 0, j=segsegPtrSize-1; i<j; i++, j--){
      tmp = segsegPtr[i];
      segsegPtr[i] = segsegPtr[j];
      segsegPtr[j] = tmp;
    }
  }
  /// get the size of segseg array
  inline int getsegsegptrsize() {return segsegPtrSize;}
  inline int getsegsize() {return segsegPtrSize;}
  /// get the out data size 
  inline int getflattensize() {return flattenSize;}
protected:
  /// array to fill when "flattening" all subsegments
  LOchar::CharPtrArray lochars;
  /// char segment size
  int locharsSize;
  ///charsegment pointer
  LOseg::SegPtrArray segsegPtr;
  /// seg segment size
  int segsegPtrSize;
  /// the out data size
  int flattenSize;
};

/** Special LOsegSeg class.
    Only one instance of this class should exist, to be used
    as the root node in the segment stack in LOresolve. The
    Only additive value of this class over LOsegSeg is it's 
    destructor - when the single instance of this class ends
    its scope, it will call the erase() method, which will 
    be in turn recursively called for all contained segments.
    @see erase
*/
class LOtopSeg : public LOsegSeg {
public:
  /** The constructor.
      @param in_dir direction type of this segment
      @param in_embedLevel embeding level of this segment
  */
  LOtopSeg(const direction_t in_dir=NONE, 
	   int in_embedLevel=0, int in_segSize=0) :
    LOsegSeg(in_dir, in_embedLevel, in_segSize) {}
  /// erase (and free) all contained segment
  ~LOtopSeg() {
    erase();
  }
};
#endif // LOSEG_H
