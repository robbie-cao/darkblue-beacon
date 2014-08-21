/*
********************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2007 Broadcom Corp.
*                  ALL RIGHTS RESERVED
*
********************************************************************

********************************************************************
*    File Name: spar_setup.c
*
*           The Stackable Patch and Application Runtime
*
*    Abstract: C-runtime setup of this SPAR tier
*
********************************************************************
*/

#include "spar_utils.h"
#include "string.h"
#include "types.h"

/*****************************************************************
*   Definitions
*
*****************************************************************/
#ifndef SPAR_APP_SETUP
#error "You must define SPAR_APP_SETUP to indicate the application setup."
#endif

void SPAR_APP_SETUP(void);

extern char* idle_saveThreadxAvailMem;
extern BOOL32 uart_IsConnected(void );
extern BOOL32 uart_swd_Autodetect(void );
extern void install_libs(void);

#ifdef DEBUG
UINT8 spar_debug_continue = 0;
#endif

/*****************************************************************
 *   Function: spar_setup()
 *
 *   Abstract: Process the information in .secinfo, copying and
 *   clearing sections as needed.
 *
 *
 *****************************************************************/
#ifndef __GNUC__
#pragma arm section code = "spar_setup"
void SPAR_CRT_SETUP(void)
{
    typedef struct
    {
        UINT32 source;
        UINT32 target;
        UINT32 len;
    } armlink_copy_secinfo_t;
    
    extern void *_tx_initialize_unused_memory;
    extern UINT32 Region$$Table$$Base;
    extern UINT32 Region$$Table$$Limit;
    
    extern UINT32 Image$$SPAR_DRAM_ZI_AREA$$ZI$$Base;
    extern UINT32 Image$$SPAR_DRAM_ZI_AREA$$ZI$$Length;
    extern UINT32 Image$$first_free_section_in_spar_NV_RAM$$Base;
    
    armlink_copy_secinfo_t *cpysecinfo;
    
    // Get the section info base of this spar slice
    UINT32 cpysecinfobase = (UINT32)&Region$$Table$$Base;
    UINT32 cpysecinfolim = (UINT32)&Region$$Table$$Limit;
    
    UINT32 clrsecbase = (UINT32)&Image$$SPAR_DRAM_ZI_AREA$$ZI$$Base;
    UINT32 clrseclen = (UINT32)&Image$$SPAR_DRAM_ZI_AREA$$ZI$$Length;
    UINT32 endofspar = (UINT32)&Image$$first_free_section_in_spar_NV_RAM$$Base;
    
    // Here we ought to assert that we're linked against the right
    // image, before we call memcpy/memset in ROM/Flash...
    
    if(cpysecinfobase != cpysecinfolim)
    {
        // Section info length is not zero
        // which means that there is RW data
        cpysecinfo = (armlink_copy_secinfo_t *)cpysecinfobase;
        memcpy((void *)cpysecinfo->target, (void *)cpysecinfo->source, cpysecinfo->len);
    }
    
    // Clear ZI section
    if(clrseclen != 0)
        memset((void *)clrsecbase, 0x00, clrseclen);
    
    // And move avail memory above this spar if required
    // Note that if there are other spars will be placed with minimum
    // alignment (because of the linker option to IRAM_SPAR_BEGIN) and itself
    // is responsible for moving the avail mem ptr.
#ifndef DEBUG
    if(uart_swd_Autodetect() && !uart_IsConnected())
#endif
    	idle_saveThreadxAvailMem = (char *)endofspar;
	
	// Install included libraries and patches if any
	install_libs();
    
    // Call the application setup code
    SPAR_APP_SETUP();
}
#pragma arm section code

#else

ATTRIBUTE((section(".spar_setup")))
void SPAR_CRT_SETUP(void)
{
    extern void* spar_iram_bss_begin;
    extern unsigned spar_iram_data_length, spar_iram_bss_length;
    extern void* spar_irom_data_begin, *spar_iram_data_begin, *spar_iram_end;


    // Initialize initialized data if .data length is non-zero and it needs to be copied from NV to RAM.
    if(((UINT32)&spar_irom_data_begin != (UINT32)&spar_iram_data_begin) && ((UINT32)&spar_iram_data_length != 0))
        memcpy((void*)&spar_iram_data_begin, (void*)&spar_irom_data_begin, (UINT32)&spar_iram_data_length);
    
    // // Clear the ZI section
    if((UINT32)&spar_iram_bss_length != 0)
    {
        memset((void*)&spar_iram_bss_begin, 0x00, (UINT32)&spar_iram_bss_length);
    }
    
    // And move avail memory above this spar if required
    // Note that if there are other spars will be placed with minimum
    // alignment (because of the linker option to IRAM_SPAR_BEGIN) and itself
    // is responsible for moving the avail mem ptr.
#ifndef DEBUG
    if(uart_swd_Autodetect() && !uart_IsConnected())
#endif
    	idle_saveThreadxAvailMem = (char *)(((UINT32)&spar_iram_end + 32) & 0xFFFFFFF0);

	// Install included libraries and patches if any
	install_libs();
	
    // Call the application setup code
    SPAR_APP_SETUP();
}

#endif
