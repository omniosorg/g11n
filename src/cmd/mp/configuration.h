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
 * Copyright (c) 1999 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma ident   "@(#)configuration.h	1.1 99/07/26 SMI"

#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <string.h>
#include <sys/types.h>
#include "general_header.h"
#define CONF_FILE	"mp.conf"
#define DEFAULT_CONF_PATH	"/usr/lib/lp/locale/C/mp"
#define KEYWORD_PRESENTFORM	"PresentationForm"
#define KEYWORD_ORIENT		"Orientation"
#define KEYWORD_NUMERALS	"Numerals"
#define KEYWORD_TEXTSHAPE	"TextShaping"
#define KEYWORD_CONTEXT		"context"
#define KEYWORD_SWAPPING	"swapping"
#define KEYWORD_FONTNAMEALIAS	"FontNameAlias"
#define KEYWORD_FONTGROUP	"FontGroup"
#define KEYWORD_AFMFILE		"AFMfile"
#define KEYWORD_MAPCODE2FONT	"MapCode2Font"
#define KEYWORD_CNVCODE2FONT	"CnvCode2Font"
#define KEYWORD_PCF		"PCF"
#define KEYWORD_TTF		"TrueType"
#define KEYWORD_TP1		"Type1"
#define CMNT_CHAR               '#'
#define TYPE_PCF                0
#define TYPE_TTF                1
#define TYPE_TP1                2

#define ROMAN_INDEX		0
#define BOLD_INDEX		1
#define ITALIC_INDEX		2
#define BOLDITALIC_INDEX	3

#define ROMAN			0
#define BOLD			1
#define ITALIC			2
#define BOLDITALIC		3

#define TOT_INDEX		4
#define TOT_FONT_TYPES		3

#define XU_IGNORE               -2
#define XU_UCS4UNDEF            -4
#define XU_MOTION_CHAR          1
#define XU_PSBM_CACHED          2
#define XU_PSBM_NOTCACHED       3


typedef struct conf conf_t;
typedef struct fontgroup fontgroup_t;
typedef struct mapcode2font mapcode2font_t;
typedef struct cnvcode2font cnvcode2font_t;
typedef struct ucs4fontndx ucs4fontndx_t;
typedef struct cparam cparam_t;

enum pform_t { INVALID, WC , PLSOutput };
/*
 * This struct holds values of one time parameters in the configuration
 * file
 */

struct cparam {
	enum pform_t pform;
	unsigned int orient;
	unsigned int numeral;
	unsigned int textshape;
	unsigned int swapping;
	unsigned int context;
};
/*
 * struct fontgroup contains scanned information
 * from the "FontGroup" keyword entries in the
 * CONF_FILE. type indicates PCF/TrueType/Type1
 * fonts, name is alias name given for groups,
 * gndx is an array of 4 indices pointing to
 * Roman, Bold, Italic and BoldItalic font entries
 * entries in the corresponding list of PCF/TrueType/Type1
 * font structures.
 */

struct fontgroup {
	int  type;
	char *name;
	int  gndx[4];
};

/*
 * struct mapcode2font contains scanned information
 * from the "MapCode2Font" entries in the configuration file.
 * n1, n2 are the presentation form range to which the 
 * fontgroups are mapped. pcf_name, ttf_name & tp1_name 
 * are the PCF/TrueType/Type1 font group alias names to which
 * the range is mapped.
 */

struct mapcode2font {
	unsigned int n1;
	unsigned int n2;
	char *pcf_name;
	char *ttf_name;
	char *tp1_name;
};

/*
 * struct cnvcode2font contains scanned information
 * from  the "CnvCode2Font" entries in the configuration file.
 * Information about the function which will convert the
 * presentation form of each coderange to the index of the
 * font to which it is mapped is stored in this struct.
 * file - path of the .so file.
 * name - alias name of the font file to which the shared module 
 * is mapped.
 * fsym - The function within the shared object which does the
 * conversion.
 */

struct cnvcode2font {
	char *file;
	char *name;
	char *fsym;
};

/*
 * struct conf holds the information about
 * the total number of FontGroup entries
 * in the file in gmapN, MapCode2Font entries
 * in mmapN, CnvCode2Font entries in cmapN
 * scanned information from FontGroup, MapCode2Font
 * and CnvCode2Font entries are stored in FontGroup_t,
 * MapCode2Font_t and CnvCode2Font_t structures
 * respectively.
 */
struct conf {
	int gmapN;
	int mmapN;
	int cmapN;
	fontgroup_t *gmap;
	mapcode2font_t *mmap;
	cnvcode2font_t *cmap;
};

/*
 * holds scanned info from the configuration file
 * about the presentation code ranges and the fonts 
 * mapped to those ranges.
 */

struct ucs4fontndx {
	unsigned int n1;
	unsigned int n2;
	int pcf_ndx[4];                     /* index into pcf fontlist */
	int ttf_ndx[4];                     /* index into TTfont list */
	int tp1_ndx[4];                     /* index into printer fontlist */
	char group_order[TOT_FONT_TYPES];
};


#endif /* _CONFIGURATION_H */
