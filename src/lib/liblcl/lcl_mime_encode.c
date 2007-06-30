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
 
#pragma ident	"@(#)lcl_mime_encode.c	1.2	00/01/06 SMI"

#include <stdio.h>
#include <locale.h>

#include "lcl.h"
#include "lcl_internal.h"


/********** Define of line length and CRLF **********/

/* line length without CRLF */
/* refered throuth extern in lcl_mime_encode.h */
const int	LclMaxHeaderLineLen = 75;

static char	LclCRLFString[] = "\n";
static int	LclCRLFStringLength = 1;

/* use LclSoftReturnStringList[0] when insersion */
/* refered throuth extern in lcl_mime_encode.h */
char	*LclSoftReturnStringList[] = {
	"\n ",
	"\n\t",
	(char *)NULL
};
int LclSoftReturn0StringLength = 2;

/* refered through extern in lcl_mime_encode.h */
const int	LclAfterSoftReturnPos = 1;



/********** mime header parsing **********/

static char	*NotTextFieldStrList[] = {
	"Return-path",
	"Received",
	"Reply-To",
	"From",
	"Sender",
	"Resent-Reply-To",
	"Resent-From",
	"Resent-Sender",
	"Date",
	"Resent-Date",
	"To",
	"Resent-To",
	"cc",
	"Resent-cc",
	"bcc",
	"Resent-bcc",
	"Message-ID",
	"Resent-Message-ID",
	"In-Reply-To",
	"References",
	"Keywords",
	"Encrypted",
	"Content-Type",
	"Content-Transfer-Encoding",
	"Content-MD5",
	(char *)NULL
};

static char	*DelimiterStrListFieldname[] = {
	": ",
	":",
	(char *)NULL
};

static char	*StrListEncodeBegin[] = {
	"=?",
	(char *)NULL
};

/* \n should be substituted if LF/CR is valid */
static char	*StrListEncodeEnd[] = {
	" ",
	"\t",
	"\n",
	(char *)NULL
};

static char	*StrListRemoveWhenEncodeEnd[] = {
/* according to RFC1522, space should not be removed
	" ",
*/
	"\n ",
	"\n\t",
	(char *)NULL
};

typedef struct _FieldList FieldList;
static struct _FieldList{
	char	*field;
	int	length;
	FieldList	*next;
};


/* general functions */

int
_lcl_next_token(char **p, int *len, char **token, int *token_len, char **delimiter_str)
{
	*token = (char *)NULL;
	*token_len = 0;

	while(*len > 0){
		int	delimiter_len;
		delimiter_len = _lcl_strncmp_with_string_list(*p, *len, delimiter_str, 0);
		if(delimiter_len > 0){
			if(*token){
				*token_len = *p - *token;
				return 0;
			}
			else{
				*token = *p;
				*token_len = delimiter_len;
				*len -= delimiter_len;
				*p += delimiter_len;
				return 1;
			}
		}
		else{
			if(*token == (char *)NULL)
				*token = *p;
			(*len)--;
			(*p)++;
			if(*len == 0){
				*token_len = *p - *token;
				return 0;
			}
		}
	}
	return -1;
}

int
_lcl_strncmp_with_string_list(char *ptr, int len, char **str_list, int case_flag)
{
        while(*str_list){
                int cmp_len = strlen(*str_list);
                if(len >= cmp_len){
                        if(case_flag){
                                if(!strncasecmp(ptr, *str_list,cmp_len))
                                        return cmp_len;
                        }
                        else{
                                if(!strncmp(ptr, *str_list, cmp_len))
                                        return cmp_len;
                        }
                }
                str_list++;
        }
        return 0;
}


/* encode */

static int
encode_mail_mode(LclBuffer *buf, char *ptr, int len, int *pos, LclMailEncoding encoding, char *charset)
{
	switch(encoding){
		case LclQPEncoding:
			return _lcl_q_encode_header_line(buf, ptr, len, charset);
			break;
		case LclBase64Encoding:
			return _lcl_b_encode_header_line(buf, ptr, len, charset);
		default:
			return -1; 
			break;
	}
}

static int
encode_text_mode(LclBuffer *buf, char *ptr, int len, int *pos, LclMailEncoding encoding, char *charset)
{
	switch(encoding){
		case LclQPEncoding:
			return _lcl_q_encode_text(buf, ptr, len, pos, charset);
			break;
		case LclBase64Encoding:
			return _lcl_b_encode_text(buf, ptr, len, pos, charset);
		default:
			return -1; 
			break;
	}
}

static char *
encode_one_line(char *header, int length, LclMailEncoding encoding, char *charset)
{
	char	*ptr;
	int	len;
	int	ret;
	LclBuffer	*buf;
	char	*encoded_buf;
	int	pos;
	char	*field_name, *delimiter;
	int	field_name_len, delimiter_len;


	buf = _LclBuffer_create(length * 4);
	if(buf == (LclBuffer *)NULL)
		return (char *)NULL;

	ptr = header;
	len = length;
	pos = 0;

	/* check the first token */
	if(_lcl_next_token(&ptr, &len, &field_name, &field_name_len, DelimiterStrListFieldname) == 0){
		if(_lcl_next_token(&ptr, &len, &delimiter, &delimiter_len, DelimiterStrListFieldname) > 0){
			if(_LclBuffer_add(buf, field_name, field_name_len) != 0){
				_LclBuffer_destroy(buf);
				return (char *)NULL;
			}
			pos += field_name_len;
			if(_LclBuffer_add(buf, delimiter, delimiter_len) != 0){
				_LclBuffer_destroy(buf);
				return (char *)NULL;
			}
			pos += delimiter_len;

			/* if this field is not "text" */
			if(_lcl_strncmp_with_string_list(field_name, field_name_len, NotTextFieldStrList, 1))
				ret = encode_mail_mode(buf, ptr, len, &pos, encoding, charset);
			/* this field is "text" */
			else
				ret = encode_text_mode(buf, ptr, len, &pos, encoding, charset);
		}
		else
			ret = encode_text_mode(buf, header, length, &pos, encoding, charset);
	}
	else
		ret = encode_text_mode(buf, header, length, &pos, encoding, charset);

	if(ret){
		_LclBuffer_destroy(buf);
		return (char *)NULL;
	}
	else{
		encoded_buf = _LclBuffer_get_string(buf);
		_LclBuffer_destroy(buf);
		return encoded_buf;
	}
}


static int
get_next_field(char **p, int *len, char **field)
{
	LclBuffer	*buf;

	*field = (char *)NULL;

	buf = _LclBuffer_create(256);
	if(buf == (LclBuffer *)NULL)
		return -1;

	while(*len > 0){
		int comp_len = _lcl_strncmp_with_string_list(*p, *len, LclSoftReturnStringList, 0);
		if(comp_len){
			*p += comp_len;
			*len -= comp_len;
		}
		else if((*len >= LclCRLFStringLength) && !strncmp((*p), LclCRLFString, LclCRLFStringLength)){
			*p += LclCRLFStringLength;
			*len -= LclCRLFStringLength;
			break;
		}
		else{
			if(_LclBuffer_add(buf, *p, 1) != 0){
				_LclBuffer_destroy(buf);
				return -1;
			}
			(*p)++;
			(*len)--;
		}
	}

	*field = _LclBuffer_get_string(buf);
	_LclBuffer_destroy(buf);

	return 0;
}

static int
add_new_field(FieldList **list, char *new_field, int new_field_len)
{
	FieldList	*new_list, *ptr, *prev_ptr;

	new_list = (FieldList *)malloc(sizeof(struct _FieldList));
	if(new_list == (FieldList *)NULL)
		return -1;
	new_list->field = new_field;
	new_list->length = new_field_len;
	new_list->next = (FieldList *)NULL;

	if(*list == (FieldList *)NULL){
		*list = new_list;
		return 0;
	}
	else{
		prev_ptr = *list;
		ptr = (*list)->next;
		while(ptr){
			prev_ptr = ptr;
			ptr = ptr->next;
		}
		prev_ptr->next = new_list;
		return 0;
	}
}

static void
destroy_field_list(FieldList *list)
{
	FieldList	*ptr, *prev_ptr;

	ptr = list;
	while(ptr){
		prev_ptr = ptr;
		ptr = ptr->next;
		if(prev_ptr->field)
			free(prev_ptr->field);
		free(prev_ptr);
	}
}

static char *
make_new_header(FieldList *list)
{
	FieldList	*list_ptr;
	int	total_len;
	char	*new_header, *ptr;

	total_len = 0;

	list_ptr = list;
	while(list_ptr){
		total_len += list_ptr->length + LclCRLFStringLength;
		list_ptr = list_ptr->next;
	}

	new_header = (char *)malloc(total_len + 1);
	if(new_header == (char *)NULL)
		return (char *)NULL;

	ptr = new_header;
	list_ptr = list;
	while(list_ptr){
		memcpy(ptr, list_ptr->field, list_ptr->length);
		ptr += list_ptr->length;
		memcpy(ptr, LclCRLFString, LclCRLFStringLength);
		ptr += LclCRLFStringLength;
		list_ptr = list_ptr->next;
	}
	*ptr = (char)0;

	return new_header;
}
		
char *
_lcl_mime_encode_header(char *header, int header_len, LclMailEncoding encoding, char *charset)
{
	FieldList	*field_list;
	char	*new_header;

	field_list = (FieldList *)NULL;
	while(1){
		char	*field, *new_field;

		if(get_next_field(&header, &header_len, &field) != 0){
			destroy_field_list(field_list);
			return (char *)NULL;
		}
		if(field == (char *)NULL)
			break;

		new_field = encode_one_line(field, strlen(field), encoding, charset);
		free(field);

		if(new_field){
			if(add_new_field(&field_list, new_field, strlen(new_field))){
				destroy_field_list(field_list);
				return (char *)NULL;
			}
		}
		else{
			destroy_field_list(field_list);
			return (char *)NULL;
		}
	}

	new_header = make_new_header(field_list);
	destroy_field_list(field_list);

	return new_header;
}


/* decode */

static int
encode_check(char *ptr, int len)
{
	char	*org_ptr = ptr;
	int	question_num = 0;

	if(_lcl_strncmp_with_string_list(ptr, len, StrListEncodeBegin, 0) <= 0)
		return 0;

	ptr += 2;
	len -= 2;
	while(len){
		if(*ptr == '?')
			question_num++;
		ptr++;
		len--;
		if(question_num >= 3)
			break;
	}
	if(len <= 0)
		return 0;

	if(*ptr != '=')
		return 0;
	ptr++;
	len--;

	return ptr - org_ptr;
}

static int
get_next_decode_segment(char **ptr, int *len, char **token, int *token_len)
{
	int	encode_len;
	int	comp_len;

	*token = (char *)NULL; 
	while(*len > 0){
		encode_len = encode_check(*ptr, *len);
		if(encode_len > 0){
			if(*token == (char *)NULL){
				*token = *ptr;
				*token_len = encode_len;
				*ptr += encode_len;
				*len -= encode_len;
				return 1;
			}
			else{
				*token_len = *ptr - *token;
				return 0;
			}
		}

		if(*token == (char *)NULL)
			*token = *ptr;
		(*ptr)++;
		(*len)--;
	}
	if(*token == (char *)NULL)
		return -1;

	*token_len = *ptr - *token;
	return 0;
}

static int
get_next_decode_token(char **ptr, int *len, char **token, int *token_len)
{
	if((*len > 0) && (**ptr == '?')){
		(*ptr)++;
		(*len)--;
	}

	*token = *ptr;
	while(*len > 0){
		if(**ptr == '?'){
			*token_len = *ptr - *token;
			return 1;
		}
		else{
			(*ptr)++;
			(*len)--;
		}
	}
	return -1;
}

static int
decode_segment(LclBuffer *buf, char *ptr, int len, char **charset)
{
	LclMailEncoding	encoding;
	char	*token;
	int	token_len;
	
	*charset == (char *)NULL;

	if((len < 1) || (*ptr != '='))
		return -1;
	ptr++;
	len--;

	if((len < 1) || (*ptr != '?'))
		return -1;
	ptr++;
	len--;

	if (get_next_decode_token(&ptr, &len, &token, &token_len) > 0){
		*charset = (char *)malloc(token_len + 1);
		if(*charset == (char *)NULL)
			return -1;
		memcpy(*charset, token, token_len);
		(*charset)[token_len] = (char)0;
	}
	else
		return -1;
	if (get_next_decode_token(&ptr, &len, &token, &token_len) > 0){
		if((token[0] == 'Q') || (token[0] == 'q'))
			encoding = LclQPEncoding;
		else if((token[0] == 'B') || (token[0] == 'b'))
			encoding = LclBase64Encoding;
		else
			goto error_return;
	}
	else
		goto error_return;

	if (get_next_decode_token(&ptr, &len, &token, &token_len) > 0){
		switch(encoding){
			case LclQPEncoding:
				if(_lcl_add_qp_decode(buf, token, token_len) != 0)
					goto error_return;
				break;
			case LclBase64Encoding:
				if(_lcl_add_b64_decode(buf, token, token_len) != 0)
					goto error_return;
				break;
			default:
				goto error_return;
				break;
		}
	}
	else
		goto error_return;

	return 0;

error_return:
	if(*charset){
		free(*charset);
		*charset = (char *)NULL;
	}
	return -1;
}

static int
check_linear_white_space(char *ptr, int len)
{
	while(len > 0){
		if((len >= LclCRLFStringLength) && !strncmp(ptr, LclCRLFString, LclCRLFStringLength)){
			ptr += LclCRLFStringLength;
			len -= LclCRLFStringLength;
		}
		else if((*ptr == ' ') || (*ptr == '\t')){
			ptr++;
			len--;
		}
		else
			return 0;
	}
	return 1;
}

char *
_lcl_mime_decode_header(char *header, int length, char **charset)
{
	LclBuffer	*buf;
	char	*ptr;
	int	len;
	int	ret;
	int	comp_len;
	char	*decode_buf;
	char	*segment_charset;

	*charset = (char *)NULL;

	buf = _LclBuffer_create(length);
	if(buf == (LclBuffer *)NULL)
		return (char *)NULL;

	while(length > 0){
		ret = get_next_decode_segment(&header, &length, &ptr, &len);
		if(ret > 0){
			char	*segment_charset = (char *)NULL;
			if (decode_segment(buf, ptr, len, &segment_charset) != 0){
				if(_LclBuffer_add(buf, ptr, len) != 0){
					_LclBuffer_destroy(buf);
					return (char *)NULL;
				}
			}
			if(segment_charset){
				if(*charset)
					free(*charset);
				*charset = segment_charset;
			}
		}
		if(ret == 0){
			if(!check_linear_white_space(ptr, len)){
				if(_LclBuffer_add(buf, ptr, len) != 0){
					_LclBuffer_destroy(buf);
					return (char *)NULL;
				}
			}
		}
		if(ret < 0)
			break;
	}
	decode_buf = _LclBuffer_get_string(buf);
	_LclBuffer_destroy(buf);
	return decode_buf;
}
