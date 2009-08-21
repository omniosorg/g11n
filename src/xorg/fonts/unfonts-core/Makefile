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

PROJECT		= unfonts
VER 		= un-fonts
TARBALL		= un-fonts-core-1.0.2-080608.tar.gz
SOURCE_URL	= http://kldp.net/frs/download.php/4695/$(TARBALL)

ROOT_FONT_DIR 	= $(FILEROOT)/X11/lib/X11/fonts/TrueType/$(PROJECT)
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/unfonts-core

FONTS =	UnBatang.ttf \
	UnBatangBold.ttf \
	UnDinaru.ttf \
	UnDinaruBold.ttf \
	UnDinaruLight.ttf \
	UnDotum.ttf \
	UnDotumBold.ttf \
	UnGraphic.ttf \
	UnGraphicBold.ttf \
	UnGungseo.ttf \
	UnPilgi.ttf \
	UnPilgiBold.ttf

DOCS =	README

all: $(VER)/.unpacked

install: $(VER)/.installed


include ../Makefile.fonts
include $(SRC)/Makefile.targ
