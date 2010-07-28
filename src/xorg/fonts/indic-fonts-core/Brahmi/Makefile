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
# Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
#

include $(SRC)/Makefile.master

PROJECT		= kanotf
VER 		= $(PROJECT)
TARBALL		= $(VER).zip
SOURCE_URL	= http://prdownloads.sourceforge.net/brahmi/$(TARBALL)?download
WGET_OPT	+= -O $(TARBALL)

FONTS = Malige-b.ttf \
	Malige-n.ttf

DOCS = 

FONTS_VER	= $(FONTS:%=$(VER)/%)

$(VER)/%.ttf: $(VER)/fonts/%.TTF
	$(CP) $< $@

$(VER)/fonts/%.TTF: $(VER)/.unpacked

install: $(FONTS_VER) $(VER)/.installed

include ../Makefile.common
