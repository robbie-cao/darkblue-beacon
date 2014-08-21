#!/usr/bin/perl

###############################################################################
#
# THIS INFORMATION IS PROPRIETARY TO
#     BROADCOM CORPORATION
#  All rights reserved
#
#
#  This file extracts given sections out of an elf given the base address of this SPAR. Applicable only to SPAR in RAM.
###############################################################################


# Args
# 0 elf file $(ELF_OUT)
# 1 readelf path $(READ_ELF)
# 2 spar setup function $(SPAR_CRT_SETUP)

my $elf_filename = $ARGV[0];
my $readelf_path = $ARGV[1];
my $spar_func    = $ARGV[2];
my $output_filename = $elf_filename . ".spar_setup_call.cgs";

my $readelf_output = `$readelf_path -sW $elf_filename`;

if (  $readelf_output !~ m/^\s+.*([0-9A-Fa-f]{8}).*$spar_func/mi )
{
    die "ERROR: CRT Setup Address for $spar_func not found in readelf output of $elf_filename\n\n";
}

my $spar_crt_setup_addr = $1;

open(OUTFILE, '>', $output_filename) || die "Can't open $output_filename\n";

print "Call to $spar_func @ $spar_crt_setup_addr\n";
print OUTFILE "\n";
print OUTFILE "# SPAR: call to  $spar_func\n";
print OUTFILE "# SPAR: (address from $elf_filename)\n";
print OUTFILE "ENTRY \"Function Call\"\n";
print OUTFILE "{\n";
print OUTFILE "   \"Address\" = 0x$spar_crt_setup_addr\n";
print OUTFILE "}\n";
print OUTFILE "\n";

close(OUTFILE);
exit 0;



