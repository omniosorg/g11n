#
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
#ident	"@(#)Makefile	1.1	04/06/16 SMI"
#

include	../../../Makefile.master

ROOT_PROG_DIR	= $(ROOT)/usr/bin
ROOT_PO_DIR	= $(ROOT)/usr/lib/locale/C/LC_MESSAGES

PROG		= idnconv
OBJS		= idnconv.o
SRCS		= $(OBJS:%.o=%.c)
HDRS		= idnconv.h

MSG_DOMAIN	= SUNW_I18N_CMD
PO		= $(MSG_DOMAIN).po

ROOT_PROG	= $(ROOT_PROG_DIR)/$(PROG)
ROOT_PO		= $(ROOT_PO_DIR)/$(PO)

CFLAGS		+= -I. -DTEXT_DOMAIN=\"$(MSG_DOMAIN)\"
LDFLAGS		+= -lidnkit

.PARALLEL: $(PROG) $(PO)


all: $(PROG) $(PO)


$(PROG): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)
	$(PROCESS_COMMENT) $@

$(PO): $(SRCS) $(HDRS)
	$(XGETTEXT) $(SRCS)

$(ROOT_PROG_DIR) $(ROOT_PO_DIR):
	$(INS.dir)

$(ROOT_PROG):
	$(RM) $@; $(INSTALL) -s -m 555 -u root -g bin -f $(@D) $(@F)

$(ROOT_PO):
	$(RM) $@; $(INSTALL) -s -m 644 -u root -g bin -f $(@D) $(@F)


clean:
	$(RM) $(OBJS) core *.s

clobber: clean
	$(RM) $(PROG) $(PO)

install: all $(ROOT_PROG_DIR) $(ROOT_PO_DIR) $(ROOT_PROG) $(ROOT_PO)
