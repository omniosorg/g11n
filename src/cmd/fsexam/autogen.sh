#!/bin/sh
# Run this to generate all the initial makefiles, etc.

# 
# CDDL HEADER START
# 
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
# 
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
# 
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
# 
# CDDL HEADER END
# 

# 
# Copyright 2008 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms.
#

set -e

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir
PROJECT=fsexam
TEST_TYPE=-f
FILE=src/fsexam.c
ACLOCAL=${ACLOCAL-aclocal}
INTLTOOLIZE=${INTLTOOLIZE-intltoolize}
LIBTOOLIZE=${LIBTOOLIZE-libtoolize}
AUTOMAKE=${AUTOMAKE-automake}
AUTOHEADER=${AUTOHEADER-autoheader}
AUTOCONF=${AUTOCONF-autoconf}
GNOMEDOCCOMMON=${GNOMEDOCCOMMON-gnome-doc-common}
LIBTOOLIZE_FLAGS="--copy --force --automake"
INTLTOOLIZE_FLAGS="--copy --force --automake"

DIE=0

have_intltool=false
if $INTLTOOLIZE --version < /dev/null > /dev/null 2>&1 ; then
    intltool_version=`$INTLTOOLIZE --version | sed 's/^[^0-9]*\([0-9].[0-9.]*\).*/\1/'`
    if [ X${intltool_version} != "X" ]; then
        have_intltool=true
    fi
fi

if $have_intltool ; then : ; else
    echo 
    echo "You must have intltool installed to compile $PROJECT."
    DIE=1
fi

have_libtool=false
if $LIBTOOLIZE --version < /dev/null > /dev/null 2>&1 ; then
	libtool_version=`$LIBTOOLIZE --version | sed 's/^[^0-9]*\([0-9].[0-9.]*\).*/\1/'`
	case $libtool_version in
	    1.4*|1.5*|1.6*|1.7*|2*)
		have_libtool=true
		;;
	esac
fi
if $have_libtool ; then : ; else
	echo
	echo "You must have libtool 1.4 installed to compile $PROJECT."
	echo "Install the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
fi

($AUTOCONF --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile $PROJECT."
	echo "libtool the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
}

have_automake=false
need_libtoolize=true
if $AUTOMAKE --version < /dev/null > /dev/null 2>&1 ; then
	automake_version=`$AUTOMAKE --version | grep 'automake (GNU automake)' | sed 's/^[^0-9]*\(.*\)/\1/'`
	case $automake_version in
	   1.2*|1.3*|1.4) 
		;;
	   1.4*)
	   	have_automake=true
	        need_libtoolize=false
		;;
	   *)
		have_automake=true
		;;
	esac
fi
if $have_automake ; then : ; else
	echo
	echo "You must have automake 1.4-p1 installed to compile $PROJECT."
	echo "Get ftp://ftp.gnu.org/pub/gnu/automake/automake-1.4-p1.tar.gz"
	echo "(or a newer version if it is available)"
	DIE=1
fi

if test "$DIE" -eq 1; then
	exit 1
fi

test $TEST_TYPE $FILE || {
	echo "You must run this script in the top-level $PROJECT directory"
	exit 1
}

if test -z "$AUTOGEN_SUBDIR_MODE"; then
        if test -z "$*"; then
                echo "I am going to run ./configure with no arguments - if you wish "
                echo "to pass any to it, please specify them on the $0 command line."
        fi
fi

echo Running $INTLTOOLIZE $INTLTOOLIZE_FLAGS
$INTLTOOLIZE $INTLTOOLIZE_FLAGS

case $need_libtoolize in
   true)
   	echo Running $LIBTOOLIZE $LIBTOOLIZE_FLAGS
   	$LIBTOOLIZE $LIBTOOLIZE_FLAGS
	;;
esac

echo Running $ACLOCAL $ACLOCAL_FLAGS
$ACLOCAL $ACLOCAL_FLAGS

# optionally run autoheader
if $AUTOHEADER --version  < /dev/null > /dev/null 2>&1; then
	echo Running $AUTOHEADER
	$AUTOHEADER
fi

echo Running $GNOMEDOCCOMMON --copy
$GNOMEDOCCOMMON --copy
echo Running $AUTOMAKE -a -c -f $am_opt
$AUTOMAKE -a -c -f $am_opt
echo Running $AUTOCONF
$AUTOCONF
cd $ORIGDIR

if test -z "$AUTOGEN_SUBDIR_MODE"; then
	echo Running $srcdir/configure
        $srcdir/configure --enable-maintainer-mode "$@"

        echo 
        echo "Now type 'make' to compile $PROJECT."
fi
