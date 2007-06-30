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

install		:=	TARGET = install
all		:=	TARGET = all
lint		:=	TARGET = lint
clean		:=	TARGET = clean
clobber		:=	TARGET = clobber

#LOCALE:sh = basename `pwd`
LOCALE=	

TARGETDIR =	$(FILEROOT)/usr/lib/

SPEC_SRC =	auto_ef.spec

MAPFILE =	mapfile

FILES =  

SUBDIRS = 	

TARGETDIRS = 	$(TARGETDIR)

install all: $(MAPFILE)

include $(SRC)/src/Makefile.targ


clean:
	$(RM) $(MAPFILE) $(MAPFILE:%=%.$(MACH64)) 

$(MAPFILE): $(SPEC_SRC)
	/usr/lib/abi/spec2map -p -a $(MACH) -l libauto_ef   -v versions  -o $@ $?
	/usr/lib/abi/spec2map -p -a $(MACH64) -l libauto_ef   -v versions  -o $(@:%=%.$(MACH64)) $?

FRC:


