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
# Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
#

include $(SRC)/Makefile.master
include $(SRC)/Makefile.buildnum

PRE_POUND=				pre\#
POUND_SIGN=				$(PRE_POUND:pre\%=%)

#
# Tentatively import SVR4 packages which have not yet migrated to
# IPS based build.
#

#
# Make sure we're getting a consistent execution environment for the
# embedded scripts.
#
SHELL= /usr/bin/ksh93

IMPORTER=env ROOT=/ ./importer.py 

i386_DEFINES=           \
	ARCH=i386       \
	ARCH32=i86      \
	ARCH64=amd64    \
	i386_ONLY=''    \
	sparc_ONLY=$(POUND_SIGN)

sparc_DEFINES=          \
	ARCH=sparc      \
	ARCH32=sparcv7  \
	ARCH64=sparcv9  \
	i386_ONLY=$(POUND_SIGN) \
	sparc_ONLY=''

ARCH_DEFINES=$($(PKGMACH)_DEFINES)

GLOBAL_CHATTRS= gnome variants

SVR4DIR1=../../../inputmethod/specs/build.$(PKGMACH)/PKGS
SVR4DIR2=PKGS2.$(PKGMACH)
SVR4PKGS2=g11nsvc

IMPLIST=import_list

REPOD=$(PKGDEST)/repo.import
REPO=file\://$(REPOD)

L10N_PUBLISHER=l10n-nightly
REFREPO=http://ipkg.sfbay.sun.com/dev/solaris

install: all imp
all:
clean:
	rm -rf $(SVR4DIR2)
clobber: clean
	rm -rf $(REPOD)

# workaround: specify @0.5.11-0.152 to enable creating b152 based iso images
imp: $(REPO) $(SVR4PKGS2:%=$(SVR4DIR2)/%) FRC
	$(IMPORTER) -b 0.$(L10N_BUILDNUM) -T \* \
	$(GLOBAL_CHATTRS:%= -G %) $(ARCH_DEFINES:%= -m %) \
	-R $(REFREPO)@entire@0.5.11-0.152 -s $(REPO) \
	-w $(SVR4DIR1) -w $(SVR4DIR2) -I. $(IMPLIST)

$(SVR4DIR2)/%: $(SVR4DIR2) %.$(PKGMACH).tar
	(cd $(SVR4DIR2); tar xf ../$(@F).$(PKGMACH).tar)

$(SVR4DIR2):
	mkdir -p $@

$(REPO):
	rm -rf $(REPOD)
	pkgrepo create --version 3 $(REPO)
	pkgrepo set -s $(REPO) publisher/prefix=$(L10N_PUBLISHER)
	/usr/lib/pkg.depotd -d $(REPOD) --add-content --exit-ready

FRC:
