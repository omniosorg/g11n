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

LOCALE:sh = basename `pwd`

TARGETDIR =	$(FILEROOT)/usr/openwin/lib/locale/iso8859-1



#XOM_SO	= xomEuro.so.2

FILES =  Compose XI18N_OBJS XLC_LOCALE

SUBDIRS = 

TARGETDIRS = $(TARGETDIR)

all: $(TARGETDIRS) $(MACH64)  $(SUBDIRS)

include $(SRC)/src/Makefile.targ

CFLAGS		= -KPIC -O -Dsun -DSVR4 -DXTHREADS -D_REENTRANT -DXUSE_MTSAFE_API -DXUSE_NETDB_R_API -DMALLOC_0_RETURNS_NULL -DSUNSOFT -I/usr/openwin/share/include/X11
LDFLAGS		= -Bdynamic -G -ztext -z defs -h xomEuro.so.2 -L/usr/openwin/lib -lX11 -lc -xc99=%none -Xa
CFLAGS_sparcv9	= $(CFLAGS) -xarch=v9 -dalign
LDFLAGS_sparcv9	= $(LDFLAGS) -xarch=v9

CFLAGS_amd64	= $(CFLAGS) -xarch=amd64 -dalign
LDFLAGS_amd64	= $(LDFLAGS) -xarch=amd64

install: all $(FILES.obj) $(SUBDIRS)
	$(INSDIR) $(TARGETDIR)/$(MACH64)
	$(INS) $(TARGETDIR) 
	$(INS) $(TARGETDIR)/$(MACH64) 

clean: objclean $(SUBDIRS)
	$(RM) -r $(TARGETDIR)
	$(RM) *.o *.so*
	$(RM) -r $(MACH64)

$(MACH64):
	$(INSDIR) $(MACH64)



FRC:


