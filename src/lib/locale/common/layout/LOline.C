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


#include "LOline.H"
#include "LOexception.H"
#include <memory.h>
#ifdef MAIN
///
int main() {
  unsigned char buff[1024];
  while (cin) {
    cin.getline(buff, sizeof(buff));
    LOline l(buff, strlen(buff), RTL);
    l.fixdir();
    l.fixpos();
    l.print();
  }
  return 0;
}
#endif // MAIN

void LOline::fixpos()
{
  size_t len, i;
  // set position in the 'in' array
  for(len=inSize, i=0;
      i < len;
      i++) {
    in[i].setpos(i);
  }
  /* set positions in the 'out' array,
   * but the positioning is set according
   * to pointer position in the
   * 'sortout' array of pointers.
   */
  for(len=sortoutSize, i=0;
      i < len;
      i++) {
    sortout[i]->setpos(i);
  }
}
LOtypes::direction_t
LOline::findFirstStrongChar()
{
  direction_t res = NONE;
  // iterate on characters untill strong direction type is found
  for ( int k= 0 ; k < inSize && NONE == res; k++) {
    res = in[k].getStrongDir();
  }
  if (NONE == res) {
    return RTL;			// return default - RTL
  }
  return res;			// return strong direction found
}
LOline::direction_t
LOline::resolveDirectionType(direction_t defaultDir) 
{
  if (0 == sortoutSize) {
    return defaultDir;
  }
  // check either defaultDir == NONE, in which case search for strong type
  if (NONE == defaultDir) {
    defaultDir = findFirstStrongChar();
  }
  // resolve strong/weak 
  resolveWeakOrStrongDir();
  // resolve "sandwiched" between two weak types  - don't force
  resolveNeutralType(defaultDir, true);
  // resolve begin neutral types 
  resolveBeginType();
  // resolve end neutral types 
  resolveEndType();
  // resolve (sandwiched) other Nutrals - force
  resolveNeutralType(defaultDir);
  #ifdef DEBUG
  for ( int k = 0; k < sortoutSize; k++) {
    if (NONE == sortout[k]->getdir()) {
      // by now ALL character's direction should be resolved
      throw LOexception("HDIGH:",__FILE__ , __LINE__);
    }
  }
  #endif			// DEBUG
  return defaultDir;
}
void
LOline::resolveWeakOrStrongDir()
{
  for (int k = 0; k < sortoutSize; k++) {
    sortout[k]->resolveWeakOrStrongDir();
  }
}
void
LOline::resolveBeginType()
{
  // move forward untill a valid type is found
  direction_t prev_dir = NONE;
  for ( int k = sortoutSize-1; k >=0 ; k--) {
    // set prev_dir to current if is a valid type, and continue
    if ( sortout[k]->lo_isWeakOrStronDir()) {
      prev_dir = sortout[k]->getdir();
      continue;
    }
    // if this is a begine neutral type set it to previous type
    if (sortout[k]->lo_isbegneutral()) {
      sortout[k]->setdir(prev_dir);
    }
  }
}
    
void
LOline::resolveEndType()
{
  // move forward untill a valid type is found
  direction_t prev_dir = NONE;
  direction_t prev_strong_dir = NONE;
  for ( int k = 0; k < sortoutSize; k++) {
    // set prev_dir to current if is a valid type, and continue
    if (NONE != sortout[k]->getdir()) {
      prev_dir = sortout[k]->getdir();
      if (sortout[k]->lo_isStrong()) {
	prev_strong_dir = sortout[k]->getdir();
      }
      continue;
    }
    // if this is a begine neutral type set it to previous type
    if (sortout[k]->lo_isendneutral()) {
      /* characters such as '[', ']', etc. make sense at end of 
       * a number segment. However, other terminating characters (input order),
       * such as '!', '.', ',' etc. should take the strong previous type and ignore
       * the weak previous type
       */
      if (sortout[k]->lo_isnumendneutral()) {
	sortout[k]->setdir(prev_dir);
      } else {
	sortout[k]->setdir(prev_strong_dir);
      }
    }
  }
}
void
LOline::resolveNeutralType(direction_t defaultDir,
			   BOOL resolveOnlySandwiched)
{
  LOdefs::direction_t prev_dir = defaultDir; // previous WEAK or STRONG dir
  LOdefs::direction_t prev_strong_dir = defaultDir; // previous STRONG dir
  LOdefs::direction_t next_dir;		// next WEAK or STRONG dir
  LOdefs::direction_t next_strong_dir;	// next STRONG dir
  int k, nxt;
  for (k = 0; k < sortoutSize; k++) {
    /*
     * if this characer is already resolved, use its direction
     * type value for prev_strong_dir and prev_dir and continue;
     */
    if (NONE != sortout[k]->getdir()) {
      prev_dir =sortout[k]->getdir(); // always set prev_dir, if direction != NONE
      if (sortout[k]->lo_isStrong()) {
	prev_strong_dir = sortout[k]->getdir(); // only set prev_strong_dir if direction is STRONG
      }
      // nothing else to resolve about this char
      continue;
    }
    else if (sortout[k]->lo_isCtrl()){
      sortout[k]->setdir(defaultDir);
      prev_dir =sortout[k]->getdir();
      if (sortout[k]->lo_isStrong()) 
	prev_strong_dir = sortout[k]->getdir(); 
      // nothing else to resolve about this char
      continue;
   }



    for ( nxt = k + 1, next_strong_dir = NONE, next_dir = NONE;
      	 nxt < sortoutSize && (next_strong_dir == NONE || next_dir == NONE);
	 nxt++) {

      if (NONE == next_dir) {
	next_dir = sortout[nxt]->getdir();// always set next_dir, if direction != NONE
      }
      if (sortout[nxt]->lo_isStrong()) {
	next_strong_dir = sortout[nxt]->getdir(); // only set next_strong_dir if direction is STRONG
	break;
      }
    }
    // Make sure next_dir and next_strong_dir are not NONE!
    next_dir = (NONE == next_dir) ? defaultDir : next_dir;
    next_strong_dir = (NONE == next_strong_dir) ? defaultDir : next_strong_dir;
    /*
     * got all information we need,
     * check what is this neutral "sandwiched" between:
     * 1. weak types get priority, check prev_dir and next_dir first
     * 2. neutral "sandwiched" between STRONG type and NUM type get the STRONG type
     * 3. strong (next_strong_dir and prev_strong_dir) take control
     * 4. if still unresolved - defaultDir 
     */
    if (prev_dir == next_dir) {
      sortout[k]->setdir(next_dir);	// 1.
    } else {
      /* first check either we should continue trying to resolve 
       * strong types "sandwich"
       */
      if (true==resolveOnlySandwiched) {
	continue;
      }
       if (NUM == next_dir) {
      	sortout[k]->setdir(prev_strong_dir); // 2.
      } else if (prev_strong_dir == next_strong_dir) {
	sortout[k]->setdir(next_strong_dir); // 3.
      } else {
	sortout[k]->setdir(defaultDir);	// 4.
      }
    }
  }
}
void 
LOline::setLine(const unsigned char *str, int len) 
{
  if (NULL !=in)
    delete [] in;
  inSize=outSize=sortoutSize=len;
    in = new LOchar[(sizeof (LOchar)*len) +1];
    for (size_t i=0; i<len; i++)
      in[i].setval(int (str[i]));
  setLineSetLinks();
}

void 
LOline::setLine(const wchar_t *str, int len) 
{
  if (NULL !=in)
    delete [] in;
  inSize=outSize=sortoutSize=len;
    in = new LOchar[(sizeof (LOchar)*len) +1];
    for (size_t i=0; i<len; i++)
      in[i].setval(int (str[i]));
  setLineSetLinks();
}
void 
LOline::setLine(const unsigned short *str, int len) 
{
  if (NULL !=in)
    delete [] in;
  inSize=outSize=sortoutSize=len;
    in = new LOchar[(sizeof (LOchar)*len) +1];
    for (size_t i=0; i<len; i++)
      in[i].setval(int (str[i]));    
    setLineSetLinks();
}
void
LOline::setLineSetLinks()
{
  if (NULL !=sortout)
    delete [] sortout;
  if (NULL !=out)
    delete [] out;
  out = new LOchar[(sizeof (LOchar)*outSize) +1];
  sortout = new LOchar*[(sizeof (LOchar*)*sortoutSize) +1];
  memcpy(out,in, (sizeof (LOchar)*outSize));
  for (size_t i=0; i < inSize; i++) {
    in[i].setdown(&out[i]);
    out[i].setup(&in[i]);
    sortout[i]= (&out[i]);
  }
}

#if (defined (DEBUG) && defined (PRINT))
void
LOline::print()
{
  size_t i;
  printf("\n---------");
  for (i = 0; i < inSize; i++) {
    printf("%4s-", "----");
  }
  printf("\n");
  printf("  Index: ");
  for (i = 0; i < inSize; i++) {
    printf("%4d ", i);
  }
  printf("\n");

  printf("  InBuf: ");
  for (i = 0; i < inSize; i++) {
    printf("%4c ", (unsigned char)in[i].getval());
  }
  printf("\n");

  printf("  Inter: ");
  for (i = 0; i < sortoutSize; i++) {
    printf("%4c ", (unsigned char)sortout[i]->getval());
  }
  printf("\n");

  printf(" OutBuf: ");
  for (i = 0; i < sortoutSize; i++) {
    if (0xff < sortout[i]->getshaped()) {
      printf("%4x ", (int)sortout[i]->getshaped());
    } else {
      printf("%4c ", (unsigned char)sortout[i]->getshaped());
    }
  }
  printf("\n");


  printf("InToOut: ");
  for (i = 0; i < inSize; i++) {
    printf("%4d ", in[i].getdown()->getpos());
  }
  printf("\n");
  
  printf("OutToIn: ");
  for (i = 0; i < sortoutSize; i++) {
    printf("%4d ", sortout[i]->getup()->getpos());
  }
  printf("\n");
  
  printf("   Attr: ");
  for (i = 0; i < inSize; i++) {
    printf("0x%2x ", in[i].getproperty()| in[i].getcellIndicator());
  }
  printf("\n");
}
#endif
