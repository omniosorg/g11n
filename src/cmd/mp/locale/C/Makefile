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

# Copyright (c) 2008, by Sun Microsystems, Inc.
# All rights reserved.


include $(SRC)/Makefile.master

LOCALE:sh = basename `pwd`

FILES1 = mp.conf
FILES2 = mp.msg

TARGETDIR1 = $(FILEROOT)/usr/lib/lp/locale/$(LOCALE)/mp
TARGETDIR2 = $(FILEROOT)/usr/lib/locale/$(LOCALE)/LC_MESSAGES
TARGETDIRS = $(TARGETDIR1) $(TARGETDIR2)

all: $(TARGETDIRS)

install: all $(FILES1:%=$(TARGETDIR1)/%) $(FILES2:%=$(TARGETDIR2)/%)

$(TARGETDIRS):
	$(INSDIR) $@

$(TARGETDIR1)/%: %
	$(INS) $(@D) $(@F)

$(TARGETDIR2)/%: %
	$(INS) $(@D) $(@F)

clean:
	$(RM) -r $(TARGETDIRS)
