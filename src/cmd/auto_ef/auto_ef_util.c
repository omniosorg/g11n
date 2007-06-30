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

#ident  "@(#)auto_ef_util.c 1.18 07/04/12 SMI"
#include "auto_ef_lib.h"

int IdentfyEncoding(int, size_t, int *, char *, char *, const char *,
	_auto_ef_t *, int *, char *);
int IsSingleByte_buf(const char *, double *, char *, int, srd *,
	double *, double *, _auto_ef_t *);
int IsHKSCSOrBIG5(char *, const char *, char *, size_t, _auto_ef_t *, double);
int RegistBIG5(char *, size_t, char *, const char *, _auto_ef_t *,
	srd *, double *, double *);
int RegistEUC(char *, int, char *, _auto_ef_t *, srd *, double *, double *);
int RegistHashTable(unsigned char, unsigned char, srd *);
int HashTableOpen(char *, srd *, double *, double *);
void get_hash_name(char *, char *);
int Regist_ASCII_ISO2022JP(int, char *, _auto_ef_t *);
int IsAsciiOr2022_buf(const char *, int, char *);
int Is2022KROrCN(int, char *, char *, size_t, _auto_ef_t *);
int Hash(unsigned char, unsigned char);
int TotalScore_buf(const char *, double *, int, srd *, double *, double *);
int FindKeyWord(unsigned char, unsigned char, srd *);
int GetScore(unsigned char, unsigned char, srd *);
double Calc_SD(int, double *, double *);
char *chopbuf(char *);
void FreeHashTable(srd *);
int CheckISO2022CN(unsigned char, unsigned char, unsigned char, unsigned char);
int CheckISO2022KR(unsigned char, unsigned char, unsigned char, unsigned char);
void ThaiSpecificCheck(const char *, char *, size_t);

/*
 Map from Single Byte encoding to Language
 */
const char single_byte_langs[SINGLE_ENCODING_MAX][ICONV_LOCALE_MAX][ENCODING_LENGTH] = {
  /* ISO-8859-1 : 0 */
  {"Germany", "Spain", "France", "Italy", "Sweden", "Denmark", "Finland",
   "Iceland", "Catalonia", "Netherland", "Norway", "Portugal"},
  /* ISO-8859-2 : 1 */
  {"Croatia", "Hungary", "Poland", "Serbia", "Slovakia", "Slovenia"},
  /* ISO-8859-5 : 2 */
  {"Bulgaria", "Russia"},
  /* ISO-8859-6 : 3 */
  {"Arabia"},
  /* ISO-8859-7 : 4 */
  {"Greece"},
  /* ISO-8859-8 : 5 */
  {"Hebrew"},
  /* koi8_r : 6 */
  {"Russia"},
  /* CP1250 : 7 */
  {"Croatia", "Hungary", "Poland", "Serbia", "Slovakia", "Slovenia"},
  /* CP1251 : 8 */
  {"Bulgaria", "Russia"},
  /* CP1252 : 9 */
  {"Germany", "Spain", "France", "Italy", "Sweden", "Denmark", "Finland", 
   "Iceland", "Catalonia", "Netherland", "Norway", "Portugal"},
  /* CP1253 : 10 */
  {"Greece"},
  /* CP1255 : 11 */
  {"Hebrew"},
  /* CP1256 : 12 */
  {"Arabia"},
  /* CP874 : 14 */
  {"Thai"},
  /* TIS620.2533 : 15 */
  {"Thai"} 
};

extern const char *to_code;
const char roothash[64]={
	"/usr/lib/auto_ef/hashtable."
};

int IdentfyEncoding(int code_num, size_t buf_size,
	int *found_target, char *from_code, char *inputp, const char *to_code,
	_auto_ef_t *root_autoef, int *end_auto_ef, char *input_buf) {
	
	int i, utf_flag;
	char hashfilename[PATH_MAX];
	srd hashtable[HASHSIZE];
	double average = 0.0, SD = 0.0;
	
	for (i = 0; i < HASHSIZE; i++)
		hashtable[i] = NULL;
	
	for (i = 0; i < PATH_MAX; i++)
		hashfilename[i] = '\0';

	switch (code_num) {
	case 1: 
		/* UTF-8 */
		*found_target = 1;
		utf_flag = 0;
		for (i = 0; i < (int)buf_size; i++) {
			if (inputp[i] == '\0')
				break;
			if ((unsigned char)inputp[i] > 127) {
				utf_flag = 1;
				break;
			}
		}

		if (Is2022KROrCN(utf_flag, from_code, input_buf, buf_size, root_autoef) != -1)
				*end_auto_ef = 1;
					

		break;
		
	case 2: /* ISO-2022-JP or ASCII */
		*found_target = 1;
		
		i = IsAsciiOr2022_buf(inputp, buf_size, from_code);
		if (Regist_ASCII_ISO2022JP(i, from_code, root_autoef) == -1)
			return (-1);
			
		*end_auto_ef = 1;
		break;
	
	case 3: /* EUC series */
		get_hash_name(hashfilename, from_code);
		if (HashTableOpen(hashfilename, hashtable,
			&average, &SD) >= 0) {
			
			*found_target = RegistEUC(from_code, buf_size, inputp, 
				root_autoef, hashtable, &average, &SD);
				
			if (*found_target == -1)
				return (-1);
		} else {
			errno = EACCES;
			return (-1);
		}
		FreeHashTable(hashtable);
		break;
	
	case 7: /* PCK, zh_HK.hkscs, GB18030, ISO-2022-KR, zh_CN.iso2022-CN */
		get_hash_name(hashfilename, from_code);
		if (HashTableOpen(hashfilename, hashtable,
			&average, &SD) >= 0) {
			
			if (RegistBIG5(from_code, buf_size,
				inputp, to_code, root_autoef, hashtable,
				&average, &SD) == -1)

				return (-1);
		} else {
			errno = EACCES;
			return (-1);
		}
		FreeHashTable(hashtable);
		break;
	
	case 8: /* 8859 or CP series */
		if (!*found_target) {
			double total_score = 0.0;
			double single_byte_score = 0.0;
			double highest_score = -3.0;
			int i;

			if (IsSingleByte_buf(inputp, &total_score,
				from_code, buf_size, hashtable,
				&average, &SD, root_autoef) == -1) {

				return (-1);
			}
		}
		break;

	default:
		errno = EACCES;
		return (-1);	

	}
	
	return (0);
}

int IsSingleByte_buf(const char *input_buf,
	double *total_score, char *encoding, int buf_size, srd *hashtable,
	double *average, double *SD, _auto_ef_t *root_autoef)
{
	FILE *fp;
	int i;
	int sflag = -1;
	char tablename[PATH_MAX];

	double highest_score = -3.0;

	if (strcmp(encoding, I8859_1) == 0) sflag = 0;
	if (strcmp(encoding, I8859_2) == 0) sflag = 1;
	if (strcmp(encoding, I8859_5) == 0) sflag = 2;
	if (strcmp(encoding, I8859_6) == 0) sflag = 3;
	if (strcmp(encoding, I8859_7) == 0) sflag = 4;
	if (strcmp(encoding, I8859_8) == 0) sflag = 5;
	if (strcmp(encoding, KOI8)    == 0) sflag = 6;
	if (strcmp(encoding, CP1250)  == 0) sflag = 7;
	if (strcmp(encoding, CP1251)  == 0) sflag = 8;
	if (strcmp(encoding, CP1252)  == 0) sflag = 9;
	if (strcmp(encoding, CP1253)  == 0) sflag = 10;
	if (strcmp(encoding, CP1255)  == 0) sflag = 11;
	if (strcmp(encoding, CP1256)  == 0) sflag = 12;
	if (strcmp(encoding, CP874)  == 0) sflag = 14;
	if (strcmp(encoding, TIS620)  == 0) sflag = 15;

	for (i = 0; i < ICONV_LOCALE_MAX; i++) {
		if (single_byte_langs[sflag][i][0] == '\0')
			break;
		*total_score = 0.0;

		get_hash_name(tablename, encoding);
		strcat(tablename, "_");
		strcat(tablename, "\0");
		strcat(tablename, single_byte_langs[sflag][i]);
		strcat(tablename, "\0");

		if (HashTableOpen(tablename, hashtable, average, SD) >= 0) {
			if (TotalScore_buf(input_buf, total_score, buf_size,
				hashtable, average, SD) >= 0) {
				
				/*			*/
				/* encoding specific 	*/
				/*			*/

				if (sflag == 14 || sflag == 15) {
					ThaiSpecificCheck(input_buf, encoding, buf_size);
				}

				if (*total_score != 0.0) {
					if (Regist_AUTOEF(encoding, *total_score,
						(char *)single_byte_langs[sflag][i],
						root_autoef) == -1) {
						
						errno = ENOMEM;
						return (-1);
					}
				}
			}

		} else {
			errno = EACCES;
			return (-1);
		}
		FreeHashTable(hashtable);
	}
	return (0);
}


int IsHKSCSOrBIG5(char *from_code, const char *to_code,
	char *inputp, size_t buf_size, _auto_ef_t *root_autoef, double total_score)
{

	iconv_t cd;
	const char *context;
	char *convert;
	char *tbuf, *retbuf;
	size_t fsize, tsize;
	size_t ret;
	size_t comp_size = 0;
	const char *p;
	int succeed_flag = 0;
	
	int tmp_bufsize = 0;

	if ((cd = iconv_open(to_code, BIG5)) == (iconv_t)-1) {
		/* Use iconv_open errno */
		return (-1);
	}

	errno = 0;
	p = context = &inputp[0];
	while (*p)
		if ( *p == '\0') {
			break;
		} else p++;
		
	fsize = p - &inputp[0];
	comp_size = fsize;
	tsize = fsize * 4;
	if ((tbuf = (char *) malloc(tsize)) == NULL) {
		errno = ENOMEM;
		iconv_close(cd);
		return (-1);
	}
	tbuf[0] = '\0';
	convert = &tbuf[0];
	ret = iconv(cd, &context, &fsize, &convert, &tsize);
	if (ret == (size_t) -1)
		succeed_flag = -1;
	else
		*convert = '\0';
	iconv_close(cd);

	if (succeed_flag != -1) {
		if ((cd = iconv_open(BIG5, to_code)) == (iconv_t)-1) {
			/* Use iconv_open errno */
			free(tbuf);
			return (-1);
		}

		errno = 0;
		p = context = &tbuf[0];
		while (*p)
			if (*p == '\0'){
				break;
			} else p++;
			
		fsize = p - &tbuf[0];
		tsize = fsize * 4;
		if ((retbuf = (char *) malloc(tsize)) == NULL) {
			errno = ENOMEM;
			iconv_close(cd);
			free(tbuf);
			return (-1);
		}
		retbuf[0] = '\0';
		convert = &retbuf[0];
		ret = iconv(cd, &context, &fsize, &convert, &tsize);

		if (ret == (size_t) -1)
			succeed_flag = -1;
		else
			*convert = '\0';
		iconv_close(cd);

		if (succeed_flag != -1) {
			if (strcmp(inputp, retbuf) == 0) {
				if (Regist_AUTOEF(BIG5, total_score,
					M_FromCodeToLang(from_code),
					root_autoef) == -1) {
					
					free(tbuf);
					free(retbuf);
					return (-1);
				}
			} else {
				if (Regist_AUTOEF(from_code, total_score,
					M_FromCodeToLang(from_code),
					root_autoef) == -1) {
					
					free(tbuf);
					free(retbuf);
					return (-1);
				}
			}
		}
	}
	free(tbuf);
	free(retbuf);
	return (0);
}


int RegistBIG5(char *from_code, size_t buf_size,
	char *inputp, const char *to_code, _auto_ef_t *root_autoef,
	srd *hashtable, double *average, double *SD)
{
	double total_score = 0.0;

	if (TotalScore_buf(inputp, &total_score, buf_size, 
		hashtable, average, SD) >= 0) {
		if (total_score != 0.0) {
			/* If the encoding is zh_HK.hkscs, have to check */
			/* the buf have extended code point from zh_TW.BIG5 */
			if (strcmp(from_code, HKSCS) == 0) {
				if (IsHKSCSOrBIG5(from_code, to_code, 
					inputp, buf_size, root_autoef, total_score) == -1)

					return (-1);
			} else {
				if (Regist_AUTOEF(from_code, total_score,
					M_FromCodeToLang(from_code),
					root_autoef) == -1) {

					return (-1);
				}
			}
		}
	}
	return (0);
}


int RegistEUC(char *from_code, int buf_size, char *inputp,
	_auto_ef_t *root_autoef, srd *hashtable,
	double *average, double *SD) {
	
	double total_score = 0.0;
	int found_target = 0;

	if (TotalScore_buf(inputp, &total_score, buf_size,
		hashtable, average, SD) >= 0) {
		if (total_score != 0.0) {
			found_target = 1;
			if (Regist_AUTOEF(from_code, total_score,
				M_FromCodeToLang(from_code),
				root_autoef) == -1) {

				return (-1);
			}
		}
	}
	return (found_target);
}

int RegistHashTable(unsigned char a, unsigned char b, srd *hashtable) {
	int i;
	srd p, lastp;
	int hashval;

	srd newrecordsrd = (srd) malloc(sizeof (SRD));
	if (newrecordsrd == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	newrecordsrd->keyword[0] = a;
	newrecordsrd->keyword[1] = b;
	newrecordsrd->score = 1;

	hashval = Hash(a, b);
	if (hashtable[hashval] == NULL) {
		hashtable[hashval] = newrecordsrd;
		newrecordsrd->nextsrd = NULL;
	} else {
		p = hashtable[hashval];
		while (p->nextsrd != NULL) {
			p = p->nextsrd;
		}
		p->nextsrd = newrecordsrd;
		newrecordsrd->nextsrd = NULL;
	}
	
	return (0);
}


int HashTableOpen(char *table, srd *hashtable, double *average,
	double *SD) {
	
	FILE *fp;
	char buf[LONG_BIT];
	int i;
	int tableline = 0;
	int hash_score = 0;
	int total_ent = 0;
	double sum_of_score = 0.0;
	double sum_of_deviation = 0.0;
	srd srdp;

	if ((fp = fopen(table, "r")) == NULL) {
		errno = EACCES;
		return (-1);
	}
	
	while (fgets(buf, LONG_BIT, fp) != NULL) {
		char *p;
		srd srdp;
		unsigned char point[3];
		unsigned char keyword_a, keyword_b;

		chopbuf(buf);

		if (tableline == 3)
			tableline = 0;
			
		switch (tableline) {
		case 0:
			break;
		case 1:
			point[0] = buf[0]; point[1] = buf[1]; point[2] = '\0';
			keyword_a =
				(unsigned char)strtol((const char *)point,
				(char **)NULL, 16);

			point[0] = buf[2]; point[1] = buf[3]; point[2] = '\0';
			keyword_b =
				(unsigned char)strtol((const char *)point,
				(char **)NULL, 16);

			if (RegistHashTable(keyword_a, keyword_b,
				hashtable) == -1) {
				
				errno = EACCES;
				return (-1);
			}
			break;
		case 2:
			hash_score = atoi(buf);
			for (srdp = hashtable[Hash(keyword_a, keyword_b)];
				srdp != NULL; srdp = srdp->nextsrd) {
				if ((srdp->keyword[0] == keyword_a) &&
					(srdp->keyword[1] == keyword_b)) {

					srdp->score = hash_score;
					total_ent++;
					sum_of_score += hash_score;
					break;
				}
			}
			break;
		}
		tableline++;
	}

	*average = sum_of_score / (double)total_ent;

	for (i = 0; i < HASHSIZE; i++) {
		for (srdp = hashtable[i]; srdp != NULL; srdp = srdp->nextsrd) {
			sum_of_deviation +=
				((double)srdp->score - *average) *
				((double)srdp->score - *average);
		}
	}
	*SD = sqrt(sum_of_deviation/(total_ent -1));
	fclose(fp);
	return (0);
}

void get_hash_name(char *hashfile, char *encoding) {
	strcpy(hashfile, roothash);
	strcat(hashfile, "\0");
	strcat(hashfile, encoding);
	strcat(hashfile, "\0");
}

int Regist_ASCII_ISO2022JP(int i, char *from_code, _auto_ef_t *root_autoef) {
	switch (i) {
	case 0:
		if (Regist_AUTOEF(ASCII, FULL, ASCII, root_autoef) == -1)
			return (-1);
		break;
	case 1:
		if (Regist_AUTOEF(from_code, FULL, M_FromCodeToLang(from_code), root_autoef) == -1)
			return (-1);
		break;
	case -1:
		/*
		 * errno is from IsAsciiOr2022_buf
		 */
		return (-1);
	}
	return (0);
}

int IsAsciiOr2022_buf(const char *input_buf, int buf_size,
	char *from_encoding) {

	char *tbuf;
	iconv_t cd;
	char *convert;
	const char *context, *p;
	size_t fsize, tsize, ret;
	int i;
	int tmp_bufsize = 0;

	if ((cd = iconv_open(UTF8, from_encoding)) == (iconv_t)-1) {
		/* Use iconv_open errno */
		iconv_close(cd);
		return (-1);
	}

	errno = 0;
	context = &input_buf[0];
	tmp_bufsize = buf_size;
	p = lengthbuf(input_buf, &tmp_bufsize);
	fsize = p - &input_buf[0];
	tsize = fsize * 4;
	if ((tbuf = (char *) malloc(tsize)) == NULL) {
			errno = ENOMEM;
			iconv_close(cd);
			return (-1);
		}

	tbuf[0] = '\0';
	convert = &tbuf[0];

	ret = iconv(cd, &context, &fsize, &convert, &tsize);
	if (ret == (size_t) -1) {
		iconv_close(cd);
		errno = EINVAL;
		free(tbuf);
		return (-1);
	}

	*convert = '\0';
	for (i = 0; i < buf_size; i++) {
		if (tbuf[i] == '\0') break;
		if ((unsigned char)tbuf[i] > 127) {
			iconv_close(cd);
			free(tbuf);
			return (1);
		}
	}
	iconv_close(cd);
	free(tbuf);
	return (0);
}

int Is2022KROrCN(int utf_flag, char *from_code, char *inputp, 
	size_t buf_size, _auto_ef_t *root_autoef) {
	
	int i;
	char iso_2022_encoding[PATH_MAX];
	
	switch (utf_flag) {
	case 0: 
		/* For ISO-2022-KR, CN/CN-EXT encoding */
		for (i = 0; i < buf_size; i++) {
			unsigned char fst, snd, trd, fth;

			fst = (unsigned char) inputp[i];
			snd = (unsigned char)inputp[i+1];
			trd = (unsigned char)inputp[i+2];
			fth = (unsigned char)inputp[i+3];
			if (CheckISO2022KR(fst, snd, trd, fth) == 1) {
				if (Regist_AUTOEF(ISOKR, FULL, M_FromCodeToLang(ISOKR),
					root_autoef) != -1) {

					return (0);
				}
				break;
			} else if (CheckISO2022CN(fst, snd, trd, fth) == 1) {
				if (Regist_AUTOEF(ISOCN, FULL, M_FromCodeToLang(ISOCN),
					root_autoef) != -1) {

					return (0);
				}
				break;
			}
		}
		break;

	case 1: /* Not ISO-2022-KR, CN/CN-EXT is UTF-8 */
		if (Regist_AUTOEF(from_code, FULL, from_code, root_autoef) != -1) {
			return (0);
		}
		break;
	}
	return (-1);
}

int Hash(unsigned char a, unsigned char b) {
	unsigned int hashval = 0;
	hashval = (unsigned int)a + (unsigned int)b;
	return (hashval % HASHSIZE);
}


int TotalScore_buf(const char *input_buf, double *total_score,
	int buf_size, srd *hashtable, double *average, double *SD) {
	
	int i;
	int score = 0;
	int found = 0;

	*total_score = 0.0;

	for (i = 0; i < buf_size - 1; i++) {
		unsigned char keywords[2];
		if (input_buf[i] == '\0') break;

		if ((unsigned)input_buf[i] < MSBFLAG)
			continue;

		if (i == 0 && input_buf[i+1] != '\0' ) {
			if ((FindKeyWord(input_buf[i], input_buf[i+1], hashtable)) == TRUE) {
				score = GetScore(input_buf[i], input_buf[i+1], hashtable);
				if (score != -1) {
					*total_score += Calc_SD(score, average, SD);
					found = 1;
				}
			}

		} else {
			if (input_buf[i+1] != '\0') {
				if ((FindKeyWord(input_buf[i], input_buf[i+1], hashtable)) == TRUE) {
					score = GetScore(input_buf[i], input_buf[i+1], hashtable);
					if (score != -1) {
						*total_score += Calc_SD(score, average, SD);
						found = 1;
					}
				}
			}
				
			if ((FindKeyWord(input_buf[i-1], input_buf[i], hashtable)) == TRUE) {
				score = GetScore(input_buf[i-1], input_buf[i], hashtable);
				if (score != -1) {
					*total_score += Calc_SD(score, average, SD);
					found = 1;
				}
			}
		}
	}
	return (found);
}

int FindKeyWord(unsigned char a, unsigned char b, srd *hashtable) {
	
	srd srdp;

	for (srdp = hashtable[Hash(a, b)]; srdp != NULL; srdp = srdp->nextsrd) {
		if ((srdp->keyword[0] == a) && (srdp->keyword[1] == b)) {
			return (TRUE);
		}
	}
	return (FALSE);
}

int GetScore(unsigned char a, unsigned char b, srd *hashtable) {
	srd srdp;

	for (srdp = hashtable[Hash(a, b)]; srdp != NULL;
		srdp = srdp->nextsrd) {
		if ((srdp->keyword[0] == a) && (srdp->keyword[1] == b)) {
			return (srdp->score);
		}
	}
	return (-1);
}

double Calc_SD(int score, double *average, double *SD) {
	double z_score;
	int SD_index = 0;

	z_score = ((double)score - *average)/(*SD);
	return (z_score);
}

char *chopbuf(char *buf) {
	char *p;

	p = &buf[0];

	while (*p)
		if (*p == '\n') {
			*p = '\0';
			break;
		} else {
			p++;
		}

	return (p);
}

void FreeHashTable(srd *hashtable) {

	int i;
	srd p, q;

	for (i = 0; i < HASHSIZE; i++) {
		for (p = hashtable[i]; p != NULL; ) {
			if ((p->nextsrd) != NULL) {
				q = p->nextsrd;
				free(p);
				p = q;
			} else {
				free(p);
				break;
			}
		}
		hashtable[i] = (srd)NULL;
	}
}

int CheckISO2022CN(unsigned char a, unsigned char b,
	unsigned char c, unsigned char d)
{

	if (a == 0x1b && b == 0x24 && c == 0x29 && d == 0x41 ||
	a == 0x1b && b == 0x24 && c == 0x29 && d == 0x47 ||
	a == 0x1b && b == 0x24 && c == 0x2a && d == 0x48 ||
	a == 0x1b && b == 0x24 && c == 0x29 && d == 0x45 ||
	a == 0x1b && b == 0x24 && c == 0x2b && d == 0x49 ||
	a == 0x1b && b == 0x24 && c == 0x2b && d == 0x4a ||
	a == 0x1b && b == 0x24 && c == 0x2b && d == 0x4b ||
	a == 0x1b && b == 0x24 && c == 0x2b && d == 0x4c ||
	a == 0x1b && b == 0x24 && c == 0x2b && d == 0x4d) {
		return (1);
	} else {
		return (0);
	}
}

int CheckISO2022KR(unsigned char a, unsigned char b,
	unsigned char c, unsigned char d)
{
	if (a == 0x1b && b == 0x24 && c == 0x29 && d == 0x43) {
		return (1);
	} else {
		return (0);
	}
}

void ThaiSpecificCheck(const char *input_buf, char *encoding, size_t buf_size)
{
	int i = 0;
	unsigned char a = 0;
	for (i=0; i < buf_size; i++) {
		if (input_buf[i] == '\0') break;
		a = (unsigned char) input_buf[i];
		if (a == 0x80 || a == 0x85 || a == 0x91 ||
			a == 0x92 || a == 0x93 || a == 0x94 ||
			a == 0x95 || a == 0x96 || a == 0x97) {
			strlcpy(encoding, CP874, ENCODING_LENGTH);
			return;
		}
	}
	
	strlcpy(encoding, TIS620, ENCODING_LENGTH);
}
