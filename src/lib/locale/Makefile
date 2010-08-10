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

LOCALES = \
	af_ZA.UTF-8 \
	ar_EG.ISO8859-6 \
	ar_EG.UTF-8 \
	ar_SA.UTF-8 \
	as_IN.UTF-8 \
	az_AZ.UTF-8 \
	be_BY.UTF-8 \
	bg_BG.ISO8859-5 \
	bg_BG.UTF-8 \
	bn_IN.UTF-8 \
	ca_ES.ISO8859-1 \
	ca_ES.ISO8859-15 \
	ca_ES.UTF-8 \
	common \
	cs_CZ.ISO8859-2 \
	cs_CZ.UTF-8 \
	cs_CZ.UTF-8@euro \
	da_DK.ISO8859-1 \
	da_DK.ISO8859-15 \
	da_DK.UTF-8 \
	de_AT.ISO8859-15 \
	de_DE.ISO8859-15 \
	de_DE.UTF-8 \
	de_DE.UTF-8@euro \
	el_CY.UTF-8 \
	el_GR.ISO8859-7 \
	el_GR.ISO8859-7@euro \
	el_GR.UTF-8 \
	en_AU.ISO8859-1 \
	en_AU.UTF-8 \
	en_CA.ISO8859-1 \
	en_CA.UTF-8 \
	en_GB.ISO8859-1 \
	en_GB.ISO8859-15 \
	en_GB.UTF-8 \
	en_IE.ISO8859-1 \
	en_IE.ISO8859-15 \
	en_IE.UTF-8 \
	en_MT.UTF-8 \
	en_NZ.ISO8859-1 \
	en_NZ.UTF-8 \
	en_US.ISO8859-1 \
	en_US.ISO8859-15 \
	es_ES.ISO8859-15 \
	es_ES.UTF-8 \
	es_ES.UTF-8@euro \
	es_US.UTF-8 \
	et_EE.ISO8859-15 \
	et_EE.UTF-8 \
	fi_FI.ISO8859-1 \
	fi_FI.ISO8859-15 \
	fi_FI.UTF-8 \
	fr_BE.ISO8859-15 \
	fr_BE.UTF-8 \
	fr_BE.UTF-8@euro \
	fr_FR.ISO8859-15 \
	fr_FR.UTF-8@euro \
	gu_IN.UTF-8 \
	he_IL.ISO8859-8 \
	he_IL.UTF-8 \
	hi_IN.UTF-8 \
	hr_HR.ISO8859-2 \
	hr_HR.UTF-8 \
	hu_HU.ISO8859-2 \
	hu_HU.UTF-8 \
	hy_AM.UTF-8 \
	id_ID.UTF-8 \
	is_IS.ISO8859-1 \
	is_IS.UTF-8 \
	iso_8859_1 \
	iso_8859_13 \
	iso_8859_15 \
	iso_8859_2 \
	iso_8859_5 \
	iso_8859_6 \
	iso_8859_7 \
	iso_8859_8 \
	iso_8859_9 \
	it_CH.UTF-8 \
	it_IT.ISO8859-15 \
	it_IT.UTF-8 \
	it_IT.UTF-8@euro \
	ja_JP.PCK \
	ja_JP.UTF-8 \
	ja_JP.eucJP \
	ka_GE.UTF-8 \
	kk_KZ.UTF-8 \
	kn_IN.UTF-8 \
	ko_KR.EUC \
	ko_KR.UTF-8 \
	ks_IN.UTF-8 \
	ku_TR.UTF-8 \
	ky_KG.UTF-8 \
	lt_LT.ISO8859-13 \
	lt_LT.UTF-8 \
	lv_LV.ISO8859-13 \
	lv_LV.UTF-8 \
	mk_MK.ISO8859-5 \
	mk_MK.UTF-8 \
	ml_IN.UTF-8 \
	mr_IN.UTF-8 \
	ms_MY.UTF-8 \
	mt_MT.UTF-8 \
	nb_NO.UTF-8 \
	nl_BE.ISO8859-1 \
	nl_BE.ISO8859-15 \
	nl_BE.UTF-8 \
	nl_NL.ISO8859-1 \
	nl_NL.ISO8859-15 \
	nl_NL.UTF-8 \
	nn_NO.UTF-8 \
	or_IN.UTF-8 \
	pa_IN.UTF-8 \
	pl_PL.ISO8859-2 \
	pl_PL.UTF-8 \
	pt_BR.ISO8859-1 \
	pt_BR.UTF-8 \
	pt_PT.ISO8859-1 \
	pt_PT.ISO8859-15 \
	pt_PT.UTF-8 \
	ro_RO.ISO8859-2 \
	ro_RO.UTF-8 \
	ru_RU.ANSI1251 \
	ru_RU.ISO8859-5 \
	ru_RU.KOI8-R \
	ru_RU.UTF-8 \
	sa_IN.UTF-8 \
	sk_SK.ISO8859-2 \
	sk_SK.UTF-8 \
	sl_SI.ISO8859-2 \
	sl_SI.UTF-8 \
	sq_AL.ISO8859-2 \
	sq_AL.UTF-8 \
	sr_ME.UTF-8 \
	sr_RS.UTF-8 \
	sv_SE.ISO8859-15 \
	sv_SE.UTF-8 \
	sv_SE.UTF-8@euro \
	ta_IN.UTF-8 \
	te_IN.UTF-8 \
	th_TH.TIS620 \
	th_TH.UTF-8 \
	tr_TR.ISO8859-9 \
	tr_TR.UTF-8 \
	uk_UA.UTF-8 \
	vi_VN.UTF-8 \
	zh_CN.EUC \
	zh_CN.GB18030 \
	zh_CN.GBK \
	zh_CN.UTF-8 \
	zh_HK.BIG5HK \
	zh_HK.UTF-8 \
	zh_TW.BIG5 \
	zh_TW.EUC \
	zh_TW.UTF-8


all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber

TARGETDIR = $(FILEROOT)/usr/lib/locale/

TARGETDIRS = $(TARGETDIR)

FILES = lcttab

all: $(LOCALES)

include $(SRC)/src/Makefile.targ

install: $(LOCALES) $(FILES.obj)

$(LOCALES): $(TARGETDIRS) FRC
	cd $@; pwd; $(MAKE) $(MFLAGS) $(TARGET)

clean: $(LOCALES) objclean

FRC:

