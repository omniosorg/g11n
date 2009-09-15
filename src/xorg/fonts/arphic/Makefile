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

ROOT_FONT_DIR 	= $(FILEROOT)/X11/lib/X11/fonts/TrueType/arphic
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/arphic

FONT_TAR1 = uming.tar.bz2
FONT_TAR2 = ukai.tar.bz2
FONT_DOCS = license
FONT_SCALE	= fonts.scale



all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean
clobber:=	TARGET= clobber
lint:=		TARGET= lint

.KEEP_STATE:

all: $(FONT_TAR1) $(FONT_TAR2)

font_install:
	mkdir -p $(ROOT_FONT_DIR) $(ROOT_DOC_DIR)
	bzip2 -d -c $(FONT_TAR1) | (cd $(ROOT_FONT_DIR) ; tar xpf -)
	bzip2 -d -c $(FONT_TAR2) | (cd $(ROOT_FONT_DIR) ; tar xpf -)
	cp -r $(FONT_DOCS) $(ROOT_DOC_DIR)
	cp fonts.dir $(ROOT_FONT_DIR)/fonts.dir
	cp fonts.alias $(ROOT_FONT_DIR)/fonts.alias
	cp fonts.scale $(ROOT_FONT_DIR)/fonts.scale

install: all  font_install

clean:

clobber: clean

$(SUBDIRS): FRC
	-@cd $@; pwd; $(MAKE) $(TARGET)

FRC:
