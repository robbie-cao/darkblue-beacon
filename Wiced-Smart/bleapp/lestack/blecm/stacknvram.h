#ifndef _STACKNVRAM_H_
#define _STACKNVRAM_H_
/*******************************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP 
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2011 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: stacknvram.h
*
* Abstract: This file provides a central place for the NVRAM access items.
*
*
* Functions:
*
*******************************************************************************/
#include "cfa.h" // Get the Core Firmware API interfaces.

#ifdef __cplusplus
extern "C" {
#endif

//The cfa interfaces uses an item number to identify the items stored in NVRAM.
//We want to use a relative scheme so that shifting numbers becomes 
//less painfull. 
// If the base number is not from 0, we should update this one.
#define   STACKNVRAM_FIRST_USABLE_ITEM_NUMBER    0x00

// This item has the local root keys.
#define   STACKNVRAM_LOCAL_KEYS (STACKNVRAM_FIRST_USABLE_ITEM_NUMBER+1)
// This item has the index table associated with the bonded devices.
#define   STACKNVRAM_BOND_INDEX (STACKNVRAM_LOCAL_KEYS + 1)


// Note: This one should be the last one so that the idx can grow 
// to the end. We reserve 5 bonded device, defined in lesmpkeys.h
#define   STACKNVRAM_FIRST_BONDED_IDX (STACKNVRAM_BOND_INDEX + 1)

#define   VS_BLE_HOST_LIST 0x70 //0x70 is working // (STACKNVRAM_FIRST_BONDED_IDX + 1)
#define   VS_BLE_BPM_DATA (VS_BLE_HOST_LIST + 1)
#define   VS_BLE_HRM_DATA (VS_BLE_HOST_LIST + 1)
#define   VS_BLE_THER_DATA (VS_BLE_HOST_LIST + 1)
#define   VS_BLE_GEN_DATA (VS_BLE_HOST_LIST + 1)

#define   VS_BLE_WS_DATA (VS_BLE_HOST_LIST + 1)
#define   VS_BLE_GM_DATA (VS_BLE_HOST_LIST + 1)

#define   VS_BLE_BAT_DATA (VS_BLE_HOST_LIST - 1)

#define   VS_BLE_BAT_DATA1_1 (VS_BLE_HOST_LIST - 2)

#ifdef __cplusplus
}
#endif

#endif //end of #ifndef _STACKNVRAM_H_
