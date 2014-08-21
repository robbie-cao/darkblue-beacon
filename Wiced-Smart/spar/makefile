##############################################################################
# THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP                           #
#                                                                            #
# Copyright (c) 2007 Broadcom Corp.                                          #
#                                                                            #
#          ALL RIGHTS RESERVED                                               #
#                                                                            #
##############################################################################
default: help

ifeq ($(VERBOSE),1)
QUIET :=
QUIET_SHELL =$(shell $(1)$(info $(1)))
SILENT_MAKE :=
else
SILENT_MAKE :=-s
QUIET :=@
QUIET_SHELL =$(shell $(1))
endif

include ../tools.inc
include sparenv.inc
BLD_DIR = $(BLD)

# Include defalt platform.cgs from the current directory if
# nothing is specified. This may come in from WICED SDK platform directory.
PLATFORM_CGS_PATH ?= platform.cgs

DEVICE_TYPE_DIR = bleapp

ifeq ($(CONFIG_DEFINITION),)
  CGS_LIST += config_header_$(CHIP).cgs
endif

ifeq ($(SPAR_IN),ram)
ifeq ($(BOOT_SECTOR),y)
  CGS_LIST += ../bootsector/obj-A_$(CHIP)/bootsector.cgs

  CGS_TRAILERS_LIST = ../bootsector/footer.cgs
endif
endif

# Add patches only if base is in ROM.
ifeq ($(BASE_IN),rom)
  CGS_LIST  += ../tier2/$(APP_PATCH_DIR)/bld/$(CHIP)/$(notdir $(APP_PATCH_DIR)).cgs
endif

ifneq ($(PLATFORM_DISABLE),y)
# Add the platform specific CGS.
CGS_LIST += $(PLATFORM_CGS_PATH)
endif

ifdef DIR
   include $(DIR)/makefile.mk
endif

INCS += $(DIR)

# Include the chip common make file
include spar_2073x.inc

# Include generic CM3 options
include $(TC)/makecm3_$(TC).inc

# Include the ADK rules and BOM
-include makefile.adk

SRCDIR      = $(DIR)
BIN_OUT_DIR ?= $(SRCDIR)/obj-$(BLD_DIR)-$(BASE_IN)-$(SPAR_IN)
HEXMERGE    = ../tier2/mergearmhex.pl
STEER_OUTPUT  = y

################################################################################
# Help and exit on error
################################################################################
.PHONY: help clean

FORCE:

help:
	@$(ECHO) 'Usage: make help'
	@$(ECHO) '       make clean | cgs                            '
	@$(ECHO) '               DIR=your/dir/here                   '
	@$(ECHO) '               SPAR_IN=rom                         '
	@$(ECHO) '               BASE_IN=ram                         '
	@$(ECHO) '               [TC=gcc]                            '
	@$(ECHO) '               RVDS_TOOL_DIR= TOOLSBIN=            '
	@$(ECHO) '               BLD=A_20736A1                       '
	@$(ECHO)
	@$(ECHO) 'You should have a DIR/makefile.inc, which should include'
	@$(ECHO) 'the appropriate spar_20xx.inc.'
	@$(ECHO)
	@$(ECHO) 'Either might specify any or all the flags above.'

clean:
	$(QUIET)$(ECHO) 'Cleaning...'
	$(QUIET)$(XRM) $(BIN_OUT_DIR) $(DIR)/obj-adk $(ADDITIONAL_CLEAN_OBJS)
	$(QUIET)$(ECHO) 'Done'

NEEDED_MAKE_VARS += ELF_LIST CGS_LIST BLD
$(foreach var,$(NEEDED_MAKE_VARS), \
   $(if $(findstring undefined, $(origin $(var))), \
	$(warning "Warning: $(var) is undefined.")))

################################################################################
# Common Flags. DON'T PUT CHIP-SPECIFIC CRUD HERE.
################################################################################

SPAR_CRT_SETUP = $(notdir $(DIR))_spar_crt_setup
SPAR_APP_SETUP = application_setup

# Includes
INCS += .
# include the common spar directory
INCS += common
# include for this SPAR; if you need subdirs, do it from makefile.inc
INCS += $(DIR)
# includes for previous tiers; not exhaustive, no subdirs
INCS += $(foreach d,$(ELF_LIST),$(dir $(d))..)
# add tier2 includes
INCS += ../tier2/patch/inc
INCS += ../tier2/brcm/libraries/inc
# Add SDK includes if they exist
INCS += $(SDK_INC_FLAGS)

INCS_OPTS = $(addprefix -I, $(INCS))

ASM_FLAGS           +=  $(COMMON_FLAGS) $(CL_C_FLAGS)

# If we don't need the setup (for spar in ROM only), don't include it.
ifneq ($(NO_SPAR_SETUP), y)
  OBJS := spar_setup.c $(APP_SRC) $(APP_SECRET_SRC)
else
  OBJS := $(APP_SRC) $(APP_SECRET_SRC)
endif

# Add common sparinit.c
OBJS += sparinit.c
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.arm=.o)
OBJS := $(OBJS:.s=.o)
OBJS := $(addprefix $(BIN_OUT_DIR)/, $(OBJS))

# This is the final target elf
ELF_OUT = $(BIN_OUT_DIR)/$(BLD_DIR)-$(notdir $(DIR))-$(BASE_IN)-$(SPAR_IN)-spar.elf

.PHONY: elf
elf: $(ELF_OUT)

$(ELF_OUT) : $(ELF_LIST) $(OBJS) $(SPAR_LINK_LOAD_FILE)
	$(QUIET)$(ECHO) "Linking target ELF"
	$(QUIET)$(XLD) $(LINK_FLAGS) $(LINK_ELFS) $(OBJS) $(EXTRA_LD_FLAGS) --start-group $(LIBS) --end-group -o $@
	$(QUIET)$(MAKE_ASSEMBLY)
	$(QUIET)-$(ECHO) "OK, made elf."


################################################################################
# CGS generation, foo.elf.text.hex etc. -> spar.cgs
################################################################################
# Note: ARM7 and CM3 have slightly different rules. See respective rules file
# (Not ideal: we leave a few derived .hex objects outside of
# BIN_OUT_DIR. On the other hand, these hex files usually are produced
# by their respective builds, so typically we won't actually run these
# rules.

SPAR_CGS_TARGET = $(ELF_OUT:.elf=.cgs)

# Make the spar-setup call CGS element
SPAR_SETUP_CALL_CGS = $(ELF_OUT:.elf=.elf.spar_setup_call.cgs)

SECTIONS_CGS_LIST += $(patsubst %, $(ELF_OUT)%.cgs, $(UNCOMPRESSED_SECTIONS))
SECTIONS_CGS_LIST += $(patsubst %, $(ELF_OUT)%.ccgs, $(COMPRESSED_SECTIONS))
SECTIONS_CGS_LIST += $(patsubst %, $(ELF_OUT)%.ocgs, $(OVERLAID_SECTIONS))

# Convert the bin into hex (16 bytes per line), or a 0-length file if
# empty.
HEXDUMP = ./hexdump.pl
$(ELF_OUT)%.hex: $(ELF_OUT)%.bin $(HEXDUMP)
	$(QUIET)$(PERL) $(HEXDUMP) $< > $@

# Final CGS file
$(SPAR_CGS_TARGET): $(CGS_LIST) $(SECTIONS_CGS_LIST) $(SPAR_SETUP_CALL_CGS) $(CGS_TRAILERS_LIST)
	$(QUIET)-$(XMD) $(@D)
	$(QUIET)$(ECHO) "# SPAR: start of file $@" > $@
	$(QUIET)$(ECHO) "# SPAR: contents are: " >> $@
	$(QUIET)$(ECHO) "# $^ " >> $@
	$(QUIET)$(ECHO) "" >> $@
ifneq ($(CONFIG_DEFINITION),)
	$(QUIET)$(ECHO) "DEFINITION <$(CONFIG_DEFINITION)>" >> $@
endif
	$(QUIET)$(CAT) $^ >> $@
	$(QUIET)$(ECHO) "# SPAR: end of file $@" >> $@
	$(QUIET)$(ECHO) OK, made $(CURDIR)/$@. MD5 sum is:
	$(QUIET)$(MD5SUM) $@
	$(QUIET)-$(XCP) -f ../$(DEVICE_TYPE_DIR)/config/*.hdf $(@D)
	$(QUIET)$(XRM) $(ELF_OUT)*.cgs $(ELF_OUT_SYM) $(ELF_OUT_BIN)
	$(QUIET)$(ECHO)


.PHONY: cgs
cgs: $(SPAR_CGS_TARGET) $(if $(filter ram,$(SPAR_IN)),print_stats,)

################################################################################
# Intel Hex images (ready to burn with e.g. Bluetool)
################################################################################
HEX_TARGET = $(ELF_OUT:.elf=.full.hex)

$(HEX_TARGET): $(ELF_LIST:.elf=.hex) $(ELF_OUT:.elf=.hex) #$(FBSEC_ELF:.elf=.hex)
	$(QUIET)$(ECHO) "Writing Hex image"
	$(QUIET)-$(XMD) $(@D)
	$(QUIET)$(PERL) $(HEXMERGE) $^ -o $@
	$(QUIET)$(ECHO) OK, made $(CURDIR)/$@.
	$(QUIET)$(ECHO)

.PHONY: hex
hex: $(HEX_TARGET)


################################################################################
# Do doxygen documentation
################################################################################
.PHONY: doxygen
doxygen: ../$(DEVICE_TYPE_DIR)/docs/html/index.html

../$(DEVICE_TYPE_DIR)/docs/html/index.html:
	$(QUIET)cd ../$(DEVICE_TYPE_DIR) ; $(DOXYGEN) Doxyfile_$(CHIP)

################################################################################
# Dependencies
################################################################################

.DELETE_ON_ERROR:

$(BIN_OUT_DIR)/%.d:
	$(QUIET)$(MAKE_DEPS)

DEPS =  $(OBJS:.o=.d)
-include $(DEPS)


################################################################################
# Compile/assemble rules
################################################################################

# C (common)
$(BIN_OUT_DIR)/%.o $(BIN_OUT_DIR)/%.d: common/%.c
	$(QUIET)-$(XMD) $(@D)
	$(eval $(info Compiling $(notdir $<)))
	$(QUIET)$(XCC) $(C_FLAGS) $(LIST_OPTIONS) -o $@ $< $(REDIRECT_OUTPUT)
	$(QUIET)$(MAKE_DEP_FILE)

# C++ (common)
$(BIN_OUT_DIR)/%.o $(BIN_OUT_DIR)/%.d: common/%.cpp
	$(QUIET)-$(XMD) $(@D)
	$(eval $(info Compiling $(notdir $<)))
	$(QUIET)$(XCC) $(C_FLAGS) $(CPP_FLAGS) $(LIST_OPTIONS) -o $@ $< $(REDIRECT_OUTPUT)
	$(QUIET)$(MAKE_DEP_FILE)

# ASM (common)
$(BIN_OUT_DIR)/%.o $(BIN_OUT_DIR)/%.d: common/%.arm
	$(QUIET)-$(XMD) $(@D)
	$(eval $(info Assembling $(notdir $<)))
	$(QUIET)$(XAS) $(ASM_FLAGS) $(LIST_OPTIONS) -o $@ $<

# C (application)
$(BIN_OUT_DIR)/%.o $(BIN_OUT_DIR)/%.d: $(SRCDIR)/%.c
	$(QUIET)-$(XMD) $(@D)
	$(eval $(info Compiling $(notdir $<)))
	$(QUIET)$(XCC) $(C_FLAGS) $(LIST_OPTIONS) -o $@ $< $(REDIRECT_OUTPUT)
	$(QUIET)$(MAKE_DEP_FILE)

# C++ (application
$(BIN_OUT_DIR)/%.o $(BIN_OUT_DIR)/%.d:  $(SRCDIR)/%.cpp
	$(QUIET)-$(XMD) $(@D)
	$(eval $(info Compiling $(notdir $<)))
	$(QUIET)$(XCC) $(C_FLAGS) $(CPP_FLAGS) $(LIST_OPTIONS) -o $@ $< $(REDIRECT_OUTPUT)
	$(QUIET)$(MAKE_DEP_FILE)

# ASM (application)
$(BIN_OUT_DIR)/%.o $(BIN_OUT_DIR)/%.d: $(SRCDIR)/%.arm
	$(QUIET)-$(XMD) $(@D)
	$(eval $(info Assembling $(notdir $<)))
	$(QUIET)$(XAS) $(ASM_FLAGS) $(LIST_OPTIONS) -o $@ $<

$(BIN_OUT_DIR)/%.o $(BIN_OUT_DIR)/%.d: $(BIN_OUT_DIR)/%.s
	$(QUIET)-$(XMD) $(@D)
	$(eval $(info Assembling $(notdir $<)))
	$(QUIET)$(XAS) $(ASM_FLAGS) $(LIST_OPTIONS) -o $@ $< $(REDIRECT_ASM_OUTPUT)

$(BIN_OUT_DIR)/%.o $(BIN_OUT_DIR)/%.d: $(BIN_OUT_DIR)/%.c
	$(QUIET)-$(XMD) $(@D)
	$(eval $(info Compiling $(notdir $<)))
	$(QUIET)$(XCC) $(C_FLAGS) $(LIST_OPTIONS) -o $@ $< $(REDIRECT_OUTPUT)
	$(QUIET)$(MAKE_DEP_FILE)

# If no source is applicable, try this: Copy an object derived from a
# "secret" source.
$(BIN_OUT_DIR)/%.o : $(SRCDIR)/obj_secret/%.o
	$(QUIET)-$(XMD) $(@D)
	$(QUIET)$(XCP) -f $< $@

################################################################################
# Other generic rules that are target specific
################################################################################

include $(TC)/rules_cm3_$(TC).inc
