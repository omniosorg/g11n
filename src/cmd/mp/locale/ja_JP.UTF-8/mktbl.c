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
 * Copyright 2002 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <stdio.h>

#define TRUE	(0)
#define FALSE	(1)
#define BUFF	(256)
#define MAX	(0x2FFFF)
#define UNIT	(0xFFFF)
#define GL(x)	((x)&0x7F7F)

static int tbl[MAX];

main(
    int argc,
    char **argv
)
{
  int i;
  
  i = 1;
  while (--argc) {
    if(read_and_set(argv[i++]) == FALSE)
      return FALSE;
  }

  dump_tbl(0, UNIT, MAX);

  return 0;
}

read_and_set(
    char *filename
)
{
  FILE *fp;
  char line[BUFF+1];
  int  left, right;
  int  i, gr;

  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Can't open: %s\n", filename);
    return FALSE;
  }
  fprintf(stderr, "[%s] reading...", filename);

  gr = (strstr(filename, ".GR.")?1:0);

  while (fgets(line, BUFF, fp)) {
    sscanf(line, "\\x%lx->\\x%lx,\n", &left, &right);
    /*    if (tbl[right])
      goto ERROR;
     * Don't sed a value, if tbl[right] have old.
     */
    if (!tbl[right])
      tbl[right] = (gr?GL(left):left);
  }
  fprintf(stderr, "Complete.\n");
  
  fclose(fp);
  
  return TRUE;

 ERROR:
  fprintf(stderr, "\n\t   Already: tbl[x%04X] = x%04X\n", right, tbl[right]);
  fprintf(stderr, "\tTry to set: tbl[x%04X] = x%04X\n", right, (gr?GL(left):left));
  fprintf(stderr, "\tcheck if %s have [x%04X] on the right side.\n", filename, right);
  return FALSE;
}

dump_tbl(
    int start,
    int unit,
    int last
)
{
  int i, index;

  index = start;

  while (index < last) {
    fprintf(stdout, "/*\n    Mapping table: U+%08X - U+%08X\n*/\n", index, index+unit);
    for (i = 0; i <= unit && index < last; i++) {
      fprintf(stdout, "%d, ", tbl[index++]);
      if (i%16 == 15)
	fprintf(stdout, "\n");
    }
  }
  fprintf(stdout, "\n");
}
