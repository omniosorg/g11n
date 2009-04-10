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

LOCALES:sh = cd locale_variant; ls


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
JAVA_FLAGS	+= -Xmx256m

JARS		= jars/icu4j.jar jars/utilities.jar jars/xalan.jar jars/xml-apis.jar jars/cldr.jar

LOCALEDEF	= localedef -c -P /opt/SUNWspro/bin $(LDF_OPT) -W cc,"$(CFLAGS)" -L "$(LOPT_ORIGIN)"
LOCALEDEF64	= localedef -c -P /opt/SUNWspro/bin $(LDF_OPT64) -W cc,"$(CFLAGS64)" -L "$(LOPT_ORIGIN)"
GENERATE_POSIX	= $(JAVA_ROOT)/bin/java  $(JAVA_FLAGS) -cp $(JAVA_CP_SH:sh) -DCLDR_DTD_CACHE=dtd_cache org.unicode.cldr.posix.GeneratePOSIX -s cldr_core -d posix

JAVA_CP_SH	= echo $(JARS) | tr ' ' ':'


all: $(LOCALES:%=locale/%.so.3 locale64/%.so.3)

doc: diffs.html

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
	find extension/ -type l -exec rm -f {} \;
	rm -Rf .unpack_*

distclean: clean
	rm -Rf download



CHARMAP_SH	= echo $* | cut -d. -f2| cut -d@ -f1

# "... || [ $$? -eq 1 ]" - we ignore localedef warnings
locale/%.so.3: posix/%.src charmap/%.cm extension/%.x locale/.dir 
	cd locale; $(LOCALEDEF) -x ../extension/$*.x -f ../charmap/$*.cm -i ../posix/$*.src $* || [ $$? -eq 1 ]

locale64/%.so.3: posix/%.src charmap/%.cm extension/%.x locale64/.dir
	cd locale64; $(LOCALEDEF64) -x ../extension/$*.x -f ../charmap/$*.cm -i ../posix/$*.src $* || [ $$? -eq 1 ]



LOCALE_SH	= cat "locale_variant/$*"| grep -v '^\#' | grep -v '^[ \t]*$$' |head -n 1

posix/%.src:  $(JARS) cldr_core/.src posix/.dir dtd_cache/.dir locale_variant/%
	$(GENERATE_POSIX) -c $(CHARMAP_SH:sh) -m $(LOCALE_SH:sh)
	$(MV) "posix/$(LOCALE_SH:sh).$(CHARMAP_SH:sh).src" "$@"



extension/%.UTF-8.x: extension/UTF-8.x
	ln -s UTF-8.x $@

extension/%.ISO8859-1.x extension/%.ISO8859-2.x extension/%.ISO8859-5.x extension/%.ISO8859-7.x extension/%.ISO8859-9.x extension/%.ISO8859-13.x extension/%.ISO8859-15.x: extension/ISO8859.x
	ln -s ISO8859.x $@

extension/%.eucJP.x extension/%.ANSI1251.x extension/%.KOI8-R.x: extension/ISO8859.x
	ln -s ISO8859.x $@

# workaround of make(1S) known bug: "Filenames with the characters =, :, or @, do not work"
VARIANT_SH	= echo $* | cut -d@ -f2
WITHOUT_AT_SH	= echo $* | cut -d@ -f1
extension/%.x:
	$(MAKE) extension/$(WITHOUT_AT_SH:sh).x
	ln -s $(WITHOUT_AT_SH:sh).x $@

# Solaris does not support Unicode 5.1 yet so GENERATE_POSIX needs more params for the UTF-8 locales
#IS_UTF8_SH	= expr "$*" : ".*UTF-8" > /dev/null && echo " -u [\\\\u0000-\\\\uFFFF] -x [\\\\u0000-\\\\uFFFF]" || true
IS_UTF8_SH	= expr "$*" : ".*UTF-8" > /dev/null && echo " -u '[[\\\\u0000-\\\\U0010FFFF]-[[:Phoenician:][:Kharoshthi:][:Cuneiform:][\\\\U0001D200-\\\\U0001D24F]]]'" || true


.NO_PARALLEL: $(LOCALES:%=charmap/%.cm)
charmap/%.cm: $(JARS) charmap/.dir
	[ -f charmap/$(CHARMAP_SH:sh).cm ] || $(JAVA_ROOT)/bin/java $(JAVA_FLAGS) -cp $(JAVA_CP_SH:sh) org.unicode.cldr.posix.GenerateCharmap -d charmap -c $(CHARMAP_SH:sh) $(IS_UTF8_SH:sh)
	ln -s $(CHARMAP_SH:sh).cm $@



jars/cldr.jar: jars/icu4j.jar jars/utilities.jar jars/xalan.jar jars/xml-apis.jar cldr_tools/.src
	cd cldr_tools; ICU4J_JAR=../jars/icu4j.jar UTILITIES_JAR=../jars/utilities.jar CLDR_JAR=../jars/xalan.jar XML_APIS_JAR=../jars/xml-apis.jar ant jar
	cp cldr_tools/cldr.jar $@

jars/utilities.jar: cldr_tools/.src  jars/.dir
	cp cldr_tools/utilities.jar $@

jars/xalan.jar: xalan/.unpack jars/.dir
	cp xalan/xalan-j_2_7_1/xalan.jar $@

jars/xml-apis.jar: xalan/.unpack jars/.dir
	cp xalan/xalan-j_2_7_1/xml-apis.jar $@

jars/icu4j.jar: cldr_tools/.src jars/.dir
	cp cldr_tools/icu4j-3_8_1.jar $@

#jars/icu4j.jar: download/icu4j-4_0.jar jars/.dir
#	cp download/icu4j-4_0.jar $@



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
	@[ ! -d patches/$(@D) -o ! "$$(ls -A patches/$(@D))" ] || for a in patches/$(@D)/*.patch; do \
		echo "applying $$a"; \
		(cd $(@D); $(GNU_PATCH) -p1) < $$a; \
	done
	touch $@

cldr_core/.src: cldr_core/.unpack
	@[ ! -d patches/$(@D) -o ! "$$(ls -A patches/$(@D))" ] || for a in patches/$(@D)/*.patch; do \
		echo "applying $$a"; \
		(cd $(@D); $(GNU_PATCH) -p1) < $$a; \
	done
	touch $@



download/icu4j-4_0.jar: download/.dir
	$(WGET) -q -O $@ http://download.icu-project.org/files/icu4j/4.0/icu4j-4_0.jar
	touch $@

download/cldr_tools.zip: download/.dir
	$(WGET) -q -O $@ http://unicode.org/Public/cldr/1.6.1/tools.zip
	touch $@

download/cldr_core.zip: download/.dir
	$(WGET) -q -O $@ http://unicode.org/Public/cldr/1.6.1/core.zip
	touch $@

download/xalan-j_2_7_1-bin.zip: download/.dir
	$(WGET) -q -O $@ http://www.mirrorgeek.com/apache.org/xml/xalan-j/xalan-j_2_7_1-bin.zip
	touch $@



diffs.html: ldump tools/lcmp.pl
	tools/lcmp.pl $(LOCALES) >$@



# tools
ldump: tools/ldump.c cmap.dat
	# $(CC) -I . -o ldump tools/ldump.c - Sunstudio cc is not able to compile such big files
	gcc -I . -o ldump tools/ldump.c

cmap.dat: tools/cmap.pl $(LOCALES:%=locale/%.so.3) charmap/.dir
	tools/cmap.pl `find charmap/ -type f -name '*.cm'` > $@


%/.dir:
	rm -Rf $*; mkdir -p $*
	touch $@
