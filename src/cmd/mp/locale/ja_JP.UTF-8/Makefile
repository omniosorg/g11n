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
#ident  "@(#)Makefile 1.13 04/10/26 SMI"
#

include 	$(SRC)/Makefile.master

ROOTLIBDIR	= $(FILEROOT)/usr/lib
ROOTLIBLP	= $(ROOTLIBDIR)/lp

LOCALE:sh = basename `pwd`
MKTBL_PWD:sh = pwd

ROOT_MP_DIR	= $(ROOTLIBLP)/locale/$(LOCALE)/mp

HEADER_INC	= -I$(SRC)/src/lib/iconv/ja

JOW_UTF8	= $(SRC)/src/openwin/lib/locale/ja_JP.UTF-8/conv_tables
JISX0212	= $(JOW_UTF8)/JISX0212-0.GR.txt
JISX02131	= $(JOW_UTF8)/JISX0213-1.GL.txt
JISX02132	= $(JOW_UTF8)/JISX0213-2.GL.txt
JISX0212_TO_UTF8  = $(JISX0212:$(JOW_UTF8)/%.GR.txt=%_tbl)
JISX02131_TO_UTF8 = $(JISX02131:$(JOW_UTF8)/%.GL.txt=%_tbl)
JISX02132_TO_UTF8 = $(JISX02132:$(JOW_UTF8)/%.GL.txt=%_tbl)
JISX0213_TO_UTF8  = $(JISX02131_TO_UTF8) $(JISX02132_TO_UTF8)

MPSRC_FILES  	= xu2UTF8.c
MPLD_FILES	= $(MPSRC_FILES:%.c=%.so)
MPCONF_FILES 	= mp.conf
MKTBL_SRC	= mktbl.c

MPLD_DIR_FILES 	= $(ROOT_MP_DIR)/$(MPLD_FILES) \
		  $(ROOT_MP_DIR)/$(MPCONF_FILES)

CFLAGS += -G -Kpic

all:		$(ROOT_MP_DIR)  $(MPLD_FILES)

install:	all $(MPLD_DIRS) $(MPLD_DIR_FILES)

$(ROOT_MP_DIR) :
	$(INSDIR) $@

$(ROOT_MP_DIR)/% : $(ROOT_MP_DIR)
	$(INS) $(@D) $(@F)

$(MPLD_FILES): $(MPSRC_FILES)
	$(CC) -o mktbl mktbl.c
	$(GREP) '^\\' $(JISX0212) > JISX0212
	$(MKTBL_PWD)/mktbl JISX0212 > $(JISX0212_TO_UTF8)
	$(GREP) '^\\' $(JISX02131) > JISX02131
	$(MKTBL_PWD)/mktbl JISX02131 > $(JISX02131_TO_UTF8)
	$(GREP) '^\\' $(JISX02132) > JISX02132
	$(MKTBL_PWD)/mktbl JISX02132 > $(JISX02132_TO_UTF8)
	$(CC) $(CFLAGS) -o $@  $(MPSRC_FILES)

clean:
	$(RM) $(MPLD_FILES) $(MPSRC_FILES:%.c=%.o) $(MPLD_DIR_FILES)
	$(RM) mktbl $(JISX0212_TO_UTF8) $(JISX0213_TO_UTF8)
	$(RM) JISX0212 JISX02131 JISX02132
