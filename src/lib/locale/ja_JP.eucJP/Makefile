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
# Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.
#

include $(SRC)/Makefile.master


all :=		TARGET=all
install :=	TARGET=install
clean :=	TARGET=clean
clobber :=	TARGET=clobber
lint :=		TARGET=lint

LOCALE=	ja_JP.eucJP

SONAME=	$(RESOLVE_SO)

dummy: all

#include 	../common/Makefile.asian
include 	../common/Makefile.ja

FILES = locale_description locale_map

#CPPFLAGS= -D_TS_ERRNO -I/usr/include

CLOBBERFILES=

.KEEP_STATE:

all: $(MACH64)  $(RESOLVE_SO) 

install: all $(FILES.obj)
	$(INSDIR) $(LC_CTYPE_DIR_64)
	$(INS) $(LC_CTYPE_DIR) $(RESOLVE_SO)
	$(INS) $(LC_CTYPE_DIR_64) $(MACH64)/$(RESOLVE_SO)
