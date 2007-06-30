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
/* Copyright (c) 2000, Sun Microsystems, Inc. All rights reserved. */
 
#pragma ident	"@(#)lcl_query_type.c	1.2	00/01/06 SMI"

#include <iconv.h>
#include <errno.h>

#include "lcl.h"
#include "lcl_internal.h"

static int
is_printable_ascii(unsigned char uc)
{
	if((uc >= 0x20) && (uc <= 0x7e))
		return 1;
	if((uc == 0x09) || (uc == 0x0d) || (uc == 0x0a))
		return 1;
	else
		return 0;
}

static int
check_null_character(char *ptr, size_t len)
{
	while(len > 0){
		if(*ptr == (char)0)
			return 1;
		ptr++;
		len--;
	}
	return 0;
}

static int
check_ascii_string(char *ptr, size_t len, size_t *num)
{
	unsigned char	*uc = (unsigned char *)ptr;
	*num = 0;

	while(*num < len){
		if((*uc < 0x20) || (*uc > 0x7f)){
			if(!is_printable_ascii(*uc))
				return 0;
		}
		uc++;
		(*num)++;
	}
	return 1;
}

static int
check_7bit_string(char *ptr, size_t len, size_t *num)
{
	unsigned char	*uc = (unsigned char *)ptr;
	*num = 0;

	while(*num < len){
		if(*uc & 0x80)
			return 0;
		uc++;
		(*num)++;
	}
	return 1;
}

static int
check_iso9496_string(char *ptr, size_t len, size_t *num)
{
	unsigned char	*uc = (unsigned char *)ptr;
	*num = 0;

	while(*num < len){
		if(*uc < 0x20){
			if(!is_printable_ascii(*uc))
				return 0;
		}
		if((*uc >= 0x7f) && (*uc < 0xa0))
			return 0;
		uc++;
		(*num)++;
	}
	return 1;
}

static int
check_iso94Ext_string(char *ptr, size_t len, size_t *num)
{
	unsigned char	*uc = (unsigned char *)ptr;
	*num = 0;

	while(*num < len){
		if(*uc < 0x20){
			if(!is_printable_ascii(*uc))
				return 0;
		}
		uc++;
		(*num)++;
	}
	return 1;
}

static int
check_multibyte_string(char *ptr, size_t len, size_t *num)
{
	int	c_len;

	*num = 0;
	while(*num < len){
		c_len = mblen(ptr, len - *num);
		if(c_len <= 0)
			return 0;
		else{
			ptr += c_len;
			*num -= c_len;
		}
	}
	return 1;
}

static LclContentInfo *
make_content_info(LCTd lctd, LctNEAttribute form, LclContentType type, char *charset)
{
	LclCharsetLinkedList *list;
	LclContentInfo	*info;

	info = (LclContentInfo *)malloc(sizeof(LclContentInfo));
	if(info == (LclContentInfo *)NULL)
		return (LclContentInfo *)NULL;

	info->type = type;
	info->charset_list = (LclCharsetList *)NULL;

	if(charset){
		list = _lcl_create_one_charset_linked_list(charset);
		if(list){
			info->charset_list = _lct_create_context_charset_list(lctd, form, list);
			_lcl_destroy_charset_linked_list(list);
		}
	}

	return info;
}
	
LclContentInfo *
_lct_get_query_content_type(LCTd lctd, LctNEAttribute form)
{
	char	*ptr = lctd->contents->buf;
	size_t	len = lctd->contents->buf_len;
	char	*charset;
	size_t	num;

	if((ptr == (char *)NULL) || (len <= 0))
		return make_content_info(lctd, form, LclContentUnknown, (char *)NULL);

	/* check null character */
	if(check_null_character(ptr, len))
		return make_content_info(lctd, form, LclContentBinary, (char *)NULL);
	
	/* check ascii */
	if(check_ascii_string(ptr, len, &num)){
		charset = _lct_get_charsetname_from_format(lctd, LclCsFormatASCII);
		if(charset)
			return  make_content_info(lctd, form, LclContentText, charset);
	}

	/* check 7bit */
	charset = _lct_get_charsetname_from_format(lctd, LclCsFormat7bit);
	if(charset){
		if(check_7bit_string(ptr, len, &num))
			return  make_content_info(lctd, form, LclContentText, charset);
	}

	/* check to see if correct Multibyte string format */
	charset = _lct_get_charsetname_from_format(lctd, LclCsFormatMBString);
	if(charset){
		if(check_multibyte_string(ptr, len, &num))
			return  make_content_info(lctd, form, LclContentText, charset);
	}

	/* check ISO 94/96 */
	charset = _lct_get_charsetname_from_format(lctd, LclCsFormatISO9496);
	if(charset){
		if(check_iso9496_string(ptr, len, &num))
			return  make_content_info(lctd, form, LclContentText, charset);
	}

	/* check ISO 94 + Ext */
	charset = _lct_get_charsetname_from_format(lctd, LclCsFormatISO94Ext);
	if(charset){
		if(check_iso94Ext_string(ptr, len, &num))
			return  make_content_info(lctd, form, LclContentText, charset);
	}

	/* unknown */
	return make_content_info(lctd, form, LclContentUnknown, (char *)NULL);
}

/* check if the content is in the range
	return	-1	unknown
		0	false
		1	true
*/
int
_lct_check_content_from_charset(LCTd lctd, char *ptr, size_t len, size_t *num, char *charset)
{
	LclFormat	format;

	if((ptr == (char *)NULL) || (charset == (char *)NULL))
		return -1;

	format = _lct_get_format_from_charsetname(lctd, charset);

	if(format == LclCsFormatASCII)
		return check_ascii_string(ptr, len, num);
	else if(format == LclCsFormat7bit)
		return (check_7bit_string(ptr, len, num));
	else if(format == LclCsFormatMBString)
		return (check_multibyte_string(ptr, len, num));
	else if(format == LclCsFormatISO9496)
		return (check_iso9496_string(ptr, len, num));
	else if(format == LclCsFormatISO94Ext)
		return (check_iso94Ext_string(ptr, len, num));
	else
		return -1;
}

void
lcl_destroy_content_info(LclContentInfo *info)
{
	if(info){
		if(info->charset_list)
			lcl_destroy_charset_list(info->charset_list);
		free(info);
	}
}

