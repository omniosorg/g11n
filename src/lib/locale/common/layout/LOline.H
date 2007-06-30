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
#pragma ident "@(#)LOline.H	1.11     99/11/19  SMI%"

#ifndef LOLINE_H
#define LOLINE_H

#include "define_bad_alloc.h"
#include "LOchar.H"



/** class to manage arrays of LOchar instances.
    This class is used for synchronizing the original input with
    the product - directional sorted and potentially shaped/combined 
    (if Arabic) visual output characters. The single most important data member
    of this class is sortout, which is of type LOchar::CharPtrArray. This
    data memger is a vector of pointers. Each pointer points at an actuall LOchar
    class instance which exist in the 'out' data member. Instead of working directly
    on the instances in 'out' (of the type LOchar::CharArray), all the shaping, 
    bidirectional sorting, combining etc. is done via the pointer in the sortout
    pointer array. This way, throughout the various processings, the output characters
    in the 'out' vector array and input characters in the 'in' vector array maintain
    their relative reference to each other, which is esential for producing the final
    OutToIn and InToOut positioning indexes required by m_*transform_layout.
 */
class LOline : public LOdefs {
public:
  friend class LOcontrol;
  /** re-initialize in, out and sortout to a new sting.
      @param str the input string
      @param len significant input length
  */
  void setLine(const unsigned char *str, int len);
  /**  initialize line from wchar_t string type.
      @param str the input string
      @param len significant input length
  */

  void setLine(const wchar_t *str, int len);
  /* initialize line from an u_short * type
     @param str the input string
     @param len significant input length
  */
  void setLine(const unsigned short *str, int len);
  /// an access routine to sortout
  inline LOchar::CharPtrArray &getline() {return sortout;}
  /** set the relative positions of chars.
      This step is done after all processing is done - i.e. bidirectional
      processing, shaping etc. are finished. Then the pos data member 
      in each LOchar instance is set to either its current position in the
      'in' array or - for the instances in the 'out' array - to its current 
      position in the 'sortout' array of LOchar pointers.
  */
  void fixpos();
  /** determine the global direction of input text.
      This routine is usually called from the LOresolve class,
      when the PLS library sets the value orientation=contextual.
      in this case the global layout direction is determined by the 
      direction of the first LTR or RTL character.
      @return direction type of first STRONG char, default is RTL
  */
  direction_t findFirstStrongChar();
#if (defined (DEBUG) && defined (PRINT))
  void print();
#endif

  /** Sets the direction tag for each character in 'out' array.
      This is one of the most important routines used for resolving
      bidi for text in implicit input mode. After a call to this function,
      there should be no character in the 'out' array whiches direction tag is set
      to NONE - i.e all neutral characters as well as strong and weak characters'
      direction tag is set.

      \begin{center}
      {\Large{\bf How character direction is resolved in implicit input mode}}
      \end{center}
      
      The process is done as follows: 
      
      * \begin{itemize}
      *   \item Set Independant direction attribute for each character
      *   \begin{itemize}
      *         \item RTL - strong right-to-left direction (e.g. a-z, A-Z)
      *         \item LTR - strong left-to-right direction
      *                  (e.g. Hebrew/Arabic characters)
      *         \item NUM - weak left-to-rigth direction (digits, number
      *                  separators)
      *         \item NONE - this character can not be resolved independantly
      *   \end{itemize}
      *
      *   \item resolve neutral types  stage I - at this stage only
      *         neutrals (direction of which should be NONE after the first
      *         stage above was performed) embeded between either weak or
      *         strong direction type (i.e. NUM, RTL, LTR) are set to also have the
      *         same direction type. However, at this stage with don't FORCE
      *         resolving of direction, so neutral characters embeded between, 
      *         for example, an 'a' and a '1' will remain NON (because the
      *         direction type to their right and left is not the same)
      *   \item resolve begin neutrals - resolve direction of special
      *         mirrored characters which should get the direction tag of the
      *         FOLLOWING strong (i.e. RTL/LTR) or weak (i.e. NUM)
      *         direction. For example: '[', '(', '{', etc.
      *   \item resolve end neutrals - this the same as above, only the
      *         characters are mirrored characters which receive the direction 
      *         tag ot the PREVIOUS strong/weak character
      *   \item resolve neutral types  stage II - at this stage 
      *         resolving of the neutral character is FORCED, i.e., after this 
      *         stage none of the characters will have the direction type of
      *         NONE. The following rules apply:
      *         \begin{itemize}
      *               \item next non NONE character is of weak type (NUM) -
      *                     character receives the previous STRONG direction type
      *                     (either RTL or LTR)
      *               \item next and previous STRONG direction types are the same
      *                     - character's tag is set to that same STRONG type.
      *               \item default - non-resolved characters always receive the
      *                     default, which is the global direction type.
      *         \end{itemize}
      * \end{itemize}
      */
  direction_t resolveDirectionType(direction_t defaultDir);
  /// Constructor
  LOline():in(NULL),out(NULL),sortout(NULL){}
  /// destructor
  //inline void deleteLine(){ delete this;}
  ~LOline() {
    if (NULL != in)
      delete [] in;
    if (NULL != out)
      delete [] out;
    if (NULL != sortout)
      delete [] sortout;
    }
  /// set the size of in array 
  inline void setinsize(const int &newVal) {inSize=newVal;}
  /// set the size of out array 
  inline void setoutsize(const int &newVal) {outSize=newVal;}
  /// set the size of sortout array 
  inline void setsortoutsize(const int &newVal) {sortoutSize=newVal;}
  /// get the size of out array
  inline int getinsize() const {return inSize;}
  /// get the size of out array
  inline int getoutsize() const {return outSize;}
  /// get the size of sortout array
  inline int getsortoutsize() const {return sortoutSize;}

protected:
  /** sets the up/down values for characters.
      It sets the links between the input CharArray in and
      ouput CharArray out. This routine is a service routine for the public
      routine setLine() .
  */
  inline void setLineSetLinks();
  /** Routines to resolve direction type of each char in line.
      These routines are service routines called from the public 
      resolveDirectionType() routine.
      @name Direction Tag Resolve Routines
  */
  //@{
  /** resolve strong/weak direction types.
   * This routine calls the resolveWeakOrStrongDir() public method of each
   * LOchar instance in the 'out' array (which is accessed only via its pointer
   * in the 'sortout' array of pointers)
   */
  inline void resolveWeakOrStrongDir();
  /// resolve begin neutral types.
  inline void resolveBeginType();
  /// resolve end neutral types.
  inline void resolveEndType();
  /** resolve (sandwiched) other Nutrals.
   * this routine is called twice by resolveDirectionType(), once	
   * with resolveDirectionType set to 'true', which leaves any non 
   * truely sandwiched neutrals as NONE, and once at the end, to 
   * force setting of ALL neutral characters whiches direction
   * type is still NONE, with resolveDirectionType set to 'false'
   @param defaultDir direction to use for unresolved neutrals
   @param resolveDirectionType flag indicating either to force resolving or not
   */
  inline void resolveNeutralType(direction_t defaultDir,
			  BOOL resolveOnlySandwiched=false);
  //@}
  /** @name Input-Output synchronizing
      The following data members are used to save an original copy
      of the unprocessed input characters, but also to maintain a link
      throughout the bidi, combo and shaping processing from each output 
      character to its original input character and vice versa. The reason 
      for this is to enable production of the InpToOut and OutToInp indexing
      buffers required by m_*transform_layout.
  */
  //@{
  /// array of original input characters.
  LOchar::CharArray in;
  /// in data size
  int inSize;
  /// array of output, processed characters.
  LOchar::CharArray out;
  /// out data size
  int outSize;
  /** array of manipulable LOchar instance pointers.
      This is the single most important data member, because
      none of the actuall LOchar instances in the 'out' array
      are actually accessed directly - rather all operations 
      are done via the pointer in the 'sortout' data member.
  */
  LOchar::CharPtrArray sortout;
  /// sortout data size
  int sortoutSize;
  //@}
};

#endif // LOLINE_H
