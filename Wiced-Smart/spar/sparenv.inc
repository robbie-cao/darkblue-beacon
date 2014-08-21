##############################################################################
# THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP                           #
#                                                                            #
# Copyright (c) 2009 Broadcom Corp.                                          #
#                                                                            #
#          ALL RIGHTS RESERVED                                               #
#                                                                            #
##############################################################################

# Basic environment for SPAR builds
NUM_DIGITS := $(strip $(call QUIET_SHELL,$(PERL) ../getChipNameLen.pl $(BLD)))
ifeq ($(NUM_DIGITS),5)
  # Commands need to be in variables since they contain commas which would confuse the Call function
  CHIP_CMD   := $(ECHO) $(BLD) | $(CUT) -c 3,4,5,6,7
  REVNUM_CMD := $(ECHO) $(BLD) | $(CUT) -c 8,9
  CHIP       := $(call QUIET_SHELL,$(CHIP_CMD))
  REVNUM     := $(call QUIET_SHELL,$(REVNUM_CMD))
else

ifeq ($(NUM_DIGITS),4)
  # Commands need to be in variables since they contain commas which would confuse the Call function
  CHIP_CMD   := $(ECHO) $(BLD) | $(CUT) -c 3,4,5,6
  REVNUM_CMD := $(ECHO) $(BLD) | $(CUT) -c 7,8
  CHIP       := $(call QUIET_SHELL, $(CHIP_CMD))
  REVNUM     := $(call QUIET_SHELL, $(REVNUM_CMD))
endif
endif

PLATFORM := $(call QUIET_SHELL, $(ECHO) $(BLD) | $(CUT) -c 1)
VARIANT  := $(call QUIET_SHELL, $(ECHO) $(BLD) | $(CUT) -d_ -f 3-)


