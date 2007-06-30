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

#include "LOtypes.H"
/*#include CUR_LANG_LOTYPES*/

#ifdef HE_DATA
#include "he_data.h"
#endif

#ifdef AR_DATA
#include "ar_data.h"
#endif


const int LOtypes::binSearch(int val, int start, int end, const int* array)const
{
	while(start <= end){
	    int mid = start + ((end - start) >> 1);
	    if(val == array[mid])
	       return mid;
	    if(val < array[mid])
		end = mid - 1;
	    else
		start = mid + 1;
	}
	return -1;
    }

const LOtypes::lotype_t &
LOtypes::charmapGet(const val_t &c) const 
{
  static lotype_t defaultVal=0; /* dummy value to
				      * return if c not found in map
				      */
  int key = c; 
  int idx;
  if (key < 0x20){
    defaultVal=(lotype_t) (_CTRL);
    return (defaultVal);
  }
  else if (key >=0x20 &&key <=0x7f ) //latin range
    idx= key-0x20;
  else if (key >=0xa0 && key <=0xff) //iso range
    idx= (key-0x40);
  else if (key >= 0x80 &&key <=0x9f ) //latin range
    idx= -1;
  else if (key >=0x064b && key <=0x0652) //unicode
    idx= (key-0x064b)+191; // was 192 
  else if (key >=0xfc5e && key <=0xfc62) //unicode
    idx= (key-0xfc5e)+199; // was 200
  else if (key >=0xfef5 && key <=0xfefb) //unicode
    idx= (key-0xfef5)+204; // was 205

  else  //other unicode values 
    idx = binSearch( key, 0 ,(sizeof(charDataIndex)/sizeof(charDataIndex[0]))-1 , charDataIndex);
  if (idx != -1 && idx <=(sizeof(charData)/sizeof(charData[0]))-1 ) 
    return ((lotype_t)charData[idx]);
  return defaultVal;
}

const LOtypes::val_t &
LOtypes::swapmapGet(const val_t &c) const 
{
  int key = c; 
  int idx = binSearch( key, 0, sizeof (swapDataIndex)/sizeof(swapDataIndex[0]-1) , swapDataIndex);
  if (idx != -1) 
      return ((val_t)swapData[idx]);
  return c;
}
const LOtypes::val_t &
LOtypes::nummapGet(const val_t &c) const 
{
  int key = c;
  if(key >=0x30 && key <=0x39 && (sizeof(numData)/sizeof(numData[0])) >=9 )
    return ((val_t)numData[key-0x30]);
  return c;
}
LOtypes::shapeData_t LOtypes::shapemapGet(const val_t &c) const
{
  int key = c; 
  int idx;
  static shapeData_t defaultVal; /* dummy value to
				  * return if c not found in map
				  */
  defaultVal.begin = defaultVal.end = 
    defaultVal.middle = defaultVal.isolated = c; // initialize all to c
  if (key >=0xa0 &&key <=0xf2 ) //Arabic Iso range
    idx= key-0xa0;
  else if (key >=0xfef5 && key <=0xfefb) //unicode
     idx=(key-0xfef5)+83;
  else  //other unicode values 
    idx = binSearch(key, 0, sizeof(shapeDataIndex)/sizeof(shapeDataIndex[0]-1), shapeDataIndex);
  if (idx != -1) 
    return  ((shapeData_t)shapeData[idx]);
  return defaultVal;
}
const  LOtypes::val_t &
LOtypes::combomapGet(const val_t &base, const val_t &next) const
{
  static const  val_t defaultVal=0;  /* dummy value to
				      * return if combo not found in map
				      */
  // create the search key from the base and next combined values
  const unsigned long key = ((unsigned int)base << 16) | next;
  int idx =  binSearch( key, 0,  sizeof(comboDataIndex)/sizeof(comboDataIndex[0]-1) , comboDataIndex); 
  if (idx != -1) 
    return  ((val_t)comboData[idx]);
  return defaultVal;
}







