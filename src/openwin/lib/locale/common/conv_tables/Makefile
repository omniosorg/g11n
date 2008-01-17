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
# Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

include $(SRC)/Makefile.master

EN_CONV_TABLES = \
		BIG5-1.GLGR \
		GB2312-0.GR \
		GB18030.2000-0.GLGR \
		GB18030.2000-1.GLGR \
		GBK-0.GLGR \
		HKSCS-1.GLGR \
		ISO8859-15.GR \
		ISO8859-2.GR \
		ISO8859-4.GR \
		ISO8859-5.GR \
		ISO8859-6.GLGR \
		ISO8859-7.GR \
		ISO8859-8.GR \
		ISO8859-9.GR \
		JISX0201-0.GR \
		JISX0208-0.GR \
		JISX0212-0.GR \
		JISX0213-1.GR \
		JISX0213-2.GR \
		KSC-JOHAP.GLGR \
		SUN.UNICODE.INDIA-0.GLGR \
		SUNUDCJA.GL \
		TIS620.2533-0.GR \
		UNICODE-FONTSPECIFIC.GLGR

XUCTBLGEN = $(SRC)/src/cmd/xuctblgen/xuctblgen

FILES = $(EN_CONV_TABLES)

TARGETDIR = $(FILEROOT)/usr/openwin/lib/locale/common/conv_tables

TARGETDIRS = $(TARGETDIR)

SUBDIRS = 

all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber

all: $(EN_CONV_TABLES)

include $(SRC)/src/Makefile.targ

install: $(FILES.obj)

$(EN_CONV_TABLES): $(XUCTBLGEN) $(EN_CONV_TABLES:%=%.txt) ../XLC_LOCALE
	# FIXME: xuctblgen use en_US/XLC_LOCALE for common tables
	$(MV) ../../en_US.UTF-8/XLC_LOCALE XLC_LOCALE.en_US
	$(SYMLINK) ../common/XLC_LOCALE ../../en_US.UTF-8/XLC_LOCALE 
	XLOCALEDIR=$(SRC)/src/openwin/lib/locale $(XUCTBLGEN) -locale en_US.UTF-8 -use_common_dir -no_wc_to_cs_tables
	$(MV) XLC_LOCALE.en_US ../../en_US.UTF-8/XLC_LOCALE

$(XUCTBLGEN): 
	cd $(SRC)/src/cmd/xuctblgen/;make all

clean: objclean
	$(RM) $(EN_CONV_TABLES)
	$(RM) -r $(TARGETDIR)

FRC:

