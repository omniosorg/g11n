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
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
# @(#)Makefile 1.7     06/03/28 SMI
#

SRCS	= euc2utf_main.c \
	  utf2euc_main.c \
	  874_to_838_main.c \
	  838_to_874_main.c \
	  common_utf.c \
	  euc2utf_sub.c \
	  utf2euc_sub.c

dummy: all

include ../Makefile.asian

E2U	= eucTH%UTF-8.so
U2E	= UTF-8%eucTH.so
723	= IBM-874%IBM-838.so
327	= IBM-838%IBM-874.so

PROGS	= $(U2E) $(E2U) $(723) $(327)

install: all $(TARGETDIRS)
	for i in $(PROGS); do \
		$(INS) $(ICONV_DIR) -m 644 -u bin -g bin $$i; \
		$(INS) $(ICONV_DIR_64) -m 644 -u bin -g bin $(MACH64)/$$i; \
	done

all:	$(MACH64) $(PROGS)

#
# Library
#
$(E2U): euc_to_utf_main.o euc_to_utf_sub.o common_utf.o common_def.h
	$(CC) $(CFLAGS) -o $@ euc_to_utf_main.o euc_to_utf_sub.o common_utf.o
	cd $(MACH64); $(CC) $(CFLAGS_64) -o $@ euc_to_utf_main.o euc_to_utf_sub.o common_utf.o

$(U2E): utf_to_euc_main.o utf_to_euc_sub.o common_utf.o common_def.h
	$(CC) $(CFLAGS) -o $@ utf_to_euc_main.o utf_to_euc_sub.o common_utf.o
	cd $(MACH64); $(CC) $(CFLAGS_64) -o $@ utf_to_euc_main.o utf_to_euc_sub.o common_utf.o

$(723): 874_to_838_main.o 874_to_838_sub.o common_utf.o common_def.h
	$(CC) $(CFLAGS) -o $@ 874_to_838_main.o 874_to_838_sub.o common_utf.o
	cd $(MACH64); $(CC) $(CFLAGS_64) -o $@ 874_to_838_main.o 874_to_838_sub.o common_utf.o

$(327): 838_to_874_main.o 838_to_874_sub.o common_utf.o common_def.h
	$(CC) $(CFLAGS) -o $@ 838_to_874_main.o 838_to_874_sub.o common_utf.o
	cd $(MACH64); $(CC) $(CFLAGS_64) -o $@ 838_to_874_main.o 838_to_874_sub.o common_utf.o

