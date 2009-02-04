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


include $(SRC)/Makefile.master

LOCALES = 

COMMON_CFLAGS	= $(CCVERBOSE) -K pic -D PIC -G -I. -z defs -z text -z ignore -D_REENTRANT

# escape ',' for localedef
sparc_CCUNBOUND		= -Wd\,-xsafe=unboundsym
sparc_SPACEFLAG		= -xspace -W0\,-Lt -W2\,-Rcond_elim
sparcv9_SPACEFLAG	= -xspace -W0\,-Lt -W2\,-Rcond_elim
CCREGSYM		= -Wc\,-Qiselect-regsym=0

CERRWARN	=
CFLAGS          += $(COMMON_CFLAGS) $(XREGSFLAG)
CFLAGS64        += $(COMMON_CFLAGS) $(XREGSFLAG64)

LDF_OPT		= 
LDF_OPT64	= -m lp64

LOPT_ORIGIN	= "-R\\\$$ORIGIN"

JAVA_FLAGS	= -Dhttp.proxyHost=webcache.sfbay -Dhttp.proxyPort=8080
JAVA_FLAGS	+= -Xms128m -Xmx128m

JARS		= jars/icu4j.jar jars/utilities.jar jars/xalan.jar jars/xml-apis.jar jars/cldr.jar
JAVA_CP		= echo $(JARS) | tr ' ' ':'

LOCALEDEF	= $(SRC)/tools/localedef.sh -P /opt/SUNWspro/bin $(LDF_OPT) -c -v -W cc,"$(CFLAGS)" -L "$(LOPT_ORIGIN)"
LOCALEDEF64	= $(SRC)/tools/localedef.sh -P /opt/SUNWspro/bin $(LDF_OPT64) -c -v -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)"
GENERATE_POSIX	= $(JAVA_ROOT)/bin/java  $(JAVA_FLAGS) -cp $(JAVA_CP:sh) -DCLDR_DTD_CACHE=dtd_cache org.unicode.cldr.posix.GeneratePOSIX -s cldr_core -d posix


all: $(LOCALES:%=locale/.%) diffs.html

install: all
	for l in $(LOCALES); do \
		$(INSDIR) $(FILEROOT)/usr/lib/locale/$$l/$(MACH64); \
		cp locale/$$l.so.3 $(FILEROOT)/usr/lib/locale/$$l; \
		cp locale64/$$l.so.3 $(FILEROOT)/usr/lib/locale/$$l/$(MACH64); \
	done

DIRS = charmap cldr_tools cldr_core locale locale64 posix xalan jars

clean:
	rm -Rf $(DIRS)
	rm -Rf cldr.jar icu4j.jar xalan.jar utilities.jar xml-apis.jar
	rm -Rf cmap.dat ldump diffs.html
	rm -Rf .unpack_*

distclean: clean
	rm -Rf download



locale/.%.UTF-8: charmap/UTF-8.cm extension/UTF-8.x posix/%.UTF-8.src locale/.dir locale64/.dir
	cd locale; $(LOCALEDEF) -x ../extension/UTF-8.x -f ../charmap/UTF-8.cm -i ../posix/$*.UTF-8.src $*.UTF-8
	cd locale64; $(LOCALEDEF64) -x ../extension/UTF-8.x -f ../charmap/UTF-8.cm -i ../posix/$*.UTF-8.src $*.UTF-8
	touch $@

locale/.%.ISO8859-1: charmap/ISO8859-1.cm extension/ISO8859.x posix/%.ISO8859-1.src locale/.dir locale64/.dir
	cd locale; $(LOCALEDEF) -x ../extension/ISO8859.x -f ../charmap/ISO8859-1.cm -i ../posix/$*.ISO8859-1.src $*.ISO8859-1
	cd locale64; $(LOCALEDEF64) -x ../extension/ISO8859.x -f ../charmap/ISO8859-1.cm -i ../posix/$*.ISO8859-1.src $*.ISO8859-1
	touch $@

locale/.%.ISO8859-2: charmap/ISO8859-2.cm extension/ISO8859.x posix/%.ISO8859-2.src locale/.dir locale64/.dir
	cd locale; $(LOCALEDEF) -x ../extension/ISO8859.x -f ../charmap/ISO8859-2.cm -i ../posix/$*.ISO8859-2.src $*.ISO8859-2
	cd locale64; $(LOCALEDEF64) -x ../extension/ISO8859.x -f ../charmap/ISO8859-2.cm -i ../posix/$*.ISO8859-2.src $*.ISO8859-2
	touch $@



charmap/%.cm: $(JARS) charmap/.dir
	$(JAVA_ROOT)/bin/java  $(JAVA_FLAGS) -cp $(JAVA_CP:sh) org.unicode.cldr.posix.GenerateCharmap -d charmap -c $*



posix/%.UTF-8.src: $(JARS) charmap/UTF-8.cm cldr_core/.src posix/.dir dtd_cache/.dir
	$(GENERATE_POSIX) -c UTF-8 -m $*@platform=solaris,yesno=short -u [\\\\u0000-\\\\uFFFF] -x [\\\\u0000-\\\\uFFFF] 
	mv posix/$*@platform=solaris,yesno=short.UTF-8.src $@

posix/%.ISO8859-1.src: $(JARS) charmap/ISO8859-1.cm cldr_core/.src posix/.dir dtd_cache/.dir
	$(GENERATE_POSIX) -c ISO8859-1 -m $*@platform=solaris,yesno=short 
	mv posix/$*@platform=solaris,yesno=short.ISO8859-1.src $@

posix/%.ISO8859-2.src: $(JARS) charmap/ISO8859-2.cm cldr_core/.src posix/.dir dtd_cache/.dir
	$(GENERATE_POSIX) -c ISO8859-2 -m $*@platform=solaris,yesno=short 
	mv posix/$*@platform=solaris,yesno=short.ISO8859-2.src $@



jars/cldr.jar: jars/icu4j.jar jars/utilities.jar jars/xalan.jar jars/xml-apis.jar cldr_tools/.src
	cd cldr_tools; ICU4J_JAR=../jars/icu4j.jar UTILITIES_JAR=../jars/utilities.jar CLDR_JAR=../jars/xalan.jar XML_APIS_JAR=../xml-apis.jar ant jar
	cp cldr_tools/cldr.jar $@

jars/utilities.jar: cldr_tools/.src  jars/.dir
	cp cldr_tools/utilities.jar $@

jars/xalan.jar: xalan/.unpack jars/.dir
	cp xalan/xalan-j_2_7_1/xalan.jar $@

jars/xml-apis.jar: xalan/.unpack jars/.dir
	cp xalan/xalan-j_2_7_1/xml-apis.jar $@

jars/icu4j.jar: download/icu4j-4_0.jar jars/.dir
	cp download/icu4j-4_0.jar $@



cldr_core/.unpack: download/cldr_core.zip cldr_core/.dir
	unzip -q download/cldr_core.zip -d cldr_core
	touch $@

cldr_tools/.unpack: download/cldr_tools.zip cldr_tools/.dir
	unzip -q download/cldr_tools.zip -d cldr_tools
	touch $@

xalan/.unpack: download/xalan-j_2_7_1-bin.zip xalan/.dir
	unzip -q download/xalan-j_2_7_1-bin.zip -d xalan
	touch $@

cldr_tools/.src: cldr_tools/.unpack
	@[ ! "$$(ls -A patches/cldr_tools $(@D))" ] || for a in patches/$(@D)/*.patch; do \
		echo "applying $$a"; \
		(cd $(@D); patch -p1) < $$a; \
	done
	touch $@

cldr_core/.src: cldr_core/.unpack
	@[ ! "$$(ls -A patches/$(@D))" ] || for a in patches/$(@D)/*.patch; do \
		echo "applying $$a"; \
		(cd $(@D); patch -p1) < $$a; \
	done
	touch $@



download/icu4j-4_0.jar: download/.dir
	wget -q -O $@ http://download.icu-project.org/files/icu4j/4.0/icu4j-4_0.jar
	touch $@

download/cldr_tools.zip: download/.dir
	wget -q -O $@ http://unicode.org/Public/cldr/1.6.1/tools.zip
	touch $@

download/cldr_core.zip: download/.dir
	wget -q -O $@ http://unicode.org/Public/cldr/1.6.1/core.zip
	touch $@

download/xalan-j_2_7_1-bin.zip: download/.dir
	wget -q -O $@ http://www.mirrorgeek.com/apache.org/xml/xalan-j/xalan-j_2_7_1-bin.zip
	touch $@



diffs.html: ldump tools/lcmp.pl
	tools/lcmp.pl $(LOCALES) >$@



# tools
ldump: tools/ldump.c cmap.dat
	$(CC) -I . -o ldump tools/ldump.c

cmap.dat: tools/cmap.pl $(LOCALES:%=locale/.%)
	tools/cmap.pl charmap/*.cm > $@



%/.dir:
	rm -Rf $*; mkdir -p $*
	touch $@
