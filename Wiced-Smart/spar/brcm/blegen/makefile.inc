########################################################################
#
# kb makefile.inc -- do not modify. Add files to fileslist.inc
#
########################################################################

include $(DIR)/fileslist.inc

CGS_LIST += 


# Include the chip common make file
include spar_2073x.inc

# No need to include the revision common include overrides here
# because  this is the ROM build.
