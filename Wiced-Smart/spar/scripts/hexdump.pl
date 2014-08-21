#!/usr/bin/perl

###############################################################################
#
# THIS INFORMATION IS PROPRIETARY TO
#     BROADCOM CORPORATION
#  All rights reserved
#
#
#  Dumps the contents of the given file as hex bytes for a cgs entry.
###############################################################################

my $line = "           ";
my $byte;
my $byte_index = 0;
if ( ! -s $ARGV[0])
{
	exit 0;
}
open(MYFILE, '<', $ARGV[0]) || die "Can't open $ARGV[0]\n";

# while(read(FILE, $byte, 1) == 1)
while(read(MYFILE, $byte, 1) != 0)
{
    $line .= sprintf( " %02x", ord($byte) );
    if( ($byte_index % 16) == 15 )
    {
        $line .= "\r\n           ";
    }	
    $byte_index++;
}

close(MYFILE);

$line .= "\r\n";
print $line;
