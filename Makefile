#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#


MAKEFILE_TARGETS := help build download recover clean Release

include $(SOURCE_ROOT)wiced_toolchain_common.mk


.PHONY: $(MAKEFILE_TARGETS)

BASE_LOCATIONS  := BASErom BASEram BASEflash
SPAR_LOCATIONS  := SPARrom SPARram SPARflash
TOOLCHAINS      := RealView Wiced CodeSourcery
BUILD_TYPE_LIST := release

define USAGE_TEXT

Usage: make <target> [download]  [recover] [DEBUG=1|0] [VERBOSE=1] [UART=yyyy] [JOBS=x] [PLATFORM_NV=EEPROM|SFLASH] [BT_DEVICE_ADDRESS=zzzzzzzzzzzz|random]

  <target>
    One each of the following mandatory [and optional] components separated by '-'
      * Application (Apps in sub-directories are referenced by subdir.appname)
      * Hardware Platform ($(filter-out common  include README.txt,$(notdir $(wildcard Platforms/*))))
      * [BASE location] ($(BASE_LOCATIONS))
      * [SPAR location] ($(SPAR_LOCATIONS))
      * [Toolchain] ($(TOOLCHAINS))

  [download]
    Download firmware image to target platform

  [build]
    Builds the firmware and OTA images.

  [recover]
    Recover a corrupted target platform

  [DEBUG=1|0]
    Enable or disable debug code in application. When DEBUG=1, watchdog will be disabled,
    sleep will be disabled and the app may optionally wait in a while(1) for the debugger
    to connect
    
  [VERBOSE=1]
    Shows the commands as they are being executed

  [JOBS=x]
    Sets the maximum number of parallel build threads (default=4)

  [UART=yyyy]
    Use the uart specified here instead of trying to detect the Wiced-Smart device.
    This is useful when you are working on multiple smart devices simultaneously.

  [PLATFORM_NV=EEPROM|SFLASH]
    The non-volatile storage. Default is EEPROM.

  [BT_DEVICE_ADDRESS=zzzzzzzzzzzz|random]
    Use the 48-bit Bluetooth address specified here instead of the default setting from
    Platform/*/*.btp file. The special string 'random' (without the quotes) will generate
    a random Bluetooth device address on every download.

  Notes
    * Component names are case sensitive
    * 'Wiced', 'SPI', 'UART' and 'debug' are reserved component names
    * Component names MUST NOT include space or '-' characters
    * Building for release is assumed unless '-debug' is appended to the target

  Example Usage
    Build for Release
      $> make proximity-BCM920737TAG_Q32 build

    Build, Download and Run using the default programming interface
      $> make proximity-BCM920737TAG_Q32 download

    Build, Download and Run using specific UART port, with a specific Bluetooth decice address
      $> make proximity-BCM920737TAG_Q32 download UART=COMx BT_DEVICE_ADDRESS=20736A1C0FFE

    Build, Download to Serial Flash and Run using default programming interface, select a random Bluetooth device address
      $> make proximity-BCM920737TAG_Q32 download PLATFORM_NV=SFLASH BT_DEVICE_ADDRESS=random

    Clean output directory
      $> make clean

endef


define MAKE_TARGET_HELP

   Please use the targets in the Make Target window to build your application

endef

.PHONY: help

.DEFAULT_GOAL := help

bad_target_specified:
	$(info Invalid target specified)
	$(error $(USAGE_TEXT))


help:
	$(info $(USAGE_TEXT))

.PHONY: Release

Release:
	$(info $(MAKE_TARGET_HELP))

clean:
	$(QUIET)$(ECHO) Cleaning...
	$(QUIET)$(CLEAN_COMMAND)
	$(QUIET)$(RM) -rf .gdbinit
	$(QUIET)$(ECHO) Done

build: $(BUILD_STRING) convert_cgs_to_hex create_ota_image

ifneq ($(UART), )
UART:=$(shell $(ECHO) $(UART) > com_port.txt)) 
download: build download_using_chipload
else
download: build find_com_port download_using_chipload
endif

recover:  build recover_using_chipload

find_com_port:
	$(QUIET)$(ECHO_BLANK_LINE)
	$(QUIET)$(ECHO) Detecting device...
	$(QUIET)$(call CONV_SLASHES,$(PERL)) $(TOOLS_ROOT)/get_com_port/get_com_port.pl $(DETECTANDID_FULL_NAME) $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM_IDFILE) $(SOURCE_ROOT)com_port.txt > build/$(OUTPUT_NAME)/detect.log 2>&1 \
	&& $(ECHO) Device found \
	|| $(call CONV_SLASHES,$(PERL)) $(TOOLS_ROOT)/print_file/print_file.pl  $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/find_com_port_error.txt
	$(QUIET)$(ECHO_BLANK_LINE)

convert_cgs_to_hex:
	$(QUIET)$(ECHO_BLANK_LINE)
	$(QUIET)$(ECHO) Converting CGS to HEX...
	$(QUIET)$(call CONV_SLASHES,$(CGS_FULL_NAME)) -D $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL) $(BT_DEVICE_ADDRESS_OVERRIDE) -A $(PLATFORM_STORAGE_BASE_ADDR) -B $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM_BOOTP) -I build/$(OUTPUT_NAME)/$(OUTPUT_NAME).hex --cgs-files build/$(OUTPUT_NAME)/$(BLD)-$(APP)-$(BASE_LOC)-$(SPAR_LOC)-spar.cgs > build/$(OUTPUT_NAME)/cgs2hex.log 2>&1 && $(ECHO) Conversion complete || $(ECHO) **** Conversion failed ****

create_ota_image:
	$(QUIET)$(ECHO_BLANK_LINE)
	$(QUIET)$(ECHO) Creating OTA images...
	$(QUIET)$(call CONV_SLASHES,$(CGS_FULL_NAME)) -D $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL) -O DLConfigFixedHeader:0 -B $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM_BOOTP) -I build/$(OUTPUT_NAME)/$(OUTPUT_NAME).ota.hex --cgs-files build/$(OUTPUT_NAME)/$(BLD)-$(APP)-$(BASE_LOC)-$(SPAR_LOC)-spar.cgs > build/$(OUTPUT_NAME)/cgs2hex.log 2>&1 && $(ECHO) Conversion complete || $(ECHO) **** Conversion failed ****
	$(QUIET)$(call CONV_SLASHES,$(HEX_TO_BIN_FULL_NAME)) build/$(OUTPUT_NAME)/$(OUTPUT_NAME).ota.hex build/$(OUTPUT_NAME)/$(OUTPUT_NAME).ota.bin
	$(QUIET)$(call CONV_SLASHES,$(PERL)) -e '$$s=-s "build/$(OUTPUT_NAME)/$(OUTPUT_NAME).ota.bin";printf "OTA image footprint in NV is %d bytes",$$s;'
	$(QUIET)$(ECHO_BLANK_LINE)

download_using_chipload:
	$(QUIET)$(ECHO_BLANK_LINE)
	$(QUIET)$(eval UART:=$(shell $(CAT) < com_port.txt))
	$(QUIET)$(if $(UART), \
	        	$(ECHO) Downloading application... && $(call CONV_SLASHES,$(CHIPLOAD_FULL_NAME)) -BLUETOOLMODE -PORT $(UART) -BAUDRATE $(PLATFORM_BAUDRATE) -MINIDRIVER $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM_MINIDRIVER) -BTP $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM_BOOTP) -CONFIG build/$(OUTPUT_NAME)/$(OUTPUT_NAME).hex -CHECKCRC -NOVERIFY -DLMINIDRIVERCHUNKSIZE 251 > build/$(OUTPUT_NAME)/download.log 2>&1 \
	        		&& $(ECHO) Download complete && $(ECHO_BLANK_LINE) && $(ECHO) $(QUOTES_FOR_ECHO)Move DIP switch 2 of SW4 to off position and push Reset button to start application$(QUOTES_FOR_ECHO) \
	        		|| $(ECHO) '****Download failed - Press the reset button on the device and retry ****', \
	        	$(ECHO) Download failed. This version of the SDK only supports download to BCM20736A1 and BCM20737A1 devices)

recover_using_chipload:
	$(QUIET)$(ECHO_BLANK_LINE)
	$(QUIET)$(ECHO) Recovering platform ...
	$(QUIET)$(call CONV_SLASHES,$(CHIPLOAD_FULL_NAME)) -BLUETOOLMODE -PORT $(UART) -BAUDRATE $(PLATFORM_BAUDRATE) -NODLMINIDRIVER -MINIDRIVER $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM_MINIDRIVER) -BTP $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM_BOOTP) -CONFIG build/$(OUTPUT_NAME)/$(OUTPUT_NAME).hex -LOGTO build/$(OUTPUT_NAME)/$(OUTPUT_NAME).log -CHECKCRC -NOVERIFY -DLMINIDRIVERCHUNKSIZE 251 > build/$(OUTPUT_NAME)/download.log 2>&1 && $(ECHO) Recovery complete && $(ECHO_BLANK_LINE) && $(ECHO) $(QUOTES_FOR_ECHO)Move DIP switch 2 of SW4 to off position and push Reset button to start application$(QUOTES_FOR_ECHO) || $(ECHO) $(QUOTES_FOR_ECHO)**** Recovery failed - retry ****$(QUOTES_FOR_ECHO)


ifneq ($(DIR_BUILD_STRING),)

# Separate the build string into components
COMPONENTS := $(subst -, ,$(DIR_BUILD_STRING))
BASE_LOC  ?= rom

#Find app and platform
PLATFORM_FULL :=$(strip $(foreach comp,$(COMPONENTS),$(if $(wildcard $(SOURCE_ROOT)Platforms/$(comp)),$(comp),)))
APP_FULL      :=$(strip $(foreach comp,$(COMPONENTS),$(if $(wildcard $(SOURCE_ROOT)Apps/$(comp)),$(comp),)))
PLATFORM      :=$(notdir $(PLATFORM_FULL))
APP           :=$(notdir $(APP_FULL))

ifneq ($(filter $(COMPONENTS),$(BASE_LOCATIONS)),)
BASE_LOC      :=$(strip $(subst BASE,,$(filter $(COMPONENTS),$(BASE_LOCATIONS))))
endif
ifneq ($(filter $(COMPONENTS),$(SPAR_LOCATIONS)),)
SPAR_LOC      :=$(strip $(subst SPAR,,$(filter $(COMPONENTS),$(SPAR_LOCATIONS))))
endif
ifneq ($(filter $(COMPONENTS),$(TOOLCHAINS)),)
TOOLCHAIN     :=$(strip $(filter $(COMPONENTS),$(TOOLCHAINS)))
endif
BUILD_TYPE    :=$(strip $(filter $(COMPONENTS),$(BUILD_TYPE_LIST)))


ifeq ($(wildcard $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM).mk),)
$(error Platform makefile not found: $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM).mk)
endif

include $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM).mk

ifeq ($(CHIP)$(CHIP_REV),)
$(error CHIP not defined in Platform makefile $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM).mk)
endif

ifeq ($(BASE_LOC),)
$(error BASE Location not defined  in command-line target or in Platform makefile $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM).mk)
endif

ifeq ($(SPAR_LOC),)
$(error SPAR Location not defined  in command-line target or in Platform makefile $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM).mk)
endif


ifeq ($(TOOLCHAIN),)
$(error Toolchain not defined in command-line target or in Platform makefile $(SOURCE_ROOT)Platforms/$(PLATFORM_FULL)/$(PLATFORM).mk)
endif

ifeq ($(BUILD_TYPE),)
#default to release build
BUILD_TYPE := release
endif


JOBS ?=4
ifeq (,$(SUB_BUILD))
JOBSNO := $(if $(findstring 1,$(LINT)), , -j$(JOBS) )
endif

OUTPUT_NAME                := $(APP)-$(PLATFORM)-$(BASE_LOC)-$(SPAR_LOC)-$(TOOLCHAIN)-$(BUILD_TYPE)

BIN_OUT_DIR                := ../../build/$(OUTPUT_NAME)

# Include SDK include directories
SDK_INC_FLAGS              :=../../include/ ../../include/Platforms/$(PLATFORM)/ ../../include/Drivers/ ../../include/Drivers/$(CHIP)

# Override the BD_ADDR parameter if provided on the command line.
ifeq ($(BT_DEVICE_ADDRESS),random)
BT_DEVICE_ADDRESS_OVERRIDE := -O DLConfigBD_ADDRBase:$(shell $(call CONV_SLASHES,$(PERL)) -e '$$r=int(rand(32768));printf "$(CHIP)$(CHIP_REV)00%X",$$r;')
else
ifneq ($(BT_DEVICE_ADDRESS),)
BT_DEVICE_ADDRESS_OVERRIDE := -O DLConfigBD_ADDRBase:$(BT_DEVICE_ADDRESS)
endif
endif

export BIN_OUT_DIR
export VERBOSE
export SDK_INC_FLAGS

ifeq ($(TOOLCHAIN),Wiced)
GCC_FLAGS :=$(call CONV_SLASHES,-isystem ../../Tools/ARM_GNU/include -isystem ../../Tools/ARM_GNU/lib/include -isystem ../../Tools/ARM_GNU/lib/include-fixed)
export GCC_FLAGS
TC :=wiced
GCC_TOOL_DIR=$(call CONV_SLASHES,../../Tools/ARM_GNU/bin/$(HOST_OS)/)

else
ifeq ($(TOOLCHAIN),RealView)
TC :=rv
else
ifeq ($(TOOLCHAIN),CodeSourcery)
TC :=gcc
else
$(error Toolchain $(TOOLCHAIN) has no mapping to a name for ADK)
endif
endif
endif

ifeq ($(GCC_TOOL_DIR),)
$(error Toolchain not found - implement wildcard macro to find toolchain and define GCC_TOOL_DIR )
endif

export TC
export GCC_TOOL_DIR

BLD :=A_$(CHIP)$(CHIP_REV)
export BLD


TOOLSBIN=$(call CONV_SLASHES,../../$(subst $(SOURCE_ROOT),,$(COMMON_TOOLS_PATH)))
export TOOLSBIN

BASE_IN :=$(BASE_LOC)
SPAR_IN :=$(SPAR_LOC)
export BASE_IN
export SPAR_IN


DIR := ../../Apps/$(APP_FULL)

export DIR

CGS_LIST := $(addprefix ../../Platforms/$(PLATFORM)/,$(PLATFORM_CONFIGS))
export CGS_LIST

COMPRESSOR := $(call CONV_SLASHES,../../$(LZSS_FULL_PATH))
export COMPRESSOR

ifeq ($(PLATFORM_NV),EEPROM)
COMPRESSION_ENABLED ?= y
export COMPRESSION_ENABLED
endif

export CONFIG_DEFINITION

export PLATFORM_FULL
export PLATFORM_NV
export DEBUG

PLATFORM_CGS_PATH := $(addprefix ../../Platforms/$(PLATFORM)/,$(PLATFORM_CONFIGS))
export PLATFORM_CGS_PATH

ifeq ($(VERBOSE),1)
$(info ADK parameters:)
$(info GCC_TOOL_DIR="$(GCC_TOOL_DIR)")
$(info GCC_FLAGS="$(GCC_FLAGS)")
$(info TC="$(TC)")
$(info DIR="$(DIR)")
$(info BLD="$(BLD)")
$(info TOOLSBIN="$(TOOLSBIN)")
$(info BASE_IN="$(BASE_IN)")
$(info SPAR_IN="$(SPAR_IN)")
$(info BIN_OUT_DIR="$(BIN_OUT_DIR)")
$(info CGS_LIST="$(CGS_LIST)")
$(info CONFIG_DEFINITION="$(CONFIG_DEFINITION)")
$(info PLATFORM_CGS_PATH="$(PLATFORM_CGS_PATH)")
$(info APP_PATCH_DIR="$(APP_PATCH_DIR)")
$(info PLATFORM_NV="$(PLATFORM_NV)")
$(info DEBUG="$(DEBUG)")
endif

$(BUILD_STRING):
	$(QUIET)$(MAKE) -C $(SOURCE_ROOT)Wiced-Smart/spar $(SILENT) $(JOBSNO) cgs
	$(QUIET)$(call MKDIR,$(SOURCE_ROOT)build/eclipse_debug/)
	$(QUIET)$(COPY) $(SOURCE_ROOT)build/$(OUTPUT_NAME)/$(BLD)-$(APP)-$(BASE_LOC)-$(SPAR_LOC)-spar.elf $(SOURCE_ROOT)build/eclipse_debug/last_built.elf

endif
