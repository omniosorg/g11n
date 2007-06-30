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
 * Copyright (c) 2000 by Sun Microsystems, Inc.
 * All rights reserved.
 */
/*
 * Copyright (C) 1994 X Consortium
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNEC-
 * TION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the X Consortium shall not
 * be used in advertising or otherwise to promote the sale, use or other deal-
 * ings in this Software without prior written authorization from the X Consor-
 * tium.
 *
 * X Window System is a trademark of X Consortium, Inc.
 */
#ifndef _XPR_CLIENT_H
#define _XPR_CLIENT_H

#pragma ident "@(#)xpr_client.h	1.1	00/10/05	SMI"

#define DEF_DISPLAY ":2100"

#define DEFAULT_HEADING_FONTSPEC (char *) \
	"-adobe-helvetica-medium-r-normal--*-%d-300-300-*-*-iso8859-1," \
	"-b&h-lucida sans typewriter-medium-r-normal-sans-*-%d-*-*-m-0-iso8859-1," \
	"-b&h-lucidatypewriter-medium-r-normal-sans-*-%d-*-*-m-0-iso8859-8," \
	"-monotype-Gill Sans-Regular-r-normal--*-%d-*-*-p-0-iso8859-2," \
	"-monotype-century gothic-regular-r-normal--*-%d-*-*-p-0-iso8859-*," \
        "-monotype-arial-regular-r-normal--*-%d-*-*-*-0-iso8859-*," \
 	"-b&h-*-normal-r-normal--*-%d-*-*-*-0-iso8859-*," \
	"-b&h-*-medium-r-normal--*-%d-*-*-*-0-iso8859-*," \
	"-Hanyi-Hei-Medium-r-normal--*-%d-*-*-m-0-big5-1," \
	"-monotype-shayyal-medium-r-normal--*-%d-*-*-m-0-iso8859-6," \
	"-hanyi-fangsong-medium-r-normal--*-%d-*-*-m-0-gb2312.1980-0," \
	"-ricoh-hg mincho l-medium-r-normal--*-%d-*-*-m-0-jisx0208.1983-0," \
	"-monotype-arial-regular-r-normal--*-%d-*-*-p-0-koi8-r," \
	"-zhongyi-fangsong-medium-r-normal--*-%d-*-*-m-0-gbk-0," \
	"-Hanyi-Hei-Medium-r-normal--*-%d-*-*-m-0-cns11643-*," \
	"-bigelow-lucida-medium-r-normal--*-%d-*-*-m-0-tis620.2533-0," \
	"-morisawa-*-medium-r-normal-*-*-%d-*-*-*-0-jisx0201.1976-*," \
	"-hanyang-roundgothic-medium-r-normal--*-%d-*-*-m-0-KSC5601.1987-0,"\
	    "-*-*-medium-r-normal-*-*-%d-*-*-m-0-*," \
	    "-*-*-medium-r-normal-*-*-%d-*-*-c-0-*," \
	    "-*-*-medium-r-normal-*-*-%d-*-*-m-0-*," \
	    "-*-*-medium-r-normal-*-*-%d-*-*-c-0-*,"\
	    "-*-*-medium-r-normal-*-*-%d-*-*-*-*-*,"\
	    "-*-*-*-r-normal-*-*-%d-*-*-*-*-*"
#define DEFAULT_BODY_FONTSPEC (char *) \
	"-adobe-helvetica-medium-r-normal--*-%d-300-300-*-*-iso8859-1," \
	"-b&h-lucida sans typewriter-medium-r-normal-sans-*-%d-300-300-m-0-iso8859-1," \
	"-b&h-lucidatypewriter-medium-r-normal-sans-*-%d-*-*-m-0-iso8859-8," \
	"-monotype-Gill Sans-Regular-r-normal--*-%d-*-*-p-0-iso8859-2," \
	"-monotype-century gothic-regular-r-normal--*-%d-*-*-p-0-iso8859-*," \
        "-monotype-arial-regular-r-normal--*-%d-*-*-*-0-iso8859-*," \
 	"-b&h-*-normal-r-normal--*-%d-*-*-*-0-iso8859-*," \
	"-b&h-*-medium-r-normal--*-%d-*-*-*-0-iso8859-*," \
	"-Hanyi-Hei-Medium-r-normal--*-%d-*-*-m-0-big5-1," \
	"-monotype-shayyal-medium-r-normal--*-%d-*-*-m-0-iso8859-6," \
	"-hanyi-fangsong-medium-r-normal--*-%d-*-*-m-0-gb2312.1980-0," \
	"-ricoh-hg mincho l-medium-r-normal--*-%d-*-*-m-0-jisx0208.1983-0," \
	"-monotype-arial-regular-r-normal--*-%d-*-*-p-0-koi8-r," \
	"-zhongyi-fangsong-medium-r-normal--*-%d-*-*-m-0-gbk-0," \
	"-Hanyi-Hei-Medium-r-normal--*-%d-*-*-m-0-cns11643-*," \
	"-bigelow-lucida-medium-r-normal--*-%d-*-*-m-0-tis620.2533-0," \
	"-morisawa-*-medium-r-normal-*-*-%d-*-*-*-0-jisx0201.1976-*," \
	"-hanyang-roundgothic-medium-r-normal--*-%d-*-*-m-0-KSC5601.1987-0,"\
	    "-*-*-medium-r-normal-*-*-%d-*-*-m-0-*," \
	    "-*-*-medium-r-normal-*-*-%d-*-*-c-0-*," \
	    "-*-*-medium-r-normal-*-*-%d-*-*-m-0-*," \
	    "-*-*-medium-r-normal-*-*-%d-*-*-c-0-*,"\
	    "-*-*-medium-r-normal-*-*-%d-*-*-*-*-*,"\
	    "-*-*-*-r-normal-*-*-%d-*-*-*-*-*"
#endif /* _XPR_CLIENT_H */
