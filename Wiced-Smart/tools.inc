# Include makefile.env -- You can include tools.inc from anywhere and
# it will still find makefile.env (which sits in the same directory)
# $(lastword a b c) doesn't work at least on Make 3.80, so use word+words

THIS_DIR := $(dir $(word $(words $(MAKEFILE_LIST)), $(MAKEFILE_LIST)))

ifndef TOOLS_DIR
    TOOLS_DIR = $(THIS_DIR)../fwtools
endif

TC ?= rv

ifeq ($(TC),gcc)
  override RVDS_TOOL_DIR=
  override TOOLSBIN=
endif

ifeq ($(TC),wiced)
  override RVDS_TOOL_DIR=
  override TC :=gcc
endif

ifneq ($(wildcard $(dir $(MAKE))make.exe),)
PERL_FIX_QUOTES = $(subst !@%^,',$(subst ',",$(subst ",!@%^,$(1))))
else
PERL_FIX_QUOTES = $(1)
endif

include $(THIS_DIR)makefile.env

# Assume toolchain is RealView (other option is gcc)
ifeq ($(TC),rv)
################################################################################
# Build tools for spar application
################################################################################
# We are using RealView
  COMPILER_TOOL_DIR   := $(RVDS_TOOL_DIR)
  ACC                 := $(COMPILER_TOOL_DIR)armcc
  XCC                 := $(ACC)
  AAS                 := $(COMPILER_TOOL_DIR)armasm
  XAS                 := $(AAS)
  XAR                 := $(COMPILER_TOOL_DIR)armar
  XAROPT              := -rc 
  XLD                 := $(COMPILER_TOOL_DIR)armlink
  FROMELF             := $(COMPILER_TOOL_DIR)fromelf
  FROMELF_SYMTABOPTS  := --text -s
  HEXCOPY			  := $(FROMELF) --text -c -t --i32
  SHOWSIZEOPT         :=  -z 
  BUILD_TOOL_DIR 	  := RVDS_COMPILER_VERSION
  SPAR_SCRIPTS_DIR     := scripts/
  OBJCOPY             := $(SPAR_SCRIPTS_DIR)objcpy.pl
  OBJDUMP             := $(SPAR_SCRIPTS_DIR)objcpy.pl
  SECTION_TO_CGS      := $(SPAR_SCRIPTS_DIR)section_to_cgs.pl
  GEN_OVERLAY_FILES   := $(SPAR_SCRIPTS_DIR)generateoverlayfiles.pl
  GEN_LIB_INSTALLER   := $(SPAR_SCRIPTS_DIR)generatelibinstaller.pl
else
# We are using GCC
  COMPILER_TOOL_DIR   := $(GCC_TOOL_DIR)
  ACC                 := $(GCC_TOOL_DIR)arm-none-eabi-gcc $(GCC_FLAGS)
  XCC                 := $(ACC)
  AAS                 := $(GCC_TOOL_DIR)arm-none-eabi-gcc
  XAS                 := $(AAS)
  XAR                 := $(GCC_TOOL_DIR)arm-none-eabi-ar
  XAROPT              := -rc
  XLD                 := $(GCC_TOOL_DIR)arm-none-eabi-ld
  FROMELF             := $(GCC_TOOL_DIR)arm-none-eabi-objdump
  READ_ELF            := $(GCC_TOOL_DIR)arm-none-eabi-readelf
  HEXCOPY			  := $(GCC_TOOL_DIR)arm-none-eabi-objcopy
  SHOWSIZEOPT         := 
  BUILD_TOOL_DIR 	  := 
  SPAR_SCRIPTS_DIR     := scripts/
  OBJCOPY             := $(SPAR_SCRIPTS_DIR)objcpy.pl 
  OBJDUMP             := $(GCC_TOOL_DIR)arm-none-eabi-objdump
  NM                  := $(GCC_TOOL_DIR)arm-none-eabi-nm
  HEX_TO_CGS          := $(SPAR_SCRIPTS_DIR)hex_to_cgs.pl
  SETUP_CALL_SCRIPT   := $(SPAR_SCRIPTS_DIR)setup_call_cgs.pl
  PRINT_STATS         := $(SPAR_SCRIPTS_DIR)print_stats.pl
  SECTION_TO_CGS      := $(SPAR_SCRIPTS_DIR)section_to_cgs.pl
  GEN_OVERLAY_FILES   := $(SPAR_SCRIPTS_DIR)generateoverlayfiles.pl
  GEN_LIB_INSTALLER   := $(SPAR_SCRIPTS_DIR)generatelibinstaller.pl
endif

# These are always assumed to be in the path/view when building tier2
# These tools are used to 'sanitize' the built image, appropriate for inclusion in ADK.
GCC_OBJCPY            := $(GCC_TOOL_DIR)arm-none-eabi-objcopy
GCC_OBHDUMP           := $(GCC_TOOL_DIR)arm-none-eabi-objdump
GCC_OBJDUMP           := $(GCC_TOOL_DIR)arm-none-eabi-objdump
SANITIZE_LST          := sanitizelst.pl
SANITIZE_SYMDEF       := sanitizesymdef.pl

################################################################################
# General tools for spar builds
################################################################################
ifeq  ($(findstring $(WIN_OS_NAME),$(HOSTOS)),$(WIN_OS_NAME))
  COMPRESSOR          ?= ./lzss.exe
else
  COMPRESSOR          ?= ./lzss
endif

XRM     := $(TOOLSBIN)rm -rf
RM      := $(TOOLSBIN)rm -f
XCP     := $(TOOLSBIN)cp
XMV		:= $(TOOLSBIN)mv
CAT     := $(TOOLSBIN)cat
XMD     := $(TOOLSBIN)mkdir -p
ECHO    := $(TOOLSBIN)echo
AWK     := $(TOOLSBIN)awk
SORT    := $(TOOLSBIN)sort
GREP    := $(TOOLSBIN)grep
EGREP   := $(TOOLSBIN)egrep
FGREP   := $(TOOLSBIN)fgrep
SED     := $(TOOLSBIN)sed
PERL    := $(TOOLSBIN)perl
CUT     := $(TOOLSBIN)cut
TAR     := $(TOOLSBIN)tar
TAIL    := $(TOOLSBIN)tail
HEAD    := $(TOOLSBIN)head
WC      := $(TOOLSBIN)wc
MD5SUM  := $(TOOLSBIN)md5sum
GZIP    := $(TOOLSBIN)gzip
FIND    := find
XARGS   := xargs
OD      := od -v

# The Doxygen in SCM's PATH wants a version of a lib which is found in
# /lib; the version in /tools/oss/... isn't correct. Once
# $(TOOLSBIN)doxygen is present, you can just use that.
#DOXYGEN := LD_LIBRARY_PATH=/lib:$${LD_LIBRARY_PATH} ; export LD_LIBRARY_PATH ; doxygen
DOXYGEN_VER = 1_5_5
DOXYGEN = $(TOOLSBIN_DOXYGEN_PATH)doxygen
#DOXYGEN := $(TOOLS_DIR)
TOUCH   := $(TOOLSBIN)touch

BIN2HEX_ROM_MASK_TOOL  := $(PERL) $(THIS_DIR)bin2hex.pl
TAPEOUT_TOOL           := $(PERL) $(THIS_DIR)tapeoutAll.pl
PROCBUILDLIST          := $(PERL) $(THIS_DIR)procbuildlist.pl
