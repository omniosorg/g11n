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

TARGETDIR = 	$(FILEROOT)/usr/openwin/lib/X11/fonts/TrueType/ttmap

TARGETDIRS = 	$(TARGETDIR)

TTMAPS = ansi-1251.ttmap big5-1.ttmap \
		cns11643-1.ttmap cns11643-2.ttmap cns11643-3.ttmap cns11643.ttmap gb18030.2000-0.ttmap \
		gb18030.2000-1.ttmap gb2312.ttmap gbk.ttmap hindi-1.ttmap hkscs-1.ttmap \
		iso8859-13.ttmap iso8859-15.ttmap iso8859-2.ttmap iso8859-4.ttmap iso8859-5.ttmap \
		iso8859-6.ttmap iso8859-7.ttmap iso8859-8.ttmap iso8859-9.ttmap koi8-r.ttmap \
		ksc5601-1987.ttmap ksc5601-1992.ttmap sun.unicode.india-0.ttmap sun_eu_greek.ttmap \
		tis620.ttmap \
		jis201.ttmap jis201sjis.ttmap \
		jis208.ttmap jis208sjis.ttmap jis212.ttmap jis212sjis.ttmap jis213-1.ttmap \
		jis213-2.ttmap sunudcja.ttmap

TTMAP_DIRS = 	ttmaps.dir ttmaps.dir.SUNW1251f ttmaps.dir.SUNW5ttf ttmaps.dir.SUNWarrf ttmaps.dir.SUNWcttf \
		ttmaps.dir.SUNWgttf ttmaps.dir.SUNWhkttf ttmaps.dir.SUNWhttf ttmaps.dir.SUNWi13rf \
		ttmaps.dir.SUNWi15rf ttmaps.dir.SUNWi2rf ttmaps.dir.SUNWi4rf ttmaps.dir.SUNWi5rf \
		ttmaps.dir.SUNWi7rf ttmaps.dir.SUNWi8rf ttmaps.dir.SUNWi9rf ttmaps.dir.SUNWinttf \
		ttmaps.dir.SUNWkoi8f ttmaps.dir.SUNWkttf ttmaps.dir.SUNWtxfnt ttmaps.dir.SUNWjxcft

TTMAP_TARGET =	$(TTMAP_DIRS:%=$(TARGETDIR)/%)

FILES = 	$(TTMAPS)

SUBDIRS = 

all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
objclean := 	TARGET = objclean
clobber :=	TARGET= clobber

all: $(TARGETDIRS)

include $(SRC)/src/Makefile.targ

install: all $(FILES.obj) $(SUBDIRS) $(TTMAP_TARGET)

clean: objclean $(SUBDIRS)
	$(RM) -r $(TARGETDIR)


$(TTMAP_TARGET): $$(@:$(TARGETDIR)/%=%)
	sed -e '/\#/d' $? > $@

FRC:
