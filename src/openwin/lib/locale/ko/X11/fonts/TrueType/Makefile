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
# Copyright (c) 2006, by Sun Microsystems, Inc.
# All rights reserved.
#

include $(SRC)/Makefile.master

LOCALE = 	ko

FILES =		fonts.alias fonts.alias.base fonts.alias.ext fonts.scale fonts.scale.base fonts.scale.ext h2drrm.ttf h2hsrm.ttf h2mjsm.ttf 
UTF_FONT = 	h2gtrm.ttf

TARGETDIR = 	$(FILEROOT)/usr/openwin/lib/locale/$(LOCALE)/X11/fonts/TrueType
UTF_TARGETDIR = $(FILEROOT)/usr/openwin/lib/locale/$(LOCALE).UTF-8/X11/fonts/TrueType

TARGETDIRS = 	$(TARGETDIR)

SUBDIRS = 

all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
objclean := 	TARGET = objclean
clobber :=	TARGET= clobber

all: $(TARGETDIRS)
	$(INSDIR) $(UTF_TARGETDIR)

include $(SRC)/src/Makefile.targ

install: all $(FILES.obj) $(SUBDIRS)
	$(INS) $(UTF_TARGETDIR) $(UTF_FONT)

clean: objclean $(SUBDIRS)
	$(RM) -r $(TARGETDIR) $(UTF_TARGETDIR)

FRC:
