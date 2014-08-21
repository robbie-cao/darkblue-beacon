# Spar make file overrides and definitions for *_20732A0

# This is a CM3 target.
override    COMPILER            := ARM
override    CPU                 := Cortex-M3
override    RTOS                := RTOS_TX_CM3
override    COMPILER_ARM		:= y

# build 591 is broken, use 902 instead
override RVDS_COMPILER_VERSION = arm/rvds40_build902
override RVDS_COMPILER_BUILD   = 902

ifeq ($(SPAR_IN), rom)
# The X0 is what it is, no CRT.
override    SPAR_CRT_SETUP      = $(SPAR_APP_SETUP)
endif

# We dont want USB transport
override 	USB_TRANSPORT 		:= n

APP_PATCH_DIR ?= brcm/mandatory

LMPREV                = 8719

# Define the base chip family name
ifeq ($(BASE_IN),flash)
	BASE_NAME = 20732_flash
else
	BASE_NAME = 20732
endif

# Define 20732 common limits
# Begin address of ROM
IROM_BEGIN_ADDR		= 0x00000000
# Available ROM = 320K
IROM_LENGTH			= 0x00050000
# Begin address of RAM
SRAM_BEGIN_ADDR		= 0x00200000
# Available RAM = 60K
SRAM_LENGTH			= 0x0000F000
# Length of patch table = 256 bytes
PATCH_TABLE_LENGTH	= 256
# Reusable area for spar init - 12K
SPAR_APP_INIT_LENGTH = 12288
# Begin address of flash0
FLASH0_BEGIN_ADDR	= 0x00100000
# Available flash = 320K
#FLASH0_LENGTH		= 0x00050000
# increase size
FLASH0_LENGTH		= 0x00060000

# Patch directory
TIER2_DIR = ../tier2/$(APP_PATCH_DIR)

SYMBOL_TABLE_EXTENSION ?= .st

# If BASE is in ROM, this has to be a patch
ifeq ($(BASE_IN),rom)
  TIER2_ELF := $(TIER2_DIR)/bld/$(CHIP)/patch.elf
  ELF_LIST  += $(TIER2_ELF)
ifeq ($(APP_PATCH_DIR),)
    $(error APP_PATCH_DIR is not defined in Application makefile)
endif
  CGS_LIST  += ../tier2/$(APP_PATCH_DIR)/bld/$(CHIP)/$(notdir $(APP_PATCH_DIR)).cgs
else
  TIER2_ELF:=
  ELF_LIST += ../$(BLD_DIR)/$(BASE_NAME).elf
endif

# Path to the patch symbol table
TIER2_SYMDEFS_FILE := $(TIER2_DIR)/bld/$(CHIP)/patch.symdefs

# Now add the optional libraries and patches this app needs to the final objs to link in.
LIBS += $(addprefix ../tier2/brcm/libraries/lib/$(CHIP)/,$(APP_PATCHES_AND_LIBS))

BLEAPP = y

# Include base chip family definitions
CHIP_FLAGS += BCM20732 BB_20732A1 GPIO_NUMBER_OF_PINS=40 GPIO_MAX_PINS_PER_PORT=16 GPIO_NUMBER_OF_PORTS=3
CHIP_FLAGS += NUM_SPIFFY_BLOCKS=2 DNUM_PATCH_ENTRIES=64 OVERLAY_SUPPORT CONFIG_IN_NVRAM ASIC_BD_2045
CHIP_FLAGS += SERIAL_FLASH

C_FLAGS  += $(addprefix -D,$(CHIP_FLAGS))

# Add a few includes: bleapp incs
INCS += . ../inc ../cfa ../bsp/inc ../bleapp ../bleapp/app ../bleapp/utils ../misc ../../include

# OS incs
INCS += ../rtos/threadx ../rtos/threadx/cm3

# LE includes
#INCS += ../hci ../ulp ../bm ../misc
C_FLAGS += -DBLE_LRK_LIST_SIZE=4

ifeq ($(DEBUG),1)
    C_FLAGS += -DENABLE_ASSERT -DDEBUG
endif

# Stack incs
STACK_DIR		= ../bleapp/lestack
STACK_COMPONENTS = att l2cap blecm gatt profile smp ap

INCS  += $(addprefix $(STACK_DIR)/, $(STACK_COMPONENTS))


# Define patch entries
NUM_PATCH_ENTRIES = 64
PATCH_ENTRY_SIZE  = 4

APP_PATCH_DIR_CGS = $(if $(wildcard ../tier2/$(APP_PATCH_DIR)/bld/$(CHIP)/$(notdir $(APP_PATCH_DIR)).cgs) , "cgs" , "not_found" )
APP_PATCH_DIR_MAKEFILE_INC = $(if $(wildcard ../tier2/$(APP_PATCH_DIR)/makefile.inc) , "makefile.inc" , "not_found" )

### Teach this spar how to make its dependent
# Everything in base dir depends on base elf
../$(BLD_DIR)/%: ../$(BLD_DIR)/$(BASE_NAME).elf
# Base dir depends on base elf
../$(BLD_DIR): ../$(BLD_DIR)/$(BASE_NAME).elf
# this is how you build base elf
../$(BLD_DIR)/$(BASE_NAME).elf:
	$(QUIET)$(MAKE) -C .. MAKEFLAGS=  -j6 BLD=$(BLD) DEBUG=$(DEBUG) LMPREV=$(LMPREV) \
                OS_TARGET_DIR= TOOLSBIN_DOXYGEN_PATH= TOOLSBIN_GRAPHVIZ_PATH= $(BUILD_TOOL_DIR)= TOOLSBIN=


# If we are linking against patches, ensure that patches are there first and then link against them
ifeq ($(SPAR_IN),ram)
# Everything in the patch directory depends on the patch elf
$(TIER2_DIR)/bld/%: $(TIER2_SYMDEFS_FILE)

$(CGS_LIST): $(TIER2_SYMDEFS_FILE)

$(TIER2_ELF): $(TIER2_SYMDEFS_FILE)

$(TIER2_ELF:.elf=.lst): $(TIER2_SYMDEFS_FILE)

$(TIER2_SYMDEFS_FILE):
	$(MAKE) base
endif

../bootsector/obj-A_$(CHIP)$(REVNUM)/bootsector.cgs:
ifeq "$(wildcard ../bootsector/obj-A_$(CHIP)$(REVNUM)/bootsector.cgs)" ""
	$(QUIET)( cd ../bootsector && \
	 $(MAKE) BLD=$(BLD) cgs )
endif
