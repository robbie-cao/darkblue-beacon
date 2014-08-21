/*******************************************************************************
*
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2004 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: get_nv_info.c
*
* Abstract:  Provides some extra API for getting the type and size of NV.
*
*******************************************************************************/
#include "types.h"
#include "cfa.h"

/// Get the current type of the NV.
/// \return One of CFA_CONFIG_LOCATION_EEPROM, CFA_CONFIG_LOCATION_SERIAL_FLASH
///             or CFA_CONFIG_LOCATION_RAM_BUF
UINT8 get_nv_info_Type(void);

/// Returns the size of NV in bytes. Recognizes 16K, 32K, 64K, 128K, 256K and 1M.
/// Larger NVs will return 0xFFFFFFFF. There is one caveat to using this - it determines
/// the size heuristically based on address wraparound at the max boundary. If this will not happen
/// with this NV, then the behavior is undefined.
UINT32 get_nv_info_Size(void);
