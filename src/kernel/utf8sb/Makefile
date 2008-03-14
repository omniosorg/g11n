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

TARGETDIR =	$(FILEROOT)/kernel/strmod

SOURCES =	u8sb.c common.h iso-8859-1_to_utf8.h iso-8859-2_to_utf8.h koi8-r_to_utf8.h utf8_to_iso-8859-1.h utf8_to_iso-8859-2.h utf8_to_koi8-r.h

MODULES	= 	u8koi8 u8lat1 u8lat2

FILES =		$(MODULES) $(MODULES:%=$(MACH64)/%)

SUBDIRS = 

TARGETDIRS =	$(TARGETDIR) $(TARGETDIR)/$(MACH64)

CFLAGS		= -O -xildoff -xc99=%none -D_KERNEL -D_SYSCALL32
CFLAGS_sparcv9	= -xO3 -xarch=v9 -dalign -xregs=no%appl -xspace -W0,-Lt -Wd,-xsafe=unboundsym -D_SYSCALL32_IMPL -D_ELF64 $(CFLAGS)
CFLAGS_amd64	= -xarch=generic64 -Ui386 -U__i386 -W0,-y-a -D_SYSCALL32_IMPL -D_ELF64 $(CFLAGS)
LDFLAGS		= -r -dn

u8lat1.o := CFLAGS += -DISO_8859_1                                                                                                                                                                                     
u8lat2.o := CFLAGS += -DISO_8859_2                                                                                                                                                                                     
u8koi8.o := CFLAGS += -DKOI8_R  


all: $(MODULES) $(SUBDIRS)

include $(SRC)/src/Makefile.targ

install: $(FILES.obj) $(SUBDIRS)

clean: $(SUBDIRS)
	$(RM) -r *.so* *.o $(MACH64)

$(MODULES:%=%.o): $(MACH64) $(SOURCES)
	$(CC) -c -o $@ $(CFLAGS) u8sb.c
	$(CC) $(CFLAGS_$(MACH64)) -c -o $(MACH64)/$@ u8sb.c

$(MODULES): $$(@:%=%.o)
	$(LD) $(LDFLAGS) -o $@ $^
	$(LD) $(LDFLAGS) -o $(MACH64)/$@ $(^:%=$(MACH64)/%)

$(MACH64):
	$(INSDIR) $(MACH64)


FRC:

