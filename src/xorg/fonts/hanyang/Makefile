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
# Copyright 2011 Every City Ltd. All rights reserved.
#

include $(SRC)/Makefile.master

TARGETFONTDIR	= $(FILEROOT)/usr/share/fonts/TrueType/hanyang

FILES =	h2drrm.ttf \
	h2hsrm.ttf \
	h2gtrm.ttf \
	h2mjsm.ttf \
	sundotump.ttf \
	sundotumf.ttf \
	fonts.alias \
	fonts.scale \
	fonts.dir


install: all
	$(INSDIR) -m 775 -u root -g bin $(TARGETFONTDIR)
	for f in $(FILES); do \
		$(INS) $(TARGETFONTDIR) -m 644 -u root -g bin $$f ; \
	done

all: $(FILES)

lint: FRC

clean: $(SUBDIRS)
	$(RM) -r $(TARGETFONTDIR)

clobber: FRC

FRC:
