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

TARGETDIR =	$(FILEROOT)/usr/lib

LCL_SRC = 	lcl.c lcl_db.c lcl_str.c lcl_args.c lcl_cs_info.c \
		lcl_segment.c lcl_attr.c lcl_convert.c lcl_ascii.c \
		lcl_create.c lcl_buf.c lcl_mime_encode.c lcl_qencode.c \
		lcl_bencode.c lcl_query_type.c lcl_endecode.c lcl_mimehead.c \
		lcl_str_stdio.c lcl_str_codes.c lcl_parse_header.c

HEADERS =	lcl_cs_info.h lcl_internal.h lcl_str_stdio.h \
		lcl_db.h lcl_locale_db.h lcl_args.h lcl_db_def.h \
		lcl_mime_encode.h lcl.h

OBJS	= $(LCL_SRC:%.c=%.o)
OBJS64	= $(LCL_SRC:%.c=$(MACH64)/%.o)

LIBLCL_SO	= liblcl.so.1

FILES =  

SUBDIRS = 

TARGETDIRS = $(TARGETDIR)

all: $(TARGETDIRS) $(MACH64) $(LIBLCL_SO) $(SUBDIRS)

include $(SRC)/src/Makefile.targ

CFLAGS		=  -xO3 -xildoff -xc99=%none -KPIC -D_REENTRANT -I. -DPU=1 -DNOT_X_ENV -DPARSE -DLCL_LIB 
CFLAGS_sparcv9	= -xO3 -xarch=v9 -dalign -xregs=no%appl -xspace -W0,-Lt -Wd,-xsafe=unboundsym $(CFLAGS)
CFLAGS_amd64	= -xarch=generic64 -Ui386 -U__i386 -W0,-y-a  $(CFLAGS)

LDFLAGS	= -G -z defs -z text -h $(LIBLCL_SO) -M mapfile -ldl -lc

install: $(FILES.obj) $(SUBDIRS) all
	/usr/bin/test -d $(TARGETDIR)/$(MACH64) || /usr/bin/mkdir -p $@$(INSDIR) $(TARGETDIR)/$(MACH64)
	$(INS) $(TARGETDIR) $(LIBLCL_SO)
	$(INS) $(TARGETDIR)/$(MACH64) $(MACH64)/$(LIBLCL_SO)
	/usr/bin/test -d $(FILEROOT)/usr/include/sys || $(INSDIR) $(FILEROOT)/usr/include/sys
	$(INS) $(FILEROOT)/usr/include/sys lcl.h

clean: objclean $(SUBDIRS)
	$(RM) *.o *.so*
	$(RM) -r $(MACH64)

$(MACH64):
	$(INSDIR) $(MACH64)

$(LIBLCL_SO): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	$(LD) $(LDFLAGS) -o $(MACH64)/$@ $(OBJS64)

$(OBJS): $$(@:%.o=%.c)
	$(CC) -c -o $@ $(CFLAGS) $*.c
	$(CC) -c -o $(MACH64)/$@ $(CFLAGS_$(MACH64))  $*.c

FRC:


