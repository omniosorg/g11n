#!/usr/bin/perl
#
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
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

use strict;
use warnings;

# to level associative array, because some locales can be in more geozones (e.g "en")
# keys %{region{en}} gives you all geozones for given locale
my %region;

# Create an array to temporarily store region values on

my %Regionarray;

# official names of geoZones
my %geoNames;

# Retrieve the installation cd image sources
# First take the cdname as command line argument passed to the program

my $cdname = $ARGV[0];
chomp $cdname;

#retrieve configfile name
my $versionname = $ARGV[1];
chomp $versionname;

# Prompt the user to enter the path of the install image cd
my $path = $ARGV[2];
chomp $path;

# Read geozones config file
open(LOCALES, "$versionname") or die ("Couldn't open geozonesFile $versionname \n $!");
while (<LOCALES>) {
       next if ($_=~/^#/);
       next if ($_!~/locname/);
# each geozone on one line:
# in format: locname Ausi aua en_AU en_NZ
	$_ =~ /locname\s+(\w+)\s+(\w+)\s+(.*)/;
	my $zone = $2;
        $geoNames{$zone}=$1;
# locales should be coma separated
	my @locales= split(/[\s,]/, $3);
	for (@locales) {
           chomp;
# create value for each locale-geozone item e.g. $region{en_NZ}{aua}++ and also $region{en_GB}{weu}++
           $region{$_}{$zone}++;
           #get just the language code and assign it too geozone too e.g. region{"es"}{sam}++
           $_ =~ s/^([a-z][a-z]).*/$1/;           
           $region{$_}{$zone}++;
        }
}



#create the Temp File to write region data into to.
open (TEMPFILE, ">> $versionname.tmp") or die ("Couldn't open $versionname.tmp");

if ($path !~ /Product/) {
	opendir(DIR, $path); 
        while (my $d = readdir(DIR)) {
          if ($d =~ /Solaris_/ ) {
            $path = "$path/$d/Product";
            last;
          }
       }
      closedir(DIR);
}

# Change to the directory of the path

chdir("$path") or die ("Cannot change dir to $path ! \n $!");

opendir(DIR, $path);
while (my $pkgPath=readdir(DIR)) {
        next if (! -e "$pkgPath/pkginfo");   # check for existence of "pkginfo", this will skip directories that are not packages
	open (PKGINFO, "$pkgPath/pkginfo") or die("Couldn't open $path/$pkgPath/pkginfo information about image \n$!");

	my $package;
	while (<PKGINFO>) {
		chomp;
	       # Retrieve the full package name from the PKG= entry in file
		if (/PKG=/) {
			$package = substr($_, 4);
		}

		# Retrieve all locales if associated with PKG, and split them if there
		# are multiple locales available
		if (/SUNW_LOC=/) {
			my $Alllocales = substr($_, 9);
			my @locales = split(/,/, $Alllocales);

			# Add each occurence of pkg, region and locale to Regionarray 
			for my $locale (@locales) {
				if (defined $region{$locale}) {
				# Push all occurences of every other locale onto Regionarray
				  for (keys %{$region{$locale}}) {
					  $Regionarray{"$package\t$_"}=$cdname;
				  }
				}
			}
		}
	}                        
	close(PKGINFO);
}
closedir(DIR);


# if regenerating config file from DVD try to keep as much information as possible
# therefore: open the original config file and for those packages that were there keep the CDinfo, new packages are added with DVD location
# thus when installing from dvd this information is ignored anyway and it will stay valid for CDs
if ($cdname =~ /dvd/) {
  open(ORIGINAL, $versionname) or print "Couldn't file original config file $versionname, creating new one valid only for DVDimage\n";

  my %originalDistr;
  while (<ORIGINAL>) {
      $_ =~ s/#.*//;                 # remove all comments
      next if ($_ =~ /locname/);     # skip "locname" lines
      next if ($_ !~ /([^\s]*)\s+[^\s]*\s+([^\s]*)/);  # get the package name and cd
# get rid of the "c_" for "c_solaris" here
      my $img = $2;
      my $pkg = $1;
      $img =~ s/c_//;
      $originalDistr{$pkg}=$img;   
  }
  close(ORIGINAL);

  for my $key (keys %Regionarray) {
    my $package=$key;
    $package =~ s/([^\s]*)\s+.*/$1/;
    if (defined $originalDistr{$package}) {
#       print $originalDistr{$package} . " $package :: $key \n";
       $Regionarray{$key}=$originalDistr{$package};
    }
  }
}

# Open the core packages file, containing the list of core solaris l10n packages, and
# substitute any cd value with our cd value if packages and regions match

open (CORE, "/usr/sadm/lib/localeadm/core_packages.txt") or die ("Couldn't open core_packages.txt");
while (<CORE>) {
        next if ($_ =~/^#/);
	my $coreline =$_;
	$coreline =~ /^([\w-]+)\s+(\w+)\s+c_(\w+)/; 

	# The core file Package Name
	my $corename = $1;
        
        my $coreregion = $2;

	foreach my $regionline ( keys %Regionarray) {
		$regionline =~ /^([\w-]+)\s+(\w+)/;
		# Our current Package Name
		my $regionname = $1;

		# Our current Package Region
		my $regionregion = $2;

		# Our current Package Image	
		my $regionimage = $Regionarray{$regionline};

		# If the core package and region match with our package and region
		# substitute the cd image to c_ (a core image)
		if($regionname eq $corename &&  $regionregion eq $coreregion && $regionimage !~ /c_/) {
                        $Regionarray{$regionline}="c_$regionimage";
		}
                
	}
}

# Sort the array of regions, first sorted aplhabetically, then sort by region
# Result is array of region entries, sorted by region, and each regions entries
# are sorted alphabetically 

#@Regionarray = sort(@Regionarray);
#@Regionarray = map {$_->[0]}
#	sort { $a->[1] cmp $b->[1] }
#	map {[$_,/(\s+\w+)/, uc($_)]} @Regionarray;


# Remove duplicate lines in array and print into TEMPFILE
my $lastline="";
foreach my $line ( keys %Regionarray) {
	next if $line eq $lastline;
	print TEMPFILE "$line\t" . $Regionarray{$line} . "\n";
	$lastline = $line;
}

close (TEMPFILE);






