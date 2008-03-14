/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef	_IDNCONV_H
#define	_IDNCONV_H


#pragma ident	"@(#)idnconv.h	1.1	04/06/16 SMI"


#ifdef	__cplusplus
extern "C" {
#endif


/* Some convenience macros. */
#define OPTION(x)		(strcmp(av[i] + 2, (x)) == 0)
#define WHITESPACE(c)		((c) == ' ' || (c) == '\f' || (c) == '\n' || \
				(c) == '\r' || (c) == '\t' || (c) == '\v')
#define	MAX_CONV_LOOP		4
#define	ACE_PREFIX		"xn--"

/*
 * Exit values of the utility as specified in the man page.
 *
 * DO NOT SPECIFY ANY NEGATIVE VALUES OR YOU WILL HAVE TO CHANGE
 * PROGRAMS FOR THAT.
 */
#define	IDNCONV_SUCCESS					0
#define	IDNCONV_UNSUPPORTED_INCODE_OR_OUTCODE		1
#define	IDNCONV_ASCII_CHECK_FAILED			2
#define	IDNCONV_BIDI_CHECK_FAILED			3
#define	IDNCONV_LABEL_LENGTH_CHECK_FAILED		4
#define	IDNCONV_NAMEPREP_ERROR				5
#define	IDNCONV_UNASSIGNED_CHARACTER_FOUND		6
#define	IDNCONV_ILLEGAL_OPTION				7
#define	IDNCONV_INPUT_FILE_NOT_FOUND			8
#define	IDNCONV_NOT_ENOUGH_MEMORY			9
#define	IDNCONV_ICONV_INTERNAL_ERROR			10
#define	IDNCONV_ICONV_NON_IDENTICAL_MAPPING		11
#define	IDNCONV_UNKNOWN_FAILURE				255


#ifdef	__cplusplus
}
#endif

#endif	/* _IDNCONV_H */
