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

TARGETDIR = 	$(FILEROOT)/usr/openwin/lib/X11/fonts/encodings

TARGETDIRS = 	$(TARGETDIR)

ENCODINGS = 	ISOLatin2TT.enc ISOLatin4.enc ISOLatin4TT.enc ISOLatin5.enc ISOLatinCyrillic.enc \
		ISOLatinGreek.enc ansi-1251.enc big5-1.enc cns11643-1.enc cns11643-2.enc cns11643-3.enc \
		gb18030.2000-0.enc gb18030.2000-1.enc gb2312.enc gbk.enc hindi-1.enc hkscs-1.enc \
		iso8859-13.enc iso8859-15.enc iso8859-6.enc iso8859-8.enc koi8-r.enc ksc5601-1987.enc \
		ksc5601-1992.enc ksc5601.enc sun.unicode.india-0.enc sun_eu_greek.enc tis620.enc \
		jis201.enc jis208.enc jis212.enc jis213-1.enc jis213-2.enc sunudcja.enc

ENC_DIRS = 	encodings.dir encodings.dir.SUNW1251f encodings.dir.SUNW5ttf encodings.dir.SUNWarrf \
		encodings.dir.SUNWcttf encodings.dir.SUNWgttf encodings.dir.SUNWhkttf encodings.dir.SUNWhttf \
		encodings.dir.SUNWi13rf encodings.dir.SUNWi15rf encodings.dir.SUNWi2rf encodings.dir.SUNWi4rf \
		encodings.dir.SUNWi5rf encodings.dir.SUNWi7rf encodings.dir.SUNWi8rf encodings.dir.SUNWi9rf \
		encodings.dir.SUNWinttf encodings.dir.SUNWkoi8f encodings.dir.SUNWkttf encodings.dir.SUNWtxfnt \
		encodings.dir.SUNWjxcft

ENC_TARGET = $(ENC_DIRS:%=$(TARGETDIR)/%)

FILES = 	$(ENCODINGS) 

SUBDIRS = 

all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
objclean := 	TARGET = objclean
clobber :=	TARGET= clobber

all: $(TARGETDIRS)

include $(SRC)/src/Makefile.targ

install: all $(FILES.obj) $(SUBDIRS) $(ENC_TARGET)

clean: objclean $(SUBDIRS)
	$(RM) -r $(TARGETDIR)

$(ENC_TARGET): $$(@:$(TARGETDIR)/%=%)
	sed -e '/\#/d' $? > $@

FRC:
