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
#ident  "@(#)Makefile	1.1	97/09/05 SMI"
#
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# jos/uts/i86/Makefile
#
#
#	This makefile drives the production of all implementation architecture
#	independent modules for the Intel x86 processor.

UTSBASE = ..

include Makefile.i86

#
#
#
all		:=	TARGET= all
install		:=	TARGET= install
clean		:=	TARGET= clean
clobber		:=	TARGET= clobber
lint		:=	TARGET= lint
lintlib		:=	TARGET= lintlib
clean.lint	:=	TARGET= clean.lint

.KEEP_STATE:

all install clean clobber lint lintlib clean.lint: $(KMODS)

$(KMODS) :	FRC
	@cd $@; pwd; $(MAKE) $(TARGET)

FRC:
