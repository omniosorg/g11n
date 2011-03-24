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

PROJECT		= Vemana
VER 		= $(PROJECT)
TARBALL		= $(VER)2k-Li.zip
SOURCE_URL	= http://dlc.openindiana.org/gpl/sol-11-exp-201011-GPLSource/l10n/src/g11n/downloads/$(TARBALL)

FONT = vemana.ttf


ROOT_FONT_DIR 	= $(FILEROOT)/usr/share/fonts/TrueType//indic-fonts-core
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/indic-fonts-core

$(VER)/$(FONT): $(VER)/.unpacked_withdir
	cp $(VER)/vemana2000.ttf $@

$(VER)/.installed: $(VER)/$(FONT)
	$(INSDIR) -m 755 -u root -g bin $(ROOT_FONT_DIR)
	( cd $(VER); $(INS) $(ROOT_FONT_DIR) -m 0444 -u root -g bin $(FONT); )
	touch $@

all: $(VER)/.unpacked_withdir

install: $(VER)/.installed

clean:
	$(RM) -r $(VER) $(ORIG_VER)

clobber: clean

include $(SRC)/Makefile.targ

$(VER)/.unpacked_withdir: $(SOURCE_ARCHIVE)
	-$(RM) -r $(VER)
	mkdir $(VER)
	(cd $(VER) && $(UNPACKARCHIVE) $(SOURCE_ARCHIVE) .)
	touch $@

