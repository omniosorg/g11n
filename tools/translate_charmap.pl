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
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

# The script translate a file from one charmap to another

use strict;
use warnings;

die "Usage: translate_charmap.pl <charmap> <new charmap> <file to translate>\n" unless $#ARGV == 2;

open F1, "<$ARGV[0]" or die $!;
open F2, "<$ARGV[1]" or die $!;
open F, "<$ARGV[2]" or die $!;

my %cm;

while (<F1>) { last if /^CHARMAP/ };
while (<F2>) { last if /^CHARMAP/ };

while (<F2>) {
	next if /^#/ or /^\s*$/;
	last if /^END CHARMAP/;

	chomp;
	my ($name, $code) = /<(.*)>\s*(.*)\s*$/ or die $_;
	$code = uc $code;

	print "# rewrite $cm{$code} to $name\n" if defined $cm{$code} and length($cm{$code}) > length($name);
	$cm{$code} = $name unless defined $cm{$code} and length($cm{$code}) > length($name);
}


my %tr;
while (<F1>) {
	next if /^#/ or /^\s*$/;
	last if /^END CHARMAP/;

	chomp;
	my ($name, $code) = /<(.*)>\s*(.*)\s*$/ or die $_;
	$code = uc $code;

	print "# unknown code $code ($name)\n" unless defined $cm{$code};

	$tr{$name} = $cm{$code};
}

while (<F>) {
	if (/^#/) { print; next };
	
	my $line = $_;

	for(my $i = 0;;) {
		my $n1 = index($line,"<",$i);
		
		if ($n1 == -1) {	
			print substr($line, $i);
			last;
		}

		print substr($line, $i, $n1-$i+1);

		my $n2 = index($line, ">", $n1);
		die "'>' is missing" if $n2 == -1;

		my $code = substr($line, $n1+1, $n2-$n1-1);
		
		die "'$code' is unkwnown" unless defined $tr{$code};
		print $tr{$code};

		$i = $n2;
	}
}
