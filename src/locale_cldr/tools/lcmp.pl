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

use strict;
use warnings;

use Encode;
use HTML::Entities;

binmode(STDOUT, ":utf8");

my %lall;

foreach my $loc (sort @ARGV) {
	next unless -f "/usr/lib/locale/$loc/$loc.so.3";

	$lall{$loc}{$_} = { lcmp1($loc,$_) } foreach ('lc_charmap', 'lc_collate', 'lc_monetary', 'lc_numeric', 'lc_messages', 'lc_time', 'lc_ctype');
}

sub lcmp1 {
	my ($loc,$lc) = @_;
	my ($lang,$ter,$enc) = ($loc =~ /(..)_(..)\.(.*)/);
	my %ret;

	open(LCMP1, "-|:bytes", "./ldump $lc /usr/lib/locale/$loc/$loc.so.3") or die $!;
	open(LCMP2, "-|:bytes", "./ldump $lc locale/$loc.so.3") or die $!;

	while (my $l1 = <LCMP1>) {
		my $l2 = <LCMP2>;

		chomp ($l1);
		chomp ($l2);

		my ($name1, $v1) = ($l1 =~ /^([^\t]*)\t(.*)$/);
		my ($name2, $v2) = ($l2 =~ /^([^\t]*)\t(.*)$/);

		my ($desc1, $desc2) = ($name1, $name2);

		($name1,$desc1) = ($1,$2) if $name1 =~ /^(.*) \/\* (.*) \*\//;
		($name2,$desc2) = ($1,$2) if $name2 =~ /^(.*) \/\* (.*) \*\//;

		$name1 eq $name2 or die "$name1 != $name2";

		$v1 = decode($enc, $1) if $v1 =~ /('.*')/;
		$v2 = decode($enc, $1) if $v2 =~ /('.*')/;

		$ret{$name1} = { desc => $desc1, v1 => $v1, v2 => $v2 } if ($v1 ne $v2);
	}

	return %ret;
}


# make HTML report

print <<EOF;
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en" dir="ltr">
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<link rel="stylesheet" type="text/css" href="http://www.netbeans.org/netbeans.css" />
	</head>
	<body>
		<h1>System Locales versus CLDR 1.6</h1>
		<table border="1">
			<tr>
				<th colspan="2">Locale</th>
				<th>LC_CHARMAP</th>
				<th>LC_COLLATE</th>
				<th>LC_MONETARY</th>
				<th>LC_NUMERIC</th>
				<th>LC_MESSAGES</th>
				<th>LC_TIME</th>
				<th>LC_CTYPE</th>
			</tr>
EOF


foreach my $loc (sort @ARGV) {
	my ($lang,$terr,$enc) = ($loc =~ /(..)_(..)\.(.*)/);

	print <<EOF;
			<tr>
				<th align="right"><a href="#$loc">${lang}_$terr</a></th>
				<th align="left">$enc</th></th>
EOF
	if (not defined %{$lall{$loc}}) {
		print "\t\t\t\t<td colspan=\"7\">new locale</td>\n";
		next;
	}
	
	foreach ('lc_charmap', 'lc_collate', 'lc_monetary', 'lc_numeric', 'lc_messages', 'lc_time', 'lc_ctype') {
		my $n = keys %{$lall{$loc}{$_}};
	
		if ($n == 0) {
		    print "\t\t\t\t<td class=\"ok\"><a href=\"#$loc-$_\">$n diffs</a></td>\n";
		} elsif ($n < 5) {
		    print "\t\t\t\t<td class=\"warn\"><a href=\"#$loc-$_\">$n diffs</a></td>\n";
		} else {
		    print "\t\t\t\t<td class=\"problem\"><a href=\"#$loc-$_\">$n diffs</a></td>\n";
		}
		

		print <<EOF;
EOF
	}

	print <<EOF;
			</tr>
EOF
}
	
print <<EOF;
		</table>
EOF


foreach my $loc (sort keys %lall) {
	next unless defined %{$lall{$loc}};

print <<EOF;

		<h2><a name="$loc">$loc</a></h2>
EOF
	foreach my $lc ('lc_charmap', 'lc_collate', 'lc_monetary', 'lc_numeric', 'lc_messages', 'lc_time', 'lc_ctype') {
		print <<EOF;
		<h3><a name="$loc-$lc">\U$lc\E ($loc)</a></h3>
		<table border="1">
			<tr><th>Attribute</th><th alt="OpenSolaris">OSo</th><th>CLDR 1.6</th></tr>
EOF
	
		my @d = sort keys %{$lall{$loc}{$lc}};
		print <<EOF foreach @d[0..($#d<100 ? $#d : 100)];
			<tr>
				<td title="$lall{$loc}{$lc}{$_}{desc}">$_</td>
				<td>$lall{$loc}{$lc}{$_}{v1}</td>
				<td>$lall{$loc}{$lc}{$_}{v2}</td>
			</tr>
EOF

		print <<EOF;
		
		</table>
EOF
	}
}




