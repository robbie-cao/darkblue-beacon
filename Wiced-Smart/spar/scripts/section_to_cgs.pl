#!/usr/bin/perl

###############################################################################
#
# THIS INFORMATION IS PROPRIETARY TO
#     BROADCOM CORPORATION
#  All rights reserved
#
#
#  Extracts the named section from the given elf
###############################################################################

# use strict;
# use warnings;
# use Getopt::Long;


my $elf_file = undef;
my $section = undef;
my $obj_dump_path = undef;
my $help = undef;
my $section_start = undef;
my $section_offset = undef;
my $section_length = undef;
my $section_bytes = "        ";
my $byte = "";
my $byte_index = 0;
my $direct_load = undef;
my $overlay_index = undef;
my $compressor = undef;
my $tc = 'gcc';
my $verbose = 0;

my $ENTRY = "ENTRY \"Data\"";

	
$optResult = 1;
for ( my $argid = 0; $argid <= $#ARGV; $argid++ )
{
	$arg = $ARGV[$argid];
	if ( ( ($argid + 1) <= $#ARGV ) &&
	     ( $ARGV[$argid+1] !~ /^\s*\-/ ) )
	{
		$arg .= " " . $ARGV[$argid+1];
	}
	$arg =~ s/^\s*(.*?)\s*$/$1/sgi;  # trim whitespace
	if ( $arg =~ /^\-h$/sgi )       { $help = 1;                     }
	elsif ( $arg =~ /^\-h$/sgi )       { $direct_load = 1;              }
	elsif ( $arg =~ /^\-e ?(\S+)$/sg ) { $elf_file = $1;      $argid++; }
	elsif ( $arg =~ /^\-s ?(\S+)$/sg ) { $section = $1;       $argid++; }
	elsif ( $arg =~ /^\-o ?(\S+)$/sg ) { $obj_dump_path = $1; $argid++; }
	elsif ( $arg =~ /^\-i ?(\d+)$/sg ) { $overlay_index = $1; $argid++; }
	elsif ( $arg =~ /^\-c ?(\S+)$/sg ) { $compressor = $1;    $argid++; }
	elsif ( $arg =~ /^\-t ?(\S+)$/sg ) { $tc = $1;    $argid++; }
	elsif ( $arg =~ /^\-v ?(\S+)$/sg ) { $verbose = int($1);    $argid++; }
	else { $optResult = 0; }
}

printf STDERR "VERBOSE is ON\n" if $verbose; 					  
						  
my $Usage =<<EOF;
    section_to_cgs.pl [-h] | (-e elf_file -s section_name -o obj_dump_bin_path) [-d] [-i overlay_index] [-t] [rv|gcc]
    -h      help
    -d      Direct Load entry (ptional). Cannot be overlay or compressed item.
                You cannot overlay or compress if you select this option.
    -e      The elf file from which to grab the named section.
    -s      Name of the section to grab.
    -o      Object dump tool to use to parse the elf file.
    -i      Overlay index if this this is to be extracted as an overlay entry.
                You cannot compress or Direct Load if you select this option.
    -c      Compressor (optional) to use to generate compressed entry.
                You cannot Direct Load or Overlay if you select this.
	-t		The tool chain syntax to use when invoking objdump. Default is gcc syntax.
				Can be used to switch syntax to ARM RealView
EOF

if ($optResult != 1)
  {
    $help = 1;
  }

if (defined $help)
  {
    printf STDERR $Usage;
    exit -1;
  }

if (!defined $elf_file || !defined $obj_dump_path || !defined $section)
  {
  	printf STDERR "No elf file given\n" if (!defined $elf_file && $verbose);
  	printf STDERR "No elf obj dump file given\n" if (!defined $obj_dump_path && $verbose);
  	printf STDERR "No elf section given\n" if (!defined $section && $verbose);
  	
    printf STDERR "Invalid arguments.\n";
    printf STDERR $Usage;
    exit -2;
  }

if (defined $direct_load)
  {
  	printf STDERR "Direct load is ON\n" if ($verbose);
  	
    if (defined $compressor || defined $overlay_index)
      {
      	printf "Compression is ON when Direct load is also ON; not valid\n" if ($verbose && defined $compressor);
      	printf "Overlay is ON when Direct load is also ON; not valid\n" if ($verbose && defined $overlay_index);
      	
        printf STDERR "Cannot compress or overlay when using direct load.\n";
        exit -3;
      }

    $ENTRY = "DIRECT_LOAD";
  }

if (defined $overlay_index)
  {
  	printf STDERR "Overlay is ON.\n" if ($verbose);
  	
    if (defined $direct_load || defined $compressor)
      {
      	printf STDERR "Direct load is ON when overlay is also ON; not valid\n" if ($verbose && defined $direct_load);
      	printf STDERR "Compression is ON when overlay is also ON; not valid\n" if ($verbose && defined $compressor);
      	
        printf STDERR "Cannot compress or direct load when overlaying.\n";
        exit -4;
      }

    $ENTRY = "ENTRY \"Overlay data\"";
  }

if ($compressor)
  {
  	printf STDERR "Compression is ON.\n" if ($verbose);
  	
    if (defined $direct_load || defined $overlay_index)
      {
        printf STDERR "Cannot direct load or overlay when compressing.\n";
        exit -5;
      }

    $ENTRY = "ENTRY \"Compressed data\"";
  }
  
if($verbose)
{
	printf STDERR "\n";
	printf STDERR "Elf file      = $elf_file\n" if (defined $elf_file);
    printf STDERR "Section       = $section\n" if (defined $section);
    printf STDERR "Object dump   = $obj_dump_path\n" if (defined $obj_dump_path);

    printf STDERR "Direct load   = $direct_load\n" if (defined $direct_load);
    printf STDERR "Overlay index = $overlay_index\n" if (defined $overlay_index);
    printf STDERR "Compressor    = $compressor\n" if (defined $compressor);
    printf STDERR "Toolchain     = $tc\n" if (defined $tc);
    printf STDERR "\n";
}

if ($tc eq 'gcc')
  {
  	printf STDERR "Getting section header info using Obj dump:\n" if ($verbose);
  	printf STDERR "$obj_dump_path -j $section -h $elf_file\n\n" if ($verbose);
  	
    open(INFO, "$obj_dump_path -j $section -h $elf_file |") || die "Failed to run $obj_dump_path -j $section -h $elf_file\n";

    while (<INFO>)
      {
        my $line = $_;

        if ($line =~ /\s+\d+\s+$section\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})(.*)/)
          {
            $section_start = oct("0x$2") + 0;
            $section_offset = oct("0x$4") + 0;
            $section_length = oct("0x$1") + 0;
            last;
          }
      }
    close(INFO);
  }
else
  {
  	printf STDERR "Getting section header info using Obj dump:\n" if ($verbose);
  	printf STDERR "$obj_dump_path -v --only $section $elf_file\n\n" if ($verbose);
  	
    open(INFO, "$obj_dump_path -v --only $section $elf_file |") || die "Failed to run $obj_dump_path -v --only $section $elf_file\n";

    while (<INFO>)
      {
        my $line = $_;

        $section_start = oct("0x$1") + 0 if $line =~ /^\s+Addr\s+\:\s+0x([0-9A-Fa-f]{8})(.*)/;
        $section_offset = oct("0x$2") + 0 if $line =~ /^\s+File Offset\s+\:\s+(\d)+\s+\(0x([0-9A-Fa-f]+)\)(.*)/;
        $section_length = oct("0x$2") + 0 if $line =~ /^\s+Size\s+\:\s+(\d)+\s+bytes\s+\(0x([0-9A-Fa-f]+)\)(.*)/;
      }
	close(INFO);
  }

if($verbose)
{
	printf STDERR "\n";
	printf STDERR "Section Start  = 0x%08X\n", $section_start  if (defined $section_start);
    printf STDERR "Section Offset = 0x%08X\n", $section_offset if (defined $section_offset);
    printf STDERR "Section Length = 0x%08X\n", $section_length if (defined $section_length);
    printf STDERR "\n";
}

if (!defined $section_start || !defined $section_offset || !defined $section_length)
  {
    # Not really an error because the section could really be empty.
    print "\n# Section $section is empty. Skipping.\n";
    exit 0;
  }

# If section is non-empty, extract it.
printf STDERR "Opening $elf_file to read\n" if ($verbose);
open(ELFFILE, '<', $elf_file) || die "Cannot open $elf_file\n";

binmode(ELFFILE);

printf STDERR "Seeking to $section_offset into $elf_file\n" if ($verbose);
# Go to offset of section in elf.
seek(ELFFILE, $section_offset, 0);

if (defined $compressor)
  {
  	printf STDERR "\n"   if ($verbose);
    # Need to compress first. Extract bytes, pass to compressor
    # then extract bytes as string.
    my $buf;
    my $bin_file = $elf_file . $section . ".bin";
    my $cbin_file = $bin_file . ".cbin";
    my $result;

	printf STDERR "Bin file:  $bin_file\n"   if ($verbose);
	printf STDERR "CBin file: $cbin_file\n\n" if ($verbose);
	
    # Extract the bytes for the compressor to operate on.
    printf STDERR "Reading $section_length bytes from elf file into buffer\n" if ($verbose);
    if (read(ELFFILE, $buf, $section_length) != $section_length)
      {
        printf STDERR "Could not extract bytes for compression.\n";
        close(ELFFILE);
        exit -6;
      }
    
    printf STDERR "Opening bin file to dump raw bytes\n" if ($verbose);
    if (!open (BIN, '>', $bin_file))
      {
        printf STDERR "Could not open $bin_file for output.\n";
        close(ELFFILE);
        exit -7;
      }
    binmode(BIN);
    
    printf STDERR "Dumping raw $section bytes from elf into bin file\n" if ($verbose);
    print BIN $buf;
    
    close(BIN);
	printf STDERR "Bin dump complete.\n\n" if ($verbose);
	
    # Invoke the compressor.
    printf STDERR "Opening compressor:\n" if ($verbose);
    printf STDERR "$compressor e $bin_file $cbin_file\n" if ($verbose);
    
    if (!open(DO_WE_NEED_THIS, "$compressor e $bin_file $cbin_file |"))
      {
        printf STDERR "Could not invoke $compressor\n";
        exit -8;
      }

    while(<DO_WE_NEED_THIS>)
    {
    	printf STDERR "$_\n" if ($verbose);
    }
    
    close(DO_WE_NEED_THIS);
    printf STDERR "Compressor closed.\n" if ($verbose);
    
    printf STDERR "Deleting binary file\n" if ($verbose);
    # Delete the binary file because we don't need it anymore.
	unlink $bin_file;
	
	printf STDERR "Opening compressed file to read compressed bytes\n" if ($verbose);
    # Now open the compressed file and put the bytes into CGS format.
    if (!open(CBIN, '<', $cbin_file))
      {
        printf STDERR "Could not open compressed file $cbin_file\n";
        exit -9;
      }
    binmode(CBIN);
    
    printf STDERR "Reading compressed file\n" if ($verbose);
    while (read(CBIN, $byte, 1) != 0)
      {
        $section_bytes .= sprintf(" %02x", ord($byte));
        if (($byte_index % 16) == 15)
          {
          	printf STDERR "Read bytes: $byte_index\n" if ($verbose);
            $section_bytes .= "\n                ";
          }
        $byte_index++;
      }
    $section_bytes .= "\n";
    close(CBIN);
    
    # Delete the compressed bin file because we don't need it anymore.
    printf STDERR "Deleting compressed file\n" if ($verbose);
    unlink $cbin_file;
    printf STDERR "\n"   if ($verbose);
  }
else
  {
  	printf STDERR "\n"   if ($verbose);
  	printf STDERR "Reading bytes out of elf\n" if ($verbose);
    # Read section bytes out and make string directly.
    while ((read(ELFFILE, $byte, 1) != 0) && ($byte_index < $section_length))
      {
        $section_bytes .= sprintf(" %02x", ord($byte));
        if (($byte_index % 16) == 15)
          {
            $section_bytes .= "\n                ";
          }
        $byte_index++;
      }
    printf STDERR "\n"   if ($verbose);
    $section_bytes .= "\n";
  }
printf STDERR "Closing elf file\n\n" if ($verbose);
close(ELFFILE);

printf STDERR "Printing CGS entry\n" if ($verbose);
# Done.
print "# SPAR: $section from $elf_file\n";
print "$ENTRY = \"$section from $elf_file\"\n";
print "{\n";
if (defined $overlay_index)
  {
    print "    \"Overlay ID\" = $overlay_index\n";
  }
print "    \"Address\" = " . sprintf("0x%08x", $section_start) . "\n";
print "    \"Data\" =\n";
print "      COMMENTED_BYTES\n";
print "          {\n";
print "            <hex>\n";
print "        $section_bytes";
print "          } END_COMMENTED_BYTES\n";
print "}\n";
print "# SPAR: end of $section\n";

printf STDERR "section_to_cgs.pl done with $section, SUCCESS! \n" if ($verbose);

