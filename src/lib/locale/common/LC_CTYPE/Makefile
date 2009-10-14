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
#ident	"@(#)Makefile	1.0	06/03/07	SMI"
#
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

include $(SRC)/Makefile.master

install		:=	TARGET = install
all		:=	TARGET = all
lint		:=	TARGET = lint
clean		:=	TARGET = clean
clobber		:=	TARGET = clobber

DIR:sh = basename `pwd`

LOCALE:sh = cd ..;basename `pwd`

FILES = ldterm_unicode.dat LCL_DEF_unicode

TARGETDIR = $(FILEROOT)/usr/lib/locale/$(LOCALE)/$(DIR)

RESOLV_SRC = 	wdresolve.c

HEADERS = 	uwidth.h mbstatet.h

RESOLVE_SO	= wdresolve_unicode.so

SUBDIRS = 

TARGETDIRS = $(TARGETDIR)

all: $(TARGETDIRS) $(MACH64) $(RESOLVE_SO)

include $(SRC)/src/Makefile.targ

CFLAGS		=  -O -xildoff -xc99=%none  -K PIC -G -I. -D_REENTRANT

CFLAGS_amd64	= -m64 -Ui386 -U__i386 -W0,-y-a $(CFLAGS)
CFLAGS_sparcv9	= -xO3 -m64 -dalign   -xspace -W0,-Lt  $(CFLAGS)

install: all $(FILES.obj) 
	$(INSDIR) $(TARGETDIR)/$(MACH64)
	$(INS) $(TARGETDIR) $(RESOLVE_SO)
	$(INS) $(TARGETDIR)/$(MACH64) $(MACH64)/$(RESOLVE_SO)

clean: objclean
	$(RM) -r $(TARGETDIR)
	$(RM) *.o *.so*
	$(RM) -r $(MACH64)

$(MACH64):
	$(INSDIR) $(MACH64)

$(RESOLVE_SO): $(RESOLVE_SRC)
	$(CC) $(CFLAGS) -o $@ wdresolve.c
	$(CC) $(CFLAGS_$(MACH64)) -o $(MACH64)/$@ wdresolve.c

FRC:


