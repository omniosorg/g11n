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
 * Copyright (c) 2009, by Sun Microsystems, Inc.
 * All rights reserved.
 */

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


struct cmap_char {
	wchar_t wc;
	const unsigned char *native;
	const unsigned char *name;
};

struct cmap {
	const char *codeset;
	unsigned int len;
	const struct cmap_char *chars;
};

#include "cmap.dat"



static void
usage(void)
{
	unsigned int i;

	printf("Usage: cmprint [");

	for(i = 0; cmaps[i].len; i++) 
		printf(" %s %s", cmaps[i].codeset, cmaps[i+1].len ? "|" : "");

	printf(" ]\n");
	exit(1);
}


static void
die2(const char *format, ...)
{
	va_list args;

	fprintf(stderr, "cmprint: ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");

	exit(1);
}




int
main(int argc, char* argv[])
{
	const struct cmap *cm;
	unsigned int i;

	if (argc != 2)
		usage();

	for (cm = cmaps;; cm++)
		if (cm->len == 0)
			die2("unknown charmap");
		else if (strcmp(cm->codeset, argv[1]) == 0)
			break;

	for (i = 0; i < cm->len; i++)
		if ((unsigned int)cm->chars[i].wc)
			printf("%s\t0x%02X\t%s\n", cm->chars[i].native, (unsigned int)cm->chars[i].wc, cm->chars[i].name);

	return 0;
}
