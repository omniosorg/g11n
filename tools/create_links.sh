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
# a small script to recreate a full locale structure in the $SRC/proto dir by recreating the file permissions
# and links in the prototype files. 
#
# usage: ./create_links.sh <locale> <path_to_fileroot> 

ALL_LOCALES="ar en_US.UTF-8 iso_8859_1 iso_8859_13 iso_8859_15 iso_8859_2 iso_8859_5 iso_8859_7 iso_8859_9 common \
 ar_EG.UTF-8 ar_SA.UTF-8 bg_BG bg_BG.ISO8859-5 bg_BG.UTF-8 ca_ES.ISO8859-1 ca ca_ES  ca_ES.ISO8859-15 \
ca_ES.ISO8859-15@euro ca_ES.UTF-8 cs_CZ cs_CZ.ISO8859-2 cs_CZ.UTF-8 cs_CZ.UTF-8@euro cs da_DK.ISO8859-1 da da.ISO8859-15 da_DK \
da_DK.ISO8859-15 da_DK.ISO8859-15@euro da_DK.UTF-8 de_DE.ISO8859-1 de.UTF-8 de de.ISO8859-15 de_DE.UTF-8 de_AT de_AT.ISO8859-1 \
de_AT.ISO8859-15 de_AT.ISO8859-15@euro de_AT.UTF-8 de_CH de_CH.ISO8859-1 de_CH.UTF-8 de_DE  de_DE.ISO8859-15 de_DE.ISO8859-15@euro \
de_DE.UTF-8@euro de_LU.UTF-8 el_GR.ISO8859-7 el el.UTF-8 el_CY.UTF-8 el_GR  el_GR.ISO8859-7@euro el_GR.UTF-8 en.UTF-8 en_AU.ISO8859-1 \
en_AU  en_AU.UTF-8 en_CA en_CA.ISO8859-1 en_CA.UTF-8 en_GB.ISO8859-1 en_GB en_GB.ISO8859-15 en_GB.ISO8859-15@euro en_GB.UTF-8 \
en_IE.ISO8859-1 en_IE en_IE.ISO8859-15 en_IE.ISO8859-15@euro en_IE.UTF-8 en_MT.UTF-8 en_NZ.ISO8859-1 en_NZ en_NZ.UTF-8 en_US.ISO8859-1 \
en_US en_US.ISO8859-15 en_US.ISO8859-15@euro es_ES.ISO8859-1 es.UTF-8 es es.ISO8859-15 es_ES.UTF-8 es_AR es_AR.ISO8859-1 es_AR.UTF-8 \
es_BO.ISO8859-1 es_BO es_BO.UTF-8 es_CL.ISO8859-1 es_CL es_CL.UTF-8 es_CO.ISO8859-1 es_CO es_CO.UTF-8 es_CR.ISO8859-1 es_CR es_CR.UTF-8 \
es_EC.ISO8859-1 es_EC es_EC.UTF-8 es_ES es_ES.ISO8859-15 es_ES.ISO8859-15@euro es_ES.UTF-8@euro es_GT.ISO8859-1 es_GT es_GT.UTF-8 \
es_MX.ISO8859-1 es_MX es_MX.UTF-8 es_NI.ISO8859-1 es_NI es_NI.UTF-8 es_PA.ISO8859-1 es_PA es_PA.UTF-8 es_PE.ISO8859-1 es_PE \
es_PE.UTF-8 es_PY.ISO8859-1 es_PY es_PY.UTF-8 es_SV.ISO8859-1 es_SV es_SV.UTF-8 es_UY.ISO8859-1 es_UY es_UY.UTF-8 es_VE.ISO8859-1 \
es_VE es_VE.UTF-8 et_EE.ISO8859-15 et et_EE et_EE.UTF-8 fi_FI.ISO8859-1 fi fi.ISO8859-15 fi_FI fi_FI.ISO8859-15 fi_FI.ISO8859-15@euro \
fi_FI.UTF-8 fr_FR.ISO8859-1 fr.UTF-8 fr fr.ISO8859-15 fr_FR.UTF-8 fr_BE fr_BE.ISO8859-1 fr_BE.ISO8859-15 fr_BE.ISO8859-15@euro \
fr_BE.UTF-8 fr_BE.UTF-8@euro fr_CA fr_CA.ISO8859-1 fr_CA.UTF-8 fr_CH fr_CH.ISO8859-1 fr_CH.UTF-8 fr_FR fr_FR.ISO8859-15 \
fr_FR.ISO8859-15@euro fr_FR.UTF-8@euro fr_LU.UTF-8 he he_IL he_IL.UTF-8 hi_IN.UTF-8 hr_HR.ISO8859-2 hr_HR hr_HR.UTF-8 hu_HU.ISO8859-2 \
hu hu_HU hu_HU.UTF-8 is_IS.ISO8859-1 is_IS is_IS.UTF-8  \
it_IT.ISO8859-1 it_IT.ISO8859-15 it.UTF-8 it it.ISO8859-15 it_IT.UTF-8 it_IT it_IT.ISO8859-15@euro it_IT.UTF-8@euro ja_JP.PCK \
ja_JP.UTF-8 ja_JP.eucJP ja ko_KR.UTF-8 ko.UTF-8 ko_KR.EUC ko_KR.EUC@dict ko_KR.UTF-8@dict ko lt_LT.ISO8859-13 lt lt_LT lt_LT.UTF-8 \
lv_LV.ISO8859-13 lv lv_LV lv_LV.UTF-8 mk_MK.ISO8859-5 mk_MK mk_MK.UTF-8 mt_MT.UTF-8 nb_NO.UTF-8 nl.ISO8859-15 nl_NL.ISO8859-1 nl \
nl_BE.ISO8859-1 nl_BE nl_BE.ISO8859-15 nl_BE.ISO8859-15@euro nl_BE.UTF-8 nl_NL nl_NL.ISO8859-15 nl_NL.ISO8859-15@euro nl_NL.UTF-8 \
nn_NO.UTF-8 no no_NO no_NO.ISO8859-1@bokmal no_NO.ISO8859-1@nynorsk no_NY nr pl_PL.ISO8859-2 pl_PL.UTF-8 pl pl.UTF-8 pl_PL \
pt_PT.ISO8859-1 pt.ISO8859-15 pt pt_BR.ISO8859-1  pt_BR  pt_BR.UTF-8 pt_PT pt_PT.ISO8859-15 pt_PT.ISO8859-15@euro pt_PT.UTF-8 \
ro_RO.ISO8859-2 ro_RO ro_RO.UTF-8 ru_RU.ISO8859-5 ru_RU.UTF-8 ru ru.UTF-8 ru.koi8-r ru_RU ru_RU.ANSI1251 ru_RU.KOI8-R \
sh_BA.ISO8859-2@bosnia sh_BA.UTF-8 sh_BA sk_SK.ISO8859-2 sk_SK sk_SK.UTF-8 sl_SI.ISO8859-2 sl_SI.UTF-8 sl_SI sq_AL.ISO8859-2 \
sq_AL.UTF-8 sq_AL sr_CS.UTF-8 sr_CS sr_SP sr_YU.ISO8859-5 sr_YU sv_SE.ISO8859-1 sv.UTF-8 sv sv.ISO8859-15 sv_SE.UTF-8 sv_SE \
sv_SE.ISO8859-15 sv_SE.ISO8859-15@euro sv_SE.UTF-8@euro th_TH.ISO8859-11 th th_TH th_TH.TIS620 th_TH.UTF-8 tr_TR.ISO8859-9 tr \
tr_TR tr_TR.UTF-8 zh.GBK zh_CN.UTF-8 zh zh.UTF-8 zh_CN.EUC zh_CN.EUC@pinyin zh_CN.EUC@radical zh_CN.EUC@stroke zh_CN.GB18030 \
zh_CN.GB18030@pinyin zh_CN.GB18030@radical zh_CN.GB18030@stroke zh_CN.GBK zh_CN.GBK@pinyin \
zh_CN.GBK@radical zh_CN.GBK@stroke zh_CN.UTF-8@pinyin zh_CN.UTF-8@radical zh_CN.UTF-8@stroke zh_HK.BIG5HK \
zh_HK.BIG5HK@radical zh_HK.BIG5HK@stroke zh_HK.UTF-8 zh_HK.UTF-8@radical zh_HK.UTF-8@stroke zh_TW zh_TW.BIG5 \
zh_TW.BIG5@pinyin zh_TW.BIG5@radical zh_TW.BIG5@stroke zh_TW.BIG5@zhuyin zh_TW.EUC zh_TW.EUC@pinyin zh_TW.EUC@radical \
zh_TW.EUC@stroke zh_TW.EUC@zhuyin zh_TW.UTF-8 zh_TW.UTF-8@pinyin zh_TW.UTF-8@radical zh_TW.UTF-8@stroke zh_TW.UTF-8@zhuyin iconv" 

FILEROOT=$2

ARCH=`uname -p`



create_file()
{
	file=$1
	echo
	echo "line: $file"

	# if the file is a symlink it will be in as "file=link" rather than just "file "
	echo "$file" | /usr/xpg4/bin/grep -q "="
	if [ $? -eq 0 ];then
	   SPACE='$'
	   echo "symlink"
	   ##
	   ## use 'grep /$locale/' to avoid special case, e.g.:
	   ## $file=usr/lib/locale/es_MX/es_MX.so.3=../es_MX.ISO8859-1/es_MX.ISO8859-1.so.3
	   ##
	   echo "$file" | awk -F"=" '{print $2}' | /usr/xpg4/bin/grep /$locale/
	   if [ $? -eq 0 ] #if the link is on the left of the symlink then we don't want it
	   then
		echo "$locale is on left side of link - ignoring"
		# set the space to somethign you'll never actually see
		SPACE='&'
	   fi
	else
	    echo "regular file"
	    SPACE=' '
	fi 

	line=`egrep -h "$file${SPACE}" $SRC/pkgmaps/prototype.* | egrep -v "^#" | sort -u`
	echo "full line is $line"
	FILETYPE=`echo $line | awk '{print $1}'`

	if [ "$FILETYPE" != "s" ];then	
	    FILE=`echo $line | awk '{print $3}'`
	    PERMS=`echo $line | awk '{print $4}'`
	    OWNER=`echo $line | awk '{print $5}'`
	    GROUP=`echo $line | awk '{print $6}'`
	else
	    FILE=`echo $line | awk '{print $3}'| awk -F"=" '{print $1}'`
	    LINKTO=`echo $line | awk '{print $3}'| awk -F"=" '{print $2}'`
	fi

	echo "Filetype: $FILETYPE"
	echo "File: $FILE"

	DIR=`dirname $FILE`
	if [ ! -d $FILEROOT/$DIR ];then
	    install -d $FILEROOT/$DIR
	fi

	if [[ "$FILETYPE" == "d" || "$FILETYPE" == "f" ]];then	
	    #echo "Permissions: $PERMS"
	    #echo "UID: $OWNER"
	    #echo "GID: $GROUP"
	    #echo
	    echo "changing $FILEROOT/$FILE to $PERMS, $OWNER:$GROUP"
	    if [ "$FILETYPE" == "d" ];then
		if [ ! -d $FILEROOT/$FILE ];then
		    install -d $FILEROOT/$FILE
		fi
	    fi
	    chmod $PERMS $FILEROOT/$FILE
	    chown $OWNER:$GROUP $FILEROOT/$FILE
	elif [ "$FILETYPE" == "s" ];then
	    ##
	    ## check before creating the symbol links
	    ##
	    if [ -e $FILEROOT/$FILE ]; then
	        echo "Error: $FILEROOT/$FILE existed!"
	    else
	        #echo "Links to: $LINKTO"
	        LINKFROM=`basename $FILE`
	        echo "linking  $FILEROOT/$DIR/$LINKFROM to $LINKTO"
	        cd $FILEROOT/$DIR; rm -f $LINKFROM; ln -s $LINKTO $LINKFROM 2>/dev/null
	    fi
	else
	    echo
	   # do nothing - just ignoring the
	fi
}

if [ "$1" != "all" ]; then
    LOCALES=$1
else
    LOCALES=${ALL_LOCALES}
fi

# we don't want the 64-bit files from the "other" arch! i.e if we're on sparc, we don't care about amd64
if [ "$ARCH" == "sparc" ]; then
    NOARCH=amd64
else
    NOARCH=sparcv9
fi
echo "/$NOARCH/d" > $FILEROOT/sedscr

# We need to make sure that we have both openwin and usr/openwin available from $FILEROOT
if [ ! -L $FILEROOT/openwin ]; then
    ln -s $FILEROOT/usr/openwin $FILEROOT/openwin
fi

if [ ! -L $FILEROOT/lib ]; then
    ln -s $FILEROOT/usr/lib $FILEROOT/lib
fi

if [ ! -L $FILEROOT/share ]; then
    ln -s $FILEROOT/usr/share $FILEROOT/share
fi

if [ ! -L $FILEROOT/include ]; then
    ln -s $FILEROOT/usr/include $FILEROOT/include
fi

for locale in $LOCALES; do
    echo "---"
    echo " looking at $locale"
    echo "---"
    # very hacky way of getting the first symlink for the locale
    #LINK1=`egrep -h "/$locale[/ ]+" $SRC/pkgmaps/prototype.* | egrep "^s" | awk '{print $3}' | awk -F"=" '{print $1}' | head -1`
    #if [ -L $FILEROOT/$LINK1 ] # if the link is already there, then we don't have to go on
    #then
    #	echo "$locale already has links created"
    #else
	for i in `egrep -h "/$locale[/ ]+" $SRC/pkgmaps/prototype.* | egrep -v "^#" | awk '{print $3}' | sed -f $FILEROOT/sedscr `; do
	    create_file ${i}
    done
    #fi
done    

rm $FILEROOT/sedscr
