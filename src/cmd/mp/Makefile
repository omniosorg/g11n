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
LOCALE=	

TARGETDIR =	$(FILEROOT)/usr/lib/lp/locale/C/mp

MP_TARG =	$(FILEROOT)/usr/bin

MAN_TARG = 	$(FILEROOT)/usr/share/man/man1/

MP_SRC 	=	header.c io.c main.c misc.c print.c ctl_processing.c print_preprocess.c \
		configuration.c tp1.c pcf.c ttf.c xpr_client.c xpr_prolog_scan.c xpr_print_line.c \
		dict.c

OBJS	= 	$(MP_SRC:%.c=%.o)

MP	= 	mp

MAILP	=	mailp

PSFILES	=	mp.common.ps mp.pro.alt.ps mp.pro.altl.ps mp.pro.ff.ps mp.pro.fp.ps mp.pro.l.ps \
		mp.pro.ll.ps mp.pro.ps mp.pro.tm.ps mp.pro.ts.ps

XPRFILES =	mp.pro.alt.xpr mp.pro.altl.xpr mp.pro.ff.xpr mp.pro.fp.xpr mp.pro.l.xpr mp.pro.ll.xpr \
		mp.pro.tm.xpr mp.pro.ts.xpr mp.pro.xpr

MANPAGES =	digestp.1 filep.1 filofaxp.1 franklinp.1 mailp.1 mp.1 newsp.1 timemanp.1 timesysp.1

FILES =  $(PSFILES) $(XPRFILES) 

SUBDIRS = cnvobj/en_US.UTF-8 conf locale

TARGETDIRS = $(TARGETDIR)

all: all_local $(SUBDIRS)

all_local: $(TARGETDIRS) $(MP)
	$(INSDIR) $(MP_TARG)
	$(INSDIR) $(MAN_TARG)

include $(SRC)/src/Makefile.targ

CFLAGS		= -O -xildoff -xc99=%none -Icnvobj -DPROLOGUE=\"/usr/lib/lp/locale/C/mp\"  -DSVR4 -DSYSV -DKYBDACC \
		-DFILECHOOSER -DOW_I18N -I../../libdeskset -I/usr/openwin/include -I/include

MPFLAGS 	=  $(CFLAGS) -i -R/usr/openwin/lib -L../../libdeskset -L/usr/openwin/lib -L/lib

LDFLAGS		=  -lw -ldl -llayout -zlazyload -lXt -lX -lXp -znolazyload

install: install_local $(SUBDIRS)

install_local: all_local $(FILES.obj)
	$(INSDIR) $(TARGETDIR)/
	$(INS) $(MP_TARG) $(MP)
	$(INS) $(MP_TARG) $(MAILP)
	for man in $(MANPAGES) ; do \
		$(INS) $(MAN_TARG) $$man ; \
	done

clean: objclean $(SUBDIRS)
	$(RM) -r $(MP_TARG)/$(MP)
	$(RM) $(MP) *.o

$(MP): $(OBJS)
	$(CC) $(MPFLAGS) $(OBJS) $(LDFLAGS) -o $@

$(OBJS): $$(@:%.o=%.c)
	$(CC) -c $(CFLAGS) -o $@ $<

FRC:


