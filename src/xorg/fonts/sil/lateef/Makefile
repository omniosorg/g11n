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

PROJECT		= LateefRegOT
VER 		= $(PROJECT)_1.001
TARBALL		= $(VER).zip
SOURCE_URL	= http://scripts.sil.org/cms/scripts/render_download.php?site_id=nrsi\&format=file\&media_id=Lateef_OT_1_001\&filename=LateefRegOT_1.001.zip
WGET_OPT	= -O $(TARBALL)

FONTS = LateefRegOT.ttf

ID =	Lateef

DOCS =	FONTLOG.$(ID).txt

ROOT_FONT_DIR 	= $(FONTROOT)/sil
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/ttf-sil

all: $(VER)/.unpacked_withdir

$(VER)/%.$(ID).txt: $(VER)/%.txt
	cp $< $@

install: all $(VER)/.installed

include ../../Makefile.fonts
include $(SRC)/Makefile.targ

$(VER)/.unpacked_withdir $(VER)/.unpacked: $(SOURCE_ARCHIVE)
	-$(RM) -r $(VER)
	-mkdir $(VER)
	(cd $(VER) && $(UNPACKARCHIVE) $(SOURCE_ARCHIVE) .)
	touch $@
	touch $(VER)/.unpacked

