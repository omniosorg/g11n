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
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
#ident "@(#)Makefile	1.0 08/08/05"
#

SRCS		=	tcvn%UCS-2.c \
            tcvn%UTF-8.c \
            tcvn%viscii.c \
            UCS-2%tcvn.c  \
            UCS-2%viscii.c  \
            UTF-8%tcvn.c  \
            UTF-8%viscii.c  \
            viscii%tcvn.c  \
            viscii%UCS-2.c  \
            viscii%UTF-8.c

dummy: all

tcvn%UCS-2LE.o: tcvn%UCS-2.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

tcvn%UCS-2BE.o: tcvn%UCS-2.c
	$(CC) $(CFLAGS) -c -o $@ $?
	$(CC) $(CFLAGS_64) -c -o $(MACH64)/$@ $?

viscii%UCS-2LE.o: viscii%UCS-2.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

viscii%UCS-2BE.o: viscii%UCS-2.c
	$(CC) $(CFLAGS) -c -o $@ $?
	$(CC) $(CFLAGS_64) -c -o $(MACH64)/$@ $?

UCS-2LE%tcvn.o: UCS-2%tcvn.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

UCS-2BE%tcvn.o: UCS-2%tcvn.c
	$(CC) $(CFLAGS) -c -o $@ $?
	$(CC) $(CFLAGS_64) -c -o $(MACH64)/$@ $?

UCS-2LE%viscii.o: UCS-2%viscii.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

UCS-2BE%viscii.o: UCS-2%viscii.c
	$(CC) $(CFLAGS) -c -o $@ $?
	$(CC) $(CFLAGS_64) -c -o $(MACH64)/$@ $?



include ../Makefile.asian

PROGS  = tcvn%UCS-2LE.so		tcvn%UCS-2BE.so
PROGS += viscii%UCS-2LE.so		viscii%UCS-2BE.so
PROGS += UCS-2LE%tcvn.so		UCS-2BE%tcvn.so
PROGS += UCS-2LE%viscii.so		UCS-2BE%viscii.so
PROGS += UTF-8%tcvn.so			tcvn%UTF-8.so
PROGS += UTF-8%viscii.so		viscii%UTF-8.so
PROGS += tcvn%viscii.so			viscii%tcvn.so


install: all $(TARGETDIRS)
	for i in $(PROGS); do \
		$(INS) $(ICONV_DIR) -m 644 -u bin -g bin $$i; \
		$(INS) $(ICONV_DIR_64) -m 644 -u bin -g bin $(MACH64)/$$i; \
	done

all: $(MACH64) $(PROGS)

