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

LOCALES = ar ar_EG ar_EG.UTF-8 ar_SA ar_SA.UTF-8 bg_BG bg_BG.ISO8859-5 bg_BG.UTF-8 ca ca_ES ca_ES.ISO8859-1 ca_ES.ISO8859-15 \
ca_ES.UTF-8 common cs_CZ cs_CZ.ISO8859-2 cs_CZ.UTF-8 cs_CZ.UTF-8@euro da_DK da_DK.ISO8859-1 da_DK.ISO8859-15 da_DK.UTF-8 de de_AT \
de_AT.ISO8859-1 de_AT.ISO8859-15 de_AT.UTF-8 de_CH de_CH.ISO8859-1 de_CH.UTF-8 de_DE de_DE.ISO8859-1 de_DE.ISO8859-15 de_DE.UTF-8 \
de_DE.UTF-8@euro de_LU de_LU.UTF-8 el_CY el_CY.UTF-8 el_GR el_GR.ISO8859-7 el_GR.ISO8859-7@euro el_GR.UTF-8 en_AU en_AU.ISO8859-1 \
en_AU.UTF-8 en_CA en_CA.ISO8859-1 en_CA.UTF-8 en_GB en_GB.ISO8859-1 en_GB.ISO8859-15 en_GB.UTF-8 en_IE en_IE.ISO8859-1 en_IE.ISO8859-15 \
en_IE.UTF-8 en_MT en_MT.UTF-8 en_NZ en_NZ.ISO8859-1 en_NZ.UTF-8 en_US en_US.ISO8859-1 en_US.ISO8859-15 en_US.UTF-8 es es_AR es_AR.ISO8859-1 \
es_AR.UTF-8 es_BO es_BO.ISO8859-1 es_BO.UTF-8 es_CL es_CL.ISO8859-1 es_CL.UTF-8 es_CO es_CO.ISO8859-1 es_CO.UTF-8 es_CR es_CR.ISO8859-1 \
es_CR.UTF-8 es_EC es_EC.ISO8859-1 es_EC.UTF-8 es_ES es_ES.ISO8859-1 es_ES.ISO8859-15 es_ES.UTF-8 es_ES.UTF-8@euro es_GT es_GT.ISO8859-1 \
es_GT.UTF-8 es_MX es_MX.ISO8859-1 es_MX.UTF-8 es_NI es_NI.ISO8859-1 es_NI.UTF-8 es_PA es_PA.ISO8859-1 es_PA.UTF-8 es_PE es_PE.ISO8859-1 \
es_PE.UTF-8 es_PY es_PY.ISO8859-1 es_PY.UTF-8 es_SV es_SV.ISO8859-1 es_SV.UTF-8 es_UY es_UY.ISO8859-1 es_UY.UTF-8 es_VE es_VE.ISO8859-1 \
es_VE.UTF-8 et_EE et_EE.ISO8859-15 et_EE.UTF-8 fi_FI fi_FI.ISO8859-1 fi_FI.ISO8859-15 fi_FI.UTF-8 fr fr_BE fr_BE.ISO8859-1 \
fr_BE.ISO8859-15 fr_BE.UTF-8 fr_BE.UTF-8@euro fr_CA fr_CA.ISO8859-1 fr_CA.UTF-8 fr_CH fr_CH.ISO8859-1 fr_CH.UTF-8 fr_FR fr_FR.ISO8859-1 \
fr_FR.ISO8859-15 fr_FR.UTF-8 fr_FR.UTF-8@euro fr_LU fr_LU.UTF-8 he he_IL he_IL.UTF-8 hi_IN.UTF-8 hr_HR hr_HR.ISO8859-2 hr_HR.UTF-8 \
hu_HU hu_HU.ISO8859-2 hu_HU.UTF-8 is_IS is_IS.ISO8859-1 is_IS.UTF-8 it it_IT it_IT.ISO8859-1 it_IT.ISO8859-15 it_IT.UTF-8 \
it_IT.UTF-8@euro iso_8859_1 iso_8859_13 iso_8859_15 iso_8859_2 iso_8859_5 iso_8859_7 iso_8859_9 ja ja_JP.PCK ja_JP.eucJP ja_JP.UTF-8 ko ko.UTF-8 lt_LT \
lt_LT.ISO8859-13 lt_LT.UTF-8 lv_LV lv_LV.ISO8859-13 lv_LV.UTF-8 mk_MK mk_MK.ISO8859-5 mk_MK.UTF-8 mt_MT mt_MT.UTF-8 nb_NO nb_NO.UTF-8 \
nl_BE nl_BE.ISO8859-1 nl_BE.ISO8859-15 nl_BE.UTF-8 nl_NL nl_NL.ISO8859-1 nl_NL.ISO8859-15 nl_NL.UTF-8 nn_NO nn_NO.UTF-8 no_NO \
no_NO.ISO8859-1@bokmal no_NO.ISO8859-1@nynorsk pl_PL pl_PL.ISO8859-2 pl_PL.UTF-8 pt_BR pt_BR.ISO8859-1 pt_BR.UTF-8 pt_PT pt_PT.ISO8859-1 \
pt_PT.ISO8859-15 pt_PT.UTF-8 ro_RO ro_RO.ISO8859-2 ro_RO.UTF-8 ru_RU ru_RU.ANSI1251 ru_RU.ISO8859-5 ru_RU.KOI8-R ru_RU.UTF-8 sh_BA \
sh_BA.ISO8859-2@bosnia sh_BA.UTF-8 sk_SK sk_SK.ISO8859-2 sk_SK.UTF-8 sl_SI sl_SI.ISO8859-2 sl_SI.UTF-8 sq_AL sq_AL.ISO8859-2 \
sq_AL.UTF-8 sr_CS sr_CS.UTF-8 sr_YU sr_YU.ISO8859-5 sv sv_SE sv_SE.ISO8859-1 sv_SE.ISO8859-15 sv_SE.UTF-8 sv_SE.UTF-8@euro ta_IN.UTF-8 te_IN.UTF-8 th_TH \
th_TH.UTF-8 tr_TR tr_TR.ISO8859-9 tr_TR.UTF-8 zh zh_CN.GB18030 zh_CN.GBK zh_CN.UTF-8 zh_HK.BIG5HK zh_HK.UTF-8 zh_TW zh_TW.BIG5 \
zh_TW.UTF-8 uk_UA uk_UA.UTF-8 ms_MY.UTF-8 id_ID.UTF-8 vi_VN.UTF-8

all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber

TARGETDIR = $(FILEROOT)/usr/lib/locale/

TARGETDIRS = $(TARGETDIR)

FILES = geo lcttab

all: $(LOCALES)

include $(SRC)/src/Makefile.targ

install: $(LOCALES) $(FILES.obj)

$(LOCALES): $(TARGETDIRS) FRC
	cd $@; pwd; $(MAKE) $(MFLAGS) $(TARGET)

clean: $(LOCALES) objclean

FRC:

