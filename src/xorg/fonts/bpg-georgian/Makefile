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

PROJECT		= bpg-georgian
VER 		= $(PROJECT)
TARBALL		= BPG_Fonts.zip
WGET_OPT	= -O $(SOURCE_ARCHIVES)/$(TARBALL)
SOURCE_URL	= http://bpg-fonts.googlegroups.com/web/BPG_GPL%26GNU_Fonts.zip?gda=8QXdOEkAAAAVNtGpDzO2EW3rYrpy6g3Pu6-IwYn2wal95b5URPfx_aRUKIkfWgVA11iZN1A9Xo3Z9hDUC4qB4Xbq3pl19fs-hAioEG5q2hncZWbpWmJ7IQ

ROOT_FONT_DIR 	= $(FONTROOT)/$(PROJECT)
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/ttf-$(PROJECT)

FONTS_RENAME = \
	BPG_Algeti.ttf \
	BPG_Chveulebrivi.ttf \
	BPG_Courier.ttf \
	BPG_Courier_S.ttf \
	BPG_Elite.ttf \
	BPG_Glaho.ttf \
	BPG_Ingiri.ttf \
	BPG_Nino_Medium_Cond.ttf \
	BPG_Nino_Medium.ttf \
	BPG_Sans.ttf \
	BPG_Sans_Medium.ttf \
	BPG_Sans_Modern.ttf \
	BPG_Sans_Regular.ttf \
	BPG_Serif.ttf \
	BPG_Serif_Modern.ttf

FONTS =	$(FONTS_RENAME) \
	../fonts.dir \
	../fonts.scale

DOCS =	README.txt

VERFONTS = $(FONTS_RENAME:%.ttf=$(VER)/%.ttf)

all: $(VER)/.created $(VERFONTS)

install: $(VER)/.created $(VERFONTS) $(VER)/.installed

include ../Makefile.fonts
include $(SRC)/Makefile.targ

$(VER)/.created: $(SOURCE_ARCHIVE)
	-$(RM) -r $(VER)
	-mkdir $(VER)
	( cd $(VER) ; $(UNPACKARCHIVE) $(SOURCE_ARCHIVE) . )
	touch $@ $(VER)/.unpacked

$(VER)/%.ttf: $(VER)/%_GPL\&GNU.ttf
	cp "$<" $@

$(VER)/README.txt: $(VER)/Docs/BPG_GPL\&GNU_Fonts.txt
	cp $(VER)/Docs/BPG_GPL\&GNU_Fonts.txt $@

