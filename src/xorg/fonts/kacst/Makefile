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

PROJECT		= kacst
VER 		= KacstArabicFonts-2.0
TARBALL		= kacst_fonts_2.0.tar.bz2
SOURCE_URL	= http://downloads.sourceforge.net/project/arabeyes/kacst_fonts/2.0/$(TARBALL)

ROOT_FONT_DIR 	= $(FILEROOT)/X11/lib/X11/fonts/TrueType/$(PROJECT)
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/ttf-$(PROJECT)

FONTS =	KacstArt.ttf \
	KacstBook.ttf \
	KacstDecorative.ttf \
	KacstDigital.ttf \
	KacstFarsi.ttf \
	KacstLetter.ttf \
	KacstNaskh.ttf \
	KacstOffice.ttf \
	KacstOne.ttf \
	KacstPen.ttf \
	KacstPoster.ttf \
	KacstQurn.ttf \
	KacstScreen.ttf \
	KacstTitle.ttf \
	KacstTitleL.ttf \
	../fonts.dir \
	../fonts.scale

DOCS =	Copyright \
	README

all: $(VER)/.unpacked $(VER)/KacstPen.ttf

$(VER)/KacstPen.ttf: $(VER)/.unpacked
	$(CP) $(VER)/kacstPen.ttf $@

install: $(VER)/.installed

clean:
	$(RM) -r $(VER)

clobber: clean

FRC:

include ../Makefile.fonts
include $(SRC)/Makefile.targ
