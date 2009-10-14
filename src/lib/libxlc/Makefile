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

TARGETDIR =	$(FILEROOT)/usr/openwin/lib/locale/common

LIB_SRC =	lcDBUnicode.c  lcPublicUnicode.c  lcUNICODE.c
HEADERS =	utf8_defs.h  XlcDBUnicode.h  XlcPublicUnicode.h

OBJS =		$(LIB_SRC:%.c=%.o)

LIB_SO	= 	xlcUTF-8.so.2

FILES =	

SUBDIRS = 

TARGETDIRS =	$(TARGETDIR)

all: $(TARGETDIRS) $(LIB_SO) $(SUBDIRS)

include $(SRC)/src/Makefile.targ

CFLAGS		= -O -Xa -Kpic -DXTHREADS  -D_REENTRANT -DXUSE_MTSAFE_API -DUSE_ICONV -DSUNSOFT -DMALLOC_0_RETURNS_NULL
CFLAGS_sparcv9	= -xO3 -m64 -dalign -xregs=no%appl -xspace -W0,-Lt -Wd,-xsafe=unboundsym $(CFLAGS)
CFLAGS_amd64	= -m64 -Ui386 -U__i386 -W0,-y-a  $(CFLAGS)
LDFLAGS		= -G -z text -z defs -h $(LIB_SO) -L/usr/openwin/bin -L/usr/bin -lX11 -lc

install: $(LIB_SO) $(TARGETDIR)/$(MACH64) $(SUBDIRS)
	$(INS) $(TARGETDIR) $(LIB_SO)
	$(INS) $(TARGETDIR)/$(MACH64) $(MACH64)/$(LIB_SO)

$(TARGETDIR)/$(MACH64):
	$(INSDIR) $(TARGETDIR)/$(MACH64)

clean: objclean $(SUBDIRS)
	$(RM) -r *.so* *.o $(MACH64)

$(LIB_SO): $(OBJS) 
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	$(LD) $(LDFLAGS) -o $(MACH64)/$@ $(OBJS:%=$(MACH64)/%)

$(MACH64):
	$(INSDIR) $(MACH64)

$(OBJS): $(MACH64) $$(@:%.o=%.c) 
	$(CC) -c -o $@ $(CFLAGS) $*.c
	$(CC) -c -o $(MACH64)/$@ $(CFLAGS_$(MACH64))  $*.c

FRC:


