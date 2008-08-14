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
#
#ident "@(#)Makefile	1.0 06/03/28"
#

SRCS		=	UTF-8%zh_CN.iso2022-7.c \
			UTF-8%zh_CN.euc.c \
			UTF-8%zh_CN.gbk.c \
			UTF-8%zh_CN.iso2022-CN.c \
			zh_CN.euc%UTF-8.c \
			zh_CN.euc%zh_CN.iso2022-CN.c \
			zh_CN.euc%zh_TW-big5.c \
			zh_CN.gbk%UTF-8.c \
			zh_CN.iso2022-7%UTF-8.c \
			zh_CN.iso2022-CN%zh_CN.euc.c \
			zh_CN.iso2022-CN%UTF-8.c \
			HZ-GB-2312%UTF-8.c \
			HZ-GB-2312%zh_CN.euc.c \
			UTF-8%HZ-GB-2312.c \
			zh_CN.euc%HZ-GB-2312.c \
			zh_CN.gbk%HZ-GB-2312.c \
			zh_TW-big5%zh_CN.euc.c \
			UTF-8%zh_CN-cp935.c \
			zh_CN-cp935%UTF-8.c \
			zh_CN.gbk%zh_CN.iso2022-CN.c \
			zh_CN.gbk%zh_TW-big5.c \
			zh_CN.gbk%zh_TW-big5p.c \
			zh_CN.iso2022-CN%zh_CN.gbk.c \
			zh_TW-big5%zh_CN.gbk.c \
			zh_TW-big5p%zh_CN.gbk.c \
			zh_HK.hkscs%zh_CN.gbk.c \
			zh_CN.gbk%zh_HK.hkscs.c \
			zh_TW-euc%UTF-8.c \
			UTF-8%zh_TW-euc.c \
			zh_TW-iso2022-7%UTF-8.c \
			UTF-8%zh_TW-iso2022-7.c \
			zh_TW-big5%UTF-8.c \
			zh_HK.hkscs%UTF-8.c \
			UTF-8%zh_HK.hkscs.c \
			zh_HK.hkscs%zh_TW-big5p.c \
			zh_TW-big5p%zh_HK.hkscs.c \
			UTF-8%zh_TW-big5.c \
			zh_TW-big5p%UTF-8.c \
			UTF-8%zh_TW-big5p.c \
			UTF-8%zh_TW-cp937.c zh_TW-cp937%UTF-8.c \
			zh_TW-euc%zh_TW-big5.c \
			zh_TW-euc%zh_TW-iso2022-7.c \
			zh_TW-euc%zh_TW-iso2022-CN-EXT.c \
			zh_TW-big5%zh_TW-euc.c \
			zh_TW-big5%zh_TW-iso2022-7.c \
			zh_TW-iso2022-7%zh_TW-euc.c \
			zh_TW-iso2022-CN-EXT%zh_TW-euc.c \
			zh_TW-iso2022-7%zh_TW-big5.c \
			zh_TW-big5%zh_TW-iso2022-CN-EXT.c \
			zh_TW-iso2022-CN-EXT%zh_TW-big5.c

dummy: all

gb2312%UCS-2LE.o: zh_CN.euc%UTF-8.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

GBK%UCS-2LE.o: zh_CN.gbk%UTF-8.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

GBK%UCS-2BE.o: zh_CN.gbk%UTF-8.c
	$(CC) $(CFLAGS) -DUCS_2BE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2BE -c -o $(MACH64)/$@ $?

GBK%UCS-4LE.o: zh_CN.gbk%UTF-8.c
	$(CC) $(CFLAGS) -DUCS_4LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_4LE -c -o $(MACH64)/$@ $?

GBK%UCS-4BE.o: zh_CN.gbk%UTF-8.c
	$(CC) $(CFLAGS) -DUCS_4BE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_4BE -c -o $(MACH64)/$@ $?

UCS-2LE%gb2312.o: UTF-8%zh_CN.euc.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

UCS-2LE%GBK.o: UTF-8%zh_CN.gbk.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

UCS-2BE%GBK.o: UTF-8%zh_CN.gbk.c
	$(CC) $(CFLAGS) -DUCS_2BE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2BE -c -o $(MACH64)/$@ $?

UCS-4LE%GBK.o: UTF-8%zh_CN.gbk.c
	$(CC) $(CFLAGS) -DUCS_4LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_4LE -c -o $(MACH64)/$@ $?

UCS-4BE%GBK.o: UTF-8%zh_CN.gbk.c
	$(CC) $(CFLAGS) -DUCS_4BE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_4BE -c -o $(MACH64)/$@ $?

cns11643%UCS-2LE.o: zh_TW-euc%UTF-8.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

BIG5%UCS-2LE.o: zh_TW-big5%UTF-8.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

Big5-HKSCS%UCS-2LE.o: zh_HK.hkscs%UTF-8.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

UCS-2LE%cns11643.o: UTF-8%zh_TW-euc.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

UCS-2LE%BIG5.o: UTF-8%zh_TW-big5.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

UCS-2LE%Big5-HKSCS.o: UTF-8%zh_HK.hkscs.c
	$(CC) $(CFLAGS) -DUCS_2LE -c -o $@ $?
	$(CC) $(CFLAGS_64) -DUCS_2LE -c -o $(MACH64)/$@ $?

common_utf8.o: ../common/common_utf8.c
	$(CC) $(CFLAGS) -c -o $@ ../common/common_utf8.c
	$(CC) $(CFLAGS_64) -c -o $(MACH64)/$@ ../common/common_utf8.c

UCS-2LE%cns11643.so: UCS-2LE%cns11643.o common_utf8.o
	$(CC) $(CFLAGS) -o $@ UCS-2LE%cns11643.o common_utf8.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/UCS-2LE%cns11643.o $(MACH64)/common_utf8.o
	
UCS-2LE%BIG5.so: UCS-2LE%BIG5.o common_utf8.o
	$(CC) $(CFLAGS) -o $@ UCS-2LE%BIG5.o common_utf8.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/UCS-2LE%BIG5.o $(MACH64)/common_utf8.o
	
UCS-2LE%Big5-HKSCS.so: UCS-2LE%Big5-HKSCS.o common_utf8.o
	$(CC) $(CFLAGS) -o $@ UCS-2LE%Big5-HKSCS.o common_utf8.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/UCS-2LE%Big5-HKSCS.o $(MACH64)/common_utf8.o
	
UTF-8%zh_TW-euc.so: UTF-8%zh_TW-euc.o common_utf8.o
	$(CC) $(CFLAGS) -o $@ UTF-8%zh_TW-euc.o common_utf8.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/UTF-8%zh_TW-euc.o $(MACH64)/common_utf8.o

UTF-8%zh_TW-big5.so: UTF-8%zh_TW-big5.o common_utf8.o
	$(CC) $(CFLAGS) -o $@ UTF-8%zh_TW-big5.o common_utf8.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/UTF-8%zh_TW-big5.o $(MACH64)/common_utf8.o

UTF-8%zh_HK.hkscs.so: UTF-8%zh_HK.hkscs.o common_utf8.o
	$(CC) $(CFLAGS) -o $@ UTF-8%zh_HK.hkscs.o common_utf8.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/UTF-8%zh_HK.hkscs.o $(MACH64)/common_utf8.o

zh_TW-iso2022-CN-EXT.o: zh_TW-iso2022-CN-EXT.c
	$(CC) $(CFLAGS) -c -o $@ zh_TW-iso2022-CN-EXT.c
	$(CC) $(CFLAGS_64) -c -o $(MACH64)/$@ zh_TW-iso2022-CN-EXT.c

zh_TW-iso2022-CN-EXT%zh_TW-big5.so: zh_TW-iso2022-CN-EXT%zh_TW-big5.o zh_TW-iso2022-CN-EXT.o
	$(CC) $(CFLAGS) -o $@ zh_TW-iso2022-CN-EXT%zh_TW-big5.o zh_TW-iso2022-CN-EXT.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/zh_TW-iso2022-CN-EXT%zh_TW-big5.o $(MACH64)/zh_TW-iso2022-CN-EXT.o

zh_TW-iso2022-CN-EXT%zh_TW-euc.so: zh_TW-iso2022-CN-EXT%zh_TW-euc.o zh_TW-iso2022-CN-EXT.o
	$(CC) $(CFLAGS) -o $@ zh_TW-iso2022-CN-EXT%zh_TW-euc.o zh_TW-iso2022-CN-EXT.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/zh_TW-iso2022-CN-EXT%zh_TW-euc.o $(MACH64)/zh_TW-iso2022-CN-EXT.o

utf8%ibm.o: ../common/utf8%ibm.c
	$(CC) $(CFLAGS) -c -o $@ ../common/utf8%ibm.c
	$(CC) $(CFLAGS_64) -c -o $(MACH64)/$@ ../common/utf8%ibm.c

tab_lookup.o: ../common/tab_lookup.c
	$(CC) $(CFLAGS) -c -o $@ ../common/tab_lookup.c
	$(CC) $(CFLAGS_64) -c -o $(MACH64)/$@ ../common/tab_lookup.c

zh_TW-cp937%UTF-8.so: zh_TW-cp937%UTF-8.o utf8%ibm.o tab_lookup.o
	$(CC) $(CFLAGS) -o $@ zh_TW-cp937%UTF-8.o utf8%ibm.o tab_lookup.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/zh_TW-cp937%UTF-8.o $(MACH64)/utf8%ibm.o $(MACH64)/tab_lookup.o

UTF-8%zh_CN-cp935.so: UTF-8%zh_CN-cp935.o utf8%ibm.o tab_lookup.o
	$(CC) $(CFLAGS) -o $@ UTF-8%zh_CN-cp935.o utf8%ibm.o tab_lookup.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/UTF-8%zh_CN-cp935.o $(MACH64)/utf8%ibm.o $(MACH64)/tab_lookup.o

zh_CN-cp935%UTF-8.so: zh_CN-cp935%UTF-8.o utf8%ibm.o tab_lookup.o
	$(CC) $(CFLAGS) -o $@ zh_CN-cp935%UTF-8.o utf8%ibm.o tab_lookup.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/zh_CN-cp935%UTF-8.o $(MACH64)/utf8%ibm.o $(MACH64)/tab_lookup.o

UTF-8%zh_TW-cp937.so: UTF-8%zh_TW-cp937.o utf8%ibm.o tab_lookup.o
	$(CC) $(CFLAGS) -o $@ UTF-8%zh_TW-cp937.o utf8%ibm.o tab_lookup.o
	$(CC) $(CFLAGS_64) -o $(MACH64)/$@ $(MACH64)/UTF-8%zh_TW-cp937.o $(MACH64)/utf8%ibm.o $(MACH64)/tab_lookup.o

include ../Makefile.asian

PROGS  = Big5-HKSCS%UCS-2LE.so          zh_CN.gbk%zh_CN.iso2022-CN.so
PROGS += BIG5%UCS-2LE.so                zh_CN.gbk%zh_HK.hkscs.so
PROGS += cns11643%UCS-2LE.so            zh_CN.gbk%zh_TW-big5p.so
PROGS += gb2312%UCS-2LE.so              zh_CN.gbk%zh_TW-big5.so
PROGS += GBK%UCS-2LE.so                 zh_CN.iso2022-7%UTF-8.so
PROGS += HZ-GB-2312%UTF-8.so            zh_CN.iso2022-CN%UTF-8.so
PROGS += HZ-GB-2312%zh_CN.euc.so        zh_CN.iso2022-CN%zh_CN.euc.so
PROGS += UCS-2LE%Big5-HKSCS.so          zh_CN.iso2022-CN%zh_CN.gbk.so
PROGS += UCS-2LE%BIG5.so                zh_HK.hkscs%UTF-8.so
PROGS += UCS-2LE%cns11643.so            zh_HK.hkscs%zh_CN.gbk.so
PROGS += UCS-2LE%gb2312.so              zh_HK.hkscs%zh_TW-big5p.so
PROGS += UCS-2LE%GBK.so                 zh_TW-big5p%UTF-8.so
PROGS += UTF-8%HZ-GB-2312.so            zh_TW-big5p%zh_CN.gbk.so
PROGS += zh_TW-big5p%zh_HK.hkscs.so     zh_TW-big5%zh_TW-iso2022-7.so
PROGS += UTF-8%zh_CN.euc.so             zh_TW-big5%UTF-8.so
PROGS += UTF-8%zh_CN.gbk.so             zh_TW-big5%zh_CN.euc.so
PROGS += UTF-8%zh_CN.iso2022-7.so       zh_TW-big5%zh_CN.gbk.so
PROGS += UTF-8%zh_CN.iso2022-CN.so      zh_TW-big5%zh_TW-euc.so
PROGS += UTF-8%zh_TW-big5p.so           zh_TW-big5%zh_TW-iso2022-CN-EXT.so
PROGS += zh_TW-euc%UTF-8.so             zh_TW-euc%zh_TW-iso2022-7.so
PROGS += UTF-8%zh_TW-iso2022-7.so       zh_TW-euc%zh_TW-big5.so
PROGS += zh_CN.euc%HZ-GB-2312.so        zh_TW-euc%zh_TW-iso2022-CN-EXT.so
PROGS += zh_CN.euc%UTF-8.so             zh_TW-iso2022-7%UTF-8.so
PROGS += zh_CN.euc%zh_CN.iso2022-CN.so  zh_TW-iso2022-7%zh_TW-big5.so
PROGS += zh_CN.euc%zh_TW-big5.so        zh_TW-iso2022-7%zh_TW-euc.so
PROGS += zh_CN.gbk%HZ-GB-2312.so        zh_CN.gbk%UTF-8.so             
PROGS += zh_TW-cp937%UTF-8.so           UTF-8%zh_TW-cp937.so
PROGS += zh_CN-cp935%UTF-8.so           UTF-8%zh_CN-cp935.so                
PROGS += gb2312%UCS-2LE.so              UCS-2LE%gb2312.so 
PROGS += cns11643%UCS-2LE.so BIG5%UCS-2LE.so Big5-HKSCS%UCS-2LE.so
PROGS += UCS-2LE%cns11643.so UCS-2LE%BIG5.so UCS-2LE%Big5-HKSCS.so
PROGS += UTF-8%zh_TW-euc.so UTF-8%zh_TW-big5.so UTF-8%zh_HK.hkscs.so
PROGS += zh_TW-iso2022-CN-EXT%zh_TW-euc.so zh_TW-iso2022-CN-EXT%zh_TW-big5.so
PROGS += GBK%UCS-2LE.so GBK%UCS-2BE.so GBK%UCS-4LE.so GBK%UCS-4BE.so
PROGS += UCS-2LE%GBK.so UCS-2BE%GBK.so UCS-4LE%GBK.so UCS-4BE%GBK.so

install: all $(TARGETDIRS)
	for i in $(PROGS); do \
		$(INS) $(ICONV_DIR) -m 644 -u bin -g bin $$i; \
		$(INS) $(ICONV_DIR_64) -m 644 -u bin -g bin $(MACH64)/$$i; \
	done

all: $(MACH64) $(PROGS)

