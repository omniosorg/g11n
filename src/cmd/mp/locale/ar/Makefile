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
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
#ident  "@(#)Makefile 1.2 08/04/17 SMI"
#

include 	$(SRC)/Makefile.master

ROOTLIBDIR	= $(FILEROOT)/usr/lib
ROOTLIBLP	= $(ROOTLIBDIR)/lp

LOCALE:sh = basename `pwd`

ROOT_MP_DIR	= $(ROOTLIBLP)/locale/$(LOCALE)/mp

HEADER_INC	= 

MPSRC_FILES  	= 
MPLD_FILES	= 
MPCONF_FILES 	= mp.conf mp.conf.nv

MPLD_DIR_FILES 	= $(MPCONF_FILES:%=$(ROOT_MP_DIR)/%)

CFLAGS += -G -Kpic

all:		$(ROOT_MP_DIR) $(MPLD_FILES)

install:	all $(MPLD_DIRS) $(MPLD_DIR_FILES)

$(ROOT_MP_DIR) :
	$(INSDIR) $@

$(ROOT_MP_DIR)/% : 
	$(INS) $(@D) $(@F)

$(MPLD_FILES): $(MPSRC_FILES)
	$(CC) $(CFLAGS) -o $@ $(HEADER_INC) $(MPSRC_FILES)

clean:
	$(RM) $(MPLD_FILES) $(MPSRC_FILES:%.c=%.o) $(MPLD_DIR_FILES)
