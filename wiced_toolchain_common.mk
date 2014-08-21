#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

TOOLS_ROOT ?= $(SOURCE_ROOT)Tools

OPENOCD_PATH     := $(TOOLS_ROOT)/OpenOCD/
CGS_PATH         := $(TOOLS_ROOT)/CGS/
CHIPLOAD_PATH    := $(TOOLS_ROOT)/ChipLoad/
DETECTANDID_PATH := $(TOOLS_ROOT)/DetectAndId/
HEX_TO_BIN_PATH  := $(TOOLS_ROOT)/IntelHexToBin/
LZSS_PATH        := $(TOOLS_ROOT)/lzss/
PATH :=

JTAG         ?= BCM9WCD1EVAL1


ifeq ($(HOST_OS),Win32)
################
# Windows settings
################
COMMON_TOOLS_PATH     := $(TOOLS_ROOT)/common/Win32/
export SHELL           = cmd.exe
EXECUTABLE_SUFFIX     := .exe
OPENOCD_FULL_NAME     := $(OPENOCD_PATH)Win32/openocd-all-brcm-libftdi$(EXECUTABLE_SUFFIX)
CGS_FULL_NAME         := $(CGS_PATH)Win32/cgs$(EXECUTABLE_SUFFIX)
CHIPLOAD_FULL_NAME    := $(CHIPLOAD_PATH)Win32/ChipLoad$(EXECUTABLE_SUFFIX)
DETECTANDID_FULL_NAME := $(DETECTANDID_PATH)Win32/DetAndId$(EXECUTABLE_SUFFIX)
HEX_TO_BIN_FULL_NAME  := $(HEX_TO_BIN_PATH)Win32/ihex2bin$(EXECUTABLE_SUFFIX)
LZSS_FULL_PATH        := $(LZSS_PATH)Win32/lzss$(EXECUTABLE_SUFFIX)
PRINT_SLASH :=$\\
SLASH_QUOTE :=\"
ESC_QUOTE:="
ESC_SPACE:=$(SPACE)
CAT               := "$(COMMON_TOOLS_PATH)cat$(EXECUTABLE_SUFFIX)"
ECHO_BLANK_LINE   := "$(COMMON_TOOLS_PATH)echo$(EXECUTABLE_SUFFIX)"
ECHO_NO_NEWLINE   := "$(COMMON_TOOLS_PATH)echo$(EXECUTABLE_SUFFIX)" -n
ECHO              := "$(COMMON_TOOLS_PATH)echo$(EXECUTABLE_SUFFIX)"
QUOTES_FOR_ECHO   :="
COPY              := "$(COMMON_TOOLS_PATH)cp$(EXECUTABLE_SUFFIX)"
CMD_TRUNC         := "$(COMMON_TOOLS_PATH)trunc$(EXECUTABLE_SUFFIX)"
PERL              := "$(COMMON_TOOLS_PATH)perl$(EXECUTABLE_SUFFIX)"
PERL_ESC_DOLLAR   :=$$
CLEAN_COMMAND     := if exist build $(call CONV_SLASHES,$(COMMON_TOOLS_PATH))rmdir /s /q build
MKDIR              = if not exist $(subst /,\,$1) mkdir $(subst /,\,$1)
CONV_SLASHES       = $(subst /,\,$1)
DIR                = $(dir $(subst /,\,$1))
TOUCH              = $(ECHO) >
CYGWIN :=

# $(1) is the content, $(2) is the file to print to.
define PRINT
@$(ECHO) $(1)>>$(2)

endef


else  # Win32
ifeq ($(HOST_OS),Linux32)
################
# Linux 32-bit settings
################

COMMON_TOOLS_PATH := $(TOOLS_ROOT)/common/Linux32/
export SHELL       = $(COMMON_TOOLS_PATH)dash
EXECUTABLE_SUFFIX  :=
OPENOCD_FULL_NAME := "$(OPENOCD_PATH)Linux32/openocd-all-brcm-libftdi"
CGS_FULL_NAME         := $(CGS_PATH)Linux32/cgs$(EXECUTABLE_SUFFIX)
CHIPLOAD_FULL_NAME    := $(CHIPLOAD_PATH)Linux32/ChipLoad$(EXECUTABLE_SUFFIX)
DETECTANDID_FULL_NAME := $(DETECTANDID_PATH)Linux32/DetAndId$(EXECUTABLE_SUFFIX)
HEX_TO_BIN_FULL_NAME  := $(HEX_TO_BIN_PATH)Linux32/ihex2bin$(EXECUTABLE_SUFFIX)
LZSS_FULL_PATH        := $(LZSS_PATH)Linux32/lzss$(EXECUTABLE_SUFFIX)
PRINT_SLASH       :=\\\\
SLASH_QUOTE       :=\\\"
ESC_QUOTE         :=\"
ESC_SPACE         :=\$(SPACE)
CAT               := "$(COMMON_TOOLS_PATH)cat"
ECHO_BLANK_LINE   := "$(COMMON_TOOLS_PATH)echo"
ECHO_NO_NEWLINE   := "$(COMMON_TOOLS_PATH)echo" -n
ECHO              := "$(COMMON_TOOLS_PATH)echo"
QUOTES_FOR_ECHO   :="
COPY              := "$(COMMON_TOOLS_PATH)cp$(EXECUTABLE_SUFFIX)"
CMD_TRUNC         := $(ECHO)
PERL              := "$(COMMON_TOOLS_PATH)perl"
PERL_ESC_DOLLAR   :=\$$
CLEAN_COMMAND     := "$(COMMON_TOOLS_PATH)rm" -rf build
MKDIR              = "$(COMMON_TOOLS_PATH)mkdir$(EXECUTABLE_SUFFIX)" -p $1
CONV_SLASHES       = $1
TOUCH              = $(ECHO) >

# $(1) is the content, $(2) is the file to print to.
define PRINT
@$(ECHO) '$(1)'>>$(2)

endef

else # Linux32
ifeq ($(HOST_OS),Linux64)
################
# Linux 64-bit settings
################

COMMON_TOOLS_PATH := $(TOOLS_ROOT)/common/Linux64/
export SHELL       = $(COMMON_TOOLS_PATH)dash
EXECUTABLE_SUFFIX  :=
OPENOCD_FULL_NAME := "$(OPENOCD_PATH)Linux64/openocd-all-brcm-libftdi"
CGS_FULL_NAME         := $(CGS_PATH)Linux64/cgs$(EXECUTABLE_SUFFIX)
CHIPLOAD_FULL_NAME    := $(CHIPLOAD_PATH)Linux64/ChipLoad$(EXECUTABLE_SUFFIX)
DETECTANDID_FULL_NAME := $(DETECTANDID_PATH)Linux64/DetAndId$(EXECUTABLE_SUFFIX)
HEX_TO_BIN_FULL_NAME  := $(HEX_TO_BIN_PATH)Linux64/ihex2bin$(EXECUTABLE_SUFFIX)
LZSS_FULL_PATH        := $(LZSS_PATH)Linux64/lzss$(EXECUTABLE_SUFFIX)
PRINT_SLASH       :=\\\\
SLASH_QUOTE       :=\\\"
ESC_QUOTE         :=\"
ESC_SPACE         :=\$(SPACE)
CAT               := "$(COMMON_TOOLS_PATH)cat"
ECHO_BLANK_LINE   := "$(COMMON_TOOLS_PATH)echo"
ECHO_NO_NEWLINE   := "$(COMMON_TOOLS_PATH)echo" -n
ECHO              := "$(COMMON_TOOLS_PATH)echo"
QUOTES_FOR_ECHO   :="
COPY              := "$(COMMON_TOOLS_PATH)cp$(EXECUTABLE_SUFFIX)"
CMD_TRUNC         := $(ECHO)
PERL              := "$(COMMON_TOOLS_PATH)perl"
PERL_ESC_DOLLAR   :=\$$
CLEAN_COMMAND     := "$(COMMON_TOOLS_PATH)rm" -rf build
MKDIR              = "$(COMMON_TOOLS_PATH)mkdir$(EXECUTABLE_SUFFIX)" -p $1
CONV_SLASHES       = $1
TOUCH              = $(ECHO) >

# $(1) is the content, $(2) is the file to print to.
define PRINT
@$(ECHO) '$(1)'>>$(2)

endef

else # Linux64
ifeq ($(HOST_OS),OSX)
################
# OSX settings
################

COMMON_TOOLS_PATH := $(TOOLS_ROOT)/common/OSX/
export SHELL       = $(COMMON_TOOLS_PATH)dash
EXECUTABLE_SUFFIX  :=
OPENOCD_FULL_NAME := "$(OPENOCD_PATH)OSX/openocd-all-brcm-libftdi"
CGS_FULL_NAME         := $(CGS_PATH)OSX/cgs$(EXECUTABLE_SUFFIX)
CHIPLOAD_FULL_NAME    := $(CHIPLOAD_PATH)OSX/ChipLoad$(EXECUTABLE_SUFFIX)
DETECTANDID_FULL_NAME := $(DETECTANDID_PATH)OSX/DetAndId$(EXECUTABLE_SUFFIX)
HEX_TO_BIN_FULL_NAME  := $(HEX_TO_BIN_PATH)OSX/ihex2bin$(EXECUTABLE_SUFFIX)
LZSS_FULL_PATH        := $(LZSS_PATH)OSX/lzss$(EXECUTABLE_SUFFIX)
PRINT_SLASH       :=\\\\
SLASH_QUOTE       :=\\\"
ESC_QUOTE         :=\"
ESC_SPACE         :=\$(SPACE)
CAT               := "$(COMMON_TOOLS_PATH)cat"
ECHO_BLANK_LINE   := "$(COMMON_TOOLS_PATH)echo"
ECHO_NO_NEWLINE   := "$(COMMON_TOOLS_PATH)echo" -n
ECHO              := "$(COMMON_TOOLS_PATH)echo"
QUOTES_FOR_ECHO   :="
COPY              := "$(COMMON_TOOLS_PATH)cp$(EXECUTABLE_SUFFIX)"
CMD_TRUNC         := $(ECHO)
PERL              := "$(COMMON_TOOLS_PATH)perl"
PERL_ESC_DOLLAR   :=\$$
CLEAN_COMMAND     := "$(COMMON_TOOLS_PATH)rm" -rf build
MKDIR              = "$(COMMON_TOOLS_PATH)mkdir" -p $1
CONV_SLASHES       = $1
TOUCH              = $(ECHO) >
# $(1) is the content, $(2) is the file to print to.
define PRINT
@$(ECHO) '$(1)'>>$(2)

endef

else # OSX

$(error incorrect 'make' used ($(MAKE)) - please use:  (Windows) .\make.exe <target_string>    (OS X, Linux) ./make <target_string>)
endif # OSX
endif # Linux64
endif # Linux32
endif # Win32


# Set shortcuts to the compiler and other tools
RM      := "$(COMMON_TOOLS_PATH)rm$(EXECUTABLE_SUFFIX)" -f
CP      := "$(COMMON_TOOLS_PATH)cp$(EXECUTABLE_SUFFIX)" -f
MAKE    := "$(COMMON_TOOLS_PATH)make$(EXECUTABLE_SUFFIX)"
BIN2C   := "$(COMMON_TOOLS_PATH)bin2c$(EXECUTABLE_SUFFIX)"


SHOULD_I_WAIT_FOR_DOWNLOAD := $(filter download, $(MAKECMDGOALS))
CONV_COMP =$(subst /,_,$(1))
BUILD_STRING ?= $(strip $(firstword $(filter-out $(MAKEFILE_TARGETS),$(MAKECMDGOALS))))
DIR_BUILD_STRING := $(BUILD_STRING)
CLEANED_BUILD_STRING := $(call CONV_COMP,$(DIR_BUILD_STRING))
BUILD_DIR    :=  build

OUTPUT_DIR   := $(BUILD_DIR)/$(CLEANED_BUILD_STRING)

ifeq ($(MAKE_VERSION),Development-Git-29cd1e9699d1101444920827b412191e0f92e1e1)
WRITE_FILE_CREATE =$(file >$(1),$(2))
WRITE_FILE_APPEND =$(file >>$(1),$(2))
else
WRITE_FILE_CREATE =$(ECHO) "$(subst \\!!,\\",$(subst ",\\",$(subst \\",\\!!,$(2))))" > $(1);
WRITE_FILE_APPEND =$(ECHO) "$(subst \\!!,\\",$(subst ",\\",$(subst \\",\\!!,$(2))))" >> $(1);
endif

# Truncates a string to the maximum length that can be handled by the shell command line
# $(1)=the string to truncate
CMD_LIMIT=$(patsubst !+%+!,%,$(filter !+%+!,$(shell $(CMD_TRUNC) $(patsubst %,!+%+!,$(1)))))

# Newline Macro
define newline


endef

# Makes a list of shell commands with each size limited to fit in the line-length limit of the shell
# $(1)= front constant part of command  $(2)=the dynamic part of command (e.g. filename list)  $(3)= end constant part of command
define BIG_CMD
$(eval BC_MAX_CMD:=$(call CMD_LIMIT,$(1) $(subst >,x,$(3)) $(2)))
$(eval BC_MAX_CMD_WORDS:=$(words $(subst $(1) $(subst >,x,$(3)),,$(BC_MAX_CMD))))
$(eval BC_STAT_CMD:=$(words $(1) $(3)))
$(if $(2), @$(1)$(wordlist 1,$(BC_MAX_CMD_WORDS),$(2))$(3)$(newline)$(or $(eval BCTEMP:= $(wordlist 2,999999999,$(wordlist $(BC_MAX_CMD_WORDS),99999999,$(2) ) )), $(call BIG_CMD,$(1),$(BCTEMP),$(3)) ), )
endef

# Use VERBOSE=1 to get full output
ifneq ($(VERBOSE),1)
QUIET:=@
SILENT:=-s
else
QUIET:=
SILENT:=
endif
export VERBOSE
export QUIET
export SILENT



COMMA :=,

SPACE :=
SPACE +=

# $(1) is a string to be escaped
ESCAPE_BACKSLASHES =$(subst \,\\,$(1))
