#!/usr/bin/perl
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

use strict;
use warnings;

my $versionname = $ARGV[0];
chomp $versionname;

# Open the temp file to retrieve region data from.

open (TEMPFILE, "$versionname.tmp") or die ("Couldn't open $versionname.tmp");
my @localedata = <TEMPFILE>;
close (TEMPFILE);

# Open the Config file $versionname.bak that will be used to write the data to

open (CONFIGFILE, "> $versionname.bak") or die ("Couldn't open $versionname.bak");

# Sort the array, first sorted aplhabetically, then sort by region
# Result is array of region entries, sorted by region, and each regions entries
# are sorted alphabetically 

@localedata = sort(@localedata);
@localedata = map {$_->[0]}
	sort { $a->[1] cmp $b->[1] }
	map {[$_,/(\s+\w+)/, uc($_)]} @localedata;

# Next generate the Config file in the exact same format as the original hardcoded version

open (ORIGINAL, "versionname") or die ("Couldn't open $versionname");
while (<ORIGINAL>) {
  if ($_ =~ /Config file created/) {
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
    printf CONFIGFILE "#                  Config File created %02d-%02d-%4d %02d:%02d:%02d\n",$mday,$mon+1,$year+1900,$hour,$min,$sec;
    print CONFIGFILE "#\n\n\n";
  } 
  elsif ($_ =~ /^#/) {
    print CONFIGFILE; 
  }
  elsif ($_ =~ /locname/) {
    print CONFIGFILE;  
    my $region = $_;
    $region =~ s/locname\s[^\s]*\s([^\s]*)\s.*/$1/;
# Search each line of @localedata, if it contains an entry containing the aua region
# print to config file
     foreach my $line (@localedata) {
        if ($line =~ /\s$region\s/) {
           print CONFIGFILE $line;
        }    
     }
  }

}
close(ORIGINAL);


## First print header and date created
# 
#print CONFIGFILE "#                             LOCALE CONFIG FILE\n";
#print CONFIGFILE "#                             ==================\n";
#print CONFIGFILE "#\n";
#($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
#printf CONFIGFILE "#                  Config File created %02d-%02d-%4d %02d:%02d:%02d\n",$mday,$mon+1,$year+1900,$hour,$min,$sec;
#print CONFIGFILE "#\n\n\n";
#
## Print aua region header data
#
#print CONFIGFILE "#\n";
#print CONFIGFILE "#                      Solaris 10, Australasia partial locales \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "#                 ( en_AU, en_AU.ISO8859-1, en_NZ, en_NZ.ISO8859-1 )\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "# loc name field is of format:\n";
#print CONFIGFILE "# loc name <region name for locales_installed file> <pkg region name> <locale name>[,<locale name>...]\n\n";
#print CONFIGFILE "locname Ausi aua en_AU en_NZ\n\n";
#
## Search each line of @localedata, if it contains an entry containing the aua region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\saua\s/) {
#		print CONFIGFILE $line;
#	}    
#}
#
## Print cam region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                   Solaris 10, Central America partial locales \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "# ( es_CR, es_CR.ISO8859-1, es_GT, es_GT.ISO8859-1, es_MX, es_MX.ISO8859-1,\n";
#print CONFIGFILE "#  es_NI, es_NI.ISO8859-1, es_PA, es_PA.ISO8859-1, es_SV, es_SV.ISO8859-1 )|n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname C_America cam es_CR es_GT es_NI es_PA es_SV es\n\n";
#
## Search each line of @localedata, if it contains an entry containing the cam region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\scam\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print ceu region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                Solaris 10, Central Europe partial locales \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "# ( cs_CZ, cs_CZ.ISO8859-2, cz, de, de_AT, de_AT.ISO8859-1, de_AT.ISO8859-15,\n";
#print CONFIGFILE "#  de_AT.ISO8859-15@euro, de_CH, de_CH.ISO8859-1, de_DE, de_DE.ISO8859-1,\n";
#print CONFIGFILE "#  de_DE.ISO8859-15, de_DE.ISO8859-15@euro, de_DE.UTF-8, de_DE.UTF-8@euro,\n";
#print CONFIGFILE "#  de.ISO8859-15, de.UTF-8, fr_CH, fr_CH.ISO8859-1, hu, hu_HU, hu_HU.ISO8859-2,\n";
#print CONFIGFILE "#  pl, pl_PL, pl_PL.ISO8859-2, sk_SK, sk_SK.ISO8859-2, pl_PL.UTF-8 )";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname C_Europe ceu cs_CZ de_AT de_CH de_DE fr_CH hu_HU pl_PL sk_SK de fr\n\n";
#
## Search each line of @localedata, if it contains an entry containing the ceu region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\sceu\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print eeu region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#               Solaris 10, Eastern Europe partial locales \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "# ( bg_BG, bg_BG.ISO8859-5,  et_EE, et_EE.ISO8859-15, hr_HR, hr_HR.ISO8859-2,\n";
#print CONFIGFILE "#  lt_LT, lt_LT.ISO8859-13,  lv_LV, lv_LV.ISO8859-13, mk_MK,\n";
#print CONFIGFILE "#  mk_MK.ISO8859-5, ro_RO, ro_RO.ISO8859-2,  ru.koi8-r, ru_RU,\n";
#print CONFIGFILE "#  ru_RU.ANSI1251, ru_RU.ISO8859-5, ru_RU.KOI8-R, sh_BA, sh_BA.ISO8859-2@bosnia,\n";
#print CONFIGFILE "#  sl_SI, sl_SI.ISO8859-2, sq_AL, sq_AL.ISO8859-2, sr_SP, sr_YU, sr_YU.ISO8859-5,\n";
#print CONFIGFILE "#  tr_TR, tr_TR.ISO8859-9, ru_RU.UTF-8, tr_TR.UTF-8 )\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname E_Europe eeu bg_BG et_EE hr_HR lt_LT lv_LV mk_MK ro_RO ru_RU sh_BA sl_SI sq_AL sr_CS sr_SP sr_YU tr_TR kk_KZ uk_UA\n\n";
#
## Search each line of @localedata, if it contains an entry containing the eeu region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\seeu\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print hi_in region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#               Solaris 10, Hindi (Unicode) Locale\n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "#                     ( hi_IN.UTF-8)";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname Asia hi_in  hi_IN.UTF-8\n\n";
#
## Search each line of @localedata, if it contains an entry containing the hi_in region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\shi_in\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print ja region header data
#
#print CONFIGFILE "\n";
#print CONFIGFILE "#                         Solaris 10, Japanese Locale\n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "#         ( ja, ja_JP.eucJP, ja_JP.EUC, ja_JP.PCK, ja_JP.UTF-8)\n";
#print CONFIGFILE "\n";
#print CONFIGFILE "locname Asia ja ja ja_JP.PCK ja_JP.UTF-8\n\n";
#
## Search each line of @localedata, if it contains an entry containing the ja region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\sja\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print korean region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                           Solaris 10, Korean Locale\n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "#                   ( ko, ko_KR.EUC, ko.UTF-8, ko_KR.UTF-8 )\n";
#print CONFIGFILE "\n";
#print CONFIGFILE "locname Asia korean ko ko_KR ko.UTF-8\n\n";
#
## Search each line of @localedata, if it contains an entry containing the korean region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\skorean\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print mea region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                      Solaris 10, Middle East partial locales \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "#                            ( he, he_IL.UTF-8 )\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname M_East mea he_IL he ar_SA\n\n";
#
## Search each line of @localedata, if it contains an entry containing the mea region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\smea\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print naf region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                      Solaris 10, Northern Africa partial locales \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "#                            ( ar, ar_EG.UTF-8 )\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname N_Africa naf ar_EG ar\n\n";
#
## Search each line of @localedata, if it contains an entry containing the naf region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\snaf\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print nam region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                 Solaris 10, Northern America partial locales \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "# ( en_CA, en_CA.ISO8859-1, en_US, en_US.ISO8859-1, en_US.ISO8859-15,\n";
#print CONFIGFILE "#  en_US.ISO8859-15@euro, fr_CA, fr_CA.ISO8859-1 )\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname N_America nam en_CA en_US es_MX fr_CA en_US.UTF-8 es fr\n\n";
#
## Search each line of @localedata, if it contains an entry containing the nam region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\snam\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print neu region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                 Solaris 10, Northern Europe partial locales \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "# ( da_DK, da_DK.ISO8859-1, da_DK.ISO8859-15, da_DK.ISO8859-15@euro,\n";
#print CONFIGFILE "#  da.ISO8859-15, fi_FI, fi_FI.ISO8859-1, fi_FI.ISO8859-15,\n";
#print CONFIGFILE "#  fi_FI.ISO8859-15@euro, fi.ISO8859-15, is_IS, is_IS.ISO8859-1, no_NO,\n";
#print CONFIGFILE "#  no_NO.ISO8859-1@bokmal, no_NO.ISO8859-1@nynorsk, no_NY, sv.ISO8859-15,\n";
#print CONFIGFILE "#  sv_SE, sv_SE.ISO8859-1, sv_SE.ISO8859-15, sv_SE.ISO8859-15@euro, sv_SE.UTF-8,\n";
#print CONFIGFILE "#  sv_SE.UTF-8@euro, sv.UTF-8, fi_FI.UTF-8 )\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname N_Europe neu da_DK fi_FI is_IS nb_NO nn_NO no_NO sv_SE sv\n\n";
#
## Search each line of @localedata, if it contains an entry containing the neu region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\sneu\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print sam region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                  Solaris 10, Southern America partial locales\n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "# ( es_AR, es_AR.ISO8859-1, es_BO, es_BO.ISO8859-1, es_CL, es_CL.ISO8859-1,\n";
#print CONFIGFILE "#  es_CO, es_CO.ISO8859-1, es_EC, es_EC.ISO8859-1, es_PE, es_PE.ISO8859-1,\n";
#print CONFIGFILE "#  es_PY, es_PY.ISO8859-1, es_UY, es_UY.ISO8859-1, es_VE, es_VE.ISO8859-1,\n";
#print CONFIGFILE "#  pt_BR, pt_BR.ISO8859-1, pt_BR.UTF-8 )\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname S_America sam es_AR es_BO es_CL es_CO es_EC es_PE es_PY es_UY es_VE pt_BR es\n";
#
## Search each line of @localedata, if it contains an entry containing the sam region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\ssam\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print seu region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#               Solaris 10, Southern Europe partial locales \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "# ( el, el_GR, el_GR.ISO8859-7, el_GR.ISO8859-7@euro, el.sun_eu_greek,\n";
#print CONFIGFILE "#  es, es_ES, es_ES.ISO8859-1, es_ES.ISO8859-15, es_ES.ISO8859-15@euro,\n";
#print CONFIGFILE "#  es_ES.UTF-8, es_ES.UTF-8@euro, es.ISO8859-15, es.UTF-8,\n";
#print CONFIGFILE "#  it, it.ISO8859-15, it_IT, it_IT.ISO8859-1, it_IT.ISO8859-15,\n";
#print CONFIGFILE "#  it_IT.ISO8859-15@euro, it_IT.UTF-8, it_IT.UTF-8@euro, it.UTF-8,\n";
#print CONFIGFILE "#  pt, pt.ISO8859-15, pt_PT, pt_PT.ISO8859-1, pt_PT.ISO8859-15,\n";
#print CONFIGFILE "#  pt_PT.ISO8859-15@euro, ca_ES, ca_ES.ISO8859-1, ca_ES.ISO8859-15 )\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname S_Europe seu ca_ES en_MT el_CY el_GR es_ES it_IT mt_MT pt_PT it es \n\n";
#
## Search each line of @localedata, if it contains an entry containing the seu region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\sseu\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print th_th region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                       Solaris 10, Thai Locale \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "#    ( th, th_TH, th_TH.TIS620, th_TH.ISO8859-11, th_TH.UTF-8)\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname Asia th_th th_TH th_TH.UTF-8 \n\n";
#
## Search each line of @localedata, if it contains an entry containing the th_th region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\sth_th\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print weu region header data
#
#print CONFIGFILE "\n\n";
#print CONFIGFILE "#            Solaris 10, Western Europe partial locales\n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "# ( en_GB, en_GB.ISO8859-1, en_GB.ISO8859-15, en_GB.ISO8859-15@euro,\n";
#print CONFIGFILE "#   en_IE, en_IE.ISO8859-1, en_IE.ISO8859-15, en_IE.ISO8859-15@euro,\n";
#print CONFIGFILE "#   fr, fr_BE, fr_BE.ISO8859-1, fr_BE.ISO8859-15, fr_BE.ISO8859-15@euro,\n";
#print CONFIGFILE "#   fr_FR, fr_FR.ISO8859-1, fr_FR.ISO8859-15, fr_FR.ISO8859-15@euro,\n"; 
#print CONFIGFILE "#   fr_FR.UTF-8, fr_FR.UTF-8@euro, fr.ISO8859-15, fr.UTF-8, nl, nl_BE,\n";
#print CONFIGFILE "#   nl_BE.ISO8859-1, nl_BE.ISO8859-15, nl_BE.ISO8859-15@euro,\n";
#print CONFIGFILE "#   nl.ISO8859-15, nl_NL, nl_NL.ISO8859-1, nl_NL.ISO8859-15,\n";
#print CONFIGFILE "#   nl_NL.ISO8859-15@euro, fr_BE.UTF-8 )\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname W_Europe weu de_LU en_GB en_IE fr_BE fr_FR fr_LU nl_BE nl_NL fr \n\n";
#
## Search each line of @localedata, if it contains an entry containing the weu region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\sweu\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print china region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                     Solaris 10, Simplified Chinese Locale \n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "# ( zh_CN.EUC, zh, zh_CN.GB18030, zh_CN.GBK, zh.GBK, zh_CN.UTF-8, zh.UTF-8 )\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname Asia china zh_cn zh_CN zh zh_CN.GB18030 zh.GBK zh.UTF-8\n\n";
#
## Search each line of @localedata, if it contains an entry containing the china region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\schina\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print hongkong region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#            Solaris 10, Traditional Chinese (Hong Kong) Locale\n";
#print CONFIGFILE "#\n";
#print CONFIGFILE "#                 ( zh_HK.BIG5HK, zh_HK.UTF-8)\n";
#print CONFIGFILE "#\n\n";
#print CONFIGFILE "locname Asia hongkong zh_hk zh_HK.BIG5HK zh_HK.UTF-8\n\n";
#
## Search each line of @localedata, if it contains an entry containing the hongkong region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\shongkong\s/) {
#		print CONFIGFILE $line;
#	}
#}
#
## Print taiwan region header data
#
#print CONFIGFILE "\n#\n";
#print CONFIGFILE "#                   Solaris 10, Traditional Chinese Locale\n";               
#print CONFIGFILE "#\n";
#print CONFIGFILE "#               ( zh_TW, zh_TW.EUC, zh_TW.BIG5, zh_TW.UTF-8 )\n";
#print CONFIGFILE "\n";
#print CONFIGFILE "locname Asia taiwan zh_tw zh_TW.BIG5 zh_TW zh_TW.UTF-8\n\n";
#
## Search each line of @localedata, if it contains an entry containing the taiwan region
## print to config file
#
#foreach $line (@localedata) {
#	
#	if ($line =~ /\staiwan\s/) {
#		print CONFIGFILE $line;
#	}
#}

# close  config file inputstream

close (CONFIGFILE);






