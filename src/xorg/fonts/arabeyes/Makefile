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

ROOT_FONT_DIR 	= $(FILEROOT)/X11/lib/X11/fonts/TrueType/arabeyes
ROOT_DOC_DIR 	= $(FILEROOT)/usr/share/doc/ttf-arabeyes

FONT_TAR	= ae_fonts_2.0.tar.bz2
FONT_DOC_TAR	= ae_fonts_doc_2.0.tar.bz2

all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean
clobber:=	TARGET= clobber
lint:=		TARGET= lint

.KEEP_STATE:

all: $(FONT_TAR)

$(ROOT_FONT_DIR):
	$(INSDIR) "$@"

$(ROOT_DOC_DIR):
	$(INSDIR) "$@"

font_install: $(ROOT_FONT_DIR) $(ROOT_DOC_DIR)
	bzip2 -d -c $(FONT_TAR) | (cd $(ROOT_FONT_DIR) ; tar xpf -)
	bzip2 -d -c $(FONT_DOC_TAR) | (cd $(ROOT_DOC_DIR) ; tar xpf -)

install: all font_install

clean:

clobber: clean

FRC: