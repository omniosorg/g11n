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

#pragma ident	"@(#)lcl_db_def.h	1.2	00/01/06 SMI"

#define FORM_CATEGORY_NAME     			"FORM_PART"
#define FORM_NAME_NAME	       			"name"
#define FORM_TYPE_NAME 				"type"
#define FORM_MSG_HEADER_CHARSET_NAME		"msg_header_charset"
#define FORM_MSG_BODY_CHARSET_NAME	       	"msg_body_charset"
#define FORM_PLAINTEXT_BODY_CHARSET_NAME	"plaintext_body_charset"
#define FORM_TAGGEDTEXT_HEADER_CHARSET_NAME	"taggedtext_header_charset"
#define FORM_TAGGEDTEXT_BODY_CHARSET_NAME	"taggedtext_body_charset"
#define FORM_MSG_HEADER_ENCODING_NAME		"msg_header_encoding"
#define FORM_MSG_BODY_ENCODING_NAME	       	"msg_body_encoding"
#define FORM_PLAINTEXT_BODY_ENCODING_NAME	"plaintext_body_encoding"
#define FORM_TAGGEDTEXT_HEADER_ENCODING_NAME	"taggedtext_header_encoding"
#define FORM_TAGGEDTEXT_BODY_ENCODING_NAME	"taggedtext_body_encoding"
#define FORM_MAIL_TYPE_NAME	       		"mail_type"
#define FORM_TYPE_DISPLAY      			"Display"
#define FORM_TYPE_INCOMINGSTREAM       		"InComingStream"
#define FORM_TYPE_OUTGOINGSTREAM      		"OutGoingStream"
#define FORM_TYPE_FILE	       			"File"
#define FORM_TYPE_APP	       			"App"
#define FORM_MAIL_TYPE_MIME    			"MIME"
#define FORM_MAIL_TYPE_V3      			"V3"
#define FORM_MAIL_TYPE_822     			"822"
#define FORM_MAIL_TYPE_UNKNOWN	       		"Unknown"

#define ICONV_CATEGORY_NAME	       		"ICONV_PART"
#define ICONV_ENCODING_NAME    			"encoding"
#define ICONV_DIRECTION_NAME		       	"direction"
#define ICONV_CONV_NAME		      		"conversion"
#define ICONV_DIRECTION_BOTH	       		"Both"

#define CS_CATEGORY_NAME       			"CHARSET_PART"
#define CS_NAME_NAME   				"name"
#define CS_MIME_NAME   				"mime_name"
#define CS_V3_NAME	      			"v3_name"
#define CS_ICONV_NAME	      			"iconv_name"
#define CS_MIME_HEADER_NAME    			"mime_header_name"
#define CS_MIME_HEADER_ENCODING		       	"mime_header_encoding"
#define CS_MIME_BODY_NAME	      		"mime_body_name"
#define CS_MIME_BODY_ENCODING	       		"mime_body_encoding"
#define CS_MIME_ATTACH_NAME    			"mime_attach_name"
#define CS_MIME_ATTACH_ENCODING		       	"mime_attach_encoding"
#define CS_ASCII_END		      		"ascii_end"
#define CS_FORMAT      				"format"
#define CS_ASCII_SUPERSET	      		"ascii_superset"
