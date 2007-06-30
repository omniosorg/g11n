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
#pragma ident "@(#)LOresolve.H	1.11     99/11/19  SMI%"

#ifndef LORESOLVE_H
#define LORESOLVE_H
#include <stdlib.h>
#include "define_bad_alloc.h"
#include "LOseg.H"
#include "LOcontrol.H"
#include "LOexception.H"

/** Class to resolve bidi, shaping, etc.
    This class is given access to both the LOcontrol instance
    and the LOline on which it does its operations. The most
    important task this class does is creating the 'stack' of
    embedded RTL, LTR and NUM segments, and calling the
    appropriate methods in these segments, according to the PLS
    settings it gets from the LOcontrol class reference.
*/
class LOresolve : public LOdefs {
public:
  /** The constructor
      @param in_loControl layout conrol class
      @param in_sortout line on which to perform
  */
  LOresolve(const LOcontrol &inloControl, LOline &in_line) :
    loControl(inloControl), 
    loLine(in_line),
    topSegPtr(NULL),
    segStack(NULL) {
    sortoutPtr = (loLine.getline());
    sortoutPtrSize= loLine.getsortoutsize();
  }
  /// destroyer needs to delete topSegPtr
  ~LOresolve() { 
    if (NULL != topSegPtr) delete topSegPtr;
    if (NULL != segStack) delete [] segStack;
  }
  /** performs "full course" of layout.
      Call the appropriate methods for creating 
      embeding levels (bidi), shaping, combo, swapping etc., 
      in accordance with the PLS settings of the LOcontrol class
      reference.
  */

  BOOL resolve();
  /** creates the visual (for outpu) LOchar::CharPtrArray. and returns the modified size
      It is the last method to be called AFTER all processing is done 
      on LOline's 'sortout' array.
  */
  int flatten();
protected:
  /** cause segments to recursively convert number formats.
      @param nationalMode force Indic conversion flag
  */
  inline void fixnumerals(BOOL nationalMode=false) {
    // top segment does not need direction
    topSegPtr->fixnumerals(NONE, nationalMode);
  }
  /// resolve shaping
  inline void fixshape();
  /// resolve combinations
  inline void fixcombo() {
    topSegPtr->fixcombo();
  }
  /// resolve mirroring
  inline void fixswap() {
    topSegPtr->fixswap();
  }
  /** resolves direction of embeded data structure (topSeg).
   */
  inline void fixdir();
  
  /** Overview documentation on How embedding is resolved.
      @name Embedding
   * \begin{center}
   * {\Large{\bf How embedding is resolved in implicit input mode}}
   * \end{center}
   * In order to create the visual-bidi-sorted output, characters (LOchar
   * pointers) from the input line are appended to segments of LOcharSeg
   * type. These LOcharSeg segments themselves are embeded in segment container
   * of the type LOsegSeg. Each such container segment itself may be contained
   * in a higher level LOsegSeg.... There is one top-most 
   * LOsegSeg segment container - topSegPtr, which containes ALL the embeded segments.
   * The way the embeding works is like thus:
   * \begin{itemize}
   *       \item Initialize the embedding structure
   *       \begin{itemize}
   *             \item segStack is empty
   *             \item currentEmbedLevel is initialized to globalEmbedLevel
   *             \item topSegPtr is created with defaultDir, currentEmbedLevel values
   *             \item curSegPtr is set to topSegPtr
   *       \end{itemize}
   *       \item read input characters from line and embed each one, the
   *             following embedding rules are maintained:
   *       \begin{itemize}
   *             \item charactes are always added to a newly created
   *                   LOcharSeg segment, which is added to the current
   *                   active segment (curSegPtr). Any subsequent input 
   *                   characters are added to the same new segment for
   *                   as long as their direction type property is the same
   *             \item If the next character direction property is either 
   *                   the SAME as that of current active segment (curSegPtr),
   *                   or the porperty is NUM, the new LOcharSeg segment is
   *                   added to curSegPtr.
   *             \item When the next input character's direction property is 
   *                   neither SAME as that of curSegPtr, nor NUM, the embeding
   *                   level changes, which means curSegPtr also changes:
   *                   \begin{itemize}
   *                         \item Increment embedding level - if the current
   *                               active segment is the bottom most segment
   *                               (topSegPtr), embedding level is incremented
   *                               by one, a new LOsegSeg is created, added
   *                               to the segStack and becomes the current
   *                               active segment (i.e. curSegPtr now refers to it).
   *                         \item Decrement embedding level - if the current
   *                               active segment is NOT topSegPtr, it means that the
   *                               direction property of the next character is the same
   *                               as that of the next LOsegSeg in the segStack, therfore,
   *                               insteed of adding a new LOsegSeg and incrementing the
   *                               embedding level, we simply reinstate the previous
   *                               segment in stack and decrement embedding level. This
   *                               is done by "popping" the stack, decrementing
   *                               the currentEmbedLevel by one and setting curSegPtr
   *                               to point at previous segment, now at top of stack.
   *                   \end{itemize}
   *
   *       \end{itemize}
   * \end{itemize}
   *
   * Example: Global embeding is LTR,
   * therefore RTL embeding level goes up, any RTL or NUM segments 
   * are contained in the new RTL segment. If an LTR type is encountered,
   * embeding level goes DOWN (popSeg()) and LTR/NUM segments are contained 
   * in the segment of current level.
   * 
   * \begin{verbatim}
   * embed level=1:      ->  RTL -> (embed RTL) -> (embed NUM) ... >-      
   *                    /                                            \
   *                   /                                              \
   * base level=0:   LTR -> (embed LTR) -> (embed NUM) ...           LTR
   *
   * Example: Global embeding is RTL - exectly oposite from above
   *
   * embed level=2       ->  LTR -> (embed LTR) -> (embed NUM) ... >-
   *                    /                                            \
   *                   /                                              \
   * base level=1:   RTL -> (embed RTL) -> (embed NUM) ...           RTL
   *
   *\end{verbatim}

   @see embedImplicit
   @see embedStackInit
   @see topSegPtr
   @see popSeg
   @see pushSeg
   */
  /** resolves the bidi by embeding segments.
      @see Embedding
  */
  inline void embedImplicit();
  /** Initializes topSegPtr with current defaultDir value.
      @see Embedding
  */
  inline void embedStackInit(direction_t dir);
  

  /** embeds one segment.
      Embed one segement, for as long as the direction type of 
      current lochar pointed to by k is same.
      @param line reference to line of input
      @param k reference to iterator at current input position
      @return pointer to the next input character to be processed.
      @see Embedding
  */
  //LOchar::CharPtrArray::iterator embedOne(LOchar::CharPtrArray &line, int k);
 inline int embedOne(LOchar::CharPtrArray &line, int k);
  /** append chars from line to a new LOcharSeg.
      This routine creates a new instance of LOcharSeg, which is 
      appended to the curent LOsegSeg at the top of the segment stack.
      Then it fills it with characters (LOchar pointers) from the 
      parameter line, for as long as the direction type of the character
      pointer by iterator k is the same as that of the new segment.
      @param line reference to line of input
      @param k reference to iterator at current input position
      @see Embedding
      @see segStack
      @see curSegPtr
  */
  inline int appendToNewSegment(LOchar::CharPtrArray &line, int k);
  /** Create new embeding level.
      This routine creates a new LOsegSeg, pushes its pointer
      to the segment stack and set it to be the current active
      segment. Untill the next call to popSeg() or pushSeg(), any new 
      segments  - either LOcharSeg or LOsegSeg are going to be added into
      this segment.

      @see Embedding
      @see popSeg
      @see curSegPtr
   */
  inline void pushSeg(direction_t dir, int k);
  /** Restores embeding to previous level.
      The segment stack is "poped", The current segment pointer 
      curSegPtr is set to point at the previously embeded "active"
      segment now at top of stack.
      @see pushSeg
      @see curSegPtr
  */
  inline void popSeg();
  /** reference to layout control object.
      The LOcontrol object reference is used
      to extract PLS/default layout engine settings 
      as to decide how (if at all) to resolve bidi, shaping,
      swapping, etc.
      @see Embedding
  */
  const LOcontrol &loControl;
  /// reference to array of LOchar pointers on which to operate
  LOchar::CharPtrArray sortoutPtr;
  /// sortoutPtr size
  int sortoutPtrSize;
  /// the LOline class instance from which to get sortoutPtr
  LOline &loLine;
  /** stack of segment pointers 
      pop and unpop each time embeding level increase/decrease
  */
  ///?? stack<LOsegSeg*, deque<LOsegSeg*> > segStack;
  LOsegSeg** segStack;
  int segStackPtr;
  /** pointer to top segment in stack
      @see segStack
  */
  LOsegSeg* curSegPtr;
  /// segment containing all embeded segments
  LOtopSeg* topSegPtr;
  /// values used by resolve class (should put this flag in a more appropriate place)
  direction_t defaultDir;
  /// base embeding level (RTL == 1, LTR == 0)
  int globalEmbedLevel;
  /// embeding level counter
  int currentEmbedLevel;
};

#endif // LORESOLVE_H
