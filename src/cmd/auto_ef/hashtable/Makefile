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

TARGETDIR =	$(FILEROOT)/usr/lib/auto_ef/

DATA_CREATE =	data_create

D_CREATE_OBJ = 	$(DATA_CREATE:%=%.o)

D_CREATE_SRC = 	$(DATA_CREATE:%=%.c)

HASHTABLE= 	hashtable.CP1256_Arabia hashtable.8859-6_Arabia \
		hashtable.CP1251_Bulgaria hashtable.8859-5_Bulgaria \
		hashtable.koi8-r_Bulgaria hashtable.CP1252_Catalonia \
		hashtable.8859-1_Catalonia hashtable.CP1250_Croatia \
		hashtable.8859-2_Croatia hashtable.CP1252_Denmark \
		hashtable.8859-1_Denmark hashtable.CP1252_Finland \
		hashtable.8859-1_Finland hashtable.CP1252_France \
		hashtable.8859-1_France hashtable.CP1252_Germany \
		hashtable.8859-1_Germany hashtable.CP1253_Greece \
		hashtable.8859-7_Greece hashtable.CP1255_Hebrew \
		hashtable.8859-8_Hebrew hashtable.CP1250_Hungary \
		hashtable.8859-2_Hungary hashtable.CP1252_Iceland \
		hashtable.8859-1_Iceland hashtable.CP1252_Italy \
		hashtable.8859-1_Italy hashtable.CP1252_Netherland \
		hashtable.8859-1_Netherland hashtable.CP1252_Norway \
		hashtable.8859-1_Norway hashtable.CP1252_Portugal \
		hashtable.8859-1_Portugal hashtable.CP1250_Poland \
		hashtable.8859-2_Poland hashtable.CP1251_Russia \
		hashtable.8859-5_Russia hashtable.koi8-r_Russia \
		hashtable.CP1250_Serbia hashtable.8859-2_Serbia \
		hashtable.CP1250_Slovakia hashtable.8859-2_Slovakia \
		hashtable.CP1250_Slovenia \
		hashtable.8859-2_Slovenia hashtable.CP1252_Spain \
		hashtable.8859-1_Spain hashtable.CP1252_Sweden \
		hashtable.8859-1_Sweden hashtable.zh_HK.hkscs \
		hashtable.zh_TW-euc hashtable.zh_CN.euc hashtable.ko_KR.euc \
		hashtable.eucJP hashtable.PCK hashtable.ko_KR.cp949 \
		hashtable.zh_CN.gb18030 \
		hashtable.CP874_Thai hashtable.TIS620.2533_Thai

HASH_SRC =	$(HASHTABLE:%=FILELIST/%)

LINT_SRC=	$(LINT_LIB:%.ln=%)

FILES =  	$(HASHTABLE)

SUBDIRS = 	

TARGETDIRS = 	$(TARGETDIR)

CFLAGS =	-O -xspace -Xa  -xildoff -errtags=yes -errwarn=%all -erroff=E_EMPTY_TRANSLATION_UNIT -erroff=E_STATEMENT_NOT_REACHED -xc99=%none -W0,-xc99=pragma   -I..  -lm  -DTEXT_DOMAIN=\"SUNW_JFP_CMD\" 

all: $(TARGETDIRS) $(DATA_CREATE) $(HASHTABLE)
	$(INSDIR) $(TARGETDIR)/

include $(SRC)/src/Makefile.targ

install: $(SUBDIRS) all $(FILES.obj)

clean: objclean $(SUBDIRS)
	$(RM) $(DATA_CREATE) $(D_CREATE_OBJ)
	$(RM) $(HASHTABLE)
	$(RM) -r $(TARGETDIR)


$(DATA_CREATE): $$(@:%=%.o)
	$(CC) $(CFLAGS) -o $@ $?

$(D_CREATE_OBJ): $$(@:%.o=%.c)
	$(CC) $(CFLAGS) -c $?

$(HASHTABLE): FILELIST/$$@
	./$(DATA_CREATE) $? > $@

FRC:


