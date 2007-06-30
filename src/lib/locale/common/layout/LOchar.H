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
#pragma ident "%W%     %E%  SMI%"

#ifndef LOCHAR_H
#define LOCHAR_H
#include "define_bad_alloc.h"
#include "LOtypes.H"
/** a class for manipulating and qeurying Layout character values.
    This class is the smallest unit in the class hirarchy, most of the 
    operations are done on its values and by accessing its attribute querying 
    and setting methods. Most of the operations are done on a CharPtrArray of LOchar
    objects.

    @see CharPtrArray
*/
class LOchar : public LOtypes {
public:
  /// Array of layout characters
  //typedef vector<LOchar> CharArray;
  typedef LOchar* CharArray;
  /// Array of pointers to layout characters
  typedef LOchar** CharPtrArray;
  /** @name Member access routines */
  //@{
  /// set link to input character sibling - "up"
  inline void setup(LOchar *ot) {up = ot;}
  /// set link to output character sibling - "down"
  inline void setdown(LOchar *ot) {down = ot;}
  /// set property (embedding level) of input char 
  inline void setproperty(const int &inproperty) {property = inproperty;}
  /// set cell indicator
  inline void setcellIndicator(const int &incellIndicator) {cellIndicator = incellIndicator;}
 /// set the default cell indicator
  inline void setcellIndicator() {cellIndicator = lo_isNewCell() ? ((0x10)) :(0);}
 /// set the charater map information
  inline void setcharMap() { charmap = charmapGet(val);}
  /// set nominal/isolated character value 
  inline void setval(const val_t &newVal) {
    val=shaped=newVal; 
    setcharMap(); setcellIndicator();
    resolveWeakOrStrongDir();
      }
  /// set index position in CharArray
  inline void setpos(const int &newPos) {pos=newPos;}
  /// set direction type for this character
  inline void setdir(const direction_t in_dir)  {dir = in_dir;}
  /// retrieve the nominal/isolated character value 
  inline val_t getval() const {return val;}
  /// retrieve the shaped value of this char
  inline val_t getshaped() const {return shaped;}
  /// retrieve ling to input char sibling "up"
  inline LOchar *getup() {return up;}
  /// retrieve ling to output char sibling "donw"
  inline LOchar *getdown() {return down;}
  /// retrive index position of this char in CharArray
  inline int getpos() const {return pos;}
  /// retrive embeding level of this char
  inline int getproperty() const {return property;}
  /// retrive setcellIndicator of this char
  inline int getcellIndicator() const {return cellIndicator;}
  /// get direction type of this character
  inline direction_t getdir() const {return dir;}
  //@}
  /** sets the class variable shaped. The shaping is done
      according to the shaping value of the previous and next characters,
      If previous or next characters are vocalization (diacretics) marks, 
      they are skipped. The argument shapeCase gives an indication of
      what kind of shaping, if at all, is to be done. the value of
      the shapeCase argument is passed down from the LOxontrol
      class, according to the default or settings received from the PLS 
      layout library.
      @param line a reference to the array in which this character resides
      @param cur a pointer to the current character
      @param prev a pointer to previous char in array
      @param next a pointer to next char in array
      @param shapeCase shaping instructions
  */
  /*
  static void fixshape(const CharPtrArray &line,
		       CharPtrArray::const_iterator cur,
		       CharPtrArray::const_iterator prev,
		       CharPtrArray::const_iterator next,
		       text_descriptor_val_t shapeCase);
		       */
  /// swap mirrored characters
  inline void fixswap() {if (lo_isswap()) lo_toswap();}
  /// convert number to indic
  inline void fixnumerals() {if (lo_isnumber()) lo_tonumber();}
  /** sets the value memer to a combo glyph. This is done according to the
      combined value of the current and next characters. The map comboMap 
      is consulted to find an entry matching the combined key. In the case
      that a valid combination if found, the value of the class variable 'val' 
      is set to the value of the new combo (which is always the isolated form
      of the combo glyph). The process as recursive and this routine returns only
      when the current character does not combine with the next one.
      @param line a reference to the array in which this character resides
      @param cur a pointer to the current character
      @return a pointer incremented by number of characters combined
  */
  /*
  static CharPtrArray::iterator fixcombo(const CharPtrArray &line,
					 CharPtrArray::iterator cur);
					 */
  /** @name character attribute query routines
   */
  //@{
  /// checkes for strong right-to-left attribute
  inline BOOL lo_isrtl() const { return (charmap & (_RTL));}
  /// checkes for strong left-to-right attribute
  inline BOOL lo_isltr() const {return (charmap & (_LTR));}
  /// checkes for digit (0-9)
  inline  BOOL lo_isdigit() const {return (charmap & _NUM);}
  /// checks for number attribute (digit + number separator)
  inline BOOL lo_isnumber() const {return (lo_issep() || lo_isdigit());}
  /// checks for number separator attribute (comma, period, etc)
  inline BOOL lo_issep() const {return (charmap & _SEP);}
  /// checks for swap attribute (mirrored characters)
  inline BOOL lo_isswap() const {return (charmap & _SWAP);}
  /// checks for the input cell indicator
  inline BOOL lo_isNewCell() const {return (charmap & _CELL);}
  /// checkes for control chars that taks the defaultdir value
  inline BOOL lo_isCtrl() const { return (charmap & (_CTRL));}
  /// checkes either this character is has a weak direction type (number)
  inline BOOL lo_isWeak() const { return (NUM == getWeakOrStrongDir()); }
  /// checkes either this character is has a strong direction type (rtl/ltr)
  inline BOOL lo_isStrong() const { 
    direction_t res = getdir();
    return ((NONE !=res) && (NUM != res));
  }
  /// checks either this character direction type is resolved (strong or weak) type
  inline BOOL lo_isWeakOrStronDir() const {
    return (NONE != getWeakOrStrongDir());
  }
  /** checks for end neutral
      this neutral character takes the direction of the preceding weak or
      strong embeding segment
  */
  inline BOOL lo_isendneutral() const {return (charmap & (_END|_NEND));}
  /** checks for number end  neutral
      this character takes the NUM direction type if previous embeded 
      segment is numbers
  */
  inline BOOL lo_isnumendneutral() const {return (charmap & _NEND);}
  /** checks for begin neutral
      this neutral character takes the direction of the next weak or
      strong embeding segment
  */
  inline BOOL lo_isbegneutral() const {return (charmap & _BGN);}
  /// checkes either this caracter is a vocalisation mark
  inline BOOL lo_isavocal() const {return (charmap & _VOC);}
  /// checks either this character "connects" only with previous shaping character
  inline BOOL lo_isashape2() const {return (charmap & _AS2);}
  /// checks either this character "connects" both with next and previous shaping characters
  inline BOOL lo_isashape4() const {return  (charmap & _AS4);}
  /// ehckes either this character may combine with next non diacretic character
  inline BOOL lo_isacombo() const {return (charmap & _ACB);}
  inline lo_shape(){
    return (((charmap & _AS4) !=0) ? _AS4 :
	    ((charmap & _AS2) !=0) ? _AS2 : _AS1);
  }
    //@}
  /**  character transformation
       @name character transformation routines
  */
  //@{
  ///  sets value of shaped to swapped charactre
  inline void lo_toswap() {shaped = swapmapGet(val);}
  ///  sets value of shaped to indic equivalent
  inline void lo_tonumber() {shaped = nummapGet(val);}
  /// sets value of shaped to initial form of nominal value
  inline void lo_tobegin() {shaped = shapemapGet(val).begin;}
  /// sets value of shaped to isolated form of nominal value
  inline void lo_toisolated() {shaped = shapemapGet(val).isolated;}
  ///  sets value of shaped to middle form of nominal value
  inline void lo_tomiddle() {shaped = shapemapGet(val).middle;}
  ///  sets value of shaped to end form of nominal value
  inline void lo_toend() {shaped = shapemapGet(val).end;}
  //@}
  /** set self's direction type.
   * The value of 'dir' data member will be set to one
   * of the following values:
   * \begin{itemize}
   *       \item RTL - if the current character is Hebrew/Arabic character.
   *       \item LTR - if the current character is English [a-zA-Z].
   *       \item NUM - if the curent character is a digit [0-9] or Indic digit
   *       \item NONE - for any other character
   * \end{itemize}
   */
  inline void resolveWeakOrStrongDir()  {
    setdir(getWeakOrStrongDir());
  }
  /// returns a strong direction type (RTL, LTR) or NONE.
  inline direction_t getStrongDir() const {
    //direction_t res = getWeakOrStrongDir();
     direction_t res = getdir();
    return (NUM == res ? NONE : res);
  }
  /// Constructor
  LOchar(const val_t c=0):
    LOtypes(), val(c), charmap(charmapGet(c)), shaped(c), up(NULL), 
    down(NULL), pos(-1), property(0),cellIndicator(0x10), dir(NONE) {}
protected:
  /// holder of nominal or isolated value of this character
  val_t val;
  /// holds the character map value
  lotype_t charmap;
  /// holds shaped (output) value of this character
  val_t shaped;
  /// link to input sibling charactr
  LOchar *up;
  /// link to output sibling
  LOchar *down;
  /// positional value of this character in CharArray or CharPtrArray
  int pos;
  /// embedding level of this character
  int property;
  /// the cell indicator for this character
  int cellIndicator;
  /// layout direction value of this character (NUM, LTR, RTL or NONE)
  direction_t dir;
  /// retrieves the direction type for this character
  inline direction_t getWeakOrStrongDir() const{
    direction_t res = 
      (lo_isltr() ? LTR :
       (lo_isrtl() ? RTL :
	(lo_isdigit() ? NUM : NONE)));
    return res;
  }
};

#endif // LOCHAR_H
