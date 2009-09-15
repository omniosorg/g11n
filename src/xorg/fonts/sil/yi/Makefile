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

PROJECT		= ttf-sil-yi
VER 		= $(PROJECT)-1.200
TARBALL		= $(PROJECT)_1.200.orig.tar.gz
SOURCE_URL	= http://ftp.debian.org/debian/pool/main/t/$(PROJECT)/$(TARBALL)

FONTS = silyi.ttf

ID =	SilYi

DOCS =	

ROOT_FONT_DIR 	= $(FILEROOT)/X11/lib/X11/fonts/TrueType/sil
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/ttf-sil

all: $(VER)/.unpacked
	touch $(VER)/.docs_installed

install: all $(VER)/.installed

include ../../Makefile.fonts
include $(SRC)/Makefile.targ

