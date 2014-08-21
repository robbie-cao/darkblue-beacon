#!/usr/bin/perl

###############################################################################
#
# THIS INFORMATION IS PROPRIETARY TO
#     BROADCOM CORPORATION
#  All rights reserved
#
#
#  This file extracts given sections out of an elf given the base address
# of this SPAR. Applicable only to SPAR in RAM.
###############################################################################

use strict;
use warnings;

my $help = undef;
my $section = undef;
my $baseadr = undef;
my $symtab  = undef;
my $binpath = undef;
my $outpath = undef;
my $bytes = undef;

# Loop through parameters
PARAM_LOOP: while(@ARGV)
{
    my $tag = shift @ARGV;

    my $value;
    if(@ARGV)
    {
        $value = shift @ARGV;
    }
    else
    {
        $help = 1;
        last PARAM_LOOP;
    }

    if($tag eq '-section')
    {
        $section = $value;
    }
    elsif($tag eq '-baseadr')
    {
        $baseadr = hex $value;
    }
    elsif($tag eq '-binpath')
    {
        $binpath = $value;
    }
    elsif($tag eq '-symtab')
    {
        $symtab = $value;
    }
    elsif($tag eq '-outpath')
    {
        $outpath = $value;
    }
    else
    {
        $help = 1;
        last PARAM_LOOP;
    }
}

if($help || !defined $section || !defined $baseadr || !defined $binpath || !defined $symtab || !defined $outpath)
{
    printf( "USAGE: objcpy.pl [-section section_name -baseadr base_address -binpath path_to_whole_bin_file -symtab symbol_table_path -outpath output_path\n]");
    exit -1;
}

# Now we hav all we want
# find the section in the symbol table first
my ($sec_size, $sec_addr) = getSectionSizeAndAddress($section, $symtab);

# Now e have the size and address.... prepare to extract
# See how many bytes to skip
my $offset = $sec_addr - $baseadr;
my $vv = 1;
open(BINFILE, '<', $binpath) || die "ERROR: Cannot open bin file $binpath, $!";
open(SECBINFILE, '>', $outpath) || die "ERROR: Cannot open $outpath for output, $!";

binmode (BINFILE);
binmode (SECBINFILE);

# Seek to the offset
seek(BINFILE, $offset, 0);

# Read in the bytes
read(BINFILE, $bytes, $sec_size) || die "ERROR: Cannot read $sec_size bytes at offset $offset from $binpath, $!";

# Write that out to the output file
print SECBINFILE $bytes || die "ERROR: Cannot write to $outpath, $!";

close(BINFILE);
close(SECBINFILE);

# done

###############################################################################
#################### SUBROUTINES #################################################
###############################################################################
sub getSectionSizeAndAddress
{
    my $sec = $_[0];
    my $sym = $_[1];
    my $size;
    my $address;
    
    open( INFILE, '<', $sym ) || die "ERROR: Unable to open symbol table file $sym, $!";
    
    my $ok = 0;
    my $line;
    
    MAP_PRE_PHASE_1_LOOP: while( $line = <INFILE> )
    {
        # look for the section
        if($line =~ /^\*\*\sSection\s.*\'$sec\'\s.*/)
        {
            $ok = 1;
            last MAP_PRE_PHASE_1_LOOP;
        }
    }
    $ok || die "ERROR: Invalid symbol table file detected, $!";
    
    # Now get the size and address
    $line = <INFILE>;  # Next line should have the size
    if($line =~ /^\s+Size\s+:\s([\d]+)+\sbytes\s+\(alignment.*/)
    {
        $size = $1;
    }
    else
    {
        die "ERROR: Size: was expected - section size unknown, $!";
    }
    
    $line = <INFILE>;  # Next line should have the address
    if($line =~ /^\s+Address:\s+0x([\dA-Fa-f]+)+/)
    {
        $address = hex $1;
    }
    else
    {
        print $line;
        die "ERROR: Address: was expected - section address unknown, $!";
    }
    
    close(INFILE);
    
    return ($size, $address);
}

