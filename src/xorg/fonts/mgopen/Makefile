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

PROJECT		= mgopen
VER 		= MgOpen
TARBALL		= $(VER).tar.gz
SOURCE_URL	= http://www.zvr.gr/typo/mgopen/files/$(TARBALL)

ROOT_FONT_DIR 	= $(FONTROOT)/$(PROJECT)
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/ttf-$(PROJECT)

FONTS =	MgOpenCanonicaBold.ttf \
	MgOpenCanonicaBoldItalic.ttf \
	MgOpenCanonicaItalic.ttf \
	MgOpenCanonicaRegular.ttf \
	MgOpenCosmeticaBold.ttf \
	MgOpenCosmeticaBoldOblique.ttf \
	MgOpenCosmeticaOblique.ttf \
	MgOpenCosmeticaRegular.ttf \
	MgOpenModataBold.ttf \
	MgOpenModataBoldOblique.ttf \
	MgOpenModataOblique.ttf \
	MgOpenModataRegular.ttf \
	MgOpenModernaBold.ttf \
	MgOpenModernaBoldOblique.ttf \
	MgOpenModernaOblique.ttf \
	MgOpenModernaRegular.ttf \
	../fonts.dir \
	../fonts.scale

DOCS =	

all: $(VER)/.created

install: $(VER)/.created $(VER)/.installed

include ../Makefile.fonts
include $(SRC)/Makefile.targ

$(VER)/.created: $(SOURCE_ARCHIVE)
	-$(RM) -r $(VER)
	-mkdir $(VER)
	$(UNPACKARCHIVE) $(SOURCE_ARCHIVE) .
	$(MV) *ttf $(VER)/
	touch $@ $(VER)/.unpacked $(VER)/.docs_installed

