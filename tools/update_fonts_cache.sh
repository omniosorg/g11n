#!/bin/bash
#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").  
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# Find outdated fonts.cache-1 (due to extracting font files from
# g11n binary archives and/or from g11n source build results)
# and update by executing fc-cache.
# 

find /usr/openwin ! -local -prune -o -name 'fonts.cache-1' -print | while read f
do
	d=`dirname "$f"`
	if [ "$d" -nt "$f" ]
	then
		echo "updating $f..."
		fc-cache "$d"
	fi
done

exit 0
