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

LOCALES_UTF8 = ja_JP.UTF-8 zh.UTF-8 ko.UTF-8 ko.UTF-8@dict th_TH.UTF-8 zh_HK.UTF-8 zh_TW.UTF-8 zh.UTF-8@pinyin zh.UTF-8@radical zh.UTF-8@stroke zh_HK.UTF-8@pinyin zh_HK.UTF-8@stroke zh_HK.UTF-8@radical zh_HK.UTF-8@zhuyin zh_TW.UTF-8@pinyin zh_TW.UTF-8@stroke zh_TW.UTF-8@radical zh_TW.UTF-8@zhuyin

# not migrated yet because different reasons
LOCALES_ISO = no_NO.ISO8859-1@bokmal no_NO.ISO8859-1@nynorsk sh_BA.ISO8859-2@bosnia 

LOCALES_OTHER = ja ja_JP.PCK ja_JP.eucJP ko ko@dict th_TH.TIS620 zh zh_CN.GB18030 zh.GBK \
		zh_HK.BIG5HK zh_TW zh_TW.BIG5 zh_CN.GB18030@pinyin zh_CN.GB18030@radical \
		zh_CN.GB18030@stroke zh.GBK@pinyin zh.GBK@radical zh.GBK@stroke \
		zh_HK.BIG5HK@pinyin zh_HK.BIG5HK@radical zh_HK.BIG5HK@stroke zh_HK.BIG5HK@zhuyin \
		zh@pinyin zh@radical zh@stroke zh_TW.BIG5@pinyin zh_TW.BIG5@radical \
		zh_TW.BIG5@stroke zh_TW.BIG5@zhuyin zh_TW@pinyin zh_TW@radical zh_TW@stroke \
		zh_TW@zhuyin

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


LOCALE = $(LOCALES_OTHER) $(LOCALES_UTF8) $(LOCALES_ISO)

LOCALE.so = $(LOCALE:%=%.so.$(METHOD_VERSION))

LOCALE.cldr = $(LOCALES_CLDR:%=%_c.so.$(METHOD_VERSION))

LOCALE_src = $(LOCALE:%=%.src.bz2) $(LOCALES_CLDR:%=%.src.bz2)

all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean

.KEEP_STATE:

all: $(MACH64) $(LOCALE.so) $(LOCALE.cldr) $(LOCALE_src) #$(LOCALE_ISO.cldr) 

%.localedef.build: %.localedef
	$(SRC)/tools/generatelocaledef -d include/ -i $< -o $@

$(LOCALE): $(LOCALE:%=%.so.$(METHOD_VERSION))

# 
# there is probably a quicker/cleaner way to do this, but for the moment this works. Each codeset needs to have a separate entry here.
# 

%.UTF-8.so.$(METHOD_VERSION): %.UTF-8.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i ../$< $(<:%.localedef.build=%)

# need to compile the CLDR locales separately as they use a different charmap to the other UTF-8s.
# some hacky code here to create a $locale.UTF-8_c.so.* file to satisy dependency checking.
%.UTF-8_c.so.$(METHOD_VERSION): %.UTF-8.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/utf-8.charmap -i $< $(<:%.localedef.build=%)
	$(RM) $@
	$(LN) -s $(@:%.UTF-8_c.so.$(METHOD_VERSION)=%.UTF-8.so.$(METHOD_VERSION)) $@
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/utf-8.charmap -i ../$< $(<:%.localedef.build=%)

#
# rules for non-UTF-8 locales
#

%.ISO8859-1.so.$(METHOD_VERSION): %.ISO8859-1.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-1.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-1.charmap -i ../$< $(<:%.localedef.build=%)

%.ISO8859-2.so.$(METHOD_VERSION): %.ISO8859-2.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-2.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-2.charmap -i ../$< $(<:%.localedef.build=%)

%.ISO8859-5.so.$(METHOD_VERSION): %.ISO8859-5.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-5.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-5.charmap -i ../$< $(<:%.localedef.build=%)

# ISO8859-6 locale
ar.so.$(METHOD_VERSION): ar.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/single_byte.x -f $(_CHARMAPS)/iso-8859-6.charmap -i ar.localedef.build ar
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/single_byte.x -f $(_CHARMAPS)/iso-8859-6.charmap -i ../ar.localedef.build ar

%.ISO8859-7.so.$(METHOD_VERSION): %.ISO8859-7.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-7.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-7.charmap -i ../$< $(<:%.localedef.build=%)

# ISO8859-8 locale
he.so.$(METHOD_VERSION): he.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/single_byte.x -f $(_CHARMAPS)/iso-8859-8.charmap -i he.localedef.build he
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/single_byte.x -f $(_CHARMAPS)/iso-8859-8.charmap -i ../he.localedef.build he

%.ISO8859-9.so.$(METHOD_VERSION): %.ISO8859-9.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-9.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-9.charmap -i ../$< $(<:%.localedef.build=%)

%.ISO8859-13.so.$(METHOD_VERSION): %.ISO8859-13.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-13.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-13.charmap -i ../$< $(<:%.localedef.build=%)

%.ISO8859-15.so.$(METHOD_VERSION): %.ISO8859-15.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-15.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-15.charmap -i ../$< $(<:%.localedef.build=%)

%.KOI8-R.so.$(METHOD_VERSION): %.KOI8-R.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/koi8-r.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/koi8-r.charmap -i ../$< $(<:%.localedef.build=%)

%.ANSI1251.so.$(METHOD_VERSION): %.ANSI1251.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/ansi-1251.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/ansi-1251.charmap -i ../$< $(<:%.localedef.build=%)

#
# japanese, asian locales
#


ja.so.$(METHOD_VERSION): ja.localedef
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "-R/usr/lib/locale/ja_JP.eucJP" -x $(_METHODSRC)/ja.x -f $(_CHARMAPS)/eucJP.charmap -i ja.localedef ja
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "-R/usr/lib/locale/ja_JP.eucJP/$(MACH64)" -x $(_METHODSRC)/ja.x -f $(_CHARMAPS)/eucJP.charmap -i ../ja.localedef ja

%.PCK.so.$(METHOD_VERSION): %.PCK.localedef
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "-R/usr/lib/locale/ja_JP.PCK" -x $(_METHODSRC)/PCK.x -f $(_CHARMAPS)/PCK.charmap -i $< $(<:%.localedef=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "-R/usr/lib/locale/ja_JP.PCK/$(MACH64)" -x $(_METHODSRC)/PCK.x -f $(_CHARMAPS)/PCK.charmap -i ../$< $(<:%.localedef=%)

%.eucJP.so.$(METHOD_VERSION): %.eucJP.localedef
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "-R/usr/lib/locale/ja_JP.eucJP" -x $(_METHODSRC)/eucJP.x -f $(_CHARMAPS)/eucJP.charmap -i $< $(<:%.localedef=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "-R/usr/lib/locale/ja_JP.eucJP/$(MACH64)" -x $(_METHODSRC)/eucJP.x -f $(_CHARMAPS)/eucJP.charmap -i ../$< $(<:%.localedef=%)

%.UTF-8@pinyin.so.$(METHOD_VERSION): %.UTF-8@pinyin.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i ../$< $(<:%.localedef.build=%)

%.UTF-8@radical.so.$(METHOD_VERSION): %.UTF-8@radical.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i ../$< $(<:%.localedef.build=%)

%.UTF-8@stroke.so.$(METHOD_VERSION): %.UTF-8@stroke.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i ../$< $(<:%.localedef.build=%)

%.UTF-8@zhuyin.so.$(METHOD_VERSION): %.UTF-8@zhuyin.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i ../$< $(<:%.localedef.build=%)

zh_CN.GB18030%.so.$(METHOD_VERSION): zh_CN.GB18030%.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/GB18030.x -f $(_CHARMAPS)/GB18030.charmap -i $< $(<:%.localedef.build=%);
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/GB18030.x -f $(_CHARMAPS)/GB18030.charmap -i ../$< $(<:%.localedef.build=%);

zh.GBK%.so.$(METHOD_VERSION): zh.GBK%.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/GBK.x -f $(_CHARMAPS)/GBK.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/GBK.x -f $(_CHARMAPS)/GBK.charmap -i ../$< $(<:%.localedef.build=%)

zh_HK.BIG5HK%.so.$(METHOD_VERSION): zh_HK.BIG5HK%.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/BIG5HK.x -f $(_CHARMAPS)/BIG5HK.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/BIG5HK.x -f $(_CHARMAPS)/BIG5HK.charmap -i ../$< $(<:%.localedef.build=%)

zh_TW.BIG5%.so.$(METHOD_VERSION): zh_TW.BIG5%.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/BIG5.x -f $(_CHARMAPS)/BIG5.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/BIG5.x -f $(_CHARMAPS)/BIG5.charmap -i ../$< $(<:%.localedef.build=%)

zh_TW%.so.$(METHOD_VERSION): zh_TW%.localedef
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/zh_TW.EUC.x -f $(_CHARMAPS)/zh_TW.charmap -i $< $(<:%.localedef=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/zh_TW.EUC.x -f $(_CHARMAPS)/zh_TW.charmap -i ../$< $(<:%.localedef=%)

zh%.so.$(METHOD_VERSION): zh%.localedef
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/zh.EUC.x -f $(_CHARMAPS)/zh.charmap -i $< $(<:%.localedef=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/zh.EUC.x -f $(_CHARMAPS)/zh.charmap -i ../$< $(<:%.localedef=%)

ko.so.$(METHOD_VERSION): ko.localedef
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ko.EUC.x -f $(_CHARMAPS)/EUC.charmap -i ko.localedef ko
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ko.EUC.x -f $(_CHARMAPS)/EUC.charmap -i ../ko.localedef ko

th_TH.TIS620.so.$(METHOD_VERSION): th_TH.TIS620.localedef
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/TIS620.charmap -i th_TH.TIS620.localedef th_TH.TIS620
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/TIS620.charmap -i ../th_TH.TIS620.localedef th_TH.TIS620

#
# special rules for variant locales (locale@variant.codeset)
#
%.UTF-8@euro_c.so.$(METHOD_VERSION): %.UTF-8@euro.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/utf-8.charmap -i $< $(<:%.localedef.build=%)
	$(RM) $@
	$(CP)  $(@:%.UTF-8@euro_c.so.$(METHOD_VERSION)=%.UTF-8.so.$(METHOD_VERSION)) $@ 
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/utf-8.charmap -i ../$< $(<:%.localedef.build=%)
	$(CP)  $(MACH64)/$(@:%.UTF-8@euro_c.so.$(METHOD_VERSION)=%.UTF-8.so.$(METHOD_VERSION)) $(MACH64)/$@ 

%.UTF-8@dict.so.$(METHOD_VERSION): %.UTF-8@dict.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)" -x $(_METHODSRC)/UTF-8.x -f $(_CHARMAPS)/UTF-8.charmap -i ../$< $(<:%.localedef.build=%)

%.ISO8859-1@bokmal.so.$(METHOD_VERSION): %.ISO8859-1@bokmal.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-1.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-1.charmap -i ../$< $(<:%.localedef.build=%)

%.ISO8859-1@nynorsk.so.$(METHOD_VERSION): %.ISO8859-1@nynorsk.localedef.build
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-1.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-1.charmap -i ../$< $(<:%.localedef.build=%)

%.ISO8859-2@bosnia.so.$(METHOD_VERSION): %.ISO8859-2@bosnia.localedef.build	
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-2.charmap -i $< $(<:%.localedef.build=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ISO8859.x -f $(_CHARMAPS)/iso-8859-2.charmap -i ../$< $(<:%.localedef.build=%)

# this won't build if I have the name as the proper locale name: %.@dict as it clashes with ko_KR.UTF-8@dict
%dict.so.$(METHOD_VERSION): %dict.localedef
	$(LOCALEDEF) $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "" -x $(_METHODSRC)/ko.EUC.x -f $(_CHARMAPS)/EUC.charmap -i $< $(<:%.localedef=%)
	cd $(MACH64); $(LOCALEDEF) $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "" -x $(_METHODSRC)/ko.EUC.x -f $(_CHARMAPS)/EUC.charmap -i ../$< $(<:%.localedef=%)

$(MACH64):
	$(INSDIR) $(MACH64)

install: all install_src
	for i in $(LOCALE) $(LOCALES_CLDR); do \
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
	for i in $(LOCALE) $(LOCALES_CLDR); do \
		$(RM) $(FILEROOT)/usr/lib/locale/$$i/$$i.so.$(METHOD_VERSION) ; \
		$(RM) $(FILEROOT)/usr/lib/locale/$$i/$(MACH64)/$$i.so.$(METHOD_VERSION); \
	done
	for i in $(LOCALE_src); do \
		$(RM) $(FILEROOT)/usr/lib/locale/src/locales/$$i; \
	done

FRC:


