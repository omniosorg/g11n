# 
# CDDL HEADER START
# 
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
# 
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
# 
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
# 
# CDDL HEADER END
# 

# 
# Copyright 2008 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms.
# 

INCLUDES = $(FSEXAM_CFLAGS)	\
			-DFSEXAM_GLADE_DIR=\"$(gladedir)\" 	\
			-DFSEXAM_GLADE_FILE=\"fsexam.glade2\" 	\
			-DFSEXAM_ICON_FILE=\"fsexam-icon.png\" 	\
			-DFSEXAM_ICON_DIR=\"$(datadir)/pixmaps\"

AM_CPPFLAGS = -DSYSCONFDIR=\""$(sysconfdir)"\" 	\
			-DDATADIR=\""$(datadir)"\" 	\
			-DLIBDIR=\""$(libdir)"\" 	\
			-DPREFIX=\""$(prefix)"\"

bin_PROGRAMS = fsexam

IDL_FILE = GNOME_Fsexam.idl

IDL_SOURCES = \
			  GNOME_Fsexam-stubs.c \
			  GNOME_Fsexam-common.c \
			  GNOME_Fsexam-skels.c \
			  GNOME_Fsexam.h

$(IDL_SOURCES):	$(IDL_FILE)
	$(ORBIT_IDL) -I $(LIBBONOBO_IDL) -I $(BONOBO_ACTIVATION_IDL) $(IDL_FILE)

idl_DATA = $(IDL_FILE)
idldir = $(datadir)/idl


fsexam_LDADD = $(FSEXAM_LIBS) $(AUTOEF_LIBS)

FSEXAM_HEADER_FILES = 		\
				auto-detect.h	\
				callbacks.h		\
				encoding.h		\
				file-filter.h	\
				file-validate.h	\
				fsexam-application-server.h	\
				fsexam-convcontent.h	\
				fsexam-convname.h		\
				fsexam-debug.h			\
				fsexam-dialog.h			\
				fsexam-dnd.h			\
				fsexam-dryrun.h			\
				fsexam-encoding-dialog.h	\
				fsexam-error.h			\
				fsexam-header.h			\
				fsexam-helper.h			\
				fsexam-history.h		\
				fsexam-log.h			\
				fsexam-plaintext.h		\
				fsexam-pref.h			\
				fsexam-preference-dialog.h	\
				fsexam-setting.h		\
				fsexam-specialfile.h	\
				fsexam-tree.h			\
				fsexam-ui.h				\
				fsexam.h

fsexam_SOURCES =			\
				$(IDL_SOURCES) \
				fsexam-application-server.c \
				fsexam.c \
				encoding.c \
				fsexam-setting.c \
				fsexam-tree.c \
				fsexam-helper.c \
				fsexam-ui.c \
				fsexam-pref.c \
				fsexam-log.c \
				fsexam-dryrun.c \
				fsexam-specialfile.c \
				fsexam-plaintext.c \
				fsexam-history.c \
				fsexam-convname.c \
				fsexam-error.c \
				file-validate.c\
				file-filter.c\
				callbacks.c \
				auto-detect.c \
				fsexam-dialog.c	\
				fsexam-debug.c \
				fsexam-convcontent.c \
				fsexam-preference-dialog.c \
				fsexam-encoding-dialog.c  \
				fsexam-dnd.c

gladedir 	= $(pkgdatadir)/glade
glade_DATA 	= fsexam.glade2

EXTRA_DIST = \
			$(glade_DATA)	\
			$(server_in_files)	\
			$(IDL_FILE)		\
			$(FSEXAM_HEADER_FILES)

CLEANFILES = 					\
			$(IDL_SOURCES)	
