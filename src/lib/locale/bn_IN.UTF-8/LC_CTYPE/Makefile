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
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

include $(SRC)/Makefile.master
#include ../../Makefile.master.ow


DIR:sh = basename `pwd`

LOCALE:sh = cd ..;basename `pwd`

FILES = LCL_DEF

TARGETDIR = $(FILEROOT)/usr/lib/locale/$(LOCALE)/$(DIR)

TARGETDIRS = $(TARGETDIR)

SUBDIRS = 

all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber

all: $(TARGETDIRS)

include $(SRC)/src/Makefile.targ

install: all $(FILES.obj) $(SUBDIRS)

clean: objclean
	$(RM) -r $(TARGETDIR)

FRC:

