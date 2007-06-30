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

#
#ident  "@(#)Makefile 1.14     04/08/24 SMI"
#
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# jos/uts/Makefile
#
# include global definitions
include		$(SRC)/Makefile.master
#include		${SRC}/jos/Makefile.master.os

#
# List of architectures to build as part of the standard build.
#
i386_ARCHITECTURES = i86
amd64_ARCHITECTURES = amd64
ppc_ARCHITECTURES = # ppc
sparcv7_ARCHITECTURES = sparc
sparcv9_ARCHITECTURES = sparcv9
sparc_ARCHITECTURES =
$(BUILD64)sparc_ARCHITECTURES += $(sparcv9_ARCHITECTURES)
$(BUILD64)i386_ARCHITECTURES += $(amd64_ARCHITECTURES)

all		:=	TARGET= all
install		:=	TARGET= install
install_h	:=	TARGET= install_h
clean		:=	TARGET= clean
clobber		:=	TARGET= clobber
lint		:=	TARGET= lint
clean.lint	:=	TARGET= clean.lint

.KEEP_STATE:

all install clean clobber lint clean.lint: $($(MACH)_ARCHITECTURES)

$($(MACH)_ARCHITECTURES) $(RC): FRC
	@cd $@; pwd; $(MAKE) $(TARGET)

#
# The following is the list of directories which contain Makefiles with
# targets to install header file. The machine independent headers are
# installed by invoking the Makefile in the directory containing the
# header files. Machine and architecture dependent headers are installed
# by invoking the main makefile for that architecture/machine which,
# in turn, is responsible for invoking the Makefiles which install headers.
# It is done this way so as not to assume that all of the header files in
# the architecture/machine dependent subdirectories are in completely
# isomorphic locations.
#
HDRDIRS= common/jfp

install_h: $(HDRDIRS)

$(HDRDIRS): FRC
	@cd $@; pwd; $(MAKE) install_h

FRC:

