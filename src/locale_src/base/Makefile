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
# Copyright (c) 2009, 2010, Oracle and/or its affiliates. All rights reserved.
#

include $(SRC)/Makefile.master

LOCALES_UTF8 =	ja_JP.UTF-8 \
		ko_KR.UTF-8 \
		ko_KR.UTF-8@dict \
		th_TH.UTF-8 \
		zh_CN.UTF-8 \
		zh_CN.UTF-8@pinyin \
		zh_CN.UTF-8@radical \
		zh_CN.UTF-8@stroke \
		zh_HK.UTF-8 \
		zh_HK.UTF-8@pinyin \
		zh_HK.UTF-8@radical \
		zh_HK.UTF-8@stroke \
		zh_HK.UTF-8@zhuyin \
		zh_TW.UTF-8 \
		zh_TW.UTF-8@pinyin \
		zh_TW.UTF-8@radical \
		zh_TW.UTF-8@stroke \
		zh_TW.UTF-8@zhuyin

LOCALES_OTHER =	ja_JP.PCK \
		ja_JP.eucJP \
		ko_KR.EUC \
		ko_KR.EUC@dict \
		th_TH.TIS620 \
		zh_CN.EUC \
		zh_CN.EUC@pinyin \
		zh_CN.EUC@radical \
		zh_CN.EUC@stroke \
		zh_CN.GB18030 \
		zh_CN.GB18030@pinyin \
		zh_CN.GB18030@radical \
		zh_CN.GB18030@stroke \
		zh_CN.GBK \
		zh_CN.GBK@pinyin \
		zh_CN.GBK@radical \
		zh_CN.GBK@stroke \
		zh_HK.BIG5HK \
		zh_HK.BIG5HK@pinyin \
		zh_HK.BIG5HK@radical \
		zh_HK.BIG5HK@stroke \
		zh_HK.BIG5HK@zhuyin \
		zh_TW.BIG5 \
		zh_TW.BIG5@pinyin \
		zh_TW.BIG5@radical \
		zh_TW.BIG5@stroke \
		zh_TW.BIG5@zhuyin \
		zh_TW.EUC \
		zh_TW.EUC@pinyin \
		zh_TW.EUC@radical \
		zh_TW.EUC@stroke \
		zh_TW.EUC@zhuyin

COMMON_CFLAGS	= $(CCVERBOSE) -K pic -D PIC -G -I. -z defs -z text -z ignore -D_REENTRANT

# escape ',' for localedef
sparc_CCUNBOUND		= -Wd\,-xsafe=unboundsym
sparc_SPACEFLAG		= -xspace -W0\,-Lt -W2\,-Rcond_elim
sparcv9_SPACEFLAG	= -xspace -W0\,-Lt -W2\,-Rcond_elim
CCREGSYM		= -Wc\,-Qiselect-regsym=0

CERRWARN	=
CFLAGS          += $(COMMON_CFLAGS) $(XREGSFLAG)
CFLAGS64        += $(COMMON_CFLAGS) $(XREGSFLAG64)

LDF_OPT		= 
LDF_OPT64	= -m lp64
LOCALEDEF	= $(SRC)/tools/localedef.sh

LOPT_ORIGIN	= "-R\\\$$ORIGIN"

_CHARMAPS	= $(SRC)/src/locale_src/charmap
_METHODSRC	= $(SRC)/src/locale_src/base/include


LOCALE = $(LOCALES_OTHER) $(LOCALES_UTF8)

LOCALE.so = $(LOCALE:%=%.so.$(METHOD_VERSION))

LOCALE_src = $(LOCALE:%=%.src.bz2)

all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean

.KEEP_STATE:

#
# create symlinks for all methods_* libraries
#
METHODS_LOCALES = \
	ja_JP.PCK \
	ja_JP.eucJP \
	ko_KR.EUC \
	zh_CN.EUC \
	zh_CN.GB18030 \
	zh_CN.GBK \
	zh_HK.BIG5HK \
	zh_TW.BIG5 \
	zh_TW.EUC \
	zh_TW.EUC

METHODS_UNICODE_SO = methods_unicode.so.$(METHOD_VERSION)

METHODS_SOS = \
	$(METHODS_LOCALES:%=methods_%.so.$(METHOD_VERSION)) \
	$(METHODS_LOCALES:%=$(MACH64)/methods_%.so.$(METHOD_VERSION)) \
	$(METHODS_UNICODE_SO) \
	$(MACH64)/$(METHODS_UNICODE_SO)

$(METHODS_UNICODE_SO):
	if [ -f $(FILEROOT)/usr/lib/locale/common/$@ ]; then \
		$(LN) -s $(FILEROOT)/usr/lib/locale/common/$@ ; \
	elif [ -f /usr/lib/locale/common/$@ ]; then\
		$(LN) -s /usr/lib/locale/common/$@ ; \
	else \
		echo "Failed to create symlink to $@." ; \
		exit 1 ; \
	fi

$(MACH64)/$(METHODS_UNICODE_SO): $(MACH64)
	if [ -f $(FILEROOT)/usr/lib/locale/common/$@ ]; then \
		$(LN) -s $(FILEROOT)/usr/lib/locale/common/$@ $(MACH64)/; \
	elif [ -f /usr/lib/locale/common/$@ ]; then \
		$(LN) -s /usr/lib/locale/common/$@ $(MACH64)/ ; \
	else \
		echo "Failed to create symlink to $@." ; \
		exit 1 ; \
	fi

methods_%.so.$(METHOD_VERSION):
	LOC=$(@:methods_%.so.$(METHOD_VERSION)=%); \
	if [ -f $(FILEROOT)/usr/lib/locale/$$LOC/$@ ]; then \
		$(LN) -s $(FILEROOT)/usr/lib/locale/$$LOC/$@ ; \
	elif [ -f /usr/lib/locale/$$LOC/$@ ]; then \
		$(LN) -s /usr/lib/locale/$$LOC/$@ ; \
	else \
		echo "Failed to create symlink to $@." ; \
		exit 1 ; \
	fi 

$(MACH64)/methods_%.so.$(METHOD_VERSION): $(MACH64)
	LOC=$(@:$(MACH64)/methods_%.so.$(METHOD_VERSION)=%); \
	if [ -f $(FILEROOT)/usr/lib/locale/$$LOC/$@ ]; then \
		$(LN) -s $(FILEROOT)/usr/lib/locale/$$LOC/$@ $(MACH64)/; \
	elif [ -f /usr/lib/locale/$$LOC/$@ ]; then \
		$(LN) -s /usr/lib/locale/$$LOC/$@ $(MACH64)/; \
	else \
		echo "Failed to create symlink to $@." ; \
		exit 1 ; \
	fi


all: $(METHODS_SOS) $(MACH64) $(LOCALE.so) $(LOCALE_src)

%.localedef.build: %.localedef
	$(SRC)/tools/generatelocaledef -d include/ -i $< -o $@

#
# split locale parts first: Lang,Country,Encoding,Variant
# - UTF-8 locales:	.x and .charmap from Encoding
# - non-UTF-8 locales:	.x and .charmap based on Lang,Country,Encoding
#
%.so.$(METHOD_VERSION): $(METHODS_SOS) %.localedef.build
	LOC=$(<:%.localedef.build=%) ; \
	LCE=`echo $$LOC | cut -d@ -f1` ; \
	ENC=`echo $$LCE | cut -d. -f2` ; \
	if [ "$$ENC" = "UTF-8" ]; then ID=$$ENC; else ID=$$LCE; fi ; \
	$(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/$$ID.x -f $(_CHARMAPS)/$$ID.charmap -i $< $$LOC; \
	$(MV) $@ $(MACH64)/ ; \
	$(LOCALEDEF) $(LDF_OPT)   -c -v -W cc,"$(CFLAGS)"   -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/$$ID.x -f $(_CHARMAPS)/$$ID.charmap -i $< $$LOC

$(MACH64):
	$(INSDIR) $(MACH64)

install: all install_src
	for i in $(LOCALE); do \
		$(INSDIR) $(FILEROOT)/usr/lib/locale/$$i/$(MACH64) ; \
		$(INS) $(FILEROOT)/usr/lib/locale/$$i $$i.so.$(METHOD_VERSION) ; \
		$(INS) $(FILEROOT)/usr/lib/locale/$$i/$(MACH64) $(MACH64)/$$i.so.$(METHOD_VERSION); \
	done

install_src: $(LOCALE_src)
	$(INSDIR) $(FILEROOT)/usr/lib/localedef/src/locales
	for i in $(LOCALE_src); do \
		$(INS) $(FILEROOT)/usr/lib/localedef/src/locales $$i; \
	done

%.src.bz2: %.localedef
	$(BZIP2) -c $< > $@

clean clobber: 
	$(RM) *.c *.o *.so.* *.localedef.build *.Z 
	$(RM) -r `ls | sed -e'/localedef/d' -e'/include/d' -e'/Makefile/d'`
	for i in $(LOCALE); do \
		$(RM) $(FILEROOT)/usr/lib/locale/$$i/$$i.so.$(METHOD_VERSION) ; \
		$(RM) $(FILEROOT)/usr/lib/locale/$$i/$(MACH64)/$$i.so.$(METHOD_VERSION); \
	done
	for i in $(LOCALE_src); do \
		$(RM) $(FILEROOT)/usr/lib/locale/src/locales/$$i; \
	done

FRC:

