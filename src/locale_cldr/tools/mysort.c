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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <locale.h>

#define MAX_LINES 250000



static int
cmp(const void *p1, const void *p2)
{
	return strcoll(*(char*const*)p1, *(char*const*)p2);
}


int set_l10n_alternate_root(char *fullpath);

int
main(int argc, char *argv[])
{
	char buf[1024];
	char *list[MAX_LINES];
	int i, n = 0;

	if (argc > 1 &&  set_l10n_alternate_root(argv[1]) != 0)
		perror(argv[1]);

	setlocale(LC_ALL, "");

	while (fgets(buf, 1023, stdin)) {
		assert(n < MAX_LINES);
		list[n++] = strdup(buf);
	}

	assert(n > 1);

	qsort(list, n, sizeof(char*), cmp);

	for (i = 0; i < n; i++)
		fputs(list[i],stdout);

	return 0;
}

