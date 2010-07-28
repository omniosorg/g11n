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

SUBDIRS = \
	assamese \
	bengali \
	gujarati \
	hindi \
	kannada \
	kashmiri \
	konkani \
	maithili \
	malayalam \
	marathi \
	nepali \
	oriya \
	punjabi \
	sindhi \
	tamil \
	telugu

FILES = fonts.dir \
	fonts.scale

DOCS =	assamese/lohit-assamese-ttf-2.4.3/COPYING \
	assamese/lohit-assamese-ttf-2.4.3/COPYRIGHT

ROOT_FONT_DIR 	= $(FILEROOT)/usr/share/fonts/TrueType/lohit
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/ttf-lohit

all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber

all: $(SUBDIRS)

install: $(SUBDIRS) .files_installed

clean: $(SUBDIRS)
	$(RM) .*installed *bz2

.files_installed: .docs_installed
	$(INSDIR) -m 755 -u root -g bin $(ROOT_FONT_DIR)
	( for f in $(FILES); do \
		$(INS) $(ROOT_FONT_DIR) -m 0444 -u root -g bin $$f; \
	done; )
	touch $@

%.bz2: %
	$(BZIP2) -c $< > $@

.docs_installed: $(DOCS:%=%.bz2) $(SUBDIRS)
	$(INSDIR) -m 755 -u root -g bin $(ROOT_DOC_DIR)
	( for f in $(DOCS); do \
		$(INS) $(ROOT_DOC_DIR) -m 0444 -u root -g bin $$f.bz2; \
	done; )
	touch $@

$(SUBDIRS): FRC
	@cd $@; pwd; $(MAKE) $(MFLAGS) $(TARGET)

tarball:

FRC:

