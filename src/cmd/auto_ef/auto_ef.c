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

#ident  "@(#)auto_ef.c 1.10 07/04/12 SMI"

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <locale.h>
#include <memory.h>
#include <regexpr.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <libintl.h>
#include <errno.h>

#include "auto_ef.h"

static int hflag;
static int eflag;
static int aflag;
static int lflag;
static int errflg;
char *eoption;
char *noption;
char *loption;

int err_status = 0;

extern int errno;

void PrintResult(auto_ef_t *, char *, size_t);
int comp_eoption(char *, char *);
void Usage();
void Auto_ef_error(int, char *);
void Auto_ef_nomuch_found(char *);
void Unknown_error(char *);

void main(int argc, char **argv) {
	int grade;

	/* Default Level is 0 */
	int level = 0;
	int c;
	char *arg;
	extern int optind;

	int argctmp = 0;
	auto_ef_t *root_auto_ef = NULL;
	size_t auto_ef_size = 0;

	(void) setlocale(LC_ALL, "");

#if !defined(TEXT_DOMAIN)	/* Should be defined by cc -D */
#define	TEXT_DOMAIN "SYS_TEST"	/* Use this only if it weren't */
#endif

	(void) textdomain(TEXT_DOMAIN);
	while ((c = getopt(argc, argv, "e:l:ah")) != -1) {
		/* optind++; */
		switch (c) {
		case 'h':
			hflag++;
			break;
		case 'e':
			eflag++;
			eoption = argv[optind -1];
			if (eoption[0] == '-')
				errflg++;
			break;
		case 'a':
			aflag++;
			noption = argv[optind -1];
			break;
		case 'l':
			loption = argv[optind -1];
			if (loption[0] == '-') errflg++;
			lflag++;
			break;
		case '?':
			errflg++;
		}
	}

	if (errflg || (optind > argc) || hflag) {
		Usage();
		exit(1);
	}

	if (strrchr(*argv, '\n') != NULL) {
		Usage();
		exit(1);
	}

	if (lflag) {
		level = (int)atoi(loption);

		if (loption[1] != '\0') {
			Usage();
			exit(1);
		}

		if (loption[0] != '0' && level == 0) {
			Usage();
			exit(1);
		}
	}

	/* In case of auto_ef don't have option */
	/* if (argc == 0) argc++; */

	argctmp = argc - optind;
	if (argctmp == 0) {
		auto_ef_size = auto_ef_file(&root_auto_ef, NULL, level);
		if (auto_ef_size == (size_t)-1) {
			Auto_ef_error(errno, NULL);
			exit(1);
		} else if (auto_ef_size > (size_t)0) {
			PrintResult(root_auto_ef, NULL, auto_ef_size);
			auto_ef_free(root_auto_ef);
		} else if (auto_ef_size == (size_t) 0) {
			/* No much encoding found */
			Auto_ef_nomuch_found(NULL);
			exit(2);
		} else {
			Unknown_error(NULL);
			exit(-1);
		}
	} else if (argctmp == 1) {
		auto_ef_size = auto_ef_file(&root_auto_ef,
			argv[argc-argctmp], level);

		if (auto_ef_size == (size_t)-1) {
			Auto_ef_error(errno, NULL);
			exit(1);
		} else if (auto_ef_size > (size_t)0) {
			PrintResult(root_auto_ef, NULL, auto_ef_size);
			auto_ef_free(root_auto_ef);
		} else if (auto_ef_size == (size_t) 0) {
			/* No much encoding found */
			Auto_ef_nomuch_found(NULL);
			auto_ef_free(root_auto_ef);
			exit(2);
		} else {
			Unknown_error(NULL);
			auto_ef_free(root_auto_ef);
			exit(-1);
		}

	} else {
		while (argctmp > 0) {
			auto_ef_size = auto_ef_file(&root_auto_ef,
				argv[argc-argctmp], level);
			if (auto_ef_size == -1) {
				Auto_ef_error(errno, argv[argc-argctmp]);
				/*
				 * If one file has error,
				 * the exit status should be 1
				 */
				err_status = 1;
			} else if (auto_ef_size > (size_t)0) {
				PrintResult(root_auto_ef, argv[argc-argctmp],
					auto_ef_size);
				auto_ef_free(root_auto_ef);
				auto_ef_size = 0;
			} else if (auto_ef_size == (size_t)0) {
			/* No much encoding found: auto_ef_size = 0 */
				Auto_ef_nomuch_found(argv[argc-argctmp]);
				err_status = 2;
			} else {
				Unknown_error(argv[argc-argctmp]);
				err_status = -1;
			}
			argctmp--;
		}
	}
	exit(err_status);
}

void PrintResult(auto_ef_t *root_autoef, char *filename, size_t length) {
	int i;
	char *tmp_e;
	double tmp_s;

	if (aflag != 0) {
		for (i = 0; i < length; i++) {
			tmp_e = auto_ef_get_encoding(root_autoef[i]);
			tmp_s = auto_ef_get_score(root_autoef[i]);
			if (tmp_s/100.0 >= 0.005) {
				if (eflag != 0) {
					if (comp_eoption(tmp_e, eoption)) {
						if (filename != NULL)
							printf("%s: ", filename);
						printf("%s  %2.2f\n",
							tmp_e, tmp_s/100.0);
					}

				} else {
					if (filename != NULL)
						printf("%s: ", filename);
					printf("%s  %2.2f\n", tmp_e, tmp_s/100.0);
				}
			}
		}
	} else {
		tmp_e = auto_ef_get_encoding(root_autoef[0]);
		tmp_s = auto_ef_get_score(root_autoef[0]);
		if (tmp_s/100.0 >= 0.005) {
			if (eflag != 0) {
				if (comp_eoption(tmp_e, eoption)) {
					if (filename != NULL)
						printf("%s: ", filename);
					printf("%s\n", tmp_e);
				}
			} else {
				if (filename != NULL)
					printf("%s: ", filename);
				printf("%s\n", tmp_e);
			}
		}
	}
}

int comp_eoption(char *encoding, char *list) {
	int i = 0, j = 0;
	char target[PATH_MAX];

	while (list[i] != '\0') {
		if (list[i] == ':') {
			target[j] = '\0';
			j = 0;
			i++;
			if (strcmp(encoding, target) == 0) {
				return (1);
			}
		} else {
			target[j] = list[i];
			i++;
			j++;
		}
	}

	target[j] = '\0';
	if (strcmp(encoding, target) == 0) {
		return (1);
	} else {
		return (0);
	}
}

void Usage() {
	fprintf(stderr, gettext("Usage: auto_ef -heal [ file ... ]\n"));
}

void Auto_ef_error(int errID, char *filename) {
	if (filename != NULL)
		fprintf(stderr, "%s: ", filename);
	fprintf(stderr, gettext("Error: auto_ef failed (errno = %d)\n"), errID);
}

void Auto_ef_nomuch_found(char *filename) {
	if (filename != NULL)
		fprintf(stderr, "%s: ", filename);
	fprintf(stderr, gettext("No corresponding encoding found.\n"));
}

void Unknown_error(char *filename) {
	if (filename != NULL)
		fprintf(stderr, "%s: ", filename);
	fprintf(stderr, gettext("Unknown error occurred.\n"));
}
