#!/usr/bin/perl

#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

# Syntax:
# ./print_file.pl <file>
if ( $#ARGV != 0 )
{
    print "Incomplete arguments";
    exit;
}

my $file = $ARGV[0];

# Open message file
open INFILE, "<:raw", $file or die "cant open " . $file;

while ($line = <INFILE>)
{
	chomp($line);
	print $line;
}

close INFILE;

# Exit successfully
exit(0);
