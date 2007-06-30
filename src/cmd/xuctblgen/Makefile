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
#ident	"@(#)Makefile	1.0	06/03/07	SMI"
#

include $(SRC)/Makefile.master

install		:=	TARGET = install
all		:=	TARGET = all
lint		:=	TARGET = lint
clean		:=	TARGET = clean
clobber		:=	TARGET = clobber

#LOCALE:sh = basename `pwd`
LOCALE=	en_US.UTF-8

TARGETDIR =	$(FILEROOT)/usr/lib/locale/common

SRCS = 		tblgen.c cstream.c range_tbl.c conv_info.c lookup_tbl.c

OBJS	= 	$(SRCS:%.c=%.o)

XUC_BIN	= 	xuctblgen

FILES =  

SUBDIRS = 

TARGETDIRS = $(TARGETDIR)

all: $(TARGETDIRS) $(XUC_BIN) $(SUBDIRS)

include $(SRC)/src/Makefile.targ

CFLAGS		=  -O -Xa -I$(SRC)/src/openwin/include -I/usr/openwin/include -Dsun -DSVR4 -DVERBOSE -DXLOCALEDIR 

LDFLAGS		= -L/usr/openwin/lib -lX11 -ldl -lintl    -lsocket -lnsl

install: all
	$(INS) $(TARGETDIR) $(XUC_BIN)

clean: objclean $(SUBDIRS)
	$(RM) *.o 
	$(RM) $(XUC_BIN)

$(XUC_BIN): $(OBJS)
	LD_RUN_PATH=/usr/openwin/lib $(CC) -o $@ -O -Xa $(OBJS) $(LDFLAGS)

$(OBJS): $$(@:%.o=%.c)
	$(CC) $(CFLAGS) -c -o $@ $*.c

FRC:


