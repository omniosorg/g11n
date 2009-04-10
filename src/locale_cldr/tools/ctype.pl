#!/usr/bin/perl

#
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
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

use warnings;
use strict;

my %cm;
my ($cmap, $ctype) = @ARGV;

open (F, "<", $cmap) or die $!;
	
while(<F>) { last if /^CHARMAP/; }

while(<F>) {
	next if /^#/ or /^\s*$/;
	last if (/^END CHARMAP/);

	chomp;
	my ($name, $code) = /<(.*)>\s*(.*)\s*$/;

	$cm{$name} = 1;
}

close(F);

open (F, "<", $ctype) or die $!;

my $line = "";
while(<F>) {
	chomp;

	next if (/^\*/);

	if (/(.*)\(<([^>]*)>,<([^>]*)>\);*(.*)/ and not (defined $cm{$2} and defined $cm{$3})) {
		$_ = $1.$4;
	} elsif (/(.*)<([^>]*)>;*(.*)/ and not defined $cm{$2}) {
		$_ = $1.$3;
	}


	$line .= $_;

	if (/\/$/) {
		$line =~ s/(.*)\//$1/;
	} else {
		$line =~ s/\s+/ /g;
		$line =~ s/;\s*$//g;
		print "$line\n";
		$line = "";
	} 
}
