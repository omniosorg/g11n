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
#
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
#ident	"@(#)mp.conf	1.26	09/06/06 SMI"
#
# Korean(EUC) font preferred version.
#

FontNameAlias	iso88591R	TrueType	/usr/X11/lib/X11/fonts/TTF/LiberationMono-Regular.ttf
FontGroup	iso88591	TrueType	iso88591R

FontNameAlias	johap92R_WC	TrueType	/usr/X11/lib/X11/fonts/TrueType/hanyang/h2gtrm.ttf
FontGroup	johap92_WC	TrueType johap92R_WC


#
#
#
MapCode2Font	0x0000 0x00FF	iso88591
MapCode2Font	0x30000020 0x3000ffff	johap92_WC

#
#
#
CnvCode2Font iso88591R _xu2unicode /usr/lib/lp/locale/en_US.UTF-8/mp/xuunicode.so
CnvCode2Font johap92R_WC _xwc2ksc5601_1987_unicode /usr/lib/lp/locale/ko/mp/xwcksc5601_1987_unicode.so
