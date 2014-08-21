#!/usr/bin/perl

#use strict;
#use warnings;
#use Devel::Peek;

(@ARGV >= 4 && $ARGV[@ARGV-2] eq '-o') || die;
my @infiles;
@infiles = @ARGV[0..@ARGV-3];
my $outfile = $ARGV[@ARGV-1];

open( OUTFILE, '>', $outfile ) || die;
printf( OUTFILE '$'."\n" );

my $infile;
foreach $infile (@infiles)
{
    open( INFILE, '<', $infile ) || die;

    my $line;
    while($line = <INFILE>)
    {
        # Strip any leading \r or \n, get rid of $
        while( length($line) )
        {
            while( length($line)
                    && (    substr( $line, 0, 1 ) eq "\r"
                            ||  substr( $line, 0, 1 ) eq "\n"
                            ||  substr( $line, 0, 1 ) eq '$' ) )
            {
                $line = substr( $line, 1 );
            }

            # Get the length to the first \r or \n
            my $valid_len = 0;
            while(  $valid_len < length($line)
                    && substr( $line, $valid_len, 1 ) ne "\r"
                    && substr( $line, $valid_len, 1 ) ne "\n" )
            {
                $valid_len++;
            }

            # Extract a real line if there was any valid length
            if($valid_len)
            {
                $valid_len >= 11 || die;
                substr( $line, 0, 1 ) eq ':' || die $line;
                my $real_line = substr( $line, 0, $valid_len );
                $line = substr( $line, $valid_len );
                if( substr( $real_line, 1, 2 ) ne '00'
                    || uc( substr(  $real_line, 7, 2 ) ) ne '01' )
                {
                    # not end of records: save it right back out
                    printf( OUTFILE  "%s\n", $real_line );
                }
            }
        }
    }

    close(INFILE);
}

printf( OUTFILE ":00000001FF\n" );
close(OUTFILE);

exit 0;

