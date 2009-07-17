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

include $(SRC)/Makefile.master

PROJECT		= gentium
VER 		= $(PROJECT)-1.02
TARBALL		= $(VER).tar.bz2

ROOT_FONT_DIR 	= $(FILEROOT)/X11/lib/X11/fonts/TrueType/gentium
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/ttf-gentium

FONTS =	GenAI102.ttf \
	GenAR102.ttf \
	GenI102.ttf \
	GenR102.ttf \
	../fonts.dir \
	../fonts.scale

DOCS =	FONTLOG \
	GENTIUM-FAQ \
	OFL \
	OFL-FAQ \
	QUOTES \
	README

all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean
clobber:=	TARGET= clobber
lint:=		TARGET= lint

.KEEP_STATE:

all: $(TARBALL) $(VER)

$(VER):
	$(BZIP2) -dc $(TARBALL) | $(TAR) xpf -

$(ROOT_FONT_DIR):
	$(INSDIR) -m 755 -u root -g bin $(ROOT_FONT_DIR)
	( cd $(VER); \
	for f in $(FONTS); do \
		$(INS) $(ROOT_FONT_DIR) -m 0444 -u root -g bin $$f; \
	done; )

$(ROOT_DOC_DIR):
	$(INSDIR) -m 755 -u root -g bin $(ROOT_DOC_DIR)
	( cd $(VER); \
	for f in $(DOCS); do \
		$(BZIP2) $$f; \
		$(INS) $(ROOT_DOC_DIR) -m 0444 -u root -g bin $$f.bz2; \
	done; )

font_install: $(ROOT_FONT_DIR) $(ROOT_DOC_DIR)

install: all font_install

clean:
	$(RM) -r $(VER)

clobber: clean

FRC:
