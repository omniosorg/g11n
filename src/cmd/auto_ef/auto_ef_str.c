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
 * Copyright (c) 2003, by Sun Microsystems, Inc.
 * All rights reserved.
 */

#ident  "@(#)auto_ef_str.c 1.19 07/04/25 SMI"

#include "auto_ef_lib.h"
#include <errno.h>

#define	LEVEL0_SIZE	64
#define LEVEL1_SIZE	256
#define LEVEL2_SIZE	1024
#define LEVEL3_SIZE	0

int ReturnCodeNum(char *);
double fact(long);
void QSort_AUTOEF(_auto_ef_t *, int, int, _auto_ef_t);
int IdentfyEncoding(int, size_t, int *, char *, char *, const char *,
        _auto_ef_t *, int *, const char *);

/* List of iconv encoding currently supported in auto_ef */
const char iconv_list[ICONV_LOCALE_MAX][ENCODING_LENGTH]={
  UTF8,
  ISOJP,
  EUCJP, EUCKR, EUCCN, EUCTW, 
  CP949, HKSCS, PCK, GB18030,
  I8859_1, I8859_2, I8859_5, I8859_6, I8859_7, I8859_8,
  CP1250, CP1251, CP1252, CP1253, CP1255, CP1256, 
  TIS620, CP874, NULL
};

size_t auto_ef_str(auto_ef_t **aef, const char *input_buf,
		size_t buf_size, int auto_ef_flag) {
	
	char *inputp;
	int level_flag = 0;
	int random_access_flag = 0;
	int random_access_seed = 0;
	int newline_flag = 0;
	int found_target = 0; /* Target found flag */
	int euc_found_flag = FALSE;
	_auto_ef_t root_autoef = (_auto_ef_t) NULL;
	int i, j;
	
	int level = LEVEL3_SIZE;
	
	level = auto_ef_flag & 0x3;
	
	switch (level) {
	case 0:
		level_flag = LEVEL0_SIZE;
		break;
	case 1:
		level_flag = LEVEL1_SIZE;
		break;
	case 2:
		level_flag = LEVEL2_SIZE;
		break;
	default:
		level_flag = LEVEL3_SIZE;
	}
	
	if ((inputp = (char *) malloc(buf_size)) == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	
	/* for random access */
	if (level_flag != LEVEL3_SIZE) {
		for (i = 0; i < buf_size; i++) {
			char a = input_buf[i];
			char b = (i + 1) < buf_size ? input_buf[i + 1] : 0;
			if (a == '\0') break;
			/* unix, windows and Mac file format */
			if (a == '\n' || a == '\r')
				random_access_flag++;
			if (a == '\r' && b == '\n')
				i++;
		}

		random_access_seed = random_access_flag/level_flag;
		if (random_access_seed <= 0)
			random_access_seed = 1;

		j = 0;
		/* adopt buf based on level */
		for (i = 0; i < buf_size; i++) {
			char a = input_buf[i];
			char b = (i + 1) < buf_size ? input_buf[i + 1] : 0;
			if (a == '\0')
				break;
			if (a == '\n' || (a == '\r' && b != '\n'))
				newline_flag++;
			if ((newline_flag % random_access_seed) == 0) {
				inputp[j] = a;
				/* Mac format file */
				if (a == '\r' && b != '\n')
					inputp[j] = '\n';
				j++;
			}
		}
		/* In case of i = buf_size, j should decrement  */
		if (j == buf_size) j--;
		inputp[j] = '\0';
	} else {
		/* Default */
		strncpy(inputp, input_buf, buf_size);
	}

	for (i = 0; i < ICONV_LOCALE_MAX; i++) {
		const char *p, *context;
		int flag;
		int succeed_flag = 0;
		int tmp_bufsize = 0;
		size_t fsize, tsize;
		char *tbuf;
		char from_code[ENCODING_LENGTH];
		char *convert;
		iconv_t cd;
		size_t ret;
		int code_num = 0;
		int end_auto_ef = 0;
		
		if (iconv_list[i][0] == '\0')
			break;

		if (strcmp(iconv_list[i], UTF8) == 0) {
			/* UTF-8 conversion shoule not be to UTF-8 */
			if ((cd = iconv_open(UTF16, iconv_list[i]))
				== (iconv_t)-1) {

				/* Use iconv_open errno */
				*aef = NULL;
				Free_AUTOEF(&root_autoef);
				free(inputp);
				return (-1);
			}
		} else {
			/* iconv OPEN from_code, to_code */
			if ((cd = iconv_open(UTF8, iconv_list[i]))
				== (iconv_t)-1) {
				/* Use incov_open errno */
				*aef = NULL;
				free(inputp);
				Free_AUTOEF(&root_autoef);
				return (-1);
			}
		}

		/* euc already succeed */
		if (strcmp(iconv_list[i], CP949) == 0 &&
			found_target == 1) {

			iconv_close(cd);
			break;
		}

		errno = 0;
		context = &inputp[0];
		tmp_bufsize = buf_size;
		p = lengthbuf(inputp, &tmp_bufsize);
		fsize = p - &inputp[0];

		/* Need to take enough buf to convert to UTF-8 */
		tsize = fsize * 4;
		if ((tbuf = (char *) malloc(tsize)) == NULL) {
			errno = ENOMEM;
			iconv_close(cd);
			*aef = NULL;
			free(inputp);
			Free_AUTOEF(&root_autoef);
			return (-1);
		}

		tbuf[0] = '\0';
		convert = &tbuf[0];
		ret = iconv(cd, &context, &fsize, &convert, &tsize);
		if (ret == (size_t) 0) {
			*convert = '\0';
			/* This is for 6547913. EUCJP has more wide */
			/* valid code range than the other EUC, because */
			/* of UDC and VDC. So need to care about it */
			if (strcmp(iconv_list[i], EUCJP) == 0)
				euc_found_flag = TRUE;
		} else {
			succeed_flag = -1;
			if (euc_found_flag == TRUE && ret == -1) {
				if ((strcmp(iconv_list[i], EUCKR) == 0) || 
					(strcmp(iconv_list[i], EUCCN) == 0) ||
					(strcmp(iconv_list[i], EUCTW) == 0)) {
					found_target = 0;
					euc_found_flag = FALSE;
				}
			}
		}
		
		free(tbuf);
		iconv_close(cd);

		if (succeed_flag != 0)
			continue;

		strncpy(from_code, iconv_list[i], ENCODING_LENGTH);
		code_num = ReturnCodeNum(from_code);
		if (strcmp(from_code, UTF8) != 0) {
			if (IdentfyEncoding(code_num, buf_size, &found_target, 
				from_code, inputp, UTF8, &root_autoef, 
				&end_auto_ef, input_buf) == -1) {
		
				*aef = NULL;
				free(inputp);
				Free_AUTOEF(&root_autoef);
				return (-1);
			}
			
		} else {
			if (IdentfyEncoding(code_num, buf_size, &found_target, 
				from_code, inputp, UTF16, &root_autoef, 
				&end_auto_ef, input_buf) == -1) {
		
				*aef = NULL;
				free(inputp);
				Free_AUTOEF(&root_autoef);
				return (-1);
			}
		}
			
		if (succeed_flag == 0 && end_auto_ef == 1)
			break;
	}
	
	if (root_autoef != NULL) {
		_auto_ef_t tmpautoef;
		size_t return_size = 0;

		tmpautoef = SortATEFO(root_autoef);
		Free_AUTOEF(&root_autoef);
		ConvScore(&tmpautoef);
		*aef = ATEFO2AUTOEF(tmpautoef, &return_size);
		Free_AUTOEF(&tmpautoef);
		free(inputp);
		return (return_size);
	} else {
		*aef = NULL;
		Free_AUTOEF(&root_autoef);
		free(inputp);
		return ((size_t)0);
	}	

}	

const char *lengthbuf(const char *buf, int *buf_size) {
        const char *p;
        int i;

        p = &buf[0];
        for (i = 0; i < *buf_size; i++) {
                if (*p == '\0') 
                	break;
                else
                	p++;
        }
        *buf_size = i;
        return (p);
}

auto_ef_t *ATEFO2AUTOEF(_auto_ef_t root_autoefp, size_t *size) {
	auto_ef_t *buf;
	_auto_ef_t p = NULL;
	int i = 0;

	for (p = root_autoefp; p != (_auto_ef_t) NULL; p = p->_next_autoef)
		i++;

	if ((buf = (auto_ef_t *)calloc((size_t)i + 1,
		sizeof (auto_ef_t))) == NULL) {

		errno = ENOMEM;
		*size = -1;
		return (NULL);
	}

	for (p = root_autoefp, i = 0; p != (_auto_ef_t) NULL;
		p = p->_next_autoef, i++) {

		buf[i] = (auto_ef_t) malloc(sizeof (AUTOEF));
		if (buf[i] == NULL) {
			errno = ENOMEM;
			auto_ef_free(buf);
			*size = -1;
			return (NULL);
		}

		(buf[i])->score = p->_score;
		strncpy((buf[i])->encoding, p->_encoding, PATH_MAX);
		strncpy((buf[i])->language, M_FromCodeToLang(p->_encoding), PATH_MAX);
	}

	buf[i] = (auto_ef_t)NULL;
	*size = i;
	return (buf);
}

_auto_ef_t SortATEFO(_auto_ef_t rtp) {
	_auto_ef_t buf[ICONV_LOCALE_MAX], buf2[ICONV_LOCALE_MAX];
	double new_score[ICONV_LOCALE_MAX];
	int i = 0, j = 0, max_buf;
	_auto_ef_t p, mid, tmp;
	int sflag;
	_auto_ef_t root_autoef2 = NULL;

	for (p = rtp; p != NULL; p = p->_next_autoef) {
		buf2[j] = p;
		j++;
	}

	max_buf = j;

	for (i = 0; i < max_buf; i++) {
		for (j = 0; j < i; j++) {
			if (buf2[j] != NULL) {
				if (strcmp(buf2[i]->_encoding,
					buf2[j]->_encoding) == 0) {

					buf2[j]->_score += buf2[i]->_score;
					buf2[i] = NULL;
					break;
				}
			}
		}
	}

	j = 0;
	for (i = 0; i < max_buf; i++) {
		if (buf2[i] != NULL) {
			buf[j] = buf2[i];
			j++;
		} else {
			free(buf2[i]);
		}
	}

	max_buf = j;
	QSort_AUTOEF(buf, 0, j-1, rtp);
	for (i = 0; i < max_buf; i++) {
		if (Regist_AUTOEF(buf[i]->_encoding,
			buf[i]->_score, M_FromCodeToLang(buf[i]->_encoding),
			&root_autoef2) == -1) {

			Free_AUTOEF(&root_autoef2);
			return (NULL);
		}
	}

	return (root_autoef2);
}

int Regist_AUTOEF(char *code, double point, char *langdef, _auto_ef_t *root) {

	_auto_ef_t p, lastp;
	_auto_ef_t newrecord = (_auto_ef_t) malloc(sizeof (_AUTOEF));
	if (newrecord == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	strncpy(newrecord->_encoding, code, PATH_MAX);
	if (langdef != NULL) {
		strncpy(newrecord->_language, langdef, PATH_MAX);
	} else {
		strncpy(newrecord->_language, code, PATH_MAX);
	}

	newrecord->_score = point;
	newrecord->_next_autoef = (_auto_ef_t)NULL;
	if (*root == NULL) {
		*root = newrecord;
	} else {
		for (p = *root, lastp = NULL; p != NULL;
			lastp = p, p = p->_next_autoef) {

			/* NULL */;
		}

		lastp->_next_autoef = newrecord;
	}
	return (0);
}

char *M_FromCodeToLang(char *from_code) {
	/*
	 * These strings should be localized when auto_ef support
	 * to identify language.
	 */

	if (strcmp(from_code, ASCII)			== 0)
		return (ASCII);
	if (strcmp(from_code, UTF8)			== 0)
		return (UTF8);
	if (strcmp(from_code, ISOJP)		== 0)
		return ("Japanese");
	if (strcmp(from_code, EUCJP)			== 0)
		return ("Japanese");
	if (strcmp(from_code, PCK)			== 0)
		return ("Japanese");
	if (strcmp(from_code, EUCCN)		== 0)
		return ("Simplified Chinese");
	if (strcmp(from_code, GB18030)		== 0)
		return ("Simplified Chinese");
	if (strcmp(from_code, ISOCN)	== 0)
		return ("Simplified Chinese");
	if (strcmp(from_code, EUCKR)		== 0)
		return ("Korian");
	if (strcmp(from_code, CP949)		== 0)
		return ("Korian");
	if (strcmp(from_code, ISOKR)		== 0)
		return ("Korian");
	if (strcmp(from_code, BIG5)		== 0)
		return ("Traditional Chinese");
	if (strcmp(from_code, EUCTW)		== 0)
		return ("Traditional Chinese");
	if (strcmp(from_code, HKSCS)		== 0)
		return ("Traditional Chinese");
	if (strcmp(from_code, ISOCNEXT)	== 0)
		return ("Traditional Chinese");
	if (strcmp(from_code, I8859_1)			== 0)
		return ("West European");
	if (strcmp(from_code, I8859_2)			== 0)
		return ("East European");
	if (strcmp(from_code, I8859_5)			== 0)
		return ("Cyrillic");
	if (strcmp(from_code, I8859_6)			== 0)
		return ("Arabic");
	if (strcmp(from_code, I8859_7)			== 0)
		return ("Greek");
	if (strcmp(from_code, I8859_8)			== 0)
		return ("Hebrew");
	if (strcmp(from_code, KOI8)			== 0)
		return ("koi8-r");
	if (strcmp(from_code, CP1250)			== 0)
		return ("East European");
	if (strcmp(from_code, CP1251)			== 0)
		return ("Cyrillic");
	if (strcmp(from_code, CP1252)			== 0)
		return ("West European");
	if (strcmp(from_code, CP1253)			== 0)
		return ("Greek");
	if (strcmp(from_code, CP1255)			== 0)
		return ("Hebrew");
	if (strcmp(from_code, CP1256)			== 0)
		return ("Arabic");
	if (strcmp(from_code, CP874)			== 0)
		return ("Thai");
	if (strcmp(from_code, TIS620)			== 0)
		return ("Thai");
	return ("Unknown");
}

void Free_AUTOEF(_auto_ef_t *rtp) {

	_auto_ef_t freep = *rtp, freeq = NULL;

	while (freep != NULL) {
		if ((freep->_next_autoef) != NULL) {
			freeq = freep->_next_autoef;
			free(freep);
			freep = freeq;
		} else {
			free(freep);
			break;
		}
	}

	*rtp = NULL;
}

void ConvScore(_auto_ef_t *rtp) {
	_auto_ef_t p, q, np;
	double percent;
	double sum = 0.0;
	
	for (p = *rtp; p != NULL; p = p->_next_autoef) {
		if (p->_score > 0)
                        sum += p->_score;
        }
        
        if (sum == 0.0){
        	Free_AUTOEF(rtp);
        	return;
        }
        
	for (p = *rtp, q = *rtp; p != NULL;
		q = p, p = p->_next_autoef) {
		
		if (p->_score != FULL) {
			/* arrange score not to over 100 of the total of them */
			percent = (int)((p->_score / sum) * 1000.0) / 10;
			p->_score = percent;
		} else if (p->_score == FULL) {
			percent = FULL;
		}
		
		if (p->_score <= 0.0) {
			Free_AUTOEF(&p);
			q->_next_autoef = NULL;
			break;
		}
	}
	
}

double prob_inte(double x) {
	double p, pp, pn, q;
	double a, b;

	int sign;
	int i, j, k;

	p = a = x;
	pp = pn = 0.0;

	i = 1; j = 3; k = 1; sign = 1;

	do {
		a *= (-x * x);
		b = j * fact(k);

		sign *= (-1);

		if (sign > 0)
			pp += (a / b);

		if (sign < 0)
			pn += (a / b);

		j += 2; k++;

	} while (fabs(a / b) > EPS);

	p += (pp + pn);
	q = 2.0 * p / sqrt(PI);
	return (q);
}

int ReturnCodeNum(char *code) {

	if (strcmp(code, ASCII)			== 0)
		return (0);
	if (strcmp(code, UTF8)			== 0)
		return (1);
	if (strcmp(code, ISOJP)			== 0)
		return (2);
	/*
	 * If create the auto_ef by GNU iconv,
	 * ISO-2022-KR and zh_CN.iso2022-CN
	 * will be 2, the same as ISO-2022-JP
	 */
	if (strcmp(code, ISOKR)			== 0)
		return (5);
	if (strcmp(code, ISOCN)		== 0)
		return (5);
	if (strcmp(code, ISOCNEXT)	== 0)
		return (5);
	if (strcmp(code, EUCJP)			== 0)
		return (3);
	if (strcmp(code, EUCKR)			== 0)
		return (3);
	if (strcmp(code, EUCCN)			== 0)
		return (3);
	if (strcmp(code, EUCTW)			== 0)
		return (3);
	if (strcmp(code, CP949)			== 0)
		return (7);
	if (strcmp(code, GB18030)		== 0)
		return (7);
	if (strcmp(code, HKSCS)			== 0)
		return (7);
	if (strcmp(code, PCK)				== 0)
		return (7);
	if (strcmp(code, I8859_1)			== 0)
		return (8);
	if (strcmp(code, I8859_2)			== 0)
		return (8);
	if (strcmp(code, I8859_5)			== 0)
		return (8);
	if (strcmp(code, I8859_6)			== 0)
		return (8);
	if (strcmp(code, I8859_7)			== 0)
		return (8);
	if (strcmp(code, I8859_8)			== 0)
		return (8);
	if (strcmp(code, CP1250)			== 0)
		return (8);
	if (strcmp(code, CP1251)			== 0)
		return (8);
	if (strcmp(code, CP1252)			== 0)
		return (8);
	if (strcmp(code, CP1253)			== 0)
		return (8);
	if (strcmp(code, CP1255)			== 0)
		return (8);
	if (strcmp(code, CP1256)			== 0)
		return (8);
	if (strcmp(code, CP874)				== 0)
		return (8);
	if (strcmp(code, TIS620)			== 0)
		return (8);
	return (-1);
}

double fact(long n) {
	int i;
	double f = n;
	if (n == 0)
		return (1.0);
	for (i = n-1; i > 0; i--)
		f *= i;

	return (f);
}

void QSort_AUTOEF(_auto_ef_t buf[], int upper, int lower, _auto_ef_t rtp) {
	int i, j;
	_auto_ef_t mid, tmp;
	_auto_ef_t swap_a, swap_b, p;

	i = upper;
	j = lower;
	mid = buf[(i+j)/2];
	do {
		while (buf[i]->_score > mid->_score) i++;
		while (mid->_score > buf[j]->_score) j--;
		if (i <= j) {
			tmp = buf[i];
			buf[i] = buf[j];
			buf[j] = tmp;
			i++; j--;
		}
	} while (i <= j);

	if (upper < j) QSort_AUTOEF(buf, upper, j, rtp);
	if (i < lower) QSort_AUTOEF(buf, i, lower, rtp);
}
