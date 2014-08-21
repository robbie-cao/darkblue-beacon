/* Linker script for ARM Simulator
 *
 * Version: Sourcery G++ Lite 2011.03-42
 * Support: https://support.codesourcery.com/GNUToolchain/
 *
 * Copyright (c) 2007, 2008, 2009, 2010 CodeSourcery, Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
/*
 * Linker script for SPAR in RAM application. Modified from generic.ld
 *   2011, 2012, 2013 Changes COPYRIGHT BROADCOM CORP.
 */
OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
SEARCH_DIR(.)
/* GROUP(-lgcc -lc) */

MEMORY
{
  ram (rwx) : ORIGIN = IRAM_BEGIN, LENGTH = IRAM_LEN
}

/*
 * Extern the linker generated symbols
 */
EXTERN(spar_irom_begin spar_irom_end spar_irom_length);
EXTERN(spar_iram_begin spar_iram_end spar_iram_length);
EXTERN(spar_iram_data_begin spar_iram_data_end spar_iram_data_length);
EXTERN(spar_iram_bss_begin spar_iram_bss_end spar_iram_bss_length);
EXTERN(spar_irom_data_begin);

/* 
 * Provide some symbols for spar init code to use
 */
PROVIDE(spar_irom_length = spar_irom_end - spar_irom_begin);
PROVIDE(spar_iram_length = spar_iram_end - spar_iram_begin);
PROVIDE(spar_iram_data_length = spar_iram_data_end - spar_iram_data_begin);
PROVIDE(spar_iram_bss_length = spar_iram_bss_end - spar_iram_bss_begin);

SECTIONS
{
  spar_irom_begin = IRAM_BEGIN;

  #include "spar_ram_overlays.ld"
  
  .text :
  {
    CREATE_OBJECT_SYMBOLS
    *(.app_init_code)
	*(.emb_text)
    *(.text)
    *(.text.* .gnu.linkonce.t.*)
    *(.glue_7t) *(.glue_7) *(.vfp11_veneer) *(.v4_bx)
    *(.ARM.extab* .gnu.linkonce.armextab.*) *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } >ram
  
  .rodata : ALIGN (4)
  {
	CREATE_OBJECT_SYMBOLS
    *(const_drom_var)
    *(.rodata)
    *(.constdata)
    *(.rodata.* .gnu.linkonce.r.*)
    *(.data.rom)    
	spar_irom_end = .;
  } >ram
  
  .data : ALIGN (4)
  {
	CREATE_OBJECT_SYMBOLS
    spar_iram_begin = .;
    spar_iram_data_begin = .;
    *(.data)
    *(.data.* .gnu.linkonce.d.*)
    spar_iram_data_end = .;
  } >ram

  /* Nothing to load because it is loaded from EEPROM/SF at boot. */
  spar_irom_data_begin = spar_iram_data_begin;
  
  .bss : ALIGN (4)
  {
	CREATE_OBJECT_SYMBOLS
    spar_iram_bss_begin = .;
    *(.bss)
    *(.bss.*  .gnu.linkonce.b.*)
    *(COMMON)
    spar_iram_bss_end = .;
    spar_iram_end = .;
  } >ram
  
  /*  Place the setup area after bss so that when dynamic allocation occurs
   *  after spar setup, it will reclaim the RAM taken up by the setup function.
  */
  .setup : ALIGN (4)
  {
    *(.init_code)
	*(.spar_setup)
  } >ram
}
/*  */
