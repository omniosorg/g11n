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
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# ident  "@(#)Makefile 1.0 10/03/07 SMI"
#

include $(SRC)/Makefile.master

SUBDIRS = 

ALL_LOCALES.tar= $(ALL_LOCALES:%=%.tar)


all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean
links:= 	TARGET= links
$(ALL_LOCALES):= TARGET= links 

.KEEP_STATE:

all: link 

install:

clean clobber: 
	$(RM) $(DEST)/*.bz2

link: 
	$(RM) -r $(FILEROOT)/lib $(FILEROOT)/openwin
	$(LN) -s $(FILEROOT)/usr/lib $(FILEROOT)/lib
	$(LN) -s $(FILEROOT)/usr/openwin $(FILEROOT)/openwin

links: 
	 $(SRC)/tools/create_links.sh all $(FILEROOT)

linkclean:
	for i in `find $(FILEROOT) -type l`; do\
		echo "removing $$i"; \
		rm $$i; \
	done


tarball:  $(DEST)
	cd $(DEST); $(TAR) cvf all_locales.tar -C $(FILEROOT) usr/; \
	$(RM) all_locales.tar.bz2; $(BZIP2) all_locales.tar

tarballs: $(DEST)
	for i in $(ALL_LOCALES); do \
		$(SRC)/tools/create_tarballs.sh $$i $(FILEROOT); \
		mv $(FILEROOT)/$$i.tar.bz2 $(DEST); \
	done
	mv $(FILEROOT)/common.tar.bz2 $(DEST)

$(DEST):
	$(INSDIR) $(DEST)

$(ALL_LOCALES): 
	$(SRC)/tools/create_tarballs.sh $@ $(FILEROOT)
	mv $(FILEROOT)/$@.tar.bz2 $(DEST)
	cp $(FILEROOT)/common.tar.bz2 $(DEST)


#%.tar: %
#	$(SRC)/tools/create_tarballs.sh $< $(FILEROOT)
#	mv $(FILEROOT)/$<.tar.bz2 $(DEST)

$(SUBDIRS): FRC
	-@cd $@; pwd; $(MAKE) $(TARGET)


FRC:
