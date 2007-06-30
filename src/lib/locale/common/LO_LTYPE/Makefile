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

TARGETDIR =	$(FILEROOT)/usr/lib/locale/common/LO_LTYPE

UMLE_SRC = 	UMLE.c

UMLE_SO		= umle.layout.so.1

FILES =  

SUBDIRS = 

TARGETDIRS = $(TARGETDIR)

all: $(TARGETDIRS) $(MACH64) $(UMLE_SO) $(SUBDIRS)

include $(SRC)/src/Makefile.targ

CFLAGS		=  -O -xildoff -xc99=%none  -K PIC -G -I. -D_REENTRANT -DUTF8_TEST -xCC

CFLAGS_sparcv9	=  -xO3 -xarch=v9 -dalign -xregs=no%appl -xspace -W0,-Lt -Wd,-xsafe=unboundsym $(CFLAGS)
#CFLAGS_amd64	= -xarch=generic64 -Ui386 -U__i386 -W0,-y-a -xildoff -xc99=%none -K PIC -G -I. -D_REENTRANT
CFLAGS_amd64	= -xarch=generic64 -Ui386 -U__i386 -W0,-y-a $(CFLAGS)

install: $(FILES.obj) $(SUBDIRS) all
	$(INSDIR) $(TARGETDIR)/$(MACH64)
	$(INS) $(TARGETDIR) $(UMLE_SO)
	$(INS) $(TARGETDIR)/$(MACH64) $(MACH64)/$(UMLE_SO)

clean: objclean $(SUBDIRS)
	$(RM) -r $(TARGETDIR)
	$(RM) *.o *.so*
	$(RM) -r $(MACH64)

$(MACH64):
	$(INSDIR) $(MACH64)

$(UMLE_SO): $(UMLE_OBJS)
	$(CC) $(CFLAGS) -o $@ $(UMLE_SRC)
	$(CC) $(CFLAGS_$(MACH64))  -o $(MACH64)/$@ $(UMLE_SRC)


FRC:


