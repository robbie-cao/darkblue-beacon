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
# ./get_com_port.pl <DetAndId> <IDFile>

# 2 arguments: <DetAndId> <IDFile>. Argument counts from 0.
if ( $#ARGV != 2 )
{
    print "Incomplete arguments\n";
    exit;
}

my $detect_and_id = $ARGV[0];
my $id_file       = $ARGV[1];
my $com_port_file = $ARGV[2];
my $chip_id       = 0;
my $comp_port     = 0;
my $expected_str  = 0;
my $device_found  = 0;
my $id_file_line  = 0;
my $line          = 0;

print "$com_port_file\n";

# Open ID file and get the chip ID.
open INFILE, "<:raw", $id_file or die "cant open " . $id_file;
open OUTFILE, ">:raw", $com_port_file or die "cant open " . $com_port_file;

($id_file_line) = <INFILE>;
 
chomp($id_file_line);      # Remove trailing newline
$id_file_line =~ s/^\s+//; # Remove leading spaces
$id_file_line =~ s/\s+$//; # Remove trailing spaces

# Get chip ID (first word)
($chip_id) = $id_file_line =~ /\A([^:\s]+)/;

# Call DetectAndId to identify the board
$output = `$detect_and_id -TRAN UART -IDFILE $id_file`;

    
# Expected string
($expected_str) = qr/(.*\S)\s+HCI\s+$chip_id/;        
# $1 will be table1, table2, table3. delim is a newline
my @tables = split(/\n/, $output);
foreach $line (@tables)
{
    chomp($line);      # Remove trailing newline
    $line =~ s/^\s+//; # Remove leading spaces
    $line =~ s/\s+$//; # Remove trailing spaces
    
    # Check if DetectAndId output contains the expected string
    if ( $line =~ $expected_str ) 
    {
        # Get COM port (first word)
        ($com_port) = $line =~ /\A([^:\s]+)/;
        
        # Dump COM port number into com_port_file
        print OUTFILE $com_port;
        
        $device_found = 1;
        break;
    } else {
    }
}
    
close INFILE;
close OUTFILE;

if ( $device_found == 0 )
{
    exit(1);
}
else
{
    exit(0);
}
