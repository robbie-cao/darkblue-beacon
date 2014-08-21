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
my $option = undef;
my $toolchain = undef;
my $filename = '';
my $dir = '';

my @objs = ();


my $argid = 0;
while ($argid <= $#ARGV ) 
{
  	my $arg = $ARGV[$argid];
  	$arg =~ s/^\s*(.*?)\s*$/$1/sgi;  # trim whitespace
	
	if($arg =~ /^-(.*)=(.*)/)
	{
		if   ($1 eq 'gen')  {$option = $2;    splice(@ARGV, $argid, 1); next;}
		elsif($1 eq 'tc')   {$toolchain = $2; splice(@ARGV, $argid, 1); next;}
		elsif($1 eq 'file') {$filename = $2;  splice(@ARGV, $argid, 1); next;}
		elsif($1 eq 'dir')  {$dir = $2;       splice(@ARGV, $argid, 1); next;}
	}
	
	$argid++;
}

# Sort the objects to keep some things simple.
@objs = sort(@ARGV);

if(length(@objs) == 0 || !defined($option) || !defined($toolchain))
{
	print STDERR "No object file list given.\n";
	$help = 1;
}

if($help)
{
    printf(STDERR "USAGE: generateoverlayfiles.pl -gen={scat|list|tramp} -toolchain={rv|gcc} list\n");
    exit -1;
}

if($option eq 'scat')
{
	my $count = 0;
	my $match = '';
	my $post = ' ';

	foreach(@objs)
	{
		$match = sprintf("_%d_.*", $count);
		chomp($_);
		if(!($_ =~ /^$match/))
		{
			print STDERR "File name $_ must begin with _[:digit:]_ and must be in sequence from _0_. Expected $match\n";
			exit -1;
		}
		$count++;
	}

	if($toolchain eq 'rv')
	{
		# Need to generate a scatter file for use by the linker. Output to stdout.
		foreach(@objs)
		{
			chomp($_);
			print "  SPAR_OVERLAY_" . basename($_, ".o") . "  +0 OVERLAY OVERLAY_AREA_LENGTH\n";
			print "  {\n";
			print "      " . $_ . " (+RO)\n";
			print "  }\n\n";
		}
	}
	else
	{
		$count = 0;
		if($dir eq '')
		{
			print STDERR "Need -dir directory path of the object file..\n";
			exit -1;
		}

		# Generate the header.
		print "  OVERLAY : NOCROSSREFS\n  {\n";

		# Now print each section/object file with the .text section.
		foreach(@objs)
		{
			chomp($_);
			print '    .' . basename($_, ".o") . "{ $dir$_(.text.*) }\n";
			$post .= "    PROVIDE(_section_idx_$count = $count);\n";
			$count++;
		}

		# Generate the footer of the overlay placement.
		print "  } >ram\n\n";
		print "\n$post\n\n";
	}
}
elsif($option eq 'tramp')
{
	my $filenum;
	my $basename;
	my $symbolsRefs = '';

	if($filename eq '')
	{
		print STDERR "Need -file to generate the trampoline sources.\n";
		exit -1;
	}

	if($filename =~ /_(\d)_(.*).o/)
	{
		$filenum = $1;
		$basename = uc $2;
	}
	else
	{
		print STDERR "File $filename does not begin with _[:digit:]_\n";
		exit -1;
	}

	if($toolchain eq 'rv')
	{
		# Import some symbols and includes.
		print "  THUMB\n\n";
		print "  AREA ||.text||, CODE, READONLY, ALIGN=2\n\n";

		print ";; Generate a short function to jump to the overlay manager to save some space.\n";
		print "CALL_OVM_FROM_$basename PROC\n";
		print "  IMPORT spar_ovm_load_and_exec\n";
		print "  MOVS  R0, \#$filenum\n";
		print "  B  spar_ovm_load_and_exec ;; Unconditionally branch to the overlay manager to load overlay.\n";
		print "  ENDP\n\n";

		print ";; Generate overlay trampolines for each function in file.\n";
		foreach(@objs)
		{
			print '  EXPORT ||$Sub$$' . $_ . "||\n";
			print '||$Sub$$' . $_ . "|| PROC\n";
			$symbolsRefs .= '  IMPORT ||$Super$$' . $_ . "||\n" ;
			print "  ;; Preserve the parameters passed in by the original caller if any.\n";
			print "  PUSH {R0 - R4, LR}\n";
			print '  ADR  R4, addr_' . $_ . "\n";
			print "  B CALL_OVM_FROM_$basename\n";
			$symbolsRefs .= "addr_$_ DCD ||" . '$Super$$' . $_ . "||\n" ;
			print "  ENDP\n\n";
		}

		print "$symbolsRefs\n\n  END\n\n";
	}
	else
	{
		my $fcount = 0;

		print "    .syntax unified\n    .cpu cortex-m3\n    .thumb\n    .extern spar_ovm_load_and_exec\n\n    .align 2\n";
		print "    .global CALL_OVM_FROM_$basename\n    .thumb_func\n    .type CALL_OVM_FROM_$basename, \%function\n\n";
		print "CALL_OVM_FROM_$basename:\n";
		print "    movs r0, \#$filenum\n";
		print "    b spar_ovm_load_and_exec\n";
		print "    .size CALL_OVM_FROM_$basename, .-CALL_OVM_FROM_$basename\n\n";

		foreach(@objs)
		{
			print "    .align 1\n";
			print "    .global __wrap_$_\n    .thumb_func\n    .type __wrap_$_, \%function\n\n";
			print "__wrap_$_:\n";
			print "    push {r0, r1, r2, r3, r4, lr}\n";
			print "    ldr r4, .L$fcount\n";
			$symbolsRefs .= "    .extern  __real_$_\n    .L$fcount: .word __real_$_\n";
			print "    b CALL_OVM_FROM_$basename\n";
			print "    .size __wrap_$_, .-__wrap_$_\n\n";
			$fcount++;
		}
		print "$symbolsRefs\n\n";
	}
}
else
{
	print STDERR "Unknown option.\n";
	exit -1;
}


sub basename($) 
{
	my $file = shift;
	$file =~ s!^(?:.*/)?(.+?)(?:\.[^.]*)?$!$1!;
	return $file;
}
