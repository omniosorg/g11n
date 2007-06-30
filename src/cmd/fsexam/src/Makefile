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

all:=	TARGET= all
install:=	TARGET= install

ALLOBJS=encoding.o \
       fsexam-conversion.o \
	 fsexam-history.o\
	 fsexam-preference.o \
	 fsexam-content.o \
	 fsexam-dnd.o     \
	 fsexam.o    \
	 fsexam-ui.o

prefix=$(FILEROOT)/usr/local
sysconfdir=$(prefix)/etc
datadir=$(prefix)/data
libdir=$(prefix)/lib

CPPFLAGS=-DSYSCONFDIR=\""$(sysconfdir)"\" -DDATADIR=\""$(datadir)"\" -DLIBDIR=\""$(libdir)"\" -DPREFIX=\""$(prefix)"\"

CFLAGS=

LDFLAGS= -lSM -lICE -lpopt -lthread -lz  -lsocket -lnsl -lm -lmlib

ALLPKGS= gnome-desktop-2.0 \
	 libgnomeui-2.0 \
	 libglade-2.0 \
	 libbonoboui-2.0 \
	 libgnome-2.0 \
	 gnome-vfs-2.0\
	 libbonobo-2.0 \
	 gconf-2.0 \
	 bonobo-activation-2.0\
	 ORBit-2.0\
	 gtk+-2.0\
	 glib-2.0\
	 libstartup-notification-1.0 \
	 eel-2.0 \
	 gail \
	 gnome-keyring-1\
	 libart-2.0 \
	 libxml-2.0

FSEXAM_BIN = 	fsexam
BINDIR = 		$(FILEROOT)/usr/bin

all:	$(ALLOBJS)
	$(CC) -o fsexam $(ALLOBJS)  $(LDFLAGS) `pkg-config --libs $(ALLPKGS)`

myins:
	$(INSDIR) $(BINDIR)
	$(INS) $(BINDIR) $(FSEXAM_BIN)

install: all myins

.c.o:
	$(CC)  -I. -I.. $(CPPFLAGS) `pkg-config --cflags $(ALLPKGS)` -g -c $<


clean:
	-rm -rf *.o fsexam
