#!/usr/bin/perl
###############################################################################
#
# THIS INFORMATION IS PROPRIETARY TO
#     BROADCOM CORPORATION
#  All rights reserved
#
#
#  Given the overlaid object files, this file creates the required linker
# scatter files or the trampoline functions for each given function in the overlay.
###############################################################################

#use strict;
#use warnings;
#use Getopt::Long;
#use File::Basename;

my $help = undef;
my $tool = undef;
my $toolchain = undef;
my $verbose = 0;

my @libs = ();


my $argid = 0;
while ($argid <= $#ARGV ) 
{
  	my $arg = $ARGV[$argid];
  	$arg =~ s/^\s*(.*?)\s*$/$1/sgi;  # trim whitespace
	
	if($arg =~ /^-(.*)=(.*)/)
	{
		if   ($1 eq 'tool')  {$tool = $2;      splice(@ARGV, $argid, 1); next;}
		elsif($1 eq 'tc')    {$toolchain = $2; splice(@ARGV, $argid, 1); next;}
		elsif($1 eq 'v')     {$verbose = $2;   splice(@ARGV, $argid, 1); next;}
        else{$help = 1; last;}
	}
	
	$argid++;
}

printf STDERR "VERBOSE is ON\n" if ($verbose);

# Sort the objects to keep some things simple.
@libs = sort(@ARGV);

if($help)
{
    printf(STDERR "USAGE: generatelibinstaller.pl -tool={path_to_objdump|path_to_fromelf} -toolchain={rv|gcc} list\n");
    exit -1;
}

# Print install function
print "void install_libs(void);\n\nvoid install_libs(void)\n{\n";

if ($toolchain eq 'rv')
  {
  	printf STDERR "Tool chain is RV\n" if ($verbose);
  	
    foreach (@libs)
      {
        my $f = $_;
        my $init = basename($f) . "_init";
        
        printf STDERR "Reading symbol table from $f:\n" if ($verbose);
        printf STDERR "$tool -s $f\n" if ($verbose);
        
        open(SYMBOLS, "$tool -s $f |") || die "Could not read symbol table from $f with $tool.\n";
        while (<SYMBOLS>)
          {
            my $line = $_;
            if ($line =~ /\s+\d+\s+install_(.*)(.*)/)
              {
              	printf STDERR "Found install_$1\n" if ($verbose);
                print "\t{\n\t\textern void install_$1(void);\n\t\tinstall_$1();\n\t}\n";
              }
            if ( $line =~ /\s+\d+\s+$init(.*)/)
              {
              	printf STDERR "Found $init\n" if ($verbose);
                print "\t{\n\t\textern void $init(void);\n\t\t$init();\n\t}\n";
              }
          }
        close(SYMBOLS);
      }
  }
else
  {
  	printf STDERR "Tool chain is GCC\n" if ($verbose);
    foreach (@libs)
      {
        my $f = $_;
        my $init = basename($f) . "_init";
        printf STDERR "Reading symbol table from $f:\n" if ($verbose);
        printf STDERR "$tool -t $f\n" if ($verbose);
        
        open(SYMBOLS, "$tool -t $f |") || die "Could not read symbol table from $f with $tool.\n";
        while (<SYMBOLS>)
          {
            my $line  = $_;
            if ($line =~ /(.*)\.init_code\s+([\dA-Fa-f]{8})(.*)install_(.*)$/ )
              {
              	printf STDERR "Found install_$4\n" if ($verbose);
                print "\t{\n\t\textern void install_$4(void);\n\t\tinstall_$4();\n\t}\n";
              }
            elsif ( $line =~ /(.*)\.init_code\s+([\dA-Fa-f]{8})(.*)$init(.*)/ )
              {
              	printf STDERR "Found $init\n" if ($verbose);
                print "\t{\n\t\textern void $init(void);\n\t\t$init();\n\t}\n";
              }
          }
        close(SYMBOLS);
      }
  }

print "}\n";

printf STDERR "Lib installer done.\n" if ($verbose);

sub basename($) 
{
	my $file = shift;
	$file =~ s!^(?:.*/)?(.+?)(?:\.[^.]*)?$!$1!;
	return $file;
}
