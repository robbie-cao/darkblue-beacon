# This contains the CM3 varient makefile customizations for RealView

# We link with symdefs and not the elf itself
LINK_ELFS 			:= $(ELF_LIST:.elf=.symdefs)
MAKE_ASSEMBLY       = $(FROMELF) --text -c --output $(@:.elf=.asm) $@
################################################################################
# General link flags
################################################################################
# Disable exceptions, datacompressor and some useless warnings
EXTRA_LD_FLAGS += --no_exceptions --info common,exceptions,inline,libraries,sizes,stack,unused 
EXTRA_LD_FLAGS += --datacompressor off -d --elf --symdefs $(ELF_OUT:.elf=.symdefs) 
EXTRA_LD_FLAGS += --diag_suppress=6312,6305,6307,6314 --load_addr_map_info

# Pre-processed 
SPAR_LINK_LOAD_FILE = $(BIN_OUT_DIR)/spar_$(SPAR_IN)_proc.scat

# Disable VFE, stdlibs and setup entry to spar_setup.
EXTRA_LD_FLAGS += --vfemode=off --scatter $(SPAR_LINK_LOAD_FILE) --list $(ELF_OUT:.elf=.list) 
EXTRA_LD_FLAGS += --map --symbols 
# --entry=$(SPAR_CRT_SETUP) 

ifneq ($(NEED_STD_LIBS), y)
  EXTRA_LD_FLAGS += --no_scanlib
endif

################################################################################
# Code/data location
################################################################################
# There are a few options when it comes to placing the image contents:
# 1. When spar is in ROM, we want to pin SPAR init to a known location 
#    for an FPGA build but place SPAR init at the top of used RAM in the ASIC
#    This will make it easier when using the debugger on the FPGA and allow
#    us to reuse the RAM used up by init after the func call to init from cgs.
# 2. When SPAR is in RAM, we need to only bother about an ASIC build because
#    there is no use case for SPAR_IN=ram on FPGA (though it can be argued that
#    we want it for testing). So, here we need to include SPAR_CRT, override no_spar_setup
#    to no.
# 3. When SPAR is in flash, there are two varients - using the 'romable' app
#    or using a new app implmented in the new spar layer. The first case
#    is similar to spar in ROM while the second is like spar in ram.
# Note: A spar in flash precludes BASE_IN anything other than flash. So,
# a BASE_IN flash works only with SPAR_IN flash whereas a BASE_IN rom
# shall work with a SPAR_IN in rom or ram.
# This is a ROM build
# It is always safe to look at the execution region for RAM area
# but for any NV area (like IROM_BEGIN), we have to look at the end
# of the LOAD region.

# For ASIC, IRAM begin is the top of RAM used by all spars below.
# This spar's CRT if it exists is responsible for moving last used ptr.
IRAM_BEGIN ?= $(shell $(GREP) --perl-regexp --max-count=1 "^[[:space:]]+Execution[[:space:]]Region[[:space:]]first_free_section_in_SRAM[[:space:]].*Base.*[[:space:]]0x[[:alnum:]]" $(ELF_LIST:.elf=.lst) | \
			$(AWK) '{print $$5};' | $(AWK) '{str1=str1 $$0 ""}END{print str1};' | $(SED) -e 's/,//')

IRAM_LENGTH ?= $(shell $(ECHO) $(IRAM_BEGIN) | \
			$(AWK) --non-decimal-data '{printf "0x%08X", ($(SRAM_BEGIN_ADDR) + $(SRAM_LENGTH) - $(PATCH_TABLE_LENGTH) - $$1)};')

ifeq ($(SPAR_IN), flash)
# If SPAR is in flash, IRAM is already setup, need to setup IROM
# Note that we are rounding up to UINT32 closest to 4 bytes above current image just to be safe
  IROM_BEGIN ?= $(shell $(GREP) --perl-regexp --max-count=1 "^[[:space:]]+Load[[:space:]]Region[[:space:]]CM3_Ver1[[:space:]].*Base.*[[:space:]]0x[[:alnum:]].*[[:space:]]Size.*[[:space:]]0x[[:alnum:]]" $(ELF_LIST:.elf=.lst) | \
					$(AWK) '{str1=str1 $$0 ""}END{print str1};' | $(AWK) --non-decimal-data '{printf "0x%08X", and(($$5+$$7+4), 0xFFFFFFFC)};')

  IROM_LEN ?= $(shell $(ECHO) $(IROM_BEGIN) | \
					$(AWK) --non-decimal-data '{printf "0x%08X", ($(FLASH0_BEGIN_ADDR) + $(FLASH0_LENGTH) - $$1)};')

  override IRAM_BEGIN = $(shell $(GREP) --perl-regexp --max-count=1 "^[[:space:]]+Execution[[:space:]]Region[[:space:]]first_free_section_in_SRAM[[:space:]].*Base.*[[:space:]]0x[[:alnum:]]" $(ELF_LIST:.elf=.lst) | \
					$(AWK) '{print $$5};' | $(AWK) '{str1=str1 $$0 ""}END{print str1};' | $(SED) -e 's/,//')

  override IRAM_LENGTH = $(shell $(ECHO) $(IRAM_BEGIN) | \
					$(AWK) --non-decimal-data '{printf "0x%08X", ($(SRAM_BEGIN_ADDR) + $(SRAM_LENGTH) - $(PATCH_TABLE_LENGTH) - $$1)};')

endif

# Now add these locations to the link flags
LINK_LOC_FLAGS += IRAM_BEGIN=$(IRAM_BEGIN) IRAM_LEN=$(IRAM_LENGTH) IROM_BEGIN=$(IROM_BEGIN) IROM_LEN=$(IROM_LEN)

# Add overlay info to linker script if required.
ifneq ($(OVERLAY_SRC),)
  OVERLAY_FLAGS += OVERLAY_AREA_LENGTH=$(OVERLAY_AREA_LENGTH)

  APP_SRC += spar_overlay_manager.c
else
  OVERLAY_FLAGS += OVERLAY_AREAS=
endif

EXTRA_LD_FLAGS += $(addsuffix ", $(addprefix --predefine="-D, $(LINK_LOC_FLAGS)))

################################################################################
# General compiler flags
################################################################################
CPP_FLAGS      += --no_exceptions_unwind --no_rtti --asm --interleave  --diag_suppress=1299,1300,1165,949,504

C_FLAGS += -DBLEAPP --no_depend_system_headers -DCOMPILER_ARM --asm $(CPP_FLAGS) 
C_FLAGS += -DSPAR_CRT_SETUP=$(SPAR_CRT_SETUP) -DSPAR_APP_SETUP=$(SPAR_APP_SETUP)

C_FLAGS += --depend $(basename $@).d
# Compiler list options not applicable to CM3
LIST_OPTIONS = 

# Common C flags
COMMON_FLAGS    = --cpu=$(CPU) --depend_format=unix_escaped --dwarf2 --debug  \
		   			  --diag_suppress=1,14,223,231,550,951,1293,1295,C3017,127,161


C_SPECIFIC_FLAGS   += -c --unsigned_chars --min_array_alignment=4 $(FEEDBACK_OPTIONS)

################################################################################
# General assembler flags
################################################################################
A_SPECIFIC_FLAGS +=--diag_suppress=A1496E,A1581W

################################################################################
# Final linker, compiler and assembler options
################################################################################
C_FLAGS += $(C_SPECIFIC_FLAGS) $(COMMON_FLAGS) $(CPP_FLAGS) $(INCS_OPTS)
ASM_FLAGS += $(COMMON_FLAGS) $(A_SPECIFIC_FLAGS) $(INCS_OPTS)

################################################################################
# Sections extraction, foo.elf -> foo.elf.text.hex etc., to make a CGS
################################################################################
ifneq ($(SPAR_IN), flash)
# enable compressed sections if compression is enabled.
ifeq ($(COMPRESSION_ENABLED),y)
  UNCOMPRESSED_SECTIONS += SPAR_INIT_AREA
  COMPRESSED_SECTIONS += SPAR_CODE_AREA SPAR_DRAM_RW_AREA
else
# We will not extract sections for a flash image, else the only sections we want to extract
# Keep all sections uncompressed by default
  UNCOMPRESSED_SECTIONS += SPAR_INIT_AREA SPAR_CODE_AREA SPAR_DRAM_RW_AREA
  COMPRESSED_SECTIONS +=
endif

ifneq ($(OVERLAY_SRC),)
# Add the overlaid sections to extract

  OVERLAID_SECTIONS += $(addprefix SPAR_OVERLAY_,$(basename $(OVERLAY_SRC)))
endif

endif


################################################################################
# CM3 Overrides
################################################################################

ELF_OUT_LIST = $(ELF_OUT:.elf=.list)

# If spar is in ROM, we don,t need the setup routine
ifeq ($(SPAR_IN), rom)
 NO_SPAR_SETUP = y
endif
