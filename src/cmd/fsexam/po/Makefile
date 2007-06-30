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
#ident	"@(#)Makefile	1.0	06/03/07	SMI"
#
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

include $(SRC)/Makefile.master

GETTEXT_PACKAGE = fsexam
PACKAGE = fsexam
VERSION = 0.3

SHELL = /bin/sh
GMSGFMT = /usr/bin/msgfmt
MSGFMT = /usr/bin/msgfmt

srcdir = .

POFILES =  de.po es.po fr.po it.po ja.po ko.po pt_BR.po sv.po zh_CN.po zh_HK.po zh_TW.po
GMOFILES =  de.gmo es.gmo fr.gmo it.gmo ja.gmo ko.gmo pt_BR.gmo sv.gmo zh_CN.gmo zh_HK.gmo zh_TW.gmo

CATALOGS =  de.mo es.mo fr.mo it.mo ja.mo ko.mo pt_BR.mo sv.mo zh_CN.mo zh_HK.mo zh_TW.mo
CATOBJEXT = .gmo
INSTOBJEXT = .mo

localedir = $(FILEROOT)/usr/share/locale
gnulocaledir = $(FILEROOT)/usr/share/locale

all: $(CATALOGS)

.po.mo:
	$(MSGFMT) -o $@ $<

de.mo:	de.po
	$(MSGFMT) -o de.mo de.po
es.mo:	es.po
	$(MSGFMT) -o es.mo es.po
fr.mo:	fr.po
	$(MSGFMT) -o fr.mo fr.po
it.mo:	it.po
	$(MSGFMT) -o it.mo it.po
ja.mo:	ja.po
	$(MSGFMT) -o ja.mo ja.po
ko.mo:	ko.po
	$(MSGFMT) -o ko.mo ko.po
pt_BR.mo:	pt_BR.po
	$(MSGFMT) -o pt_BR.mo pt_BR.po
sv.mo:	sv.po
	$(MSGFMT) -o sv.mo sv.po
zh_CN.mo:	zh_CN.po
	$(MSGFMT) -o zh_CN.mo zh_CN.po
zh_TW.mo:	zh_TW.po
	$(MSGFMT) -o zh_TW.mo zh_TW.po
zh_HK.mo:	zh_HK.po
	$(MSGFMT) -o zh_HK.mo zh_HK.po




.po.gmo:
	$(GMSGFMT) -o $@ $<


install: all
	@catalogs='$(CATALOGS)'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  destdir=$(localedir); \
	  lang=`echo $$cat | sed 's/\$(CATOBJEXT)$$//'`; \
	  dir=$(DESTDIR)/$$destdir/$$lang/LC_MESSAGES; \
	  $(INSDIR) $$dir; \
	  $(INS) $$dir $$cat; \
	  mv $$dir/$$cat $$dir/$(GETTEXT_PACKAGE)$(INSTOBJEXT); \
	  echo "installing $$cat as $$dir/$(GETTEXT_PACKAGE)$(INSTOBJEXT)"; \
	done

uninstall:
	catalogs='$(CATALOGS)'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed 's/\$(CATOBJEXT)$$//'`; \
	  rm -f $(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(GETTEXT_PACKAGE)$(INSTOBJEXT); \
	  rm -f $(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(GETTEXT_PACKAGE)$(INSTOBJEXT).m; \
	  rm -f $(DESTDIR)$(gnulocaledir)/$$lang/LC_MESSAGES/$(GETTEXT_PACKAGE)$(INSTOBJEXT); \
	  rm -f $(DESTDIR)$(gnulocaledir)/$$lang/LC_MESSAGES/$(GETTEXT_PACKAGE)$(INSTOBJEXT).m; \
	done

check: all


mostlyclean:
	rm -f core core.* *.pox $(GETTEXT_PACKAGE).po *.old.po cat-id-tbl.tmp
	rm -fr *.o

clean: mostlyclean
