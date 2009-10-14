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
# Copyright (c) 2006, by Sun Microsystems, Inc.
# All rights reserved.
#

include $(SRC)/Makefile.master

install		:=	TARGET = install
all		:=	TARGET = all
lint		:=	TARGET = lint
clean		:=	TARGET = clean
clobber		:=	TARGET = clobber

LOCALE=	

TARGETDIR =	$(FILEROOT)/usr/lib/

LIB_SRC =	auto_ef_file.c auto_ef_str.c auto_ef_util.c auto_ef_methods.c

AUTOEF_SRC =	auto_ef.c

OBJS	= 	$(LIB_SRC:%.c=%.o)
OBJS64  = 	$(LIB_SRC:%.c=$(MACH64)/%.o)

BINOBJ = 	$(AUTOEF_SRC:%.c=%.o)

AUTOEF_SO =	libauto_ef.so.1

AUTOEF_BIN =	auto_ef

LINT_LIB = 	llib-lauto_ef.ln
LINT_SRC=	$(LINT_LIB:%.ln=%)

FILES =  	$(AUTOEF_SO) $(LINT_LIB) $(LINT_SRC)

SUBDIRS = hashtable spec

TARGETDIRS = 	$(TARGETDIR)
BINDIR =	$(FILEROOT)/usr/bin
INCDIR =	$(FILEROOT)/usr/include

CFLAGS =	-O  -K pic -xspace -Xa  -xildoff -errtags=yes -errwarn=%all -erroff=E_EMPTY_TRANSLATION_UNIT -erroff=E_STATEMENT_NOT_REACHED -xc99=%none -W0,-xc99=pragma  -g -xc99=%none -W0,-xc99=pragma -W0,-noglobal -DTEXT_DOMAIN=\"SUNW_JFP_LIB\" -D_TS_ERRNO -I. -DPIC -D_REENTRANT

CFLAGS_amd64 =	$(CFLAGS) -xO3 -m64 -Ui386 -U__i386  -erroff=E_INTEGRAL_CONST_EXP_EXPECTED -erroff=E_C90_NON_INTEGRAL_ARRAY_SIZE 
CFLAGS_sparcv9 = $(CFLAGS) -m64 -dalign -Dsparc

LDFLAGS =	-G -hlibauto_ef.so.1 -ztext -zdefs -zcombreloc -M spec/mapfile -lc -lelf -lm

LDFLAGS64=	-G -hlibauto_ef.so.1 -ztext -zdefs -zcombreloc -M spec/mapfile.$(MACH64) -lc -lelf -lm

LINTFLAGS =	-nvx -DTEXT_DOMAIN=\"SUNW_JFP_LIB\" -D_TS_ERRNO -I. -errtags=yes -s -erroff=E_PTRDIFF_OVERFLOW -erroff=E_ASSIGN_NARROW_CONV -U__PRAGMA_REDEFINE_EXTNAME -Xc99=%none -errsecurity=core -erroff=E_SEC_CREAT_WITHOUT_EXCL -erroff=E_SEC_FORBIDDEN_WARN_CREAT -erroff=E_ASSIGN_INT_TO_SMALL_INT -erroff=E_CAST_INT_CONST_TO_SMALL_INT -erroff=E_CAST_INT_TO_SMALL_INT -erroff=E_CAST_TO_PTR_FROM_INT -erroff=E_COMP_INT_WITH_LARGE_INT -erroff=E_INTEGRAL_CONST_EXP_EXPECTED -erroff=E_PASS_INT_TO_SMALL_INT -erroff=E_PTR_CONV_LOSES_BITS


all: $(SUBDIRS) $(TARGETDIRS) $(AUTOEF_SO) $(AUTOEF_BIN) $(LINT_LIB) 

include $(SRC)/src/Makefile.targ

install: $(SUBDIRS) $(FILES.obj) all $(TARGETDIR) $(BINDIR) $(INCDIR)
	$(INSDIR) $(TARGETDIR)/$(MACH64)
	$(INS) $(TARGETDIR)/$(MACH64) $(MACH64)/$(AUTOEF_SO)
	$(INS) $(BINDIR) $(AUTOEF_BIN)
	$(INS) $(INCDIR) auto_ef.h

clean: objclean $(SUBDIRS)
	$(RM)  *.o *.so $(AUTOEF_SO) $(AUTOEF_BIN) $(LINT_LIB) lint.out
	$(RM) -r $(TARGETDIR)/$(MACH64)
	$(RM) -r $(MACH64)

$(MACH64):
	$(INSDIR) $(MACH64)

$(AUTOEF_SO): $(MACH64) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
	$(CC) $(CFLAGS_$(MACH64)) $(LDFLAGS64) -o $(MACH64)/$@ $(OBJS64)
	/usr/bin/test -L $(@:%.1=%) || $(LN) -s $@ $(@:%.1=%)

$(AUTOEF_BIN): $(BINOBJ)
	$(CC) $(CFLAGS) -L. -lauto_ef -o $@ $? 

$(BINOBJ): $$(@:%.o=%.c)
	$(CC) -c -I. $@ $?

$(OBJS): $$(@:%.o=%.c)
	$(CC) $(CFLAGS) -c -o $@ $?
	$(CC) $(CFLAGS_$(MACH64)) -c -o $(MACH64)/$@ $*.?

$(LINT_LIB): $(LINT_SRC)
	$(LINT) $(LINTFLAGS) -o auto_ef $? > lint.out 2>&1 

$(TARGETDIR) $(BINDIR) $(INCDIR):
	$(INSDIR) $@

FRC:


