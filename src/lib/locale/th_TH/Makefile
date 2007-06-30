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
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
#ident	"@(#)Makefile 1.0     06/03/20 SMI"
#
#
# Makefile for th_TH locale(in CSI environment).
#

LOCALE		= th_TH

dummy: all

include ../common/Makefile.asian

install: all $(TARGETDIRS) $(FILES.obj) 
	$(INS) $(LC_CTYPE_DIR) -m 755 $(RESOLVE_SO)
	$(INS) $(LC_CTYPE_DIR) -m 755 LCL_DEF
	$(INS) $(LC_CTYPE_DIR_64) -m 755 $(MACH64)/$(RESOLVE_SO)

all: LCL_DEF $(MACH64) $(RESOLVE_SO)

