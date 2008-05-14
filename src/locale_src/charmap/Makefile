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
#ident  "@(#)Makefile 1.0 10/03/07 SMI"
#
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

include $(SRC)/Makefile.master


all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean


CHARMAPS = ansi-1251 iso-8859-1 iso-8859-13 iso-8859-15 iso-8859-2 iso-8859-5 iso-8859-6 iso-8859-7 iso-8859-8 iso-8859-9 koi8-r utf-8

TARGETDIR =	$(FILEROOT)/usr/lib/localedef/src/charmaps

FILES =		$(CHARMAPS:%=charmap.%.bz2)

TARGETDIRS =	$(TARGETDIR)


all: $(TARGETDIRS) $(FILES) UTF-8.src.Z

include $(SRC)/src/Makefile.targ

install: $(FILES) UTF-8.src.Z $(TARGETDIRS) $(FILES.obj)
	$(INSDIR) $(FILEROOT)/lib/localedef/charmaps
	$(INS) $(FILEROOT)/lib/localedef/charmaps UTF-8.src.Z

clean:
	$(RM) $(FILES)

charmap.%.bz2: %.charmap
	$(BZIP2) -c $< > $@

%.src.Z: %.charmap
	$(COMPRESS) -c $< > $@
