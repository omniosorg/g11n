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

include $(SRC)/Makefile.master

TARGETDIR = $(FILEROOT)/openwin/lib/X11/xkb/

XKB_DIRFILES = compat.dir geometry.dir keycodes.dir keymap.dir symbols.dir types.dir
SUBDIRS = compat compiled geometry keycodes keymap rules semantics symbols types


all:=		TARGET= all
install :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber


$(TARGETDIR):
	$(INSDIR) "$@"

%.dir:
	(cd $(@:%.dir=%); $(XKBCOMP_XSUN) -lfhlpR -o ../$@ '*')

.install_list:
	find $(SUBDIRS) -type f ! -name Imakefile > $@

all: $(XKB_DIRFILES)

install: all $(TARGETDIR) .install_list
	$(TAR) cf - -I .install_list $(XKB_DIRFILES)| (cd $(TARGETDIR); $(TAR) xf -)

clean:
	$(RM) -f $(XKB_DIRFILES) .install_list
