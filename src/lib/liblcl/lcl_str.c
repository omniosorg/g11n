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
/* Copyright (c) 2000, Sun Microsystems, Inc. All rights reserved. */
 
#pragma ident	"@(#)lcl_str.c	1.3	00/01/07 SMI"

#include <string.h>
#include <ctype.h>

char *
_LclCreateStr(register char *str)
{
  register char *ptr;
  
  if (str == (char *)NULL)
    return (char *)NULL;

  ptr = (char *)malloc(strlen(str) + 1);
  if (ptr != (char *)NULL)
    strcpy(ptr, str);
  return ptr;
}

char *
_LclCreateStrn(char *str, int len)
{
  char	*ptr;
  
  ptr = (char *)malloc(len + 1);
  if (ptr != (char *)NULL){
    strncpy(ptr, str, len);
    ptr[len] = (char)NULL;
  }
  return ptr;
}

lcl_strncasecmp(register char* a, register char* b, int n)
{	
  register char ac,bc;
  register int i;
  
  for ( i = 1; i < n; i++ ) {
    ac = *a++;
    bc = *b++;
    
    if (ac == 0) {
      if (bc == 0)
	return 0;
      else	return -1;
    }
    else if (bc == 0)
      return 1;
    else
      if (ac != bc) {
	if (islower(ac)) ac = toupper(ac);
	if (islower(bc)) bc = toupper(bc);
	if ( ac != bc )	return ac - bc;
      }
  }
  if (islower(*a)) ac = toupper(*a); else ac = *a;
  if (islower(*b)) bc = toupper(*b); else bc = *b;
  return ac - bc;
}

lcl_strcasecmp(char *a, char *b)
{
  return strncasecmp(a,b,0xFFFFFFF);
}

char *
strcasestr(s1,s2)
char *s1,*s2;
{
  char *p1;
  
  if ( *s2 == 0 )
    return s1;
  
  for ( p1 = s1; *p1; p1 ++ )
    if ( lcl_strcasecmp(p1,s2)==0 ) 
      return p1;
  return 0;
}
