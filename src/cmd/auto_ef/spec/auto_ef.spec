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

#ident  "@(#)auto_ef.spec 1.3 03/07/18 SMI"

function	auto_ef_file
include		<auto_ef.h>
declaration	size_t auto_ef_file(auto_ef_t **, const char *, int)
version		SUNW_1.1
end		

function	auto_ef_str
include		<auto_ef.h>
declaration	size_t auto_ef_str (auto_ef_t **, const char *, size_t, int)
version		SUNW_1.1
end

function	auto_ef_free
include		<auto_ef.h>
declaration	void auto_ef_free(auto_ef_t *)
version		SUNW_1.1
end

function	auto_ef_get_encoding
include		<auto_ef.h>
declaration	char *auto_ef_get_encoding(auto_ef_t)
version		SUNW_1.1
end

function	auto_ef_get_score
include		<auto_ef.h>
declaration	double auto_ef_get_score(auto_ef_t)
version		SUNW_1.1
end
