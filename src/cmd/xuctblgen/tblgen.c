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
 * Copyright 1996, 1998, 2001-2002 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma	ident	"@(#)tblgen.c	1.18 02/03/12 SMI"

/* C headers */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

/* Xlib headers */
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>
#include <X11/Xlibint.h>

/* internal Xlib headers */
#include "XlcGeneric.h"

/* program headers */
#include "range_tbl.h"
#include "conv_info.h"
#include "lookup_tbl.h"
#include "cstream.h"
#include "utf8_defs.h"

/* Macro definitions. */
#define LOCALE_DIR_ENV			"XLOCALEDIR"

#define WC_VALID_LENGTH_ENTRY		"wc_valid_length"
#define WC_TO_CS_CONV_TABLE_ENTRY_PLANE	"wc_conversion_table_plane_"

#define CS_TO_WC_CONV_ENTRY		"cs_conversion"
#define CS_TO_WC_CONV_FILE_ENTRY	"cs_conversion_file"
#define CS_TO_WC_CONV_TABLE_ENTRY	"cs_conversion_table"

/* private functions */
static int generate_conversion_tables(XLCd lcd, boolean_t sysmode);
static int parse_range(char *str, RangeTblEntry *entry);
static int make_filename(char *str, char *filename);


char *lc_name = NULL;
int cs_num = 0;
boolean_t use_common_dir = B_FALSE;
boolean_t no_wc_to_cs_tables = B_FALSE;
boolean_t no_cs_to_wc_tables = B_FALSE;

int
main(int argc, char **argv)
{
	boolean_t sys_mode;
	int i, j;
	XLCd lcd;
	CodeSet *codesets;
	char **value;
	int num;

	/* argument parsing */
	sys_mode = B_TRUE;
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "+sys"))
			sys_mode = B_FALSE;
		else if (!strcmp(argv[i], "-locale"))
			lc_name = argv[++i];
		else if (!strcmp(argv[i], "-codeset_num"))
			cs_num = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-use_common_dir"))
			use_common_dir = B_TRUE;
		else if (!strcmp(argv[i], "-no_wc_to_cs_tables"))
			no_wc_to_cs_tables = B_TRUE;
		else if (!strcmp(argv[i], "-no_cs_to_wc_tables"))
			no_cs_to_wc_tables = B_TRUE;
	}

	if (lc_name == NULL) {
	    fprintf(stderr, "Usage: %s -locale <locale_name>\n", argv[0]);
	    exit(1);
	}
	if (sys_mode == B_TRUE) {
		/* create system tables */
		lcd = _XlcCreateLC(lc_name, _XlcGenericMethods);
		if (lcd == (XLCd)NULL) {
			fprintf(stderr, "%s: cannot create LC\n", argv[0]);
			return -1;
		}
		if (generate_conversion_tables(lcd, sys_mode)) {
			fprintf(stderr, "%s: generating conv table failed\n",
				argv[0]);
			_XlcDestroyLC(lcd);
			return -1;
		}
		_XlcDestroyLC(lcd);
	} else {
		/* create user tables */
		if (!XSupportsLocale()) {
			fprintf(stderr, "%s: X does not support locale\n",
				argv[0]);
			return -1;
		}

		if (!XSetLocaleModifiers("")) {
			fprintf(stderr,
				"%s: XSetLocaleModifiers() returns err\n",
					argv[0]);
			return -1;
		}

		lcd = _XOpenLC(NULL);
		if (lcd == (XLCd)NULL) {
			fprintf(stderr, "%s: cannot create LC\n", argv[0]);
			return -1;
		}

		if (generate_conversion_tables(lcd, sys_mode)) {
			fprintf(stderr, "%s: generating conv table failed\n",
				argv[0]);
			_XCloseLC(lcd);
			return -1;
		}
		_XCloseLC(lcd);
	}

	return 0;
}


static int
generate_conversion_tables(XLCd lcd, boolean_t sys_mode)
{
	LookupTable	*wtc_tbl[U8_MAX_PLANES];
	LookupTable	*ctw_tbl = (LookupTable *)NULL;
	RangeTbl	*byte_range_tbl = (RangeTbl *)NULL;
	RangeTbl	*usr_range_tbl = (RangeTbl *)NULL;
	ConvInfo	*conv_info_tbl = (ConvInfo *)NULL;
	int		i, j, k, l, num, codeset_num;
	int		wc_length, cs_length, cs_max_length;
	char		**str_list;
	char		resource[256];
	char		filename[PATH_MAX];
	FILE		*fp;
	int		fd;

	if (cs_num)
		codeset_num = cs_num;
	else
		codeset_num = XLC_GENERIC(lcd, codeset_num);
#ifdef VERBOSE
	fprintf(stdout, "CodeSetNumber = %d (%d)\n", codeset_num, cs_num);
#endif

	/* determin cs_length of the WC_to_CS conversion table */
	cs_max_length = 0;
	for (i = 0; i < codeset_num; i++) {
		sprintf(resource, "cs%d.length", i);
		_XlcGetResource(lcd, "XLC_XLOCALE", resource, &str_list, &num);
		if (num > 0) {
			cs_length = atoi(str_list[0]);
			if (cs_max_length < cs_length)
				cs_max_length = cs_length;
		}
	}
	if (cs_max_length < 1)
		goto ERR_RETURN;

	/* get wc_valid_len for WC_to_CS conversion table */
	_XlcGetResource(lcd, "XLC_XLOCALE", WC_VALID_LENGTH_ENTRY, &str_list,
		&num);
	if (num <= 0)
		goto ERR_RETURN;
	wc_length = atoi(str_list[0]);

	/*
	 * Create seventeen WC_to_CS conversion tables. Each table with
	 * index 'i' is for Unicode 3.1 Plane i.
	 *
	 * For codeset like Unicode where there is not much gap, i.e.,
	 * zero byte value in multibyte representation like EUC, PCK, and
	 * Big5, the Trie data structure actually costs more then a simple
	 * vector, i.e., one dimensional array, in terms of both memory and
	 * index calculation time at the XLC module of the locale. Hence,
	 * we use vectors for WC to CS conversion tables.
	 *
	 * One Unicode plane will have one conversion table to save memory
	 * space, i.e., if there is no character assigned in a plane, that
	 * plane won't be generated and also won't even loaded to
	 * the XLC module. (Unicode 3.1 has such planes at Plane 03 to 0D.)
	 *
	 * Since we know each one of these tables is for a specific plane,
	 * we will malloc a table that will be just big enough to contain
	 * pow(2, 16) mappings and that's the reason why we are cutting
	 * the wc_length to two if it's bigger than or equal to 3 at
	 * below.
	 */
	for (i = 0; i < U8_MAX_PLANES; i++) {
		wtc_tbl[i] = LookupTable_create((wc_length >= 3) ? 2 :
				wc_length, cs_max_length, B_TRUE,
				U8_TABLE_TYPE_VECTOR);
		if (wtc_tbl[i] == (LookupTable *)NULL)
			goto ERR_RETURN;
	}

	for (i = codeset_num - 1; i >= 0; i--) {

		/* read Byte-Range info */
#ifdef VERBOSE
		fprintf(stdout, "reading byteM of cs%d ...\n", i);
		fflush(stdout);
#endif
		sprintf(resource, "cs%d.length", i);
		_XlcGetResource(lcd, "XLC_XLOCALE", resource, &str_list,
			&num);
		if (num < 1)
			goto ERR_RETURN;
		cs_length = atoi(str_list[0]);
		byte_range_tbl = RangeTbl_create(cs_length);
		if (byte_range_tbl == (RangeTbl *)NULL)
			goto ERR_RETURN;
		for (j = 1; j <= cs_length; j++) {
			sprintf(resource, "cs%d.byte%d", i, j);
			_XlcGetResource(lcd, "XLC_XLOCALE", resource, &str_list,
				&num);
			for (k = 0; k < num; k++) {
				RangeTblEntry	entry;
				if (parse_range(str_list[k], &entry))
					goto ERR_RETURN;
				if (RangeTbl_add(&(byte_range_tbl[cs_length-j]),
					&entry))
					goto ERR_RETURN;
			}
		}

		/* read User-Range info */
#ifdef VERBOSE
		fprintf(stdout, "reading cs_range of cs%d ...\n", i);
		fflush(stdout);
#endif
		usr_range_tbl = RangeTbl_create(1);
		if (usr_range_tbl == (RangeTbl *)NULL)
			goto ERR_RETURN;
		sprintf(resource, "cs%d.cs_range", i);
		_XlcGetResource(lcd, "XLC_XLOCALE", resource, &str_list, &num);
		for (j = 0; j < num; j++) {
			RangeTblEntry	entry;
			if (parse_range(str_list[j], &entry))
					goto ERR_RETURN;
			if (RangeTbl_add(&(usr_range_tbl[0]), &entry))
					goto ERR_RETURN;
		}

		/* read CS_to_WC conversion info */
#ifdef VERBOSE
		fprintf(stdout, "reading cs_conversion of cs%d ...\n", i);
		fflush(stdout);
#endif
		conv_info_tbl = ConvInfo_create();
		if (conv_info_tbl == (ConvInfo *)NULL)
			goto ERR_RETURN;
		sprintf(resource, "cs%d.%s", i, CS_TO_WC_CONV_ENTRY);
		_XlcGetResource(lcd, "XLC_XLOCALE", resource, &str_list, &num);
		if (num > 0) {
			CStream	stream;
			ConvInfoEntry	entry;
			CStream_initString(&stream, str_list[0]);
			while (CStream_parseConvInfo(&stream, &entry) != EOF) {
				if (ConvInfo_add(conv_info_tbl, &entry))
					goto ERR_RETURN;
			}
		}

#ifdef VERBOSE
		fprintf(stdout, "reading cs_conversion_file of cs%d ...\n", i);
		fflush(stdout);
#endif
		sprintf(resource, "cs%d.%s", i, CS_TO_WC_CONV_FILE_ENTRY);
		_XlcGetResource(lcd, "XLC_XLOCALE", resource, &str_list, &num);
		if (num > 0) {
			CStream	stream;
			ConvInfoEntry	entry;

			if (make_filename(str_list[0], filename))
				goto ERR_RETURN;

			if ((fp = fopen(filename, "r")) == (FILE *)NULL) {
				if (! use_common_dir) {
					/*
					 * Try one more time with "common"
					 * directory.
					 */
					use_common_dir = B_TRUE;
					if (make_filename(str_list[0],
					    filename))
						goto ERR_RETURN;
					use_common_dir = B_FALSE;
					if ((fp = fopen(filename, "r")) ==
					    (FILE *)NULL)
						goto ERR_RETURN;
				} else
					goto ERR_RETURN;
			}

			CStream_initFile(&stream, fp);
			while (CStream_parseConvInfo(&stream, &entry) != EOF) {
				if (ConvInfo_add(conv_info_tbl, &entry)) {
					fclose(fp);
					goto ERR_RETURN;
				}
			}

			fclose(fp);
		}
	
		/* create CS_to_WC conversion table */
#ifdef VERBOSE
		fprintf(stdout,
			"generating cs_conversion_table of cs%d ...\n", i);
		fflush(stdout);
#endif
		ctw_tbl = LookupTable_create(cs_length, wc_length, B_FALSE,
				(cs_length == 1) ? U8_TABLE_TYPE_VECTOR :
					U8_TABLE_TYPE_TRIE);
		if (ctw_tbl == (LookupTable *)NULL)
			goto ERR_RETURN;

		/* write conversion info to conversion table */
		for (j = 0; j < conv_info_tbl->length; j++) {
			unsigned long cs_code;
			unsigned long wc_code;
			int loop_count;
			unsigned char plane;

			cs_code = conv_info_tbl->entry[j].cs_begin;
			wc_code = conv_info_tbl->entry[j].wc_begin;
			loop_count = conv_info_tbl->entry[j].cs_end -
					conv_info_tbl->entry[j].cs_begin + 1;
			for (k = 0; k < loop_count; k++) {
				if (sys_mode == B_TRUE) {
					if (LookupTable_add(ctw_tbl, cs_code,
					    wc_code, -1, cs_length))
						goto ERR_RETURN;
				}
				if (RangeTbl_in(usr_range_tbl, cs_code) ==
				    B_FALSE)
					goto NEXT_LOOP;
				for (l = 0; l < cs_length; l++) {
					if (RangeTbl_in(&(byte_range_tbl[l]),
					    (cs_code >> (l * 8)) & 0xff) ==
					    B_FALSE)
						goto NEXT_LOOP;
				}

				/*
				 * Wide character is in UTF-32.
				 */
				if (wc_code > U8_MAX_VALUE_IN_UTF32)
					goto ERR_RETURN;
				plane = (unsigned char)((wc_code >> 16) & 0xff);
				if (LookupTable_add(wtc_tbl[plane],
				    (wc_code & 0xffff), cs_code, i, cs_length))
					goto ERR_RETURN;
NEXT_LOOP:
				cs_code++;
				wc_code++;
			}
		}

		if (sys_mode == B_TRUE && (! no_cs_to_wc_tables)) {
			/* save CS_to_WC conversion file */
#ifdef VERBOSE
			fprintf(stdout,
				"writing cs_conversion_table of cs%d ...\n", i);
			fflush(stdout);
#endif
			sprintf(resource, "cs%d.%s", i,
				CS_TO_WC_CONV_TABLE_ENTRY);
			_XlcGetResource(lcd, "XLC_XLOCALE", resource, &str_list,
				&num);
			if (num > 0) {
				if (make_filename(str_list[0], filename))
					goto ERR_RETURN;
				if ((fd = open(filename,
				    O_CREAT|O_TRUNC|O_WRONLY, 0644)) == -1)
					goto ERR_RETURN;
				if (LookupTable_save(ctw_tbl, fd)) {
					close(fd);
					goto ERR_RETURN;
				}
				close(fd);
			}
		}

		/* free resource */
		LookupTable_destroy(ctw_tbl);
		ctw_tbl = (LookupTable *)NULL;
		
		RangeTbl_destroy(usr_range_tbl, 1);
		usr_range_tbl = (RangeTbl *)NULL;

		RangeTbl_destroy(byte_range_tbl, cs_length);
		byte_range_tbl = (RangeTbl *)NULL;
	}

	/* Save WC_to_CS conversion files if needed. */
	if (no_wc_to_cs_tables) {
		for (i = 0; i < U8_MAX_PLANES; i++)
			if (wtc_tbl[i])
				LookupTable_destroy(wtc_tbl[i]);
		return(0);
	}

	for (i = 0; i < U8_MAX_PLANES; i++) {
#ifdef VERBOSE
		fprintf(stdout,
			"writing wc_conversion_table_plane_%d...", i);
		fflush(stdout);
#endif
		sprintf(filename, "%s%d", WC_TO_CS_CONV_TABLE_ENTRY_PLANE, i);
		_XlcGetResource(lcd, "XLC_XLOCALE", filename, &str_list, &num);

		/*
		 * If there is no wc_conversion_table_plane_X where "X" is
		 * the current plane number in one or two decimal digits in
		 * the XLC_LOCALE and/or if there is no mapping in
		 * the lookup table for the current plane, we don't
		 * write/save the table.
		 */
		if (num < 1 || wtc_tbl[i]->length == 0) {
#ifdef VERBOSE
			fprintf(stdout, "none written.\n");
			fflush(stdout);
#endif
			LookupTable_destroy(wtc_tbl[i]);

			continue;
		}

		if (make_filename(str_list[0], filename))
			goto ERR_RETURN;
		if ((fd = open(filename, O_CREAT|O_TRUNC|O_WRONLY, 0644)) == -1)
			goto ERR_RETURN;
		if (LookupTable_save(wtc_tbl[i], fd)) {
			close(fd);
			goto ERR_RETURN;
		}

		close(fd);

		LookupTable_destroy(wtc_tbl[i]);

#ifdef VERBOSE
		fprintf(stdout, "done.\n");
		fflush(stdout);
#endif
	}

	return 0;

ERR_RETURN:
	if (ctw_tbl)
		LookupTable_destroy(ctw_tbl);
	if (usr_range_tbl)
		RangeTbl_destroy(usr_range_tbl, 1);
	if (byte_range_tbl)
		RangeTbl_destroy(byte_range_tbl, cs_length);
	for (i = 0; i < U8_MAX_PLANES; i++)
		if (wtc_tbl[i])
			LookupTable_destroy(wtc_tbl[i]);
	return -1;
}


static int
parse_range(char *str, RangeTblEntry *entry)
{
	int read_num;

	read_num = sscanf(str, "\\x%lx,\\x%lx", &(entry->begin), &(entry->end));
	switch (read_num) {
		case 1:
			entry->end = entry->begin;
			break;
		case 2:
			break;
		default:
			return -1;
			break;
	}
	return 0;
}


static int
make_filename(char *str, char *filename)
{
	char *dir_name;

	if (str == (char *)NULL)
		return -1;
	if (str[0] == '/') {
		if (strlen(str) >= PATH_MAX)
			return -1;
		strcpy(filename, str);
	} else {
#ifdef	XLOCALEDIR
		dir_name = getenv("XLOCALEDIR");
		if (dir_name == (char *)NULL)
			return -1;

		if ((use_common_dir && (strlen(dir_name) + strlen(str) + 8 >=
		    PATH_MAX)) || ((! use_common_dir) && (strlen(dir_name) +
		    strlen(str) + strlen(lc_name) >= PATH_MAX)))
			return -1;

		strcpy(filename, dir_name);
		strcat(filename, "/");
		strcat(filename, use_common_dir ? "common" : lc_name);
		strcat(filename, "/");
		strcat(filename, str);
#else
		dir_name = _XlcLocaleDirName(lc_name);
		if (dir_name == (char *)NULL)
			return -1;
		if (strlen(dir_name) + 1 + strlen(str) >= PATH_MAX)
			return -1;
		sprintf(filename, "%s/%s", dir_name, str);
		Xfree(dir_name);
#endif	/* XLOCALEDIR */
	}

	return 0;
}
