#!/bin/bash
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

locale=$1

FILEROOT=$2

cd $FILEROOT

PATTERN1=""

rm -f $locale.tar
rm -f $locale.tar.bz2
touch $locale.tar
echo "tarring up $locale"

for dir in `find . -name "$locale[._A-Z]*" -type d`
do
    /usr/bin/tar uvf $locale.tar -C $FILEROOT $dir
done	

# some legacy locales/new locales have some short names 
# we have to include as well.

case $locale in 
ar_EG)
    PATTERN1="ar"
    PATTERN2="";;
ca_ES)
    PATTERN1="ca"
    PATTERN2="";;
cs_CZ)
    PATTERN1="cz"
    PATTERN2="";;
de_DE)
    PATTERN1="de"
    PATTERN2="de.[A-Z]*";;
el_GR)
    PATTERN1="el"
    PATTERN2="el.[A-Z]*";;
es_ES)
    PATTERN1="es"
    PATTERN2="es.[A-Z]*";;
fr_FR)
    PATTERN1="fr"
    PATTERN2="fr.[A-Z]*";;
he_IL)
    PATTERN1="he"
    PATTERN2="";;
hu_HR)
    PATTERN1="hu"
    PATTERN2="";;
it_IT)
    PATTERN1="it"
    PATTERN2="it.[A-Z]*";;
ja_JP)
    PATTERN1="ja"
    PATTERN2="";;
ko_KR)
    PATTERN1="ko"
    PATTERN2="ko.[A-Z]*";;
lt_LV)
    PATTERN1="lt"
    PATTERN2="";;
lv_LV)
    PATTERN1="lv"
    PATTERN2="";;
pl_PL)
    PATTERN1="pl"
    PATTERN2="pl.[A-Z]*";;
ru_RU)
    PATTERN1="ru"
    PATTERN2="ru.[A-Za-z]*";;
sv_SE)
    PATTERN1="sv"
    PATTERN2="sv.[A-Z]*";;
th_TH)
    PATTERN1="th"
    PATTERN2="";;
tr_TR)
    PATTERN1="tr"
    PATTERN2="";;
zh_CN)
    PATTERN1="zh"
    PATTERN2="zh.[A-Z]*";;
esac

if [ "$PATTERN1" != "" ]
then
    for dir in `find . -name "$PATTERN1" -type d`
    do
	/usr/bin/tar uvf  $locale.tar -C $FILEROOT $dir
    done
    for dir in `find . -name "$PATTERN2" -type d`
    do
	/usr/bin/tar uvf $locale.tar -C $FILEROOT $dir
    done
fi

bzip2 -9 $locale.tar

if [ ! -f common.tar.bz2 ]
then
    /usr/bin/tar cvf common.tar -C $FILEROOT usr/openwin/lib/locale/en_US.UTF-8
    /usr/bin/tar uvf common.tar -C $FILEROOT usr/openwin/lib/locale/compose.dir
    /usr/bin/tar uvf common.tar -C $FILEROOT usr/openwin/lib/locale/locale.dir
    /usr/bin/tar uvf common.tar -C $FILEROOT usr/openwin/lib/locale/locale.alias
    for i in 1 2 4 5 7 8 9 13 15
    do
	/usr/bin/tar uvf common.tar -C $FILEROOT usr/openwin/lib/locale/iso_8859_$i
    done
    for i in 1 2 5 7 9 13 15
    do
	/usr/bin/tar uvf common.tar -C $FILEROOT usr/openwin/lib/locale/iso8859-$i
	/usr/bin/tar uvf common.tar -C $FILEROOT usr/lib/locale/iso_8859_$i
    done
    /usr/bin/tar uvf common.tar -C $FILEROOT usr/lib/locale/common
    bzip2 -9 common.tar
fi

