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
# @(#)Makefile	1.2 01/08/15 SMI
#
# This file is to generate supplementary bitmap font generators.
# The simple font generators are stand-alone tools for an internal use
# purpose of generating fonts.
#

TARGETS	= sample12 sample14 sample16 sample20 sample24
SRCS	= $(TARGETS:%=%.c)
BDFS	= splmt12.bdf splmt14.bdf splmt16.bdf splmt20.bdf splmt24.bdf
PCFS	= $(BDFS:%.bdf=%.pcf)

all install: $(TARGETS)

sample%: sample%.c
	$(CC) -xO3 $@.c -o $@

bdfs: $(BDFS)


splmt%.bdf: sample%
	/bin/rm -f $@
	./$? > $@

pcfs: $(PCFS)

splmt%.pcf: splmt%.bdf
	/bin/rm -f $@
	/usr/openwin/bin/bdftopcf < $? > $@

clean clobber:
	/bin/rm -f *.pcf *.bdf *.Z core *.s *.o $(TARGETS)
