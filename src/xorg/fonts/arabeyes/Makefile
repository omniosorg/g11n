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

PROJECT		= arabeyes
VER 		= ae_fonts_2.0
TARBALL		= $(VER).tar.bz2
SOURCE_URL	= http://downloads.sourceforge.net/project/arabeyes/Fonts/ae_fonts/$(TARBALL)

ROOT_FONT_DIR 	= $(FONTROOT)/$(PROJECT)
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/ttf-$(PROJECT)

FONTS =	AAHS/ae_Cortoba.ttf \
	AAHS/ae_Nagham.ttf \
	AAHS/ae_Ostorah.ttf \
	AGA/ae_Dimnah.ttf \
	AGA/ae_Furat.ttf \
	AGA/ae_Granada.ttf \
	AGA/ae_Kayrawan.ttf \
	AGA/ae_Mashq-Bold.ttf \
	AGA/ae_Mashq.ttf \
	AGA/ae_Nada.ttf \
	AGA/ae_Petra.ttf \
	AGA/ae_Rasheeq-Bold.ttf \
	AGA/ae_Sindbad.ttf \
	FS/ae_Arab.ttf \
	FS/ae_Graph.ttf \
	FS/ae_Japan.ttf \
	FS/ae_Jet.ttf \
	FS/ae_Metal.ttf \
	FS/ae_Nice.ttf \
	FS/ae_Salem.ttf \
	FS/ae_Shado.ttf \
	Kasr/ae_AlArabiya.ttf \
	Kasr/ae_AlBattar.ttf \
	Kasr/ae_AlHor.ttf \
	Kasr/ae_AlManzomah.ttf \
	Kasr/ae_AlMateen-Bold.ttf \
	Kasr/ae_AlMohanad.ttf \
	Kasr/ae_AlMothnna-Bold.ttf \
	MCS/ae_Electron.ttf \
	MCS/ae_Haramain.ttf \
	MCS/ae_Hor.ttf \
	MCS/ae_Tholoth.ttf \
	Shmookh/ae_AlYarmook.ttf \
	Shmookh/ae_Hani.ttf \
	Shmookh/ae_Khalid.ttf \
	Shmookh/ae_Ouhod-Bold.ttf \
	Shmookh/ae_Rehan.ttf \
	Shmookh/ae_Sharjah.ttf \
	Shmookh/ae_Tarablus.ttf \
	../fonts.dir \
	../fonts.scale

DOCS =	ChangeLog \
	README

all: $(VER)/.unpacked

install: $(VER)/.installed


include ../Makefile.fonts
include $(SRC)/Makefile.targ
