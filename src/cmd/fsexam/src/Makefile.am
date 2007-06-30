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

INCLUDES = $(FSEXAM_CFLAGS)

CPPFLAGS = -DSYSCONFDIR=\""$(sysconfdir)"\" -DDATADIR=\""$(datadir)"\" -DLIBDIR=\""$(libdir)"\" -DPREFIX=\""$(prefix)"\"

bin_PROGRAMS = fsexam

fsexam_LDADD = $(FSEXAM_LIBS)
fsexam_SOURCES = fsexam.c encoding.c \
			fsexam-preference.c \
			fsexam-conversion.c \
			fsexam-content.c \
			fsexam-ui.c \
			fsexam-history.c \
			fsexam-dnd.c
