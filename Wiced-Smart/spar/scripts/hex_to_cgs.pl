#!/usr/bin/perl

###############################################################################
#
# THIS INFORMATION IS PROPRIETARY TO
#     BROADCOM CORPORATION
#  All rights reserved
#
#
#  Generates a CGS file from a hex file
###############################################################################


# Args
# 0 elf file $(ELF_OUT)
# 1 section name $*
# 2 direct_load flag - optional

my $elf_filename = $ARGV[0];
my $section_name = $ARGV[1];
my $hex_filename = $elf_filename . $section_name . ".hex";
my $list_filename = $elf_filename;
$list_filename =~ s/^(.*)\.elf$/$1.list/i;
my $output_filename = $elf_filename . $section_name . ".cgs";

my $ENTRY = "ENTRY \"Data\"";

if ( ( defined $ARGV[2] ) &&
     ( $ARGV[2] eq "y" ) )
{
  $ENTRY = "DIRECT_LOAD";
}


open(OUTFILE, '>', $output_filename) || die "Can't open $output_filename\n";

if ( -s $hex_filename )
{
    my $found = 0;
    my $section_start = "";
    open(INFILE, '<', $list_filename) || die "Can't open $list_filename\n";
    while(my $line = <INFILE>)
    {
        if ($line =~ m/^${section_name}\s+(0x[0-9A-Fa-f]{8,16})\s+0x[0-9A-Fa-f]+\r?$/gi)
        {
            $found = 1;
            $section_start = $1;
            last;
        }
    }
    close(INFILE);
    
    if ($found == 0)
    {
        close(OUTFILE);
        die "could not find section ${section_name}.\n"
    }
    
    open my $fh, '<', $hex_filename or die "Error opening $hex_filename: $!";
    my $hex_data = do { local $/; <$fh> };
    close($fh);
    
    print "Making " . $output_filename  .", start at $section_start ...\n";
    
    
    print OUTFILE "\n";
    print OUTFILE "# SPAR: $section_name from $elf_filename\n";
    print OUTFILE "$ENTRY = \"$section_name from $elf_filename\"\n";
    print OUTFILE "{\n";
    print OUTFILE "  \"Address\" = $section_start\n";
    print OUTFILE "  \"Data\" =\n";
    print OUTFILE "   COMMENTED_BYTES\n";
    print OUTFILE "     {\n";
    print OUTFILE "        <hex>\n";
    print OUTFILE "$hex_data";
    print OUTFILE "     } END_COMMENTED_BYTES\n";
    print OUTFILE "}\n";
    print OUTFILE "# SPAR: end of $section_name\n";
    close(OUTFILE);
    exit 0;
}
else
{
    print OUTFILE "# SPAR: $section_name from $elf_filename: empty section\n";
    close(OUTFILE);
    exit 0;
}

exit 0;
