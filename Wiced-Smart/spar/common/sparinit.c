/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 */

#include "sparcommon.h"
#include "bleapp.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

extern void application_init( void );

/******************************************************
 *               Function Definitions
 ******************************************************/

////////////////////////////////////////////////////////////////////////////////
/// Spar entry function called early during initialization. This function or
/// or any of the called functions cannot allocate memory or create new objects.
/// This function may only be used to initialize other function pointers/register
/// a new app create function/initialize global data.
////////////////////////////////////////////////////////////////////////////////

#pragma arm section code = "spar_setup"

////////////////////////////////////////////////////////////////////////////////
/// Spar entry function called early during initialization. This function or 
/// or any of the called functions cannot allocate memory or create new objects.
/// This function may only be used to initialize other function pointers/register
/// a new app create function/initialize global data.
////////////////////////////////////////////////////////////////////////////////
// ATTRIBUTE((section(".setup")))
void application_setup(void)
{
    // Initialize Spar here.
    extern UINT8 *bleapp_sram_addr;
    bleapp_sram_addr = (UINT8 *)0x200000;
    bleapp_pre_init = application_init;
    BLE_APP_ENABLE_TRACING_ON_HCI_UART();

#ifdef RAMBUFENABLE
    {
        extern UINT8 bleapp_rambuf_enable;
        extern UINT16 cfa_ConfigRambuf_AllocatedLengthInBytes;

        bleapp_rambuf_enable = 1;
        cfa_ConfigRambuf_AllocatedLengthInBytes = 180; //40+3 + 8+3 + 120+3 +3
    }
#endif
}

#pragma arm section code
