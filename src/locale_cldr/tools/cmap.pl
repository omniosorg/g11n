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

use Encode;
use Data::Dumper;

my @cs;

foreach (@ARGV) {

	open (F, "<", $_) or die $!;
	
	my $cp;
	

	while(<F>) {
		next if /^#/ or /^\s*$/;

		if (/<code_set_name>/i) {
			($cp) = /<code_set_name>\s*"(.*)"/;
			next;
		}

		last if /^CHARMAP/;
	}

	my $last_code;
	my $len = 0;


	print "static struct cmap_char cs". ($#cs+1) . "[] = {\n";

	while(<F>) {
		next if /^#/ or /^\s*$/;
		last if (/^END CHARMAP/);

		chomp;
		my ($name, $code) = /<(.*)>\s*(.*)\s*$/;

		next if defined $last_code and $last_code eq $code;
		$last_code = $code;

		my $hexcode = $code;
		$hexcode =~ s/\\x(\w\w)/chr(hex($1))/eg;
		$hexcode = decode($cp,$hexcode);
	
		printf("\t{ 0x%04x, (unsigned char*)\"%s\", (unsigned char*)\"%s\" },\n", ord($hexcode), $code, $name);

		$len++;
	}

	print "};\n\n";
	
	close(F);

	my %o = ( name => $cp, len => $len );
	push @cs, \%o;
};

print "struct cmap cmaps[] = {\n";

printf "\t{ \"%s\", %d, cs%d },\n", $cs[$_]{name}, $cs[$_]{len}, $_ foreach (0 .. $#cs);

print "\t{ 0,0,0 }\n";
print "};\n";
