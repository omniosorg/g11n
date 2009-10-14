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

#LOCALE:sh = basename `pwd`
LOCALE=	en_US.UTF-8

TARGETDIR =	$(FILEROOT)/usr/lib

LL_SRC = 	layout_default.c layout_modifier.c layout_wrap.c layout.c

HEADERS =	ctl_threads.h layout.h layout_int.h

OBJS	= $(LL_SRC:%.c=%.o)
OBJS64	= $(LL_SRC:%.c=$(MACH64)/%.o)

LIBLAYOUT_SO	= liblayout.so.1

FILES =  

SUBDIRS = 

TARGETDIRS = $(TARGETDIR)

all: $(TARGETDIRS) $(MACH64) $(LIBLAYOUT_SO) $(SUBDIRS)

include $(SRC)/src/Makefile.targ

CFLAGS		=  -xildoff -xc99=%none  -KPIC -D_REENTRANT -I. 

CFLAGS_sparcv9	= -xO3 -m64 -dalign -xregs=no%appl -xspace -W0,-Lt -Wd,-xsafe=unboundsym $(CFLAGS)
CFLAGS_amd64	= -m64 -Ui386 -U__i386 -W0,-y-a $(CFLAGS)

LDFLAGS	= -G -z defs -z text -h liblayout.so.1 -M mapfile -ldl -lc


install: $(FILES.obj) $(SUBDIRS) all
	$(INSDIR) $(TARGETDIR)/$(MACH64)
	$(INS) $(TARGETDIR) $(LIBLAYOUT_SO)
	$(INS) $(TARGETDIR)/$(MACH64) $(MACH64)/$(LIBLAYOUT_SO)
	$(INSDIR) $(FILEROOT)/usr/include/sys
	$(INS) $(FILEROOT)/usr/include/sys layout.h

clean: objclean $(SUBDIRS)
	$(RM) *.o *.so*
	$(RM) -r $(MACH64)

$(MACH64):
	$(INSDIR) $(MACH64)

$(LIBLAYOUT_SO): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	$(LD) $(LDFLAGS) -o $(MACH64)/$@ $(OBJS64)

$(OBJS): $$(@:%.o=%.c)
	$(CC) $(CFLAGS) -c -o $@ $*.c
	$(CC) $(CFLAGS_$(MACH64)) -c -o $(MACH64)/$@ $*.c

FRC:


