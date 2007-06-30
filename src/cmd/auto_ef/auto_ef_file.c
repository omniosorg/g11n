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

#ident  "@(#)auto_ef_file.c 1.18 07/04/12 SMI"

#include <ctype.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "auto_ef_lib.h"

#define	AUTOEF_BLOCKSIZE	8192
#define	HASHSIZE		8192
#define AUTO_EF_LINE_MAX	65536
#define LEVEL0_LINE	1024
#define LEVEL1_LINE	256
#define LEVEL2_LINE	64
#define LEVEL3_LINE	0

auto_ef_t *execute_file(const char *, int, size_t *);
void CorrectATEFO(_auto_ef_t);
void remove_encoding(_auto_ef_t, char *);
int buflength_file(char *);
int SuperSetOr2022(char *);
void ConvScore_file(_auto_ef_t *);
void CalcAutoefFile(auto_ef_t *, _auto_ef_t *);
void AutoefAddScore(char *, double, _auto_ef_t *);
int AutoefFindKeyWord(char *, _auto_ef_t *);

enum eSuperSetOr2022{ eASCII, eISO_JP, eISO_CNorKR, eISO_Other, 
	eCP949, eGB18030, eHKSCS, eCP874 };

size_t auto_ef_file(auto_ef_t **aef, const char *convert_file_name,
	int auto_ef_flag) {

	struct stat filetype;

	size_t auto_ef_file_size = 0;
	_auto_ef_t root_autoef = (_auto_ef_t) NULL;

	/*
	 * Determine fine type
	 */

	if (convert_file_name != NULL) {
		if (stat(convert_file_name, &filetype) == 0) {
			if (! S_ISREG(filetype.st_mode)) {
				errno = EACCES;
				*aef = NULL;
				return ((size_t)-1);
			}
		} else {
			/* stat() failed, return -1 with errno from stat */
			*aef = NULL;
			return ((size_t)-1);
		}
	}

	*aef = execute_file(convert_file_name, auto_ef_flag, &auto_ef_file_size);

	return (auto_ef_file_size);
}

auto_ef_t *execute_file(const char *file,
	int auto_ef_flag, size_t *return_size)
{
	char	*lbuf, *p;
	long	count;
	long	offset = 0;
	char	*next_ptr = NULL;
	long	next_count = 0;
	char  auto_ef_block[AUTOEF_BLOCKSIZE] = "\0";
	int   auto_ef_offset = 0, auto_ef_blocksize = 0;
	char  *tmpbuf;
	int i;
	int autoef_flag = 0;
	int autoef_times = 0;
	int level_flag = 0;
	int level_max;
	int auto_ef_count = 0;
	
	int level = 0;
	
	long long tln;
	char *prntbuf = NULL;
	long fw_lPrntBufLen = 0;
	int temp;
	char *linebuf;
	long long lnum;
	char *ptr, *ptrend;
	int nlflag;
	auto_ef_t *rootp;
	_auto_ef_t root_autoef_file = NULL;
	
	int auto_ef_overline = 0;
	
	tln = 0;
	
	fw_lPrntBufLen = BUFSIZ + 1;
	if ((prntbuf = malloc(fw_lPrntBufLen)) == NULL) {
		errno = ENOMEM;
		*return_size = (size_t)-1;
		return (NULL);
	}
	
	if ((linebuf = malloc(fw_lPrntBufLen)) == NULL) {
		errno = ENOMEM;
		*return_size = (size_t)-1;
		free(prntbuf);
		return NULL;
	}

	if (file == NULL)
		temp = 0;
	else if ((temp = open(file, O_RDONLY)) == -1) {
		errno = EACCES;
		*return_size = (size_t)-1;
		free(prntbuf);
		free(linebuf);
		return NULL;
	}

	/* read in first block of bytes */
	if ((count = read(temp, prntbuf, BUFSIZ)) <= 0) {
		(void) close(temp);
		errno = ENOMEM;
		*return_size = (size_t)-1;
		free(prntbuf);
		free(linebuf);
		return NULL;
	}

	/* Mac file format */
	for (i = 0; i < count; i++) {
		char a = prntbuf[i];
		char b = (i + 1) < count ? prntbuf[i + 1] : 0;
		if (a == '\r' && b != '\n')
			prntbuf[i] = '\n';
	}

	lnum = 0;
	ptr = prntbuf;
	
	level = auto_ef_flag & 0x3;

	switch (level) {
	case 0:
		level_max = LEVEL0_LINE;
		break;
	case 1:
		level_max = LEVEL1_LINE;
		break;
	case 2:
		level_max = LEVEL2_LINE;
		break;
	default:
		level_max = LEVEL3_LINE;
		break;
	}

	for (;;) {
		if (level_max != 0) {
			level_flag++;
		}
		/* look for next newline */
		if ((ptrend = memchr(ptr + offset, '\n', count)) == NULL) {
			offset += count;

			/*
			 * shift unused data to the beginning of the buffer
			 */

			if (ptr > prntbuf) {
				(void) memmove(prntbuf, ptr, offset);
				ptr = prntbuf;
			}

			/*
			 * re-allocate a larger buffer if this one is full
			 */
			 
			if (fw_lPrntBufLen < AUTO_EF_LINE_MAX) {

				if (offset + BUFSIZ > fw_lPrntBufLen) {
					/*
					 * allocate a new buffer and preserve the
					 * contents...
					 */
					fw_lPrntBufLen += BUFSIZ;

					if ((prntbuf = realloc(prntbuf, fw_lPrntBufLen))
						== NULL) {
	
						errno = ENOMEM;
						*return_size = (size_t)-1;
						free(prntbuf);
						free(linebuf);
						return NULL;
					}
					/*
					 * set up a bigger linebuffer
					 * (this is only used
					 * for case insensitive
					 * operations). Contents do
					 * not have to be preserved.
					 */
					free(linebuf);
					if ((linebuf = malloc(fw_lPrntBufLen))
						== NULL) {
	
						errno = ENOMEM;
						*return_size = (size_t)-1;
						free(prntbuf);
						free(linebuf);
						return NULL;
					}

					ptr = prntbuf;
				}

				p = prntbuf + offset;
				if ((count = read(temp, p, BUFSIZ)) > 0) {
					/* Mac file format */
					for (i = 0; i < BUFSIZ; i++) {
						char a = p[i];
						char b = (i + 1) < count ? p[i + 1] : 0;
						if (a == '\r' && b != '\n')
							p[i] = '\n';
					}
					continue;
				}

				if (offset == 0)
					/* end of file already reached */
					break;

				/* last line of file has no newline */
				ptrend = ptr + offset;
				nlflag = 0;

			} else {
				/*
				char tmpbuf[BUFSIZ+1];
				*/
				char *a;

				a = prntbuf;
				
				auto_ef_overline = 1;
				if ((tmpbuf = (char *)malloc(fw_lPrntBufLen + 1)) == NULL) {
					errno = ENOMEM;
					*return_size = (size_t)-1;
					free(prntbuf);
					free(linebuf);
					return NULL;
				}

				strlcpy(tmpbuf, ptr, fw_lPrntBufLen + 1);
				
				for(;;){
					count = read(temp, a, count);
					if ((ptrend = memchr(a, '\n', count)) != NULL)
						break;
					offset += count;
					if (count == 0) break;
				}
				if (ptrend != NULL){
					next_ptr = ptrend + 1;
					next_count = count - (next_ptr - ptr);
					nlflag = 1;
				} else {
					next_count = 0;
					nlflag = 0;
				}
			}
			
		} else {
			next_ptr = ptrend + 1;
			next_count = offset + count - (next_ptr - ptr);
			nlflag = 1;
		}
		lnum++;
		if (ptrend != NULL)
			*ptrend = '\0';

		if (auto_ef_overline){
			auto_ef_overline = 0;
		
		} else {
		
			if ((tmpbuf = (char *)malloc(fw_lPrntBufLen + 1)) == NULL) {
				errno = ENOMEM;
				*return_size = (size_t)-1;
				free(prntbuf);
				free(linebuf);
				return NULL;
			}

			strlcpy(tmpbuf, ptr, fw_lPrntBufLen + 1);
		}

		auto_ef_offset = buflength_file(tmpbuf);
		auto_ef_offset++;
		auto_ef_blocksize = auto_ef_offset + auto_ef_blocksize;

		if (auto_ef_blocksize > AUTOEF_BLOCKSIZE) {
			size_t atefsize = 0;
			if (auto_ef_offset < AUTOEF_BLOCKSIZE) {

				/*
				 * In case of auto_ef_block is
				 * full and next get block is not
				 * larger than auto_ef_block
				 */
				if (level_max == 0 ||
					(autoef_times % level_max == 0) ||
					autoef_times == 0) {

					atefsize = auto_ef_str(&rootp,
						auto_ef_block, AUTOEF_BLOCKSIZE,
						level);
					if (atefsize == (size_t)0) {
						auto_ef_free(rootp);
					} else if (atefsize == -1) {
						auto_ef_free(rootp);
					} else {
						CalcAutoefFile(rootp, &root_autoef_file);
						auto_ef_free(rootp);
						autoef_flag = 0;
						autoef_times++;
					}
				}
				strncpy(auto_ef_block, (const char *)tmpbuf, AUTOEF_BLOCKSIZE);
				auto_ef_blocksize = auto_ef_offset;
			} else {
				/*
				 * In case of auto_ef_block
				 * is full and next get block
				 * is larger than auto_ef_block
				 */
				if (level_max == 0 ||
					(autoef_times % level_max == 0) ||
					autoef_times == 0) {

					atefsize = auto_ef_str(&rootp, tmpbuf,
						auto_ef_offset, level);

					if (atefsize == (size_t)0) {
						auto_ef_free(rootp);
					} else if (atefsize == -1) {
						auto_ef_free(rootp);
					} else {
						CalcAutoefFile(rootp, &root_autoef_file);
						auto_ef_free(rootp);
						auto_ef_blocksize = 0;
						autoef_flag = 0;
						autoef_times++;
					}
				}
				auto_ef_block[0] = '\0';
				auto_ef_blocksize = 0;
			}
		} else {
			strncat(auto_ef_block, (const char *)tmpbuf, AUTOEF_BLOCKSIZE);
			autoef_flag = 1;
		}

		free(tmpbuf);
		if (!nlflag)
			break;

		ptr = next_ptr;
		count = next_count;
		offset = 0;
		if (fw_lPrntBufLen > AUTO_EF_LINE_MAX)
			fw_lPrntBufLen = BUFSIZ + 1;
	}
	
	free(tmpbuf);
	free(prntbuf);
	free(linebuf);

	(void) close(temp);
	if (autoef_flag == 1) {
		size_t atefsize = 0;
		autoef_times++;
		atefsize = auto_ef_str(&rootp, auto_ef_block,
			AUTOEF_BLOCKSIZE, level);
		if (atefsize == (size_t)0) {
			auto_ef_free(rootp);
		} else if (atefsize == -1) {
			auto_ef_free(rootp);
		} else {
			CalcAutoefFile(rootp, &root_autoef_file);
			auto_ef_free(rootp);
		}
	}

	if (root_autoef_file != NULL) {
		auto_ef_t *tmp = NULL;
		size_t autoef_size;
		_auto_ef_t tmpautoef;

		CorrectATEFO(root_autoef_file);
		tmpautoef = SortATEFO(root_autoef_file);
		Free_AUTOEF(&root_autoef_file);
		ConvScore_file(&tmpautoef);
		tmp = ATEFO2AUTOEF(tmpautoef, &autoef_size);
		Free_AUTOEF(&tmpautoef);
		*return_size = autoef_size;
		return (tmp);
	} else {
		*return_size = (size_t)0;
		Free_AUTOEF(&root_autoef_file);
		return (NULL);
	}	

}

void CorrectATEFO(_auto_ef_t rtp) {
	/* Remove downward compatibility if the other encoding is included */
	_auto_ef_t p;
	int codeid;
	int ascii = FALSE, iso2022 = FALSE, cp949_gbk = FALSE;
	int gb18030 = FALSE, hkscs = FALSE, others = FALSE;
	int iso2022_krcn = FALSE;
	int cp874 = FALSE;

	for (p = rtp; p != NULL; p = p->_next_autoef) {
		switch (SuperSetOr2022(p->_encoding)) {
		case 0:
			ascii = TRUE;
			break;
		case 1:
			iso2022 = TRUE;
			break;
		case 2:
			iso2022 = TRUE;
			iso2022_krcn = TRUE;
			break;
		case 4:
			cp949_gbk = TRUE;
			break;
		case 5:
			gb18030 = TRUE;
			break;
		case 6:
			hkscs = TRUE;
			break;
		case 7:
			cp874 = TRUE;
			break;			
		default:
			others = TRUE;
		}
	}

	if ((ascii == TRUE) && (iso2022 == TRUE || cp949_gbk == TRUE ||
		gb18030 == TRUE || hkscs == TRUE || others == TRUE)) {

		/* remove ascii */
		remove_encoding(rtp, ASCII);
	}

	if (iso2022 == TRUE) {
		if (iso2022_krcn == TRUE) {
			remove_encoding(rtp, ISOJP);
		}
	}

	if (cp949_gbk == TRUE) {
		/* remove euc */
		remove_encoding(rtp, EUCJP);
		remove_encoding(rtp, EUCKR);
		remove_encoding(rtp, EUCCN);
		remove_encoding(rtp, EUCTW);
	}

	if (gb18030 == TRUE) {
		/* remove euc */
		remove_encoding(rtp, EUCJP);
		remove_encoding(rtp, EUCKR);
		remove_encoding(rtp, EUCCN);
		remove_encoding(rtp, EUCTW);
	}

	if (hkscs == TRUE) {
		/* remove big5 */
		remove_encoding(rtp, BIG5);
	}
	
	if (cp874 == TRUE) {
		/* remove TIS620.2533 */
		remove_encoding(rtp, TIS620);
	}
}

void remove_encoding(_auto_ef_t rtp, char *a) {
	_auto_ef_t p;

	for (p = rtp; p != NULL; p = p->_next_autoef) {
		if (strcmp(p->_encoding, a) == 0) {
			p->_score = 0;
			break;
		}
	}
}

int buflength_file(char *buf) {
	int i;

	for (i = 0; ; i++) {
		if (buf[i] == '\0') break;
	}

	return (i);
}

void CalcAutoefFile(auto_ef_t *autoefp, _auto_ef_t *root_autoef_file) {
	auto_ef_t *p;
	int flag;

	for (p = autoefp; *p != NULL; p++) {
		if ((AutoefFindKeyWord(auto_ef_get_encoding(*p), root_autoef_file)) == TRUE) {
			AutoefAddScore(auto_ef_get_encoding(*p),
				auto_ef_get_score(*p), root_autoef_file);
		} else {
			/*
			 * This Regist_AUTOEF can be void
			 */
			(void) Regist_AUTOEF(auto_ef_get_encoding(*p),
				auto_ef_get_score(*p),
				M_FromCodeToLang(auto_ef_get_encoding(*p)),
				root_autoef_file);
		}
	}
}

int SuperSetOr2022(char *code) {

	if (strcmp(code, ASCII)		== 0)
		return (eASCII);
	if (strcmp(code, ISOJP)		== 0)
		return (eISO_JP);
	if (strcmp(code, ISOKR)		== 0)
		return (eISO_CNorKR);
	if (strcmp(code, ISOCN)	== 0)
		return (eISO_CNorKR);
	if (strcmp(code, CP949)		== 0)
		return (eCP949);
	if (strcmp(code, GB18030)	== 0)
		return (eGB18030);
	if (strcmp(code, HKSCS)		== 0)
		return (eHKSCS);
	if (strcmp(code, CP874)		== 0)
		return (eCP874);		
	return (-1);
}

void ConvScore_file(_auto_ef_t *rtp) {
	_auto_ef_t p, q, np;
	double percent = 0.0;
	double sum = 0.0;
	int j = 0;
	int times_flag = 0;

	for (p = *rtp; p != NULL; p = p->_next_autoef) {
		if (p->_score > 0) {
			sum += p->_score;
			j++;
			if (fmod(p->_score, 100.0) != 0.0)
				times_flag = -1;
		}
	}

	if (times_flag >= 0 && j > 0) {
		(*rtp)->_score = 100.0;
		if ((*rtp)->_next_autoef != NULL) {
			Free_AUTOEF(&((*rtp)->_next_autoef));
		}
		return;
	}


	for (p = *rtp, q = *rtp; p != NULL;
			q = p, p = p->_next_autoef) {
		if (p->_score != 100.0) {
			percent = (int)((p->_score / sum) * 1000.0) / 10;
			p->_score = percent;
		}
		if (p->_score <= 0.0) {
			Free_AUTOEF(&p);
			q->_next_autoef = NULL;
			break;
		}
	}
}

void AutoefAddScore(char *a, double addscore, _auto_ef_t *root_autoef_file) {
        _auto_ef_t p;

        for (p = *root_autoef_file; p != NULL; p = p->_next_autoef) {
                if (strcmp(p->_encoding, a) == 0) {
                        p->_score = p->_score + addscore;
                }
        }
}

int AutoefFindKeyWord(char *a, _auto_ef_t *root_autoef_file) {
        _auto_ef_t p;

        for (p = *root_autoef_file; p != NULL; p = p->_next_autoef) {
                if (strcmp(p->_encoding, a) == 0)
                        return (TRUE);
        }

        return (FALSE);
}

