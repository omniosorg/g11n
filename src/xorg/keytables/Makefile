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

include $(SRC)/Makefile.master

XKBDATA_SOURCE = xkbdata-1.0.1.tar.gz
XKBDATA_SITE = ftp://ftp.x.org/pub/individual/data
XKBDATA_DIR = xkbdata-1.0.1

all:=           TARGET= all
install :=      TARGET= install
clean :=        TARGET= clean
clobber :=      TARGET= clobber

$(XKBDATA_SOURCE):
	$(WGET) $(XKBDATA_SITE)/$(XKBDATA_SOURCE)

$(XKBDATA_DIR): $(XKBDATA_SOURCE)
	gzip -dc $(XKBDATA_SOURCE)|tar xf -
	# FIXME: use the same strange way as DGE_PKG ws
	for i in `cd sun-src ; find . -type f -print` ; \
	do \
		mkdir -p $(XKBDATA_DIR)/`dirname $$i` ; \
		[ -f $(XKBDATA_DIR)/$$i ] || cp -p sun-src/$$i $(XKBDATA_DIR)/$$i ; \
	done

all install:  $(XKBDATA_DIR)
	@cd $(XKBDATA_DIR); pwd; $(MAKE) $(MFLAGS) $(TARGET)

clean:
	rm -Rf $(XKBDATA_DIR) $(XKBDATA_SOURCE)

FRC:
