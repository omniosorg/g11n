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

#ident  "@(#)data_create.c 1.2 03/01/30 SMI"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <iconv.h>
#include <errno.h>
#include "data_create.h"

void ListHashTable(void);
int AddScore(unsigned char, unsigned char);
void FreeHashTable(void);
int RegistHashTable(unsigned char, unsigned char);
int FindKeyWord(unsigned char, unsigned char);
int Hash(unsigned char, unsigned char);


srd foundsrd;

void
main(int argc, char *argv[])
{
  FILE *fp, *filelist;
  int i, j;
  char fbuf[BUFSIZE], proto_fbuf[BUFSIZE], filename[BUFSIZE];
  wchar_t words[BUFSIZE];
  char testbuf[BUFSIZE];
  
  /* Usage */
  if (argc < 2){
    fprintf(stderr, "Usage: %s <file list filename>\n", argv[0]);
    exit(1);
  }
  
  /* Specified File Open */
  if ( (filelist = fopen(argv[1], "r")) == NULL ){
    fprintf(stderr, "Can't open file %s\n", argv[1]);
    exit(1);
  }
  
  while(fgets(filename, BUFSIZE, filelist) != NULL){
    char *p;
    p=&filename[0];
    while (*p){
      if (*p == '\n'){ *p = '\0'; break; }
      p++;
    }
    
    if ( (fp = fopen(filename, "r")) == NULL ){
      fprintf(stderr, "Not found %s\n", filename);
      continue;
    }

    while(fgets(fbuf, BUFSIZE, fp) != NULL){
      /* Get a line from specified file */
      char *q;
      q=&fbuf[0];
      
      /* put '\0' in the end of line in spite of '\n' */

	while (*q)
		if (*q == '\n') { *q = '\0'; break; } else q++;

      for ( j=0; j <= BUFSIZE ; j++ ){
	unsigned char keywords[2];
	if (fbuf[j] == '\0') break;
	if ((unsigned)fbuf[j] > 127) Singlebyte_RegistHashTable(fbuf, j);
      }
    }
    fclose(fp);
  }
  ListHashTable();
  FreeHashTable();
  fclose(filelist);
  exit(0);
}

Singlebyte_RegistHashTable(char fbuf[], int j)
{
  
  if (j == 0){
    
    if (fbuf[j+1] != '\0'){
      if ((FindKeyWord(fbuf[j], fbuf[j+1])) == TRUE){
	AddScore(fbuf[j], fbuf[j+1]);
      } else {
	RegistHashTable((unsigned char)fbuf[j], (unsigned char)fbuf[j+1]);
      }
    }

  } else {
    if (fbuf[j+1] != '\0'){
      if ((FindKeyWord(fbuf[j], fbuf[j+1])) == TRUE){
	AddScore(fbuf[j], fbuf[j+1]);
      } else {
	RegistHashTable((unsigned char)fbuf[j],(unsigned char) fbuf[j+1]);
      }
    }
    
    if ((FindKeyWord(fbuf[j-1], fbuf[j])) == TRUE){
      AddScore(fbuf[j-1], fbuf[j]);
    } else {
      RegistHashTable((unsigned char)fbuf[j-1],(unsigned char)fbuf[j]);
    }
  }
}

void ListHashTable(void)
{
  int i;
  srd srdp;
  
  for (i=0; i < HASHSIZE; i++){
    for (srdp=hashtable[i]; srdp != NULL; srdp = srdp->nextsrd){
      printf("%d\n", Hash(srdp->keyword[0], srdp->keyword[1]));
      printf("%02x%02x\n", srdp->keyword[0], srdp->keyword[1]);
      printf("%d\n", srdp->score);
    }
  }
}

int AddScore(unsigned char a, unsigned char b)
{
  srd srdp;
  
  for(srdp = hashtable[Hash(a, b)]; srdp != NULL; 
      srdp = srdp->nextsrd){
    if ((srdp->keyword[0] == a) && (srdp->keyword[1] == b)){
      srdp->score = srdp->score + 1;
      return(0);
    }
  }
  
  return(-1);
}

void FreeHashTable(void) {

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

int RegistHashTable(unsigned char a, unsigned char b) {
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
}

int FindKeyWord(unsigned char a, unsigned char b) {
	srd srdp;

	for (srdp = hashtable[Hash(a, b)]; srdp != NULL; srdp = srdp->nextsrd) {
		if ((srdp->keyword[0] == a) && (srdp->keyword[1] == b)) {
			return (TRUE);
		}
	}
	return (FALSE);
}

int Hash(unsigned char a, unsigned char b) {
	unsigned int hashval = 0;
	hashval = (unsigned int)a + (unsigned int)b;
	return (hashval % HASHSIZE);
}
