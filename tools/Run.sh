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
# A shell script front-end for UTF-8 locale generation.
#

if [ $# -lt 5 ] ; then
	echo "Usage: $0 Charmap LocaleSrc MethodSrc Locale C_opt [ L_opt ]"
	exit 1
fi

CHARMAP=$1
LOCALEDEF=$2
METHOD=$3
LOCALE=$4
COPT=$5
LOPT=
if [ $# -ge 6 ] ; then
	LOPT=$6
fi

MACH=`uname -p`
sparc_COPTFLAG="-xO3 -xregs=no%appl"
i386_COPTFLAG="-xO3"

COPTFLAG=`eval echo \\$${MACH}_COPTFLAG`
CFLAGS="$COPTFLAG -K pic -D PIC -G -Xa -z text -z ignore -D_REENTRANT"

if [ "X$COPT" != "X" ] ; then
	LDF_OPT="-m lp64 "
	CFLAGS="$CFLAGS -m64"
fi

if [ "X$LOPT" != "X" ] ; then
	/usr/bin/localedef $LDF_OPT -c -v  -W cc,"$CFLAGS" \
		-L "$LOPT" -x $METHOD -f $CHARMAP -i $LOCALEDEF $LOCALE
else
	/usr/bin/localedef $LDF_OPT -c -v  -W cc,"$CFLAGS" \
		-x $METHOD -f $CHARMAP -i $LOCALEDEF $LOCALE
fi
RET_CODE=$?

if [ $RET_CODE -ge 2 ] ; then
	echo "$0: localedef returned $RET_CODE. Locale generation failed."
	exit $RET_CODE
fi

exit 0

