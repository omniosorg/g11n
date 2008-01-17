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

LOCALE = en_US.UTF-8

COMMON_CONV_TABLES = \
		ISO8859-5.GR \
		ISO8859-7.GR \
		ISO8859-2.GR \
		ISO8859-4.GR \
		ISO8859-9.GR \
		ISO8859-15.GR \
		JISX0208-0.GR \
		JISX0201-0.GR \
		KSC-JOHAP.GLGR \
		GB2312-0.GR \
		BIG5-1.GLGR \
		TIS620.2533-0.GR \
		ISO8859-6.GLGR \
		ISO8859-8.GR \
		SUN.UNICODE.INDIA-0.GLGR \
		UNICODE-FONTSPECIFIC.GLGR

HI_CONV_TABLES = \
		U32_TO_CS_0 \
		U32_TO_CS_1 \
		U32_TO_CS_14 \
		U32_TO_CS_15 \
		U32_TO_CS_16 \
		U32_TO_CS_2

XUCTBLGEN = $(SRC)/src/cmd/xuctblgen/xuctblgen

FILES = $(HI_CONV_TABLES)

TARGETDIR = $(FILEROOT)/usr/openwin/lib/locale/$(LOCALE)/conv_tables

TARGETDIRS = $(TARGETDIR)

SUBDIRS = 

all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber

all: $(TARGETDIRS) $(HI_CONV_TABLES) $(SUBDIRS)

include $(SRC)/src/Makefile.targ

install: $(FILES.obj) $(SUBDIRS)

$(HI_CONV_TABLES): $(XUCTBLGEN) ../XLC_LOCALE $(COMMON_CONV_TABLES:%=../../common/conv_tables/%.txt)
	XLOCALEDIR=$(SRC)/src/openwin/lib/locale $(XUCTBLGEN) -locale $(LOCALE) -no_cs_to_wc_tables

$(XUCTBLGEN): 
	cd $(SRC)/src/cmd/xuctblgen/;make all

clean: objclean
	$(RM) -r $(TARGETDIR)

FRC:

