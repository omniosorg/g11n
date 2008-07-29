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

LOCALES_UTF8 = hi_IN.UTF-8 ja_JP.UTF-8 zh.UTF-8 ko.UTF-8 ko.UTF-8@dict th_TH.UTF-8 zh_HK.UTF-8 zh_TW.UTF-8 ta_IN.UTF-8 te_IN.UTF-8 en_US.UTF-8 zh.UTF-8@pinyin zh.UTF-8@radical zh.UTF-8@stroke zh_HK.UTF-8@pinyin zh_HK.UTF-8@stroke zh_HK.UTF-8@radical zh_HK.UTF-8@zhuyin zh_TW.UTF-8@pinyin zh_TW.UTF-8@stroke zh_TW.UTF-8@radical zh_TW.UTF-8@zhuyin

LOCALES_CLDR = en_IE.UTF-8 en_GB.UTF-8 de_LU.UTF-8 de_AT.UTF-8 fr_LU.UTF-8 nl_BE.UTF-8 nl_NL.UTF-8 cs_CZ.UTF-8 sl_SI.UTF-8 \
sk_SK.UTF-8  hr_HR.UTF-8 hu_HU.UTF-8 pt_PT.UTF-8 el_GR.UTF-8 el_CY.UTF-8 ca_ES.UTF-8 en_MT.UTF-8 mt_MT.UTF-8 et_EE.UTF-8 lt_LT.UTF-8 \
lv_LV.UTF-8 nn_NO.UTF-8 nb_NO.UTF-8 da_DK.UTF-8 fr_CA.UTF-8 en_CA.UTF-8 es_MX.UTF-8 en_AU.UTF-8 en_NZ.UTF-8 es_CR.UTF-8 es_GT.UTF-8 \
es_NI.UTF-8 es_PA.UTF-8 es_SV.UTF-8 de_CH.UTF-8 fr_CH.UTF-8 bg_BG.UTF-8 mk_MK.UTF-8 sh_BA.UTF-8 ro_RO.UTF-8 sq_AL.UTF-8 sr_CS.UTF-8 \
is_IS.UTF-8 es_AR.UTF-8 es_BO.UTF-8 es_CL.UTF-8 es_CO.UTF-8 es_EC.UTF-8 es_PE.UTF-8 es_PY.UTF-8 es_UY.UTF-8 es_VE.UTF-8 \
cs_CZ.UTF-8@euro fr_FR.UTF-8  ar_EG.UTF-8 ar_SA.UTF-8 de_DE.UTF-8 es_ES.UTF-8 fi_FI.UTF-8 fr_BE.UTF-8 he_IL.UTF-8 it_IT.UTF-8  \
pl_PL.UTF-8 pt_BR.UTF-8 ru_RU.UTF-8 sv_SE.UTF-8 tr_TR.UTF-8 uk_UA.UTF-8  ms_MY.UTF-8 id_ID.UTF-8  vi_VN.UTF-8

LOCALES_ISO = en_AU.ISO8859-1 en_CA.ISO8859-1 en_GB.ISO8859-1 en_GB.ISO8859-15 en_IE.ISO8859-1 en_IE.ISO8859-15 en_NZ.ISO8859-1 \
en_US.ISO8859-1 en_US.ISO8859-15 es_AR.ISO8859-1 es_BO.ISO8859-1 es_CL.ISO8859-1 es_CO.ISO8859-1 es_CR.ISO8859-1 es_EC.ISO8859-1 \
es_GT.ISO8859-1 es_MX.ISO8859-1 es_NI.ISO8859-1 es_PA.ISO8859-1 es_PE.ISO8859-1 es_PY.ISO8859-1 es_SV.ISO8859-1 es_UY.ISO8859-1 \
es_VE.ISO8859-1 fr_BE.ISO8859-1 fr_BE.ISO8859-15 fr_CA.ISO8859-1 fr_FR.ISO8859-1 fr_FR.ISO8859-15 nl_BE.ISO8859-1 nl_BE.ISO8859-15 \
nl_NL.ISO8859-1 nl_NL.ISO8859-15 pt_BR.ISO8859-1 \
ar bg_BG.ISO8859-5 ca_ES.ISO8859-1  ca_ES.ISO8859-15 cs_CZ.ISO8859-2 da_DK.ISO8859-1 da_DK.ISO8859-15 de_AT.ISO8859-1 \
de_AT.ISO8859-15 de_CH.ISO8859-1 de_DE.ISO8859-1 de_DE.ISO8859-15 el_GR.ISO8859-7 es_ES.ISO8859-1 es_ES.ISO8859-15 et_EE.ISO8859-15 \
fi_FI.ISO8859-1 fi_FI.ISO8859-15 fr_CH.ISO8859-1 he \
hr_HR.ISO8859-2 hu_HU.ISO8859-2 is_IS.ISO8859-1 it_IT.ISO8859-1 it_IT.ISO8859-15 lt_LT.ISO8859-13 lv_LV.ISO8859-13 mk_MK.ISO8859-5 \
 no_NO.ISO8859-1@bokmal no_NO.ISO8859-1@nynorsk pl_PL.ISO8859-2 \
 pt_PT.ISO8859-1 pt_PT.ISO8859-15 ro_RO.ISO8859-2 ru_RU.ISO8859-5 sh_BA.ISO8859-2@bosnia sk_SK.ISO8859-2 sl_SI.ISO8859-2 \
sq_AL.ISO8859-2 sr_YU.ISO8859-5 sv_SE.ISO8859-1 sv_SE.ISO8859-15 tr_TR.ISO8859-9


LOCALES_OTHER = ja ja_JP.PCK ja_JP.eucJP ko ko@dict ru_RU.ANSI1251 ru_RU.KOI8-R th_TH zh zh_CN.GB18030 zh.GBK zh_HK.BIG5HK zh_TW zh_TW.BIG5 zh_CN.GB18030@pinyin zh_CN.GB18030@radical zh_CN.GB18030@stroke zh.GBK@pinyin zh.GBK@radical zh.GBK@stroke zh_HK.BIG5HK@pinyin zh_HK.BIG5HK@radical zh_HK.BIG5HK@stroke zh_HK.BIG5HK@zhuyin zh@pinyin zh@radical zh@stroke zh_TW.BIG5@pinyin zh_TW.BIG5@radical zh_TW.BIG5@stroke zh_TW.BIG5@zhuyin zh_TW@pinyin zh_TW@radical zh_TW@stroke zh_TW@zhuyin

METHOD_VERSION = 3

CFLAGS = -Xa -K PIC -G -xO3
CFLAGS_sparcv9 = $(CFLAGS) -xarch=v9 -dalign

LOCALE = $(LOCALES_OTHER) $(LOCALES_UTF8) $(LOCALES_ISO)

LOCALE.so = $(LOCALE:%=%.so.$(METHOD_VERSION))

LOCALE.cldr = $(LOCALES_CLDR:%=%_c.so.$(METHOD_VERSION))

LOCALE.localedef = $(LOCALE.so%.so$(METHOD_VERSION).=%.localedef) 

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
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap $< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap ../$< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) $(MACH64) ""

# need to compile the CLDR locales separately as they use a different charmap to the other UTF-8s.
# some hacky code here to create a $locale.UTF-8_c.so.* file to satisy dependency checking.
%.UTF-8_c.so.$(METHOD_VERSION): %.UTF-8.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/utf-8.charmap $< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) "" ""
	$(RM) $@
	$(LN) -s $(@:%.UTF-8_c.so.$(METHOD_VERSION)=%.UTF-8.so.$(METHOD_VERSION)) $@
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/utf-8.charmap ../$< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) $(MACH64) ""

#
# rules for non-UTF-8 locales
#

%.ISO8859-1.so.$(METHOD_VERSION): %.ISO8859-1.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-1.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-1.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

%.ISO8859-2.so.$(METHOD_VERSION): %.ISO8859-2.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-2.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-2.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

%.ISO8859-5.so.$(METHOD_VERSION): %.ISO8859-5.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-5.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-5.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

# ISO8859-6 locale
ar.so.$(METHOD_VERSION): ar.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-6.charmap ar.localedef.build $(SRC)/src/locale_src/base/include/single_byte.x ar "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-6.charmap ../ar.localedef.build $(SRC)/src/locale_src/base/include/single_byte.x ar $(MACH64) ""

%.ISO8859-7.so.$(METHOD_VERSION): %.ISO8859-7.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-7.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-7.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

# ISO8859-8 locale
he.so.$(METHOD_VERSION): he.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-8.charmap he.localedef.build $(SRC)/src/locale_src/base/include/single_byte.x he "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-8.charmap ../he.localedef.build $(SRC)/src/locale_src/base/include/single_byte.x he $(MACH64) ""

%.ISO8859-9.so.$(METHOD_VERSION): %.ISO8859-9.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-9.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-9.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

%.ISO8859-13.so.$(METHOD_VERSION): %.ISO8859-13.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-13.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-13.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

%.ISO8859-15.so.$(METHOD_VERSION): %.ISO8859-15.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-15.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-15.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

%.KOI8-R.so.$(METHOD_VERSION): %.KOI8-R.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/koi8-r.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/koi8-r.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

%.ANSI1251.so.$(METHOD_VERSION): %.ANSI1251.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/ansi-1251.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/ansi-1251.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

#
# japanese, asian locales
#

ja.so.$(METHOD_VERSION): ja.localedef
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/eucJP.charmap ja.localedef $(SRC)/src/locale_src/base/include/ja.x ja "" "-R/usr/lib/locale/ja_JP.eucJP"
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/eucJP.charmap ../ja.localedef $(SRC)/src/locale_src/base/include/ja.x ja $(MACH64) "-R/usr/lib/locale/ja_JP.eucJP/$(MACH64)"

%.PCK.so.$(METHOD_VERSION): %.PCK.localedef
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/PCK.charmap $< $(SRC)/src/locale_src/base/include/PCK.x $(<:%.localedef=%) "" "-R/usr/lib/locale/ja_JP.PCK"
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/PCK.charmap ../$< $(SRC)/src/locale_src/base/include/PCK.x $(<:%.localedef=%) $(MACH64) "-R/usr/lib/locale/ja_JP.PCK/$(MACH64)"

%.eucJP.so.$(METHOD_VERSION): %.eucJP.localedef
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/eucJP.charmap $< $(SRC)/src/locale_src/base/include/eucJP.x $(<:%.localedef=%) "" "-R/usr/lib/locale/ja_JP.eucJP"
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/eucJP.charmap ../$< $(SRC)/src/locale_src/base/include/eucJP.x $(<:%.localedef=%) $(MACH64) "-R/usr/lib/locale/ja_JP.eucJP/$(MACH64)"

%.UTF-8@pinyin.so.$(METHOD_VERSION): %.UTF-8@pinyin.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap $< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap ../$< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) $(MACH64) ""

%.UTF-8@radical.so.$(METHOD_VERSION): %.UTF-8@radical.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap $< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap ../$< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) $(MACH64) ""

%.UTF-8@stroke.so.$(METHOD_VERSION): %.UTF-8@stroke.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap $< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap ../$< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) $(MACH64) ""

%.UTF-8@zhuyin.so.$(METHOD_VERSION): %.UTF-8@zhuyin.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap $< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap ../$< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) $(MACH64) ""

zh_CN.GB18030%.so.$(METHOD_VERSION): zh_CN.GB18030%.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/GB18030.charmap $< $(SRC)/src/locale_src/base/include/GB18030.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/GB18030.charmap ../$< $(SRC)/src/locale_src/base/include/GB18030.x $(<:%.localedef.build=%) $(MACH64) ""

zh.GBK%.so.$(METHOD_VERSION): zh.GBK%.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/GBK.charmap $< $(SRC)/src/locale_src/base/include/GBK.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/GBK.charmap ../$< $(SRC)/src/locale_src/base/include/GBK.x $(<:%.localedef.build=%) $(MACH64) ""

zh_HK.BIG5HK%.so.$(METHOD_VERSION): zh_HK.BIG5HK%.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/BIG5HK.charmap $< $(SRC)/src/locale_src/base/include/BIG5HK.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/BIG5HK.charmap ../$< $(SRC)/src/locale_src/base/include/BIG5HK.x $(<:%.localedef.build=%) $(MACH64) ""

zh_TW.BIG5%.so.$(METHOD_VERSION): zh_TW.BIG5%.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/BIG5.charmap $< $(SRC)/src/locale_src/base/include/BIG5.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/BIG5.charmap ../$< $(SRC)/src/locale_src/base/include/BIG5.x $(<:%.localedef.build=%) $(MACH64) ""

zh_TW%.so.$(METHOD_VERSION): zh_TW%.localedef
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/zh_TW.charmap $< $(SRC)/src/locale_src/base/include/zh_TW.EUC.x $(<:%.localedef=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/zh_TW.charmap ../$< $(SRC)/src/locale_src/base/include/zh_TW.EUC.x $(<:%.localedef=%) $(MACH64) ""

zh%.so.$(METHOD_VERSION): zh%.localedef
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/zh.charmap $< $(SRC)/src/locale_src/base/include/zh.EUC.x $(<:%.localedef=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/zh.charmap ../$< $(SRC)/src/locale_src/base/include/zh.EUC.x $(<:%.localedef=%) $(MACH64) ""

ko.so.$(METHOD_VERSION): ko.localedef
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/EUC.charmap ko.localedef $(SRC)/src/locale_src/base/include/ko.EUC.x ko "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/EUC.charmap ../ko.localedef $(SRC)/src/locale_src/base/include/ko.EUC.x ko $(MACH64) ""

th_TH.so.$(METHOD_VERSION): th_TH.localedef
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/TIS620.charmap th_TH.localedef $(SRC)/src/locale_src/base/include/ISO8859.x th_TH "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/TIS620.charmap ../th_TH.localedef $(SRC)/src/locale_src/base/include/ISO8859.x th_TH $(MACH64) ""

#
# special rules for variant locales (locale@variant.codeset)
#
%.UTF-8@euro_c.so.$(METHOD_VERSION): %.UTF-8@euro.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/utf-8.charmap $< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) "" ""
	$(RM) $@
	$(CP)  $(@:%.UTF-8@euro_c.so.$(METHOD_VERSION)=%.UTF-8.so.$(METHOD_VERSION)) $@ 
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/utf-8.charmap ../$< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) $(MACH64) ""
	$(CP)  $(MACH64)/$(@:%.UTF-8@euro_c.so.$(METHOD_VERSION)=%.UTF-8.so.$(METHOD_VERSION)) $(MACH64)/$@ 

%.UTF-8@dict.so.$(METHOD_VERSION): %.UTF-8@dict.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap $< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/UTF-8.charmap ../$< $(SRC)/src/locale_src/base/include/UTF-8.x $(<:%.localedef.build=%) $(MACH64) ""

%.ISO8859-1@bokmal.so.$(METHOD_VERSION): %.ISO8859-1@bokmal.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-1.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-1.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

%.ISO8859-1@nynorsk.so.$(METHOD_VERSION): %.ISO8859-1@nynorsk.localedef.build
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-1.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-1.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

%.ISO8859-2@bosnia.so.$(METHOD_VERSION): %.ISO8859-2@bosnia.localedef.build	
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-2.charmap $< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/iso-8859-2.charmap ../$< $(SRC)/src/locale_src/base/include/ISO8859.x $(<:%.localedef.build=%) $(MACH64) ""

# this won't build if I have the name as the proper locale name: %.@dict as it clashes with ko_KR.UTF-8@dict
%dict.so.$(METHOD_VERSION): %dict.localedef
	$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/EUC.charmap  $<  $(SRC)/src/locale_src/base/include/ko.EUC.x $(<:%.localedef=%) "" ""
	cd $(MACH64);$(SRC)/tools/Run.sh $(SRC)/src/locale_src/charmap/EUC.charmap ../$< $(SRC)/src/locale_src/base/include/ko.EUC.x $(<:%.localedef=%) $(MACH64) ""

$(MACH64):
	$(INSDIR) $(MACH64)

install: all install_src
	for i in $(LOCALE) $(LOCALES_CLDR); do \
		$(INSDIR) $(FILEROOT)/usr/lib/locale/$$i/$(MACH64) ; \
		$(INS) $(FILEROOT)/usr/lib/locale/$$i $$i.so.$(METHOD_VERSION) ; \
		$(INS) $(FILEROOT)/usr/lib/locale/$$i/$(MACH64) $(MACH64)/$$i.so.$(METHOD_VERSION); \
	done
	$(INSDIR) $(FILEROOT)/usr/lib/localedef/src/en_US.UTF-8/
	$(CP) en_US.UTF-8.localedef.build localedef.src
	compress -f localedef.src
	$(INS) $(FILEROOT)/usr/lib/localedef/src/en_US.UTF-8/ localedef.src.Z

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


