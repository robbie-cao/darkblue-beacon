# This contains the CM3 varient makefile customizations for GCC

# We link with the base elf files
LINK_ELFS 			:= $(addprefix --just-symbols=,$(ELF_LIST))
MAKE_ASSEMBLY       = $(FROMELF)  --disassemble $@ > $(@:.elf=.asm)
################################################################################
# General link flags
################################################################################
# Disable exceptions, datacompressor and some useless warnings
EXTRA_LD_FLAGS += -nostartfiles -nodefaultlibs -EL
EXTRA_LD_FLAGS += --cref --gc-sections
EXTRA_LD_FLAGS +=

# Pre-processed
SPAR_LINK_LOAD_FILE = $(BIN_OUT_DIR)/spar_$(SPAR_IN)_proc.ld

# Disable VFE, stdlibs and setup entry to spar_setup.
EXTRA_LD_FLAGS += -T $(SPAR_LINK_LOAD_FILE) -Map $(ELF_OUT:.elf=.list) -O2
EXTRA_LD_FLAGS += --entry=$(SPAR_CRT_SETUP) -z muldefs

ifneq ($(NEED_STD_LIBS), y)
  EXTRA_LD_FLAGS += -nostdlib
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
IRAM_BEGIN_CMD  := $(PERL) -nle 'printf( "0x%08X", hex($$1) )  if /^[[:space:]]+Execution[[:space:]]Region[[:space:]]first_free_section_in_SRAM[[:space:]].*Base:[[:space:]]+(0x[[:alnum:]]+)/'                                   $(ELF_LIST:.elf=.lst)
IRAM_LEN_CMD    := $(PERL) -nle 'printf( "0x%08X", (0x00200000 + 0x0000F000 - 256 - hex($$1)) )  if /^[[:space:]]+Execution[[:space:]]Region[[:space:]]first_free_section_in_SRAM[[:space:]].*Base:[[:space:]]+(0x[[:alnum:]]+)/' $(ELF_LIST:.elf=.lst)

SPAR_LOC_PREREQ += $(ELF_LIST:.elf=.lst)

ifeq ($(SPAR_IN), flash)
# If SPAR is in flash, IRAM is already setup, need to setup IROM
# Note that we are rounding up to UINT32 closest to 4 bytes above current image just to be safe
  IROM_BEGIN_CMD := $(PERL) -nle 'printf( "0x%08X", hex($$1) + hex($$2) + 4 )  if /^[[:space:]]+Load[[:space:]]Region[[:space:]]CM3_Ver1[[:space:]].*Base.*[[:space:]](0x[[:alnum:]]+).*[[:space:]]Size\:[[:space:]]+(0x[[:alnum:]]+)/'                                            $(ELF_LIST:.elf=.lst)
  IROM_LEN_CMD   := $(PERL) -nle 'printf( "0x%08X", ($(FLASH0_BEGIN_ADDR) + $(FLASH0_LENGTH) - hex($$1) - hex($$2) - 4))  if /^[[:space:]]+Load[[:space:]]Region[[:space:]]CM3_Ver1[[:space:]].*Base.*[[:space:]](0x[[:alnum:]]+).*[[:space:]]Size\:[[:space:]]+(0x[[:alnum:]]+)/' $(ELF_LIST:.elf=.lst)


  IRAM_BEGIN_CMD := $(PERL) -nle 'printf( "0x%08X", hex($$1_ )   if /^[[:space:]]+Execution[[:space:]]Region[[:space:]]first_free_section_in_SRAM[[:space:]].*Base:[[:space:]]+(0x[[:alnum:]]+)/'                                                                 $(ELF_LIST:.elf=.lst)
  IRAM_LEN_CMD   := $(PERL) -nle 'printf( "0x%08X", ($(SRAM_BEGIN_ADDR) + $(SRAM_LENGTH) - $(PATCH_TABLE_LENGTH) - hex($$1)) )   if /^[[:space:]]+Execution[[:space:]]Region[[:space:]]first_free_section_in_SRAM[[:space:]].*Base:[[:space:]]+(0x[[:alnum:]]+)/' $(ELF_LIST:.elf=.lst)

  SPAR_LOC_PREREQ += $(ELF_LIST:.elf=.lst)
else
  IROM_BEGIN_CMD := $(ECHO) ""
  IROM_LEN_CMD := $(ECHO) ""
endif

IROM_BEGIN_CMD :=$(call PERL_FIX_QUOTES,$(IROM_BEGIN_CMD))
IROM_LEN_CMD   :=$(call PERL_FIX_QUOTES,$(IROM_LEN_CMD))
IRAM_BEGIN_CMD :=$(call PERL_FIX_QUOTES,$(IRAM_BEGIN_CMD))
IRAM_LEN_CMD   :=$(call PERL_FIX_QUOTES,$(IRAM_LEN_CMD))

# Add overlay info to linker script if required.
ifneq ($(OVERLAY_SRC),)
  OVERLAY_FLAGS += OVERLAY_AREA_LENGTH=$(OVERLAY_AREA_LENGTH)
  APP_SRC += spar_overlay_manager.c
else
  OVERLAY_FLAGS += OVERLAY_AREAS=
endif

################################################################################
# General compiler flags
################################################################################
# CPP_FLAGS      += --no_exceptions_unwind --no_rtti --no_rtti_data --asm --interleave  --diag_suppress=1299,1300,1165,949,504
CPP_FLAGS      += -fno-exceptions -fno-rtti

C_FLAGS += -DBLEAPP

# Compiler list options not applicable to CM3
LIST_OPTIONS =

# Common C flags
COMMON_FLAGS    =  -c -DCOMPILER_ARM -mlittle-endian -mcpu=cortex-m3 -mthumb -Os -g -Wa,-adhln \
					  -ffreestanding -DSPAR_CRT_SETUP=$(SPAR_CRT_SETUP) -DSPAR_APP_SETUP=$(SPAR_APP_SETUP)

# Add some more hacky flags
COMMON_FLAGS += -D__TARGET_CPU_CORTEX_M3 -D__ARMCC_VERSION=220438 -ffunction-sections


# C_SPECIFIC_FLAGS   += -c --unsigned_chars --min_array_alignment=4 $(FEEDBACK_OPTIONS)
C_SPECIFIC_FLAGS   += -funsigned-char -fshort-wchar

################################################################################
# General assembler flags
################################################################################
# A_SPECIFIC_FLAGS +=--diag_suppress=A1496E,A1581W
A_SPECIFIC_FLAGS +=

################################################################################
# Final linker, compiler and assembler options
################################################################################
C_FLAGS += $(C_SPECIFIC_FLAGS) $(COMMON_FLAGS) $(INCS_OPTS)
ASM_FLAGS += $(COMMON_FLAGS) $(A_SPECIFIC_FLAGS) $(INCS_OPTS)

################################################################################
# Sections extraction, foo.elf -> foo.elf.text.hex etc., to make a CGS
################################################################################
ifneq ($(SPAR_IN), flash)
# enable compressed sections if compression is enabled.
ifeq ($(COMPRESSION_ENABLED),y)
  UNCOMPRESSED_SECTIONS +=
  COMPRESSED_SECTIONS += .text .rodata .data .setup
else
# We will not extract sections for a flash image, else the only sections we want to extract
# Keep all sections uncompressed by default
  UNCOMPRESSED_SECTIONS +=  .text .rodata .data .setup

  COMPRESSED_SECTIONS +=
endif

 ifneq ($(OVERLAY_SRC),)
  # Add the overlaid sections to extract
  OVERLAID_SECTIONS += $(addprefix .,$(basename $(OVERLAY_SRC)))
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
