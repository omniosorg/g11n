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
#ident	"@(#)Makefile	1.0	06/03/07	SMI"
#

include $(SRC)/Makefile.master

install		:=	TARGET = install
all		:=	TARGET = all
lint		:=	TARGET = lint
clean		:=	TARGET = clean
clobber		:=	TARGET = clobber

#LOCALE:sh = basename `pwd`
LOCALE=	

TARGETDIR =	$(FILEROOT)/usr/lib/lp/locale/en_US.UTF-8/mp/

CNV_SRC = 	ctlarabic.c ctlhebrew.c ctlhindi.c ctlthai.c xubig5.c xufallback.c xugb2312.c  xugbk.c \
		xuiso8859.c xujisx0201.c xujisx0208.c xujisx0212.c xujohap92.c xuunicode.c

ISOSHARED       = xuiso88591.so xuiso88592.so xuiso88594.so xuiso88595.so \
                  xuiso88597.so xuiso88599.so xuiso885910.so

OTHERSHARED     = xubig5.so xugb2312.so xujisx0201.so xujisx0208.so \
                  xujisx0212.so xujohap92.so xuunicode.so xugbk.so \
                  ctlarabic.so ctlthai.so ctlhebrew.so ctlhindi.so \
                  xufallback.so


SHARED          = $(ISOSHARED) $(OTHERSHARED)
OBJS            = $(OTHEROBJS) $(ISOOBJS)
OTHEROBJS       = $(OTHERSHARED:%.so=%.o)
ISOOBJS         = $(ISOSHARED:%.so=%.o)

SHARED_INS	= $(SHARED:%=$(TARGETDIR)/%)

FILES =  

SUBDIRS = 

TARGETDIRS = $(TARGETDIR)

all: $(TARGETDIRS) $(SHARED) $(SUBDIRS)

include $(SRC)/src/Makefile.targ

CFLAGS		=  -O -xildoff -xc99=%none -I.. -KPIC -D_REENTRANT 

LDFLAGS		= -G -z defs

xuiso88591.so   := CFLAGS += -DISO_8859_1
xuiso88592.so   := CFLAGS += -DISO_8859_2
xuiso88594.so   := CFLAGS += -DISO_8859_4
xuiso88595.so   := CFLAGS += -DISO_8859_5
xuiso88597.so   := CFLAGS += -DISO_8859_7
xuiso88599.so   := CFLAGS += -DISO_8859_9
xuiso885910.so  := CFLAGS += -DISO_8859_10


install: $(SHARED_INS)
	$(INSDIR) $(TARGETDIR)
#	$(INS) $(TARGETDIR) $ 

$(SHARED_INS): $(SHARED)
	$(INS) $(@D) $(@F)

clean: objclean $(SUBDIRS)
	$(RM) -r $(TARGETDIR)/*.so
	$(RM) *.so

$(ISOSHARED): xuiso8859.c
	$(CC) $(CFLAGS) -c -o $@.o xuiso8859.c
	$(LD) $(LDFLAGS) -o $@ $@.o $(LDLIBS)

$(OTHERSHARED): $$(@:.so=.c)
	$(CC) $(CFLAGS) -c -o $@.o $(@:.so=.c)
	$(LD) $(LDFLAGS) -o $@ $@.o $(LDLIBS)

FRC:


