#
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
#ident	"@(#)Makefile	1.12	97/10/31 SMI"
#
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# jos/locale/ja/mbwcconv/Makefile

include $(SRC)/Makefile.master

SUBDIRS= lddat

all :=		TARGET=all
install :=	TARGET=install
clean :=	TARGET=clean
clobber :=	TARGET=clobber
lint :=		TARGET=lint

LOCALE=	ja_JP.UTF-8
SONAME=	$(RESOLVE_SO)

dummy: all

#include 	../common/Makefile.asian
include 	../common/Makefile.ja

FILES = locale_description locale_map geo_map
CLEANFILES += wdresolve.c

#CPPFLAGS= -D_TS_ERRNO -I/usr/include

CLOBBERFILES=

.KEEP_STATE:

all: $(MACH64)  $(RESOLVE_SO) $(SUBDIRS)

install: all $(FILES.obj) $(SUBDIRS)
	$(INSDIR) $(LC_CTYPE_DIR_64)
	$(INS) $(LC_CTYPE_DIR) $(RESOLVE_SO)
	$(INS) $(LC_CTYPE_DIR_64) $(MACH64)/$(RESOLVE_SO)
	$(INS) $(LC_CTYPE_DIR) LCL_DEF

wdresolve.c: ../ja_JP.eucJP/$@
	rm -f $@
	ln -s ../ja_JP.eucJP/$@ .
