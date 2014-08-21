#!/usr/bin/perl

###############################################################################
#
# THIS INFORMATION IS PROPRIETARY TO
#     BROADCOM CORPORATION
#  All rights reserved
#
#
#  Prints RAM usage statistics.
###############################################################################

# Args
# 0 elf list file $(ELF_OUT_LIST)
# 1-.. Elf file list $(ELF_LIST)

my $list_filename = shift(@ARGV);
my @elf_list = @ARGV;

my $tier2_start_addr;
my $tier2_end_adr;

foreach my $elf ( @elf_list )
{
	my $lst = $elf;
	$lst =~ s/^(.*)\.elf$/$1.lst/i;  # Change elf extension to lst

	open(my $lst_hnd, '<', $lst) || die("Error opening $lst: $!\n");
	my $lst_content = do { local $/; <$lst_hnd> };
	close( $lst_hnd );

	if ( $lst_content =~ m/^[[:space:]]+Load[[:space:]]Region[[:space:]]CM3_Ver1[[:space:]].*Base.*[[:space:]](0x[[:alnum:]]+).*[[:space:]]Size\:[[:space:]]+(0x[[:alnum:]]+)/mi )
	{
		$tier2_start_addr = hex($1);
		$tier2_end_adr    = hex($2) + $tier2_start_addr ;
		last;
	}
}


if ( ! defined($tier2_start_addr) )
{
	die "ERROR: tier2_start_addr - Load Region CM3_Ver1 not found in list files\n";
}


open(my $list_handle, '<', $list_filename) || die("Error opening $list_filename: $!\n");
my $list_file_content = do { local $/; <$list_handle> };
close( $list_handle );

if ( $list_file_content !~ m/^[[:space:]]+(0x[[:alnum:]]+)[[:space:]]+spar_irom_begin[[:space:]]=[[:space:]]0x[[:alnum:]]+/mi )
{
	die "ERROR: failed to find spar_irom_begin in $list_filename\n";
}
my $spar_start_adr = hex($1);

if ( $list_file_content !~ m/^[[:space:]]+(0x[[:alnum:]]+)[[:space:]]+spar_iram_end[[:space:]]=[[:space:]]\./mi )
{
	die "ERROR: failed to find spar_irom_end in $list_filename\n";
}
my $spar_end_adr = hex($1);


print  "--------------------------------------------------------------------------------\n";
printf "Patches start at                  0x%08X (RAM address)\n", $tier2_start_addr;
printf "Patches end at                    0x%08X (RAM address)\n", $tier2_end_adr;
printf "Application starts at             0x%08X (RAM address)\n", $spar_start_adr;
printf "Application ends at               0x%08X (RAM address)\n", $spar_end_adr;
printf "\n";
printf "Patch size (including reused RAM) %*d bytes\n", 10, ($tier2_end_adr - $tier2_start_addr),  ($tier2_end_adr - $tier2_start_addr) / 1024;
printf "Patch size                        %*d bytes\n", 10, ($spar_start_adr - $tier2_start_addr), ($spar_start_adr - $tier2_start_addr) / 1024;
printf "Application size                  %*d bytes\n", 10, ($spar_end_adr - $spar_start_adr),     ($spar_end_adr - $spar_start_adr) / 1024;
print  "                                      ------\n";
printf "Total RAM footprint               %*d bytes (%02.1fkiB)\n", 10, ($spar_end_adr - $tier2_start_addr), ($spar_end_adr - $tier2_start_addr) / 1024;
print  "--------------------------------------------------------------------------------\n";
