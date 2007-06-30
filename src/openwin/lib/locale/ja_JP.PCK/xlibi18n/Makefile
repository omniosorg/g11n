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
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

#
#ident  "@(#)Makefile 1.10 03/02/25 SMI"
#

include	$(SRC)/Makefile.master

SRCS=		lcSjis.c

LOCALE=		ja_JP.PCK

OBJS=		$(SRCS:%.c=%.o)
OBJS64=		$(OBJS:%=$(MACH64)/%)
SONAME=		xlibi18n_PCK.so.2

CLEANFILES=	*.o *.so.* *.so

INCFLAGS=	-I/usr/openwin/include -I/usr/openwin/include/X11 \
		-I../../../../include
DEFFLAGS=	-Dsun -DSVR4 -DXTHREADS -DXUSE_MTSAFE_API \
		-DMALLOC_0_RETURNS_NULL -DSUNSOFT -DDYNAMIC_LOAD

CFLAGS +=	-K pic -D_REENTRANT $(INCFLAGS) $(DEFFLAGS) $(XREGSFLAG)
CFLAGS64 +=	-K pic -D_REENTRANT $(INCFLAGS) $(DEFFLAGS) $(XREGSFLAG64)

LDFLAGS=	-G -z defs -z text -h $(FILES) -ldl

# building shared objects
#ZDEFS=          -z defs
DYNFLAGS=       -dy -G -h $(SONAME) -z text $(ZDEFS)
#BUILD.SO=       $(CC) -o $@ -G $(DYNFLAGS) $(PICS) $(LDLIBS)

LDLIBS=		-R/usr/openwin/lib -lX11
LDLIBS64=	-R/usr/openwin/lib/64 -lX11

LOCDIR=		$(FILEROOT)/usr/openwin/lib/locale/$(LOCALE)
LOCDIR64=	$(LOCDIR)/$(MACH64)
TARGETDIRS=	$(LOCDIR) $(LOCDIR64)

all:=           TARGET= all
install:=       TARGET= install
clean:=         TARGET= clean

.KEEP_STATE:

all:
install:	$(FILES.obj)
install_pck:	$(FILES) $(L10N_DIR) $(PROTOFILES)

all:	$(MACH64) $(SONAME)

install: all $(TARGETDIRS)
	$(INS) $(LOCDIR) $(SONAME)
	$(INS) $(LOCDIR64) $(MACH64)/$(SONAME)

$(TARGETDIRS):
	$(INSDIR) $@

$(MACH64):
	$(INSDIR) $(MACH64)

$(SONAME): $(OBJS)
	$(LD) -o $@ $(DYNFLAGS) $(OBJS) $(LDLIBS)
	$(LD) -o $(MACH64)/$@ $(DYNFLAGS) $(OBJS64) $(LDLIBS64)

$(OBJS): $$(@:%.o=%.c)
	$(CC) $(CFLAGS) -c -o $@ $*.c
	$(CC) $(CFLAGS64) -c -o $(MACH64)/$@ $*.c

clean:
	$(RM) -r $(SONAME) $(OBJS) $(MACH64)

FRC:
