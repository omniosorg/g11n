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
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
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
