#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
d none lib 0755 root bin
d none lib/lp 0755 root lp
d none lib/lp/locale 0755 root lp
d none lib/lp/locale/C 0755 root lp
d none lib/lp/locale/C/mp 0755 root lp
f none lib/lp/locale/C/mp/mp.common.ps 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.alt.ps 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.altl.ps 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.ff.ps 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.fp.ps 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.l.ps 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.ll.ps 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.ps 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.tm.ps 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.ts.ps 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.alt.xpr 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.altl.xpr 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.ff.xpr 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.fp.xpr 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.l.xpr 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.ll.xpr 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.xpr 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.tm.xpr 0644 root lp
f none lib/lp/locale/C/mp/mp.pro.ts.xpr 0644 root lp
d none lib/lp/locale/en_US.UTF-8 0755 root lp
d none lib/lp/locale/en_US.UTF-8/mp 0755 root lp
f none lib/lp/locale/en_US.UTF-8/mp/mp.conf 0444 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xuiso88591.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xuiso88592.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xuiso88594.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xuiso88595.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xuiso88597.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xuiso88599.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xuiso885910.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xubig5.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xugb2312.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xujisx0201.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xujisx0208.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xujisx0212.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xujohap92.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xugbk.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xufallback.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/xuunicode.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/ctlarabic.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/ctlthai.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/ctlhebrew.so 0555 root lp
f none lib/lp/locale/en_US.UTF-8/mp/ctlhindi.so 0555 root lp
d none bin 0755 root bin
f none bin/mp 0555 root bin
d none openwin 0755 root bin
d none openwin/bin 0755 root bin
s none openwin/bin/mp=../../bin/mp
f none bin/mailp 0555 root bin
s none bin/digestp=./mailp
s none bin/filep=./mailp
s none bin/filofaxp=./mailp
s none bin/franklinp=mailp
s none bin/newsp=./mailp
s none bin/timemanp=./mailp
s none bin/timesysp=./mailp

s none lib/lp/locale/ar_EG.UTF-8=./en_US.UTF-8
s none lib/lp/locale/ar_SA.UTF-8=./en_US.UTF-8
s none lib/lp/locale/bg_BG.UTF-8=./en_US.UTF-8
s none lib/lp/locale/cs_CZ.UTF-8@euro=./en_US.UTF-8
s none lib/lp/locale/de_AT.UTF-8=./en_US.UTF-8
s none lib/lp/locale/de_DE.UTF-8=./en_US.UTF-8
s none lib/lp/locale/de_DE.UTF-8@euro=./en_US.UTF-8
s none lib/lp/locale/de_CH.UTF-8=./en_US.UTF-8
s none lib/lp/locale/de_LU.UTF-8=./en_US.UTF-8
s none lib/lp/locale/de.UTF-8=./en_US.UTF-8
s none lib/lp/locale/en_AU.UTF-8=./en_US.UTF-8
s none lib/lp/locale/en_CA.UTF-8=./en_US.UTF-8
s none lib/lp/locale/en_GB.UTF-8=./en_US.UTF-8
s none lib/lp/locale/en_IE.UTF-8=./en_US.UTF-8
s none lib/lp/locale/en_NZ.UTF-8=./en_US.UTF-8
s none lib/lp/locale/en.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_AR.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_BO.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_CL.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_CO.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_CR.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_EC.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_ES.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_ES.UTF-8@euro=./en_US.UTF-8
s none lib/lp/locale/es_GT.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_NI.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_PA.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_PE.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_PY.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_SV.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_UY.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es_VE.UTF-8=./en_US.UTF-8
s none lib/lp/locale/es.UTF-8=./en_US.UTF-8
s none lib/lp/locale/fi_FI.UTF-8=./en_US.UTF-8
s none lib/lp/locale/fr_BE.UTF-8=./en_US.UTF-8
s none lib/lp/locale/fr_BE.UTF-8@euro=./en_US.UTF-8
s none lib/lp/locale/fr_CA.UTF-8=./en_US.UTF-8
s none lib/lp/locale/fr_FR.UTF-8=./en_US.UTF-8
s none lib/lp/locale/fr_FR.UTF-8@euro=./en_US.UTF-8
s none lib/lp/locale/fr_CH.UTF-8=./en_US.UTF-8
s none lib/lp/locale/fr_LU.UTF-8=./en_US.UTF-8
s none lib/lp/locale/fr.UTF-8=./en_US.UTF-8
s none lib/lp/locale/he_IL.UTF-8=./en_US.UTF-8
s none lib/lp/locale/hi_IN.UTF-8=./en_US.UTF-8
s none lib/lp/locale/is_IS.UTF-8=./en_US.UTF-8
s none lib/lp/locale/it_IT.UTF-8=./en_US.UTF-8
s none lib/lp/locale/it_IT.UTF-8@euro=./en_US.UTF-8
s none lib/lp/locale/it.UTF-8=./en_US.UTF-8
s none lib/lp/locale/mk_MK.UTF-8=./en_US.UTF-8
s none lib/lp/locale/nl_BE.UTF-8=./en_US.UTF-8
s none lib/lp/locale/nl_NL.UTF-8=./en_US.UTF-8
s none lib/lp/locale/pl_PL.UTF-8=./en_US.UTF-8
s none lib/lp/locale/pl.UTF-8=./en_US.UTF-8
s none lib/lp/locale/pt_BR.UTF-8=./en_US.UTF-8
s none lib/lp/locale/ro_RO.UTF-8=./en_US.UTF-8
s none lib/lp/locale/ru_RU.UTF-8=./en_US.UTF-8
s none lib/lp/locale/ru.UTF-8=./en_US.UTF-8
s none lib/lp/locale/sh_BA.UTF-8=./en_US.UTF-8
s none lib/lp/locale/sq_AL.UTF-8=./en_US.UTF-8
s none lib/lp/locale/sr_CS.UTF-8=./en_US.UTF-8
s none lib/lp/locale/sv_SE.UTF-8=./en_US.UTF-8
s none lib/lp/locale/sv_SE.UTF-8@euro=./en_US.UTF-8
s none lib/lp/locale/sv.UTF-8=./en_US.UTF-8
s none lib/lp/locale/th_TH.UTF-8=./en_US.UTF-8
s none lib/lp/locale/tr_TR.UTF-8=./en_US.UTF-8
s none lib/lp/locale/uk_UA.UTF-8=./en_US.UTF-8
