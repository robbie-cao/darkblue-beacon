#!/usr/local/bin/perl

my $line = "";
my $byte;
my $byte_index = 0;
if ( ! -s $ARGV[0])
{
	exit 0;
}
open(MYFILE, '<', $ARGV[0]) || die "Can't open $ARGV[0]\n";

print "unsigned char file_as_hex_bytes[] = \n{\n    ";

while(read(MYFILE, $byte, 1) != 0)
{
    $line .= sprintf( "0x%02X, ", ord($byte) );
    if( ($byte_index % 16) == 15 )
    {
        $line .= "\n    ";
    }	
    $byte_index++;
}

close(MYFILE);

$line .= "\n};\n";
print $line;
