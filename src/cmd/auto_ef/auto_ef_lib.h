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
 * Copyright (c) 2003 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#ifndef _AUTO_EF_LIB_H
#define _AUTO_EF_LIB_H

#ident  "@(#)auto_ef_lib.h 1.12 07/04/12 SMI"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iconv.h>
#include <errno.h>
#include <locale.h>
#include <math.h>
#include <limits.h>
#include "auto_ef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PI 3.141592653589793238462
#define EPS (1e-6)
#define ICONV_LOCALE_MAX 256
#define ENCODING_LENGTH 64
#define SINGLE_ENCODING_MAX 128
#define MSBFLAG 128
#define AUTOEF_FLAG_LENGTH 4
#define FULL 100.0
#define HASHSIZE 8192
#define TRUE 1
#define FALSE 0

/* auto_ef_t */
struct auto_ef_info{
  double score;
  char encoding[PATH_MAX];
  char language[PATH_MAX];
};

/* Encoding define */

#define ASCII "ASCII"
#define UTF8 "UTF-8"
#define ISOJP "ISO-2022-JP"
#define ISOKR "ISO-2022-KR"
#define ISOCN "zh_CN.iso2022-CN"
#define EUCJP "eucJP"
#define PCK "PCK"
#define EUCCN "zh_CN.euc"
#define GB18030 "zh_CN.gb18030"
#define EUCKR "ko_KR.euc"
#define CP949 "ko_KR.cp949"
#define BIG5 "zh_TW-big5"
#define EUCTW "zh_TW-euc"
#define HKSCS "zh_HK.hkscs"
#define ISOCNEXT "zh_TW.iso2022-CN-EXT"
#define I8859_1 "8859-1"
#define I8859_2 "8859-2"
#define I8859_5 "8859-5"
#define I8859_6 "8859-6"
#define I8859_7 "8859-7"
#define I8859_8 "8859-8"
#define KOI8 "koi8-r"
#define TIS620 "TIS620.2533"
#define CP1250 "CP1250"
#define CP1251 "CP1251"
#define CP1252 "CP1252"
#define CP1253 "CP1253"
#define CP1255 "CP1255"
#define CP1256 "CP1256"
#define CP874 "CP874"
#define UTF16 "UTF-16"

/* internal date struct of found encoding information */
typedef struct _auto_ef_info *_auto_ef_t;
struct _auto_ef_info{
  double _score;
  char _encoding[PATH_MAX];
  char _language[PATH_MAX];
  _auto_ef_t _next_autoef;
}_AUTOEF;

/* Hash Record */
typedef struct score_record *srd;
typedef struct score_record{
  unsigned char keyword[2];
  int score;
  srd nextsrd;
}SRD;

/* 
 * Type definition 
 */

typedef struct auto_ef_info AUTOEF;

/* Function prototype */
const char *lengthbuf(const char *, int *);
_auto_ef_t SortATEFO(_auto_ef_t);
auto_ef_t *ATEFO2AUTOEF(_auto_ef_t, size_t *);
int Regist_AUTOEF(char *, double, char *, _auto_ef_t *);
char *M_FromCodeToLang(char *);
void Free_AUTOEF(_auto_ef_t *);
void ConvScore(_auto_ef_t *);
double prob_inte(double);

#ifdef __cplusplus
}
#endif

#endif /* _AUTO_EF_LIB_H */
