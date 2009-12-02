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
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

include $(SRC)/Makefile.master

PROJECT		= gentium-basic
VER 		= $(PROJECT)-1.1
TARBALL		= $(VER).tar.bz2

ROOT_FONT_DIR 	= $(FONTROOT)/gentium
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/ttf-gentium/basic

FONTS =	GenBasB.ttf \
	GenBasBI.ttf \
	GenBasI.ttf \
	GenBasR.ttf \
	GenBkBasB.ttf \
	GenBkBasBI.ttf \
	GenBkBasI.ttf \
	GenBkBasR.ttf

DOCS = 	FONTLOG

all: $(VER)/.unpacked

$(VER)/.unpacked: $(TARBALL)
	-$(RM) -r $(VER)
	$(UNPACKARCHIVE) $(TARBALL) $(VER)
	touch $@

$(VER)/%: $(VER)/%.txt
	$(CP) $< $@

install: $(VER)/.installed


include ../Makefile.fonts
