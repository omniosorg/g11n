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

#ifndef LOEXCEPTION_H
#define LOEXCEPTION_H

#include <errno.h> 
#ifdef DEBUG
#include <iostream.h>
#endif

class LOexception {
public:
  LOexception(const char *msg=NULL, const char *file=NULL, int lnum=-1) {
#ifdef DEBUG
    if (msg) {
      cerr << msg;
      if (file) {
	cerr << file;
	if (-1 != lnum) {
	  cerr << "," << lnum;
	}
      }
      cerr << endl;
    }
#endif
  }
};
class LOnewException: public LOexception {
public:
  LOnewException() : LOexception("memory allocation problem") {
    errno = ENOMEM;
  }
};

#endif				// LOEXCEPTION_H
